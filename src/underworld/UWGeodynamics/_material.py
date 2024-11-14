from __future__ import print_function,  absolute_import
from itertools import count
from copy import copy
from collections import OrderedDict
import json
import pkg_resources
from underworld.scaling import units as u
from ._utils import PhaseChange
from ._rheology import ConstantViscosity
from ._density import ConstantDensity
from pint.errors import DimensionalityError
from ._density import LinearDensity

_dim_density = {'[mass]': 1.0, '[length]': -3.0}
_dim_diffusivity = {'[length]': 2.0, '[time]': -1.0}
_dim_capacity = {'[length]': 2.0, '[temperature]': -1.0, '[time]': -2.0}
_dim_radiogenicHeatProduction = {'[length]': -1.0, '[mass]': 1.0, '[time]': -3.0}
_dim_viscosity = {'[length]': -1.0, '[mass]': 1.0, '[time]': -1.0}
_dim_stress = {'[length]': -1.0, '[mass]': 1.0, '[time]': -2.0}
_dim_rate = {'[time]': -1.0}



class Material(object):
    """ Material class object """

    _ids = count(0)

    def __init__(self, name="Undefined", shape=None,
                 density=0.*u.kilogram/u.metre**3, diffusivity=None,
                 capacity=None, radiogenicHeatProd=0.0*u.microwatt/u.meter**3,
                 initial_viscosity=None, viscosity=None,
                 plasticity=None, elasticity=None,
                 minViscosity=None, maxViscosity=None,
                 stressLimiter=None, healingRate=0.0 / u.year,
                 solidus=None, liquidus=None,
                 latentHeatFusion=0.0, meltExpansion=0.0, meltFraction=0.0,
                 meltFractionLimit=1.0, viscosityChangeX1=0.15,
                 viscosityChangeX2=0.3, viscosityChange=1.0):
        """Create a Material

        Parameters
        ----------

        name : Name of the Material
        shape : Shape defining the space occupied by the material.
        density : Density (volumetric mass) of the material ([Mass]/[Length]**3)
        diffusivity : Diffusivity ([Length]**2 / [Time])
        capacity : Capacity ([Length]**2 / [Temperature] / [Time]**2)
        radiogenicHeatProd : Radiogenic Heat Production Value ([Mass]/[Length]/[Time])
        viscosity : Viscous Law or Value ([Mass]/[Length]/[Time])
        plasticity : Plastic Law
        elasticity : Elastic Law
        minViscosity : Minimum viscosity allowed for the material
        maxViscosity : Maximum viscosity allowed for the materil
        stressLimiter : Maximum stress allowed for the material
        healingRate : Plastic Strain Healing Rate ([Time]**(-1)]

        # Melt Related parameters:

        solidus : UWGeodynamics.Solidus object defining a Solidus
        liquidus : UWGeodynamics.Liquidus object defining a Liquidus
        latentHeatFusion : Latent Heat of Fusion (enthalpie)
        meltExpansion : Melt expansion
        meltFraction : Initial Fraction of Melt
        meltFractionLimit : Maximum Fraction of Melt (0-1)

        ## Change in viscosity associated to the presence of melt:

        viscosityChange is a factor representing a linear
        change in viscosity over a melt fraction interval
        (ViscosityChangeX1 -- ViscosityChangeX2)

        viscosityChange : Viscosity Change Factor
        viscosityChangeX1 : Start of the viscosity change interval
        viscosityChangeX2 : End of the viscosity change interval

        Returns
        -------

        A UWGeodynamics.Material class object.

        Example
        -------

        >>> from underworld import UWGeodynamics as GEO
        >>> crust = GEO.Material(name="Crust")

        """

        self.index = next(self._ids)

        self.name = name
        self.top = None
        self.bottom = None

        self.shape = shape
        self.density = density
        self.referenceDensity = self.density
        self.diffusivity = diffusivity
        self.capacity = capacity
        self._thermalExpansivity = None
        self.radiogenicHeatProd = radiogenicHeatProd

        self.minViscosity = minViscosity
        self.maxViscosity = maxViscosity
        self.stressLimiter = stressLimiter

        self.compressibility = None

        self.solidus = solidus
        self.liquidus = liquidus
        self.latentHeatFusion = latentHeatFusion
        self.meltFraction = meltFraction
        self.meltFractionLimit = meltFractionLimit
        self.meltExpansion = meltExpansion
        self.viscosityChangeX1 = viscosityChangeX1
        self.viscosityChangeX2 = viscosityChangeX2
        self.viscosityChange = viscosityChange
        self.melt = False

        self._viscosity = None
        self.viscosity = viscosity
        self._initial_viscosity = None
        self.initial_viscosity = initial_viscosity
        self._plasticity = None
        self.plasticity = plasticity

        self.elasticity = elasticity
        self.healingRate = healingRate
        self._phase_changes = list()
        self.Model = None

    def _repr_html_(self):
        return _material_html_repr(self)

    def __getitem__(self, name):
        return self.__dict__[name]

    @property
    def viscosity(self):
        return self._viscosity

    @viscosity.setter
    def viscosity(self, value):
        self._viscosity = _process_viscosity_value(value)

    @property
    def initial_viscosity(self):
        return self._initial_viscosity

    @initial_viscosity.setter
    def initial_viscosity(self, value):
        self._initial_viscosity = _process_viscosity_value(value)

    @property
    def plasticity(self):
        return self._plasticity

    @plasticity.setter
    def plasticity(self, value):
        if isinstance(value, str):
            self._plasticity = get_plasticity_from_registry(value)
        else:
            self._plasticity = value

    @property
    def density(self):
        return self._density

    @density.setter
    def density(self, value):
        if isinstance(value, LinearDensity):
            self._density = value
            self._thermalExpansivity = value.thermalExpansivity
        else:
            self._density = ConstantDensity(value)

    @property
    def thermalExpansivity(self):
        return self._thermalExpansivity

    @thermalExpansivity.setter
    def thermalExpansivity(self, value):
        self._thermalExpansivity = value

    def add_melt_modifier(self, solidus, liquidus, latentHeatFusion,
                          meltExpansion,
                          meltFraction=0.0,
                          meltFractionLimit=1.0,
                          viscosityChangeX1=0.15,
                          viscosityChangeX2=0.3,
                          viscosityChange=1e3):

        self.solidus = solidus
        self.liquidus = liquidus
        self.latentHeatFusion = latentHeatFusion
        self.meltFraction = meltFraction
        self.meltFractionLimit = meltFractionLimit
        self.meltExpansion = meltExpansion
        self.viscosityChangeX1 = viscosityChangeX1
        self.viscosityChangeX2 = viscosityChangeX2
        self.viscosityChange = viscosityChange
        self.melt = True

    @property
    def phase_changes(self):
        return self._phase_changes

    @phase_changes.setter
    def phase_changes(self, value):
        if not isinstance(value, PhaseChange):
            raise ValueError("Must be a PhaseChange object")
        else:
            self._phase_changes.append(value)


