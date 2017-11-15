import underworld as uw
import underworld.function as fn
from scaling import nonDimensionalize as nd
import scaling as sca
import numpy as np

u = UnitRegistry = sca.UnitRegistry


class PressureSmoother(object):
    
    def __init__(self, mesh, pressureField):
        
        self.mesh = mesh
        self.pressureField = pressureField
        
        self.NodePressure = uw.mesh.MeshVariable(self.mesh, nodeDofCount=1)
       
        self.Cell2Nodes = uw.utils.MeshVariable_Projection(self.NodePressure, self.pressureField, type=0)
        self.Nodes2Cell = uw.utils.MeshVariable_Projection(self.pressureField, self.NodePressure, type=0)        

    def smooth(self):
        
        self.Cell2Nodes.solve()
        self.Nodes2Cell.solve() 


class ViscosityLimiter(object):

    def __init__(self, minViscosity, maxViscosity):

        self.minViscosity = minViscosity
        self.maxViscosity = maxViscosity

    def apply(self, viscosityField):
        maxBound = fn.misc.min(viscosityField, nd(self.maxViscosity))
        minMaxBound = fn.misc.max(maxBound, nd(self.minViscosity))
        return minMaxBound 



# For backward compatibility
def layer(top, bottom, minX, maxX):
    if bottom == None or top == None:
        return None
    vertex_array = np.array( [(minX, bottom),(minX, top),(maxX, top),(maxX, bottom)] )
    return uw.function.shape.Polygon(vertex_array)


class PassiveTracers(object):

    def __init__(self, mesh, velocityField, name=None, vertices=None,
                 particleEscape=True):

        self.name = name
        self.particleEscape = particleEscape

        vertices = [nd(x) for x in vertices]

        points = np.zeros((len(vertices[0]), len(vertices)))

        for dim in range(len(vertices)):
            points[:,dim] = vertices[dim]

        self.swarm = uw.swarm.Swarm(mesh = mesh, particleEscape=particleEscape)
        self.swarm.add_particles_with_coordinates(points)
        self.advector = uw.systems.SwarmAdvector(swarm=self.swarm, velocityField=velocityField, order=2)

    def integrate(self, dt, **kwargs ):

        self.advector.integrate(dt, **kwargs)
