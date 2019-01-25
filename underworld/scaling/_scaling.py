# Utilities to convert between dimensional and non-dimensional values.
# Romain BEUCHER, December 2016
from __future__ import print_function, absolute_import
import underworld as uw
from ._coefficients import COEFFICIENTS as scaling
from ._utils import u


def nonDimensionalize(dimValue):
    """Non-dimensionalize (scale) quantity

    This function uses pint object to perform a dimension analysis and
    return a value scaled according to a set of scaling coefficients:

    Parameters
    ----------

        dimValue : pint Quantity

    Returns
    -------

        float

    example:
    --------

    >>> import underworld as uw

    >>> u = uw.UnitRegistry

    >>> # Characteristic values of the system
    >>> half_rate = 0.5 * u.centimeter / u.year
    >>> model_height = 600e3 * u.meter
    >>> refViscosity = 1e24 * u.pascal * u.second
    >>> surfaceTemp = 0. * u.kelvin
    >>> baseModelTemp = 1330. * u.kelvin
    >>> baseCrustTemp = 550. * u.kelvin

    >>> KL_meters = model_height
    >>> KT_seconds = KL_meters / half_rate
    >>> KM_kilograms = refViscosity * KL_meters * KT_seconds
    >>> Kt_degrees = (baseModelTemp - surfaceTemp)
    >>> K_substance = 1. * u.mole

    >>> uw.scaling_coefficients["[time]"] = KT_seconds
    >>> uw.scaling_coefficients["[length]"] = KL_meters
    >>> uw.scaling_coefficients["[mass]"] = KM_kilograms
    >>> uw.scaling_coefficients["[temperature]"] = Kt_degrees
    >>> uw.scaling_coefficients["[substance]"] -= K_substance

    >>> # Get a scaled value:
    >>> gravity = uw.nonDimensionalize(9.81 * u.meter / u.second**2)
    """
    global scaling

    try:
        val = dimValue.unitless
        if val:
            return dimValue
    except AttributeError:
        return dimValue

    dimValue = dimValue.to_base_units()

    length = scaling["[length]"]
    time = scaling["[time]"]
    mass = scaling["[mass]"]
    temperature = scaling["[temperature]"]
    substance = scaling["[substance]"]

    length = length.to_base_units()
    time = time.to_base_units()
    mass = mass.to_base_units()
    temperature = temperature.to_base_units()
    substance = substance.to_base_units()

    @u.check('[length]', '[time]', '[mass]', '[temperature]', '[substance]')
    def check(length, time, mass, temperature, substance):
        return

    check(length, time, mass, temperature, substance)

    # Get dimensionality
    dlength = dimValue.dimensionality['[length]']
    dtime = dimValue.dimensionality['[time]']
    dmass = dimValue.dimensionality['[mass]']
    dtemp = dimValue.dimensionality['[temperature]']
    dsubstance = dimValue.dimensionality['[substance]']
    factor = (length**(-dlength) *
              time**(-dtime) *
              mass**(-dmass) *
              temperature**(-dtemp) *
              substance**(-dsubstance))

    dimValue *= factor

    if dimValue.unitless:
        return dimValue.magnitude
    else:
        raise ValueError('Dimension Error')


def Dimensionalize(Value, units):
    """Dimensionalize a value

    Parameters
    ----------

    Value : float, int

    units : pint units
        output units

    Returns
    -------

       pint Quantity

    example
    -------

    >>> import underworld as uw

    >>> A = uw.Dimensionalize(1.0, u.metre)
    """

    global scaling

    unit = (1.0 * units).to_base_units()

    length = scaling["[length]"]
    time = scaling["[time]"]
    mass = scaling["[mass]"]
    temperature = scaling["[temperature]"]
    substance = scaling["[substance]"]

    length = length.to_base_units()
    time = time.to_base_units()
    mass = mass.to_base_units()
    temperature = temperature.to_base_units()
    substance = substance.to_base_units()

    @u.check('[length]', '[time]', '[mass]', '[temperature]', '[substance]')
    def check(length, time, mass, temperature, substance):
        return

    # Check that the scaling parameters have the correct dimensions
    check(length, time, mass, temperature, substance)

    # Get dimensionality
    dlength = unit.dimensionality['[length]']
    dtime = unit.dimensionality['[time]']
    dmass = unit.dimensionality['[mass]']
    dtemp = unit.dimensionality['[temperature]']
    dsubstance = unit.dimensionality['[substance]']
    factor = (length**(dlength) *
              time**(dtime) *
              mass**(dmass) *
              temperature**(dtemp) *
              substance**(dsubstance))

    if (isinstance(Value, uw.mesh._meshvariable.MeshVariable) or
       isinstance(Value, uw.swarm._swarmvariable.SwarmVariable)):

        tempVar = Value.copy()
        tempVar.data[...] = (Value.data[...] * factor).to(units)
        return tempVar
    else:
        return (Value * factor).to(units)

