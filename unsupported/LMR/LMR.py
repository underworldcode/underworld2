from itertools import count

class Rheology(object):
    def __init__(self):
        self.viscosity = None
        self.plasticity = None
        self.cohesion = None
        self.friction = None
        self.meltModifier = None
        self.stressLimiter = None

class Material(object):
    _ids = count(0)
    def __init__(self):
        self.top = None
        self.bottom = None
        self.shape = None
        self.index = self._ids.next()
        
        self.density = None
        self.diffusivity = None
        self.capacity = None
        self.thermalExpansivity = None
        self.radiogenicHeatProd = None
        self.meltFraction = None
        self.meltFractionLimit = None
        self.solidus = None
        self.liquidus = None
        self.latentHeatFusion = None
        self.latentSurfaceProcesses = None
        
        self.rheology = None