_default = OrderedDict()
_default["Radiogenic Heat Production"] = "radiogenicHeatProd"
_default["Diffusivity"] = "diffusivity"
_default["Capacity"] = "capacity"
_default["Min Viscosity Limit"] = "minViscosity"
_default["Max Viscosity Limit"] = "maxViscosity"

_melt = OrderedDict()
_melt["Solidus"] = ""
_melt["Liquidus"] = ""
_melt["Latent Heat Fusion"] = "latentHeatFusion"
_melt["Melt Expansion"] = "meltExpansion"
_melt["Melt Fraction"] = "meltFraction"
_melt["Melt Fraction Limit"] = "meltFractionLimit"
_melt["Viscosity Change"] = "viscosityChange"
_melt["Viscosity Change X1"] = "viscosityChangeX1"
_melt["Viscosity Change X2"] = "viscosityChangeX2"


def _material_html_repr(Material):
    header = "<table>"
    footer = "</table>"
    html = """
    <tr>
      <th colspan="2">{0}</th>
    </tr>""".format(Material.name)

    if Material.density:
        key = "Density"
        value = str(Material.density.name)
        html += "<tr><td>{0}</td><td>{1}</td></tr>".format(key, value)

    for key, val in _default.items():
        value = Material.__dict__.get(val)
        if value is None:
            value = Material.__dict__.get("_" + val)
        if value is None and Material.Model:
            value = Material.Model.__dict__.get(val)
        if value is None and Material.Model:
            value = Material.Model.__dict__.get("_" + val)
        html += "<tr><td>{0}</td><td>{1}</td></tr>".format(key, value)

    if Material.viscosity and Material.plasticity:
        type_ = "(Visco-plastic)"
    elif Material.viscosity:
        type_ = "(Viscous)"
    elif Material.plasticity:
        type_ = "(Plastic)"
    else:
        type_ = ""

    html += """
    <tr>
      <th colspan="2">Rheology {}</th>
    </tr>""".format(type_)

    if Material.viscosity:
        html += "<tr><td>{0}</td><td>{1}</td></tr>".format(
            "Viscosity", Material.viscosity.name)
    if Material.plasticity:
        html += "<tr><td>{0}</td><td>{1}</td></tr>".format(
            "Plasticity", Material.plasticity.name)
    if not Material.viscosity and not Material.plasticity:
        html += "<tr><td></td><td>{0}</td></tr>".format(
            "None")

    return header + html + footer


