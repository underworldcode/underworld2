import os
import glucifer
import scaling as sca
from scaling import nonDimensionalize as nd
import matplotlib.pyplot as plt
import numpy as np

u = UnitRegistry = sca.UnitRegistry

class Plots(object):

    def __init__(self, Model):

        self.Model = Model

    def material(self, figsize=(1200, 400), projected=False,
            script=None, cullface=False, mask=None, show=True, store=None,
            visugrid=None, tracers=[], quality=3, **kwargs):

        minCoord = tuple([nd(val) for val in self.Model.minCoord])
        maxCoord = tuple([nd(val) for val in self.Model.maxCoord])
        boundingBox =(minCoord, maxCoord)

        Fig = glucifer.Figure(figsize=figsize, store=store, title="Materials",
                              axis=True, quality=quality, boundingBox=boundingBox)

        if visugrid:
            xrange = visugrid.mesh.maxCoord[0] - visugrid.mesh.minCoord[0]
            yrange = visugrid.mesh.maxCoord[1] - visugrid.mesh.minCoord[1]
            dx = np.abs(nd(self.Model.minCoord[0]) - visugrid.mesh.minCoord[0])
            dy = np.abs(nd(self.Model.minCoord[1]) - visugrid.mesh.minCoord[1])
            xmin = (self.Model.mesh.minCoord[0] - self.Model.mesh.minCoord[0] + dx) / xrange
            xmax = (self.Model.mesh.maxCoord[0] - self.Model.mesh.minCoord[0] + dx) / xrange
            ymin = (self.Model.mesh.minCoord[1] - self.Model.mesh.minCoord[1] + dy) / yrange
            ymax = (self.Model.mesh.maxCoord[1] - self.Model.mesh.minCoord[1] + dy) / yrange
            xmin += 0.007
            xmax -= 0.007

            Fig.append(glucifer.objects.Mesh(visugrid.mesh, xmin=xmin,
                       xmax=xmax, ymin=ymin, ymax=ymax))


        if tracers:
            for tracer in tracers:
                Fig.append(glucifer.objects.Points(tracer.swarm, pointSize=3.0))
        
        if projected:
            Fig.append(glucifer.objects.Surface(self.Model.mesh,
                                               self.Model.projMaterialField,
                                               cullface=cullface, onMesh=True))
        else:
            Fig.append(glucifer.objects.Points(self.Model.swarm,
                                               fn_colour=self.Model.materialField,
                                               fn_size=2.0))
        if script:
            Fig.script(script)
       
        if show:
            Fig.show()
        return Fig

    def viscosity(self, figsize=(1200, 400), units=u.pascal*u.second,
                       projected=False, cullface=False, show=True, store=None,
                       script=None, pointSize=3.0, quality=3,**kwargs):
        Fig = glucifer.Figure(figsize=figsize, store=store, 
                              title="Viscosity Field",axis=True,
                              quality=quality)
        fact = sca.Dimensionalize(1.0, units).magnitude
        
        if projected:
            Fig.append(glucifer.objects.Surface(self.Model.mesh,
                                               self.Model.projViscosityField*fact,
                                               cullface=cullface,
                                               logScale=True, onMesh=True, **kwargs))
        else:
            Fig.append(glucifer.objects.Points(self.Model.swarm,
                                               fn_colour=self.Model.viscosityFn*fact,
                                               logScale=True,
                                               pointSize=pointSize,
                                               **kwargs))
        if script:
            Fig.script(script)

        if show:
            Fig.show()
        return Fig
    
    def strainRate(self, figsize=(1200, 400), units=1.0/u.second, show=True,
                   store=None, cullface=False, quality=3, **kwargs):
        Fig = glucifer.Figure(figsize=figsize, store=store, title="Strain Rate",
                              quality=quality)
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.Model.mesh, self.Model.strainRate_2ndInvariant*fact,
                                           cullface=cullface,
                                           colours="coolwarm", logScale=True,
                                           onMesh=True, **kwargs))
        if show:
            Fig.show()
        return Fig

    def density(self, projected=False, figsize=(1200, 400), units=u.kilogram/u.metre**3,
                store=None, show=True, cullface=False, quality=3, **kwargs):
        Fig = glucifer.Figure(figsize=figsize, store=store,
                              title="Density Field", quality=quality)
        fact = sca.Dimensionalize(1.0, units).magnitude

        if projected:
            Fig.append(glucifer.objects.Surface(self.Model.mesh,
                                               self.Model.projDensityField*fact,
                                               cullface=cullface, onMesh=True, **kwargs))

        else:
            Fig.append(glucifer.objects.Points(self.Model.swarm,
                                               fn_colour=self.Model.densityFn*fact,
                                               fn_size=2.0))
        if show:
            Fig.show()
        return Fig

    def temperature(self, figsize=(1200, 400), units=u.degK,
                         cullface=False, script=None, show=True, store=None, 
                         colourScale="coolwarm", quality=3, isotherms=[], **args):
        Fig = glucifer.Figure(figsize=figsize, store=store,
                              title="Temperature Field", quality=quality)
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.Model.mesh, self.Model.temperature*fact,
                   cullface=cullface, colours=colourScale))

        for isotherm in isotherms:
            # Kind of a hack but it works...
            Fig.append(glucifer.objects.Contours(self.Model.mesh, self.Model.temperature,
                      interval=nd(1 * isotherm.units),
                      limits=(nd(isotherm),nd(isotherm+1.0*isotherm.units)),
                      colourBar=False, colours="black"))

        if script:
            Fig.script(script)

        if show:
            Fig.show()
        return Fig
    
    def pressureField(self, figsize=(1200, 400), units=u.pascal,
                           cullface=False, show=True, store=None,
                           script=None, quality=3, **kwargs):
        Fig = glucifer.Figure(figsize=figsize, store=store,
                              title="Pressure Field", quality=quality)
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.Model.mesh, 
                                            self.Model.pressureField*fact,
                                            cullface=cullface, onMesh=True,
                                            **kwargs))
        
        if script:
            Fig.script(script)
        
        if show:
            Fig.show()
        return Fig

    def velocityField(self, figsize=(1200, 400), store=None, show=True,
                      units=u.centimeter/u.year, script=None, cullface=False, 
                      quality=3, scaling=0.03, arrowHead=10.,
                      resolution=[25,10,10], **args):
        Fig = glucifer.Figure(figsize=figsize, store=store, quality=quality)
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.Model.mesh,self.Model.velocityField[0]*fact,
                                            cullface=cullface))
        Fig.append(glucifer.objects.VectorArrows(self.Model.mesh, self.Model.velocityField,
                                                 scaling=scaling,
                                                 arrowHead=arrowHead,
                                                 resolution=resolution)) 
        
        if script:
            Fig.script(script)
        
        if show:
            Fig.show()
        return Fig
    
    def strain(self, figsize=(1200, 400), projected=False, cullface=False,
               script=None, store=None, show=True, quality=3, **kwargs):

        Fig = glucifer.Figure(figsize=figsize, store=store, title="Strain", 
                              quality=quality, axis=True)
        
        if projected:
            Fig.append(glucifer.objects.Surface(self.Model.mesh,
                                               self.Model.projPlasticStrain,
                                               cullface=cullface,
                                               colours="#FF6600:0.5, #555555:0.25, Blue:0.5",
                                               name="Strain",onMesh=True,
                                               **kwargs))
        else:
            Fig.append(glucifer.objects.Points(self.Model.swarm,
                                               fn_colour=self.Model.plasticStrain,
                                               colours="#FF6600:0.5, #555555:0.25, Blue:0.5",
                                               name="Strain", **kwargs))
        if script:
            Fig.script(script)

        if show:
            Fig.show()
        return Fig

    def stress_enveloppe(self):
        A = self.Model.yieldStressFn.evaluate(self.Model.swarm).flatten()
        B = self.Model.swarm.particleCoordinates.data[:,1]

        A = sca.Dimensionalize(A, u.megapascal)
        B = sca.Dimensionalize(B, u.kilometer)

        plt.plot(A, B, "+")
        plt.xlabel("Stress (MPa)")
        plt.ylabel("Depth (Km)")
        plt.show()
