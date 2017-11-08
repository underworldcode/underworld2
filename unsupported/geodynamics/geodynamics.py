import underworld.function as fn
import numpy as np
from unsupported import rheology
import unsupported.scaling as sca
from unsupported.scaling import nonDimensionalize as nd

u = UnitRegistry = sca.UnitRegistry
_viscousLaws = rheology.ViscousLaws
  
equivalents = {"A": "preExponentialFactor",
               "n": "stressExponent",
               "Q": "activationEnergy",
               "Va": "activationVolume",
               "DefaultSRInvariant": "defaultStrainRateInvariant"}

_plasticLaws = rheology.PlasticBehaviours

equivalents2 = {"Cohesion": "cohesion",
                "FrictionCoef": "frictionCoefficient",
                "FrictionCoefSw": "frictionAfterSoftening",
                "CohesionSw": "cohesionAfterSoftening",
                "MinimumViscosity": "minimumViscosity"}

class Rheology(object):

    def __init__(self, pressureField=None, strainRateInvariantField=None,
                 temperatureField=None, viscosityLimiter=None, stressLimiter=None):

        self.pressureField = pressureField
        self.strainRateInvariantField = strainRateInvariantField
        self.temperatureField = temperatureField
        self.viscosityLimiter = viscosityLimiter
        self.stressLimiter = stressLimiter
        self.firstIter = True
        
        return

class DruckerPrager(object):

    def __init__(self, predefined=None, cohesion=None, frictionCoefficient=None,
                 cohesionAfterSoftening = None,
                 frictionAfterSoftening = None,
                 minimumViscosity=None, plasticStrain=None, pressureField=None,
                 epsilon1=0.5, epsilon2=1.0):

        kwargs = {"cohesion": cohesion,
                 "cohesionAfterSoftening": cohesionAfterSoftening,
                 "frictionCoefficient": frictionCoefficient,
                 "frictionAfterSoftening": frictionAfterSoftening}

        if predefined:
            if predefined in _plasticLaws.keys():
                plasticCriteria = _plasticLaws[predefined]["coefficients"]
                self.init_from_dict(plasticCriteria)
                for key, val in kwargs.iteritems():
                    if key not in self.__dict__.keys():
                        setattr(self, key, val)
            else:
                raise ValueError("Available predefined plasticities: ",_viscousLaws.keys()) 
        else:
            for key, val in kwargs.iteritems():
                setattr(self, key, val)
        
        self.cohesionFn = rheology.linearCohesionWeakening
        self.frictionFn = rheology.linearFrictionWeakening
    
    def init_from_dict(self, dictionary):

        for key, val in dictionary.iteritems():
            if key in dir(self):
                setattr(self, key, val)

        for key, val in dictionary.iteritems():
            if key in equivalents2.keys():
                setattr(self, equivalents2[key], val)

    def _get_friction(self):
        if self.plasticStrain:
            friction = self.frictionFn(
                self.plasticStrain,
                FrictionCoef=nd(self.frictionCoefficient),
                FrictionCoefSw=nd(self.frictionAfterSoftening))
        else:
            friction = fn.misc.constant(nd(self.frictionCoefficient))
        return friction

    def _get_cohesion(self):
        if self.plasticStrain:
            cohesion = self.cohesionFn(
                self.plasticStrain,
                Cohesion=nd(self.cohesion),
                CohesionSw=nd(self.cohesionAfterSoftening))
        else:
            cohesion = fn.misc.constant(self.cohesion)
        return cohesion
        
    def _get_yieldStress2D(self):
        f = self._get_friction()
        C = self._get_cohesion()
        P = self.pressureField
        self.yieldStress = (C * fn.math.cos(f) + P * fn.math.sin(f))
        return self.yieldStress

    def _get_yieldStress3D(self):
        f = self._get_friction()
        C = self._get_cohesion()
        P = self.pressureField
        self.yieldStress = 6.0*C*fn.math.cos(f) + 2.0*fn.math.sin(f)*fn.misc.max(P, 0.0) 
        self.yieldStress /= (fn.math.sqrt(3.0) * (3.0 + fn.math.sin(f)))
        return self.yieldStress

    def _get_effective_viscosity(self):
        eij = self.strainRateInvariantField
        eijdef = self.strainRate_default
        self._get_yieldStress(self.plasticStrain, self.pressure)
        self.effViscosity = 0.5 * self.yieldStress / fn.misc.max(eij, eijdef)
        return self.effViscosity

