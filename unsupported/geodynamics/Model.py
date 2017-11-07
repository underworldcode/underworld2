import os
import types
import underworld as uw
import underworld.function as fn
import numpy as np
import glucifer
import unsupported.scaling as sca
from unsupported.scaling import nonDimensionalize as nd
from unsupported.lithopress import LithostaticPressure
from unsupported.LecodeIsostasy import LecodeIsostasy
from utils import PressureSmoother, ViscosityLimiter
from Material import Material
import shapes

u = UnitRegistry = sca.UnitRegistry


def default_scaling():
    half_rate = 1.8 * u.centimeter / u.year
    model_length = 360e3 * u.meter
    surfaceTemp = 273.15 * u.degK
    baseModelTemp = 1603.15 * u.degK
    bodyforce = 3300 * u.kilogram / u.metre**3 * 9.81 * u.meter / u.second**2
    
    KL = model_length
    Kt = KL / half_rate
    KM = bodyforce * KL**2 * Kt**2
    KT = (baseModelTemp - surfaceTemp)
    
    sca.scaling["[length]"] = KL
    sca.scaling["[time]"] = Kt
    sca.scaling["[mass]"] = KM
    sca.scaling["[temperature]"] = KT


class Model(Material):
    def __init__(self, elementRes, minCoord, maxCoord, gravity,
                 periodic=(False, False), elementType="Q1/dQ0",
                 swarmLayout=None, Tref=273.15 * u.degK, name="undefined",
                 outputDir="outputs", populationControl=True, scaling=None):

        super(Model, self).__init__()