class MaterialRegistry(object):
    """ Library of commonly used Materials """

    def __init__(self, filename=None):
        """Create a registry of materials

        Parameters
        ----------

        filename : (optional) filename of the json file
                   database.

        Returns
        -------

        An UWGeodynamics MaterialRegistry Class
        """

        if not filename:
            filename = pkg_resources.resource_filename(
                __name__, "resources/Materials.json")

        def get_value(item):
            value = item["value"]
            units = item["units"]
            if units != "None":
                return u.Quantity(value, units)
            else:
                return value

        with open(filename, "r") as infile:
            _materials = json.load(infile)

        self._dir = {}
        for material, parameters in _materials.items():
            name = material.replace(" ", "_").replace(",", "").replace(".", "")
            name = name.replace(")", "").replace("(", "")

            for key, item in parameters.items():
                if isinstance(item, dict):
                    if "value" in item.keys():
                        parameters[key] = get_value(item)
                    elif ("thermalExpansivity" or "beta") in item.keys():
                        for prop in item.keys():
                            item[prop] = get_value(item[prop])
                        parameters[key] = LinearDensity(**item)

            self._dir[name] = Material(name=material, **parameters)

    def __dir__(self):
        # Make all the rheology available through autocompletion
        return list(self._dir.keys())

    def __getattr__(self, item):
        # Make sure to return a new instance of ViscousCreep
        return copy(self._dir[item])


def _process_viscosity_value(value):
    """ Process the value assign to the viscosity attribute """
    if isinstance(value, (u.Quantity, float)):
        # Check dimensionality
        if (isinstance(value, u.Quantity) and
           value.dimensionality != _dim_viscosity):
            _dim_vals = u.get_dimensionality(_dim_viscosity)
            raise DimensionalityError(value, 'a quantity of',
                                      value.dimensionality,
                                      _dim_vals)

        return ConstantViscosity(value)
    elif isinstance(value, str):
        return get_viscosity_from_registry(value)
    else:
        return value


def get_viscosity_from_registry(rheology_name):
    """Get Viscosity from default registry

    Parameters
    ----------

    rheology_name : Name of the rheology

    Returns
    -------

    An UWGeodynamics ViscousCreepRheology object.
    """

    from underworld.UWGeodynamics import ViscousCreepRegistry
    rh = ViscousCreepRegistry()
    name = rheology_name.replace(",", "").replace(".", "")
    name = [word.strip() for word in name.split()
            if word.lower() not in ["viscous", "creep"]]
    name = "_".join(name)
    return rh._dir[name]


def get_plasticity_from_registry(plasticity_name):
    """Get plasticity from default plasticity registry

    Parameters
    ----------

    plasticity_name : Name of the plasticity reference

    Returns
    -------

    An UWGeodynamics DruckerPrager object
    """

    from underworld.UWGeodynamics import PlasticityRegistry
    pl = PlasticityRegistry()
    name = plasticity_name.replace(" ", "_").replace(",", "").replace(".", "")
    name = name.replace(")", "").replace("(", "")
    return pl._dir[name]