class ConstantViscosity(Rheology):

    def __init__(self, viscosity):
        super(ConstantViscosity, self).__init__()
        self._viscosity = nd(viscosity)

    @property
    def muEff(self):
        return self._effectiveViscosity()
       
    def _effectiveViscosity(self):
        return fn.misc.constant(nd(self._viscosity))


class ViscousCreep(Rheology):

    def __init__(self,
                 predefined=None,
                 strainRateInvariantField=None,
                 temperatureField=None,
                 pressureField=None,
                 preExponentialFactor=1.0,
                 stressExponent=1.0,
                 defaultStrainRateInvariant=1.0e-13 / u.seconds,
                 activationVolume=0.0,
                 activationEnergy=0.0,
                 waterFugacity=None,
                 grainSize=None,
                 meltFraction=None,
                 grainSizeExponent=0.0,
                 waterFugacityexponent=0.0,
                 meltFractionFactor=0.0,
                 f=1.0):
        
        super(ViscousCreep, self).__init__()

        kwargs = {"strainRateInvariantField": strainRateInvariantField,
                 "temperatureField": temperatureField,
                 "pressureField": pressureField,
                 "preExponentialFactor": preExponentialFactor,
                 "stressExponent": stressExponent,
                 "defaultStrainRateInvariant": defaultStrainRateInvariant,
                 "activationVolume": activationVolume,
                 "activationEnergy": activationEnergy,
                 "waterFugacity": waterFugacity,
                 "grainSize": grainSize,
                 "meltFraction": meltFraction,
                 "grainSizeExponent": grainSizeExponent,
                 "waterFugacityexponent": waterFugacityexponent,
                 "meltFractionFactor": meltFractionFactor,
                 "f":f}

        if predefined:
            if predefined in _viscousLaws.keys():
                rheology = _viscousLaws[predefined]["coefficients"]
                self.init_from_dict(rheology)
                for key, val in kwargs.iteritems():
                    if key not in self.__dict__.keys():
                        setattr(self, key, val)
            else:
                raise ValueError("Available predefined viscosities: ",_viscousLaws.keys()) 
        else:
            for key, val in kwargs.iteritems():
                setattr(self, key, val)
            
        self.f = f
        self.constantGas = 8.3144621 * u.joule / u.mole / u.degK  

    def init_from_dict(self, dictionary):

        for key, val in dictionary.iteritems():
            if key in dir(self):
                setattr(self, key, val)

        for key, val in dictionary.iteritems():
            if key in equivalents.keys():
                setattr(self, equivalents[key], val)
    
    @property
    def muEff(self):
        return self._effectiveViscosity()

    def _effectiveViscosity(self):
        A = nd(self.preExponentialFactor)
        n = nd(self.stressExponent)
        P = self.pressureField
        T = self.temperatureField
        Q = nd(self.activationEnergy)
        Va = nd(self.activationVolume)
        p = nd(self.grainSizeExponent)
        d = nd(self.grainSize)
        r = nd(self.waterFugacityexponent)
        fH2O = nd(self.waterFugacity)
        I = self._get_second_invariant()
        f = self.f
        F = nd(self.meltFraction)
        alpha = nd(self.meltFractionFactor)
        R = nd(self.constantGas)

        mu_eff = f * 0.5 * A**(-1.0 / n)

        if np.abs(n - 1.0) > 1e-5:
            mu_eff *= I**((1.0-n)/n)
        
        # Grain size dependency
        if p and d:
            mu_eff *= d**(p/n)

        # Water dependency
        if r and fH2O:
            mu_eff *= fH2O**(-r/n)

        if F:
            mu_eff *= fn.math.exp(-1.0*alpha*F/n)

        if T:
            mu_eff *= fn.math.exp((Q + P * Va) / (R*T*n))

        if self.viscosityLimiter:
            mu_eff = self.viscosityLimiter.apply(mu_eff)
        
        return mu_eff

    def _get_second_invariant(self):
        FirstIterCondition = [(self.firstIter,nd(self.defaultStrainRateInvariant)),
                              (True, self.strainRateInvariantField)]
        return fn.branching.conditional(FirstIterCondition)
