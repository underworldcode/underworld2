from unsupported.scaling import *
import underworld.function as fn

def linearCohesionWeakening(cumulativeTotalStrain, Cohesion, CohesionSw, epsilon1=0.5, epsilon2=1.5, **kwargs):

    cohesionVal = [(cumulativeTotalStrain < epsilon1, fn.misc.constant(Cohesion)),
                   (cumulativeTotalStrain > epsilon2, fn.misc.constant(CohesionSw)),
                   (True, Cohesion + ((Cohesion - CohesionSw)/(epsilon1 - epsilon2)) * (cumulativeTotalStrain - epsilon1))]

    return fn.branching.conditional(cohesionVal)

def linearFrictionWeakening(cumulativeTotalStrain, FrictionCoef, FrictionCoefSw, epsilon1=0.5, epsilon2=1.5, **kwargs):

    frictionVal = [(cumulativeTotalStrain < epsilon1, fn.misc.constant(FrictionCoef)),
                   (cumulativeTotalStrain > epsilon2, fn.misc.constant(FrictionCoefSw)),
                   (True, FrictionCoef + ((FrictionCoef - FrictionCoefSw)/(epsilon1 - epsilon2)) * (cumulativeTotalStrain - epsilon1))]

    frictionVal = fn.branching.conditional(frictionVal)
 
    return fn.math.atan(frictionVal)



Yield_UpperCrust_Rey2010 = {"Cohesion": 10. * u.megapascal,
                        "CohesionSw": 2. * u.megapascal,
                        "FrictionCoef": 0.577,
                        "FrictionCoefSw": 0.1154,
                        "MinimumViscosity": 1e19 * u.pascal * u.second}

Yield_lowerCrust_Rey2010 = {"Cohesion": 20. * u.megapascal,
                        "CohesionSw": 4. * u.megapascal,
                        "FrictionCoef": 0.577,
                        "FrictionCoefSw": 0.1154,
                        "MinimumViscosity": 1e19 * u.pascal * u.second}

Yield_mantle_Rey2010 = {"Cohesion": 10. * u.megapascal,
                        "CohesionSw": 2. * u.megapascal,
                        "FrictionCoef": 0.577,
                        "FrictionCoefSw": 0.1154,
                        "MinimumViscosity": 1e19 * u.pascal * u.second}

Yield_WeakCrust_HuismansBeaumont2007 = {"Cohesion": 20. * u.megapascal,
                        "CohesionSw": 20. * u.megapascal,
                        "FrictionCoef": 0.123,
                        "FrictionCoefSw": 0.017,
                        "MinimumViscosity": 1e19 * u.pascal * u.second}


PlasticBehaviours = {
"Rey et al., 2010 (UpperCrust)": {"coefficients":  Yield_UpperCrust_Rey2010},
"Rey et al., 2010 (LowerCrust)": {"coefficients":  Yield_lowerCrust_Rey2010},
"Rey et al., 2010 (Mantle)": {"coefficients":  Yield_mantle_Rey2010},
"Huismans and Beaumont, 2007 (WeakCrust)": {"coefficients":  Yield_WeakCrust_HuismansBeaumont2007}
}
