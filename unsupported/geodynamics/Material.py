from itertools import count


class Material(object):
    _ids = count(0)

    def __init__(self, name="Undefined", vertices=None, density=None,
                 diffusivity=None, capacity=None, thermalExpansivity=None,
                 radiogenicHeatProd=None, shape=None, viscosity=None,
                 plasticity=None):

        self.index = self._ids.next()
        
        self.name = name
        self.top = None
        self.bottom = None

        self.shape = shape
        self.density = density
        self.diffusivity = diffusivity
        self.capacity = capacity
        self.thermalExpansivity = thermalExpansivity
        self.radiogenicHeatProd = radiogenicHeatProd
        self.meltFraction = 0.
        self.meltFractionLimit = 0.
        self.compressibility = None

        self.rheology = None  # For backward compatibility

        self._viscosity = viscosity
        self._plasticity = plasticity

    def __repr__(self):
        return self.name

    @property
    def shape(self):
        return self._shape

    @shape.setter
    def shape(self, value=None):
        self._shape = value

    @property
    def density(self):
        return self._density

    @density.setter
    def density(self, value):
        self._density = value

    @property
    def diffusivity(self):
        return self._diffusivity

    @diffusivity.setter
    def diffusivity(self, value):
        self._diffusivity = value

    @property
    def capacity(self):
        return self._capacity

    @capacity.setter
    def capacity(self, value):
        self._capacity = value

    @property
    def radiogenicHeatProd(self):
        return self._radiogenicHeatProd

    @radiogenicHeatProd.setter
    def radiogenicHeatProd(self, value):
        self._radiogenicHeatProd = value
    
    @property
    def thermalExpansivity(self):
        return self._thermalExpansivity

    @thermalExpansivity.setter
    def thermalExpansivity(self, value):
        self._thermalExpansivity = value
    
    @property
    def viscosity(self):
        return self._viscosity

    @viscosity.setter
    def viscosity(self, value):
        self._viscosity = value

    @property
    def plasticity(self):
        return self._plasticity

    @plasticity.setter
    def plasticity(self, value):
        self._plasticity = value