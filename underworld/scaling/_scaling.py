##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##

"""
Utilities to convert between dimensional and non-dimensional values.
"""
from __future__ import print_function, absolute_import
import underworld as uw
from ._utils import TransformedDict
from pint import UnitRegistry

u = UnitRegistry()

COEFFICIENTS = TransformedDict()
COEFFICIENTS["[length]"] = 1.0 * u.meter
COEFFICIENTS["[mass]"] = 1.0 * u.kilogram
COEFFICIENTS["[time]"] = 1.0 * u.year
COEFFICIENTS["[temperature]"] = 1.0 * u.degK
COEFFICIENTS["[substance]"] = 1.0 * u.mole

def get_coefficients():
    """
    Returns the global scaling dictionary.
    """
    return COEFFICIENTS

def non_dimensionalise(dimValue):
    """
    Non-dimensionalize (scale) provided quantity.

    This function uses pint to perform a dimension analysis and
    return a value scaled according to a set of scaling coefficients.

    Parameters
    ----------
    dimValue : pint quantity

    Returns
    -------
    float: The scaled value.

    Example:
    --------
    >>> import underworld as uw
    >>> u = uw.scaling.units

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

    >>> scaling_coefficients = uw.scaling.get_coefficients()
    >>> scaling_coefficients["[time]"] = KT_seconds
    >>> scaling_coefficients["[length]"] = KL_meters
    >>> scaling_coefficients["[mass]"] = KM_kilograms
    >>> scaling_coefficients["[temperature]"] = Kt_degrees
    >>> scaling_coefficients["[substance]"] -= K_substance

    >>> # Get a scaled value:
    >>> gravity = uw.scaling.non_dimensionalise(9.81 * u.meter / u.second**2)
    """
    try:
        val = dimValue.unitless
        if val:
            return dimValue
    except AttributeError:
        return dimValue

    dimValue = dimValue.to_base_units()

    scaling_coefficients = get_coefficients()

    length = scaling_coefficients["[length]"]
    time = scaling_coefficients["[time]"]
    mass = scaling_coefficients["[mass]"]
    temperature = scaling_coefficients["[temperature]"]
    substance = scaling_coefficients["[substance]"]

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


def dimensionalise(value, units):
    """
    Dimensionalise a value.

    Parameters
    ----------
    value : float, int
        The value to be assigned units.
    units : pint units
        The units to be assigned.

    Returns
    -------
    pint quantity: dimensionalised value.

    Example
    -------
    >>> import underworld as uw
    >>> A = uw.scaling.dimensionalise(1.0, u.metre)
    """

    unit = (1.0 * units).to_base_units()

    scaling_coefficients = get_coefficients()

    length = scaling_coefficients["[length]"]
    time = scaling_coefficients["[time]"]
    mass = scaling_coefficients["[mass]"]
    temperature = scaling_coefficients["[temperature]"]
    substance = scaling_coefficients["[substance]"]

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

    if (isinstance(value, uw.mesh._meshvariable.MeshVariable) or
       isinstance(value, uw.swarm._swarmvariable.SwarmVariable)):

        tempVar = value.copy()
        tempVar.data[...] = (value.data[...] * factor).to(units)
        return tempVar
    else:
        return (value * factor).to(units)

