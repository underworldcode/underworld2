# Utilities to convert between dimensional and non-dimensional values.
# Romain BEUCHER, December 2016

import pint
u = pint.UnitRegistry(autoconvert_offset_to_baseunit = True)

def nonDimensionalize(dimValue, scaling):
    """
    This function uses pint object to perform a dimension analysis and
    return a value scaled according to a set of scaling coefficients:

    example:

    import pint

    u = pint.UnitRegistry(autoconvert_offset_to_baseunit = True)

    # Characteristic values of the system
    half_rate = (0.5 * u.centimeter / u.year).to(u.meter / u.second)
    model_height = 600e3 * u.meter
    refViscosity = (1e24 * u.pascal * u.second).to_base_units()
    surfaceTemp = (0. * u.degC).to_base_units()
    baseModelTemp = (1330. * u.degC).to_base_units()
    baseCrustTemp = (550. * u.degC).to_base_units()
    
    KL_meters = model_height
    KT_seconds = KL_meters / half_rate
    KM_kilograms = refViscosity * KL_meters * KT_seconds
    Kt_degrees = (baseModelTemp - surfaceTemp)
    K_substance = 1. * u.mole

    scaling = {"[time]": KT_seconds,
              "[length]": KL_meters, 
              "[mass]": KM_kilograms, 
              "[temperature]": Kt_degrees,
              "[substance]": K_substance}

    # Get a scaled value:
    gravity = nonDimensionalize(9.81 * u.meter / u.second**2, scaling)
    """
    
    if not isinstance(dimValue, u.Quantity):
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
    
    @u.check('[length]','[time]', '[mass]', '[temperature]', '[substance]')
    def check(length, time, mass, temperature, substance):
        return
    
    check(length, time, mass, temperature, substance)

    # Get dimensionality
    dlength = dimValue.dimensionality['[length]']
    dtime   = dimValue.dimensionality['[time]']
    dmass   = dimValue.dimensionality['[mass]']
    dtemp   = dimValue.dimensionality['[temperature]']
    dsubstance   = dimValue.dimensionality['[substance]']
    factor = (length**(-dlength) *
              time**(-dtime) *
              mass**(-dmass) *
              temperature**(-dtemp)*
              substance**(-dsubstance))
    
    dimValue *= factor
    
    if dimValue.unitless :
        return dimValue.magnitude
    else:
        raise ValueError('Dimension Error')


def Dimensionalize(Value, scaling, units):
    
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
    
    @u.check('[length]','[time]', '[mass]', '[temperature]', '[substance]')
    def check(length, time, mass, temperature, substance):
        return
    
    # Check that the scaling parameters have the correct dimensions
    check(length, time, mass, temperature, substance)
    
    # Get dimensionality
    dlength = unit.dimensionality['[length]']
    dtime   = unit.dimensionality['[time]']
    dmass   = unit.dimensionality['[mass]']
    dtemp   = unit.dimensionality['[temperature]']
    dsubstance   = unit.dimensionality['[substance]']
    factor = (length**(dlength) *
              time**(dtime) *
              mass**(dmass) *
              temperature**(dtemp)*
              substance**(dsubstance))    
    
    return (Value * factor).to(units)