#        if scaling:
#            sca.scaling["[length]"] = scaling["[length]"]
#            sca.scaling["[time]"] = scaling["[time]"]
#            sca.scaling["[mass]"]= scaling["[mass]"]
#            sca.scaling["[temperature]"] = scaling["[temperature]"]
#        else:
#            default_scaling()

        self.name = name
        self.top = maxCoord[-1]
        self.bottom = minCoord[-1]

        self.outputDir = outputDir
        self.checkpointID = 0
        self._checkpoint = None
        self.checkpoint = None

        self.viscosityLimiter = ViscosityLimiter(minViscosity = 1e19 * u.pascal * u.second,
                                                 maxViscosity = 1e25 * u.pascal * u.second)

        self.gravity = tuple([nd(val) for val in gravity])
        self.Tref = Tref
        self.elementType = elementType
        self.elementRes = elementRes
        self.minCoord = minCoord
        self.maxCoord = maxCoord
        self.periodic = periodic

        self._set_mesh()
        
        # Add common fields
        self.temperature = None
        self.pressureField = uw.mesh.MeshVariable(mesh=self.mesh.subMesh, nodeDofCount=1)
        self.velocityField = uw.mesh.MeshVariable(mesh=self.mesh, nodeDofCount=self.mesh.dim)
        self.pressureField.data[...] = 0.
        self.velocityField.data[...] = 0.
       
        # symmetric component of the gradient of the flow velocityField.
        self.strainRate_default = 1.0e-15 / u.second
        self.solutionExist = False
        self.strainRate = fn.tensor.symmetric(self.velocityField.fn_gradient)
        self.strainRate_2ndInvariant = fn.tensor.second_invariant(self.strainRate)
 
        # Create the material swarm
        self.swarm = uw.swarm.Swarm(mesh=self.mesh, particleEscape=True)
        self.swarmLayout = swarmLayout
        self.population_control = populationControl
        self.swarm_advector = uw.systems.SwarmAdvector(swarm=self.swarm,
                                  velocityField=self.velocityField,
                                  order=self.mesh.dim
                                  )

        # Add Common Swarm Variables
        self.materialField = self.swarm.add_variable(dataType="int", count=1)
        self.plasticStrain = self.swarm.add_variable(dataType="double", count=1)
        self.plasticStrain.data[...] = 0.0
 
        self.materials = []
        self._defaultMaterial = 0

        # Create a series of aliases for the boundary sets
        self.leftWall   = self.mesh.specialSets["MinI_VertexSet"]
        self.topWall    = self.mesh.specialSets["MaxJ_VertexSet"]
        self.bottomWall = self.mesh.specialSets["MinJ_VertexSet"]
        self.rightWall  = self.mesh.specialSets["MaxI_VertexSet"]
        
        if self.mesh.dim > 2:
            self.frontWall = self.mesh.specialSets["MinK_VertexSet"]
            self.backWall  = self.mesh.specialSets["MaxK_VertexSet"]

        self.time = 0.0 * u.megayears
        self.step = 0
        self.densityFn = None
        self.Isostasy = None

        self._lecodeRefMaterial = None
        self.pressSmoother = PressureSmoother(self.mesh, self.pressureField)

    @property
    def outputDir(self):
        return self._outputDir

    @outputDir.setter
    def outputDir(self, value):
        if uw.rank() == 0:
            if not os.path.exists(value):
                os.makedirs(value)
        self._outputDir = value

    def _set_mesh(self):

        minCoord = tuple([nd(val) for val in self.minCoord])
        maxCoord = tuple([nd(val) for val in self.maxCoord])

        self.mesh = uw.mesh.FeMesh_Cartesian(elementType=self.elementType,
                                             elementRes=self.elementRes,
                                             minCoord=minCoord,
                                             maxCoord=maxCoord,
                                             periodic=self.periodic)

    @property
    def swarmLayout(self):
        return self._swarmLayout

    @swarmLayout.setter
    def swarmLayout(self, value):
        
        if value is not None:
            self._swarmLayout = value
        else:
            self._swarmLayout = uw.swarm.layouts.GlobalSpaceFillerLayout(
                                                 swarm=self.swarm,
                                                 particlesPerCell=25)

        self.swarm.populate_using_layout(layout=self._swarmLayout)

    @property
    def population_control(self):
        return self._population_control

    @population_control.setter
    def population_control(self, value):
        self._population_control = uw.swarm.PopulationControl(
                 self.swarm,
                 aggressive=True, splitThreshold=0.15,
                 maxDeletions=2, maxSplits=10,
                 particlesPerCell=20
            )

    def set_temperatureBCs(self, left=None, right=None, top=None, bottom=None,
                           indexSets=[]):
        self.temperature = uw.mesh.MeshVariable(mesh=self.mesh,
                                                nodeDofCount=1)
        self._temperatureDot = uw.mesh.MeshVariable(mesh=self.mesh,
                                                    nodeDofCount=1)
        self.temperature.data[...] = nd(self.Tref)
        self._temperatureDot.data[...] = 0.
        
        indices = [self.mesh.specialSets["Empty"]]
        if left is not None:
            self.temperature.data[self.leftWall.data] = nd(left)
            indices[0] += self.leftWall
        if right is not None:
            self.temperature.data[self.rightWall.data] = nd(right)
            indices[0] += self.rightWall
        if top is not None:
            self.temperature.data[self.topWall.data] = nd(top)
            indices[0] += self.topWall
        if bottom is not None:
            self.temperature.data[self.bottomWall.data] = nd(bottom)
            indices[0] += self.bottomWall

        for indexSet, temp in indexSets:
            self.temperature.data[indexSet.data] = nd(temp)
            indices[0] += indexSet

        self._temperatureBCs = uw.conditions.DirichletCondition(
                variable=self.temperature,
                indexSetsPerDof=indices
            )

    def init_advection_diffusion(self):
        DiffusivityMap = {}
        for material in self.materials:
            DiffusivityMap[material.index] = nd(material.diffusivity)
        
        self.DiffusivityFn = fn.branching.map(fn_key=self.materialField,
                                              mapping=DiffusivityMap)
       
        HeatProdMap = {}
        for material in self.materials:
            HeatProdMap[material.index] = (nd(material.radiogenicHeatProd) /
                                           (nd(material.density) *
                                            nd(material.capacity)))
        
        self.HeatProdFn = fn.branching.map(fn_key=self.materialField,
                                           mapping=HeatProdMap)
        
        self.advdiffSystem = uw.systems.AdvectionDiffusion(
                self.temperature,
                self._temperatureDot,
                velocityField=self.velocityField,
                fn_diffusivity=self.DiffusivityFn,
                fn_sourceTerm=self.HeatProdFn,
                conditions=[self._temperatureBCs]
            )

    def init_stokes_system(self):
        conditions = self._velocityFieldBCs
        self._set_density()
        self.buoyancyFn = self.densityFn * self.gravity

        if any([material.viscosity for material in self.materials]): 
            self._set_viscosity()
            
            stokes = uw.systems.Stokes(velocityField=self.velocityField,
                                       pressureField=self.pressureField,
                                       conditions=conditions,
                                       fn_viscosity=self.viscosityFn,
                                       fn_bodyforce=self.buoyancyFn)

            self.solver = uw.systems.Solver(stokes)

    def set_velocityBCs(self, left=None, right=None, top=None, bottom=None,
                        indexSets=[]):
        
        indices = []

        for dim in range(self.mesh.dim):
            indices.append(self.mesh.specialSets["Empty"])

        if left is not None:
            for dim in range(self.mesh.dim):
                if left[dim] is not None:
                    self.velocityField.data[self.leftWall.data, dim] = nd(left[dim])
                    indices[dim] += self.leftWall
        if right is not None:
            for dim in range(self.mesh.dim):
                if right[dim] is not None:
                    self.velocityField.data[self.rightWall.data, dim] = nd(right[dim])
                    indices[dim] += self.rightWall
        if top is not None:
            for dim in range(self.mesh.dim):
                if top[dim] is not None:
                    self.velocityField.data[self.topWall.data, dim] = nd(top[dim])
                    indices[dim] += self.topWall
        if bottom is not None:
            if isinstance(bottom, LecodeIsostasy):
                self.Isostasy = bottom
                self.Isostasy.mesh = self.mesh
                self.Isostasy.swarm = self.swarm
                self.Isostasy.velocityField = self.velocityField
                self.Isostasy.materialIndexField = self.materialField
                self.Isostasy.densityFn = self._set_density()
                indices[-1] += self.bottomWall
            else:    
                for dim in range(self.mesh.dim):
                    if bottom[dim] is not None:
                        self.velocityField.data[self.bottomWall.data, dim] = nd(bottom[dim])
                        indices[dim] += self.bottomWall

        for indexSet, temp in indexSets:
            for dim in range(self.mesh.dim):
                if indexSet[dim] is not None:
                    self.velocityField.data[indexSet.data, dim] = nd(indexSet[dim])
                    indices[dim] += indexSet

        self._velocityFieldBCs = uw.conditions.DirichletCondition(
                                      variable=self.velocityField,
                                      indexSetsPerDof=indices)

    def use_lecode_isostasy(self, material_ref):
        self._lecodeRefMaterial = material_ref

    def add_material(self, vertices=None, reset=False, name="unknown",
                     shape=None):

        if reset:
            self.materials = []

        if vertices:
            vertices = [(nd(x), nd(y)) for x, y in vertices]
        
        mat = Material()
                
        mat.name = name
        mat.vertices           = vertices
        mat.diffusivity        = self.diffusivity
        mat.capacity           = self.capacity
        mat.thermalExpansivity = self.thermalExpansivity
        mat.radiogenicHeatProd = self.radiogenicHeatProd
        
        if isinstance(shape, shapes.Layer):
            shape.minX = self.minCoord[0]
            shape.maxX = self.maxCoord[0]

        mat.shape              = shape
        mat.indices = self._get_material_indices(mat)
        self.materials.reverse()
        self.materials.append(mat)
        self.materials.reverse()
        self._fill_model()

        return mat

    def _fill_model(self):

        conditions = [(obj.shape.fn, obj.index)
                      for obj in self.materials if obj.shape is not None]

        conditions.append((True, self._defaultMaterial))
        self.materialField.data[:] = fn.branching.conditional(conditions).evaluate(self.swarm)

    @property
    def material_drawing_order(self):
        return self.material_drawing_order()

    @material_drawing_order.setter
    def material_drawing_order(self, value):
        self._material_drawing_order = value
        self.materials = value
        self._fill_model()

    def _get_material_indices(self, mat):
        mask = mat.shape.evaluate(self.mesh.data)[:,0]
        indices = np.arange(0,self.mesh.nodesDomain)[mask]
        return uw.mesh.FeMesh_IndexSet(self.mesh, topologicalIndex=0,
                                       size=self.mesh.nodesGlobal,
                                       fromObject=indices)

    def _set_density(self):
        densityMap = {}
        for material in self.materials:
            if self.temperature:
                densityMap[material.index] = nd(material.density) * (1.0 -
                                             nd(material.thermalExpansivity)
                                             * (self.temperature - nd(self.Tref)))
            else:
                densityMap[material.index] = nd(material.density)

        self.densityFn = fn.branching.map(fn_key=self.materialField, mapping=densityMap)
        return self.densityFn

    def _set_viscosity(self):

        ViscosityMap = {}
        BGViscosityMap = {}

        # Viscous behavior
        for material in self.materials:
            ViscosityHandler = material.viscosity
            ViscosityHandler.pressureField = self.pressureField
            ViscosityHandler.strainRateInvariantField = self.strainRate_2ndInvariant
            ViscosityHandler.temperatureField = self.temperature
            ViscosityHandler.viscosityLimiter = self.viscosityLimiter
            ViscosityMap[material.index] = ViscosityHandler.muEff
            BGViscosityMap[material.index] = ViscosityHandler.muEff

        # Plasticity
        for material in self.materials:
            if material.plasticity:
                YieldHandler = material.plasticity
                YieldHandler.pressureField = self.pressureField
                YieldHandler.plasticStrain = self.plasticStrain
                yieldStress = YieldHandler._get_yieldStress()
                eij = self.strainRate_2ndInvariant
                eijdef =  nd(self.strainRate_default)
                if self.mesh.dim == 2:
                    muEff = 0.5 * yieldStress / fn.misc.max(eij, eijdef)

                muEff = self.viscosityLimiter.apply(muEff)
                ViscosityMap[material.index] = fn.misc.min(muEff, ViscosityMap[material.index])

        self.viscosityFn = fn.branching.map(fn_key=self.materialField, mapping=ViscosityMap)
        backgroundViscosityFn = fn.branching.map(fn_key=self.materialField,mapping=BGViscosityMap)

        RhConditions = [(self.viscosityFn < backgroundViscosityFn, 1.0), (True, 0.0)]

        self.isPlastic = fn.branching.conditional(RhConditions)
        self.isYielding = self.isPlastic * self.strainRate_2ndInvariant
        
        return self.viscosityFn, self.isYielding

    def solve_temperature_steady_state(self):
        DiffusivityMap = {}
        for material in self.materials:
            DiffusivityMap[material.index] = nd(material.diffusivity)
        
        self.DiffusivityFn = fn.branching.map(fn_key = self.materialField, mapping = DiffusivityMap)
       
        HeatProdMap = {}
        for material in self.materials:
            HeatProdMap[material.index] = nd(material.radiogenicHeatProd)/(nd(material.density)*nd(material.capacity))
        
        self.HeatProdFn = fn.branching.map(fn_key = self.materialField, mapping = HeatProdMap)
        
        heatequation = uw.systems.SteadyStateHeat(temperatureField=self.temperature,
                                                  fn_diffusivity=self.DiffusivityFn,
                                                  fn_heating=self.HeatProdFn,
                                                  conditions=self._temperatureBCs)
        heatsolver = uw.systems.Solver(heatequation)
        heatsolver.solve(nonLinearIterate=True)

    def solve_lithostatic_pressureField(self):
        self._set_density()
        gravity = np.abs(self.gravity[-1])  # Ugly!!!!!
        lithoPress = LithostaticPressure(self.mesh, self.densityFn, gravity)
        self.pressureField.data[:], LPresBot = lithoPress.solve()
        self.pressSmoother.smooth()

    def _calibrate_pressureField(self):
        surfaceArea = uw.utils.Integral(fn=1.0, mesh=self.mesh,
                                        integrationType='surface',
                                        surfaceIndexSet=self.topWall)
        surfacepressureFieldIntegral = uw.utils.Integral(
                fn=self.pressureField,
                mesh=self.mesh,
                integrationType='surface',
                surfaceIndexSet=self.topWall
            )
        (area,) = surfaceArea.evaluate()
        (p0,) = surfacepressureFieldIntegral.evaluate()
        offset = p0/area
        self.pressureField.data[:] -= offset
        
        for material in self.materials:
            if material.viscosity:
                material.viscosity.firstIter = False

    def solve(self):
        self.solver.solve(nonLinearIterate=True,
                          callback_post_solve=self._calibrate_pressureField)

    def init_model(self, temperature=True, pressureField=True):
        
        # Init Temperature Field
        if self.temperature and temperature:
            self.solve_temperature_steady_state()
            self.init_advection_diffusion()

        # Init pressureField Field
        if self.pressureField and pressureField:
            self.solve_lithostatic_pressureField()
        
        self.init_stokes_system()

    def run_for(self, endTime=None, checkpoint=None):
        step = self.step
        time = nd(self.time)
        units = endTime.units
        endTime = time + nd(endTime)

        next_checkpoint = None
        if checkpoint:
            next_checkpoint = time + nd(checkpoint)

        while time < endTime:
            self.solve()

            if time == next_checkpoint:
                self.checkpointID += 1
                self.checkpoint()
                next_checkpoint += nd(checkpoint)

            # Whats the longest we can run before reaching the end of the model
            # or a checkpoint?
            # Need to generalize that
            dt = self.swarm_advector.get_max_dt()

            if self.temperature:
                dt = min(dt, self.advdiffSystem.get_max_dt())

            if checkpoint:
                dt = min(dt, next_checkpoint - time)
            
            self._dt = min(dt, endTime - time)
            uw.barrier()
            
            self.update()

            step += 1
            self.time += sca.Dimensionalize(self._dt, units)
            time += self._dt

            if checkpoint or step % 1 == 0:
                print "Time: ", str(self.time.to(units))

    def update(self):

        dt = self._dt
        # Increment plastic strain
        plasticStrainIncrement = dt * self.isYielding.evaluate(self.swarm)
        self.plasticStrain.data[:] += plasticStrainIncrement
        
        # Solve for temperature
        if self.temperature:
            self.advdiffSystem.integrate(dt)

        # Integrate Swarms in time
        self.swarm_advector.integrate(dt, update_owners=True) 
        
        # Do pop control
        self.population_control.repopulate() 

        if self.Isostasy:
            self.Isostasy.solve()

    def _default_checkpoint_function(self):
        self.save_velocityField(self.checkpointID)
        self.save_pressureField(self.checkpointID)
        if self.temperature:
            self.save_temperature(self.checkpointID)
        self.save_material(self.checkpointID)

    @property
    def checkpoint(self):
        return self._checkpoint

    @checkpoint.setter
    def checkpoint(self, func=None):
        if func:
            self._checkpoint = types.MethodType(func, self)
        else:
            self._checkpoint = self._default_checkpoint_function

    def plot_material(self, figsize=(1200, 400), **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Materials")
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.materialField,
                                           fn_size=2.0))
        Fig.show()
        return Fig

    def save_velocityField(self, checkpointID, units=u.centimeter/u.year):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'velocityField-%s' % checkpointID)
        handle = self.velocityField.save('%s.h5' % file_prefix, units=units)
        self.velocityField.xdmf('%s.xdmf' % file_prefix, handle,'velocityField', mH, 'mesh', modeltime=self.time.magnitude)
    
    def save_pressureField(self, checkpointID, units=u.pascal):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'pressureField-%s' % checkpointID)
        handle = self.pressureField.save('%s.h5' % file_prefix, units=units)
        self.pressureField.xdmf('%s.xdmf' % file_prefix, handle, 'pressureField', mH, 'mesh', modeltime=self.time.magnitude)
    
    def save_temperature(self, checkpointID, units=u.degK):
        mH = self.mesh.save(os.path.join(self.outputDir, "mesh.h5"), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'temperature-%s' % checkpointID)
        handle = self.temperature.save('%s.h5' % file_prefix, units=units)
        self.temperature.xdmf('%s.xdmf' % file_prefix, handle, 'temperature', mH, 'mesh', modeltime=self.time.magnitude)

    def save_material(self, checkpointID):
        sH = self.swarm.save(os.path.join(self.outputDir, 'swarm-%s.h5' % checkpointID), units=u.kilometers)
        file_prefix = os.path.join(self.outputDir, 'material-%s' % checkpointID)
        handle = self.materialField.save('%s.h5' % file_prefix)
        self.materialField.xdmf('%s.xdmf' % file_prefix, handle, 'material', sH,
                           'swarm', modeltime=self.time.magnitude)

    def plot_viscosity(self, figsize=(1200, 400), units=u.pascal*u.second, **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Viscosity Field")
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.viscosityFn*fact,
                                           fn_size=2.0,
                                           logScale=True))
        Fig.show()
        return Fig
    
    def plot_strainRate(self, figsize=(1200, 400), units=1.0/u.second, **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Viscosity Field")
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.strainRate_2ndInvariant*fact,
                                           fn_size=2.0,
                                           logScale=True))
        Fig.show()
        return Fig

    def plot_density(self, figsize=(1200, 400), units=u.kilogram/u.metre**3, **args):
        Fig = glucifer.Figure(figsize=(1200, 400))
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Points(self.swarm,
                                           fn_colour=self.densityFn*fact,
                                           fn_size=2.0))
        Fig.show()
        return Fig

    def plot_temperature(self, figsize=(1200, 400), units=u.degK, **args):
        Fig = glucifer.Figure(figsize=(1200, 400), title="Temperature Field")
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.mesh, self.temperature*fact))
        Fig.show()
        return Fig

    def plot_velocityField(self, figsize=(1200, 400), units=u.centimeter/u.year, **args):
        Fig = glucifer.Figure(figsize=(1200, 400))
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.mesh,self.velocityField[0]*fact))
        Fig.append(glucifer.objects.VectorArrows(self.mesh, self.velocityField,
                                                 scaling=0.03, arrowHead=10.,
                                                 resolutionI=25, 
                                                 resolutionJ=10)) 
        Fig.show()
        return Fig

    def plot_pressureField(self, figsize=(1200, 400), units=u.pascal, **kwargs):
        Fig = glucifer.Figure(figsize=(1200, 400), title="pressureField Field")
        fact = sca.Dimensionalize(1.0, units).magnitude
        Fig.append(glucifer.objects.Surface(self.mesh, self.pressureField*fact, **kwargs))
        Fig.show()
        return Fig
