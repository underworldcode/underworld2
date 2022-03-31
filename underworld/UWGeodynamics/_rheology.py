from __future__ import print_function, absolute_import


import json
import abc
import underworld.function as fn
import numpy as np
from UWGeodynamics import UnitRegistry as u
from UWGeodynamics import non_dimensionalise as nd
from copy import copy
from collections import OrderedDict

ABC = abc.ABCMeta('ABC', (object,), {})


def linearCohesionWeakening(cumulativeTotalStrain, Cohesion, CohesionSw,
                            epsilon1=0.5, epsilon2=1.5):
    """Calculate the cohesion through linear weakening as
       a function of accumulated plastic strain.

    Parameters
    ----------

        cumulativeTotalStrain :
            Accumulated Plastic Strain field
        Cohesion :
            Cohesion for a pristine material Cohesion(epsilon1)
        CohesionSw :
            Cohesion(epsilon2)
        epsilon1 :
            Start of weakening (fraction of accumulated plastic strain)
        epsilon2 : End of weakening (fraction of accumulated plastic strain)

    Returns
    -------
        Underworld function which returns the cohesion as a function of
        accumulated plastic strain.

    """
    cohesionVal = [(cumulativeTotalStrain < epsilon1, Cohesion),
                   (cumulativeTotalStrain > epsilon2, CohesionSw),
                   (True, Cohesion + ((Cohesion - CohesionSw) /
                                      (epsilon1 - epsilon2)) *
                    (cumulativeTotalStrain - epsilon1))]

    return fn.branching.conditional(cohesionVal)


def linearFrictionWeakening(cumulativeTotalStrain, FrictionCoef,
                            FrictionCoefSw, epsilon1=0.5, epsilon2=1.5):
    """Calculate the friction angle through linear weakening as a function of
       accumulated plastic strain.

    Parameters
    ----------

        cumulativeTotalStrain : Underworld Mesh Variable
            Accumulated Plastic Strain field
        friction angle :
            friction angle for a pristine material (friction angle(epsilon1))
        friction angleSw : float
            friction angle(epsilon2)
        epsilon1 : float
            Start of weakening (fraction of accumulated plastic strain)
        epsilon2 : float
            End of weakening (fraction of accumulated plastic strain)

    Returns
    -------

        Underworld function which returns the friction angle as a function of
        accumulated plastic strain.
    """

    frictionVal = [(cumulativeTotalStrain < epsilon1, FrictionCoef),
                   (cumulativeTotalStrain > epsilon2, FrictionCoefSw),
                   (True, FrictionCoef + ((FrictionCoef - FrictionCoefSw) /
                                          (epsilon1 - epsilon2)) *
                    (cumulativeTotalStrain - epsilon1))]

    frictionVal = fn.branching.conditional(frictionVal)

    return fn.math.atan(frictionVal)


class Limiter(fn.Function):
    """ Viscosity Limiter Class """

    def __init__(self, value, min_value=None, max_value=None):

        self.value = fn.Function.convert(value)
        self.min_value = fn.Function.convert(nd(min_value))
        self.max_value = fn.Function.convert(nd(max_value))

        if self.max_value and self.min_value:
            self._fn = fn.misc.min(self.value, self.max_value)
            self._fn = fn.misc.max(self._fn, self.min_value)
        elif self.max_value:
            self._fn = fn.misc.min(self.value, self.max_value)
        elif self.min_value:
            self._fn = fn.misc.max(self.value, self.min_value)
        else:
            self._fn = self.value

        super(Limiter, self).__init__(
            argument_fns=[self.value,
                          self.min_value,
                          self.max_value])
        self._fncself = self._fn._fncself


class Viscosity_limiter(Limiter):

    def __init__(self, viscosity, min_viscosity, max_viscosity):
        self.min_viscosity = min_viscosity
        self.max_viscosity = max_viscosity
        super(Viscosity_limiter, self).__init__(viscosity, min_viscosity,
                                                max_viscosity)


class Stress_limiter(Limiter):

    def __init__(self, stress, max_stress):
        self.max_stress = max_stress
        super(Stress_limiter, self).__init__(stress, max_value=max_stress)


class Rheology(ABC):
    """Rheology Base Class"""

    def __init__(self, pressureField=None, strainRateInvariantField=None,
                 temperatureField=None, viscosityLimiter=None,
                 stressLimiter=None):

        self.pressureField = pressureField
        self.strainRateInvariantField = strainRateInvariantField
        self.temperatureField = temperatureField
        self.stressLimiter = stressLimiter
        self.firstIter = fn.misc.constant(True)

        self.viscosity = None
        self.plasticity = None
        self.cohesion = None
        self.friction = None

        return

    @abc.abstractmethod
    def muEff(self):
        pass


class DruckerPrager(object):
    """The Drucker Prager yield criterion class.

    The yield strength


    """

    def __init__(self, name=None, cohesion=None, frictionCoefficient=None,
                 cohesionAfterSoftening=None,
                 frictionAfterSoftening=None,
                 epsilon1=0.0, epsilon2=0.2):
        """Drucker Prager yield Rheology.

         The pressure-dependent Drucker-Yield criterion is defined as follow:

         .. math::

            :label: druckerprager

            \sigma_y = C \cos\phi + \sin\phi P \quad \text{(2D)}

            \sigma_y = \frac{6C\cos\phi}{\sqrt3(3-\sin\phi)} +
                       \frac{6\sin\phi P}{\sqrt3(3-\sin\phi)} \quad \text{(3D)}


         Setting the friction angle :math:`\phi=0` gives the von Mises Criterion.
         In 2D, equation :eq:`druckerprager` corresponds to the Mohr-Coulomb criterion,
         while in 3D it circumscribes the Mohr-Coulomb yield surface.

         Linear cohesion and friction weakening can be added by defining their
         initial and final values over a range of accumulated plastic strain.

        Parameters
        ----------
            cohesion :
                Cohesion for the pristine material(initial cohesion)
            cohesionAfterSoftening :
                Cohesion of the weakened material
            frictionCoefficient :
                friction angle for a pristine material
            frictionAfterSoftening :
                friction angle of weakened material(epsilon2)
            epsilon1 :
                Start of weakening (fraction of accumulated plastic strain)
            epsilon2 :
                End of weakening (fraction of accumulated plastic strain)

        Returns
        -------

        An UWGeodynamics DruckerPrager class

        """
        self.name = name
        self._cohesion = cohesion
        self._frictionCoefficient = frictionCoefficient
        self.cohesionAfterSoftening = cohesionAfterSoftening
        self.frictionAfterSoftening = frictionAfterSoftening

        self.plasticStrain = None
        self.pressureField = None
        self.epsilon1 = epsilon1
        self.epsilon2 = epsilon2

        self.cohesionWeakeningFn = linearCohesionWeakening
        self.frictionWeakeningFn = linearFrictionWeakening

        self.onlinePDF = None
        self.citation = None

    def __getitem__(self, name):
        return self.__dict__[name]

    def _repr_html_(self):
        attributes = OrderedDict()
        if isinstance(self.citation, str):
            attributes["Citation"] = self.citation
            if isinstance(self.onlinePDF, str):
                if self.onlinePDF.startswith("http"):
                    attributes["Citation"] = '<a href="{0}">{1}</a>'.format(
                        self.onlinePDF, self.citation)
        attributes["Cohesion"] = self.cohesion
        attributes["Cohesion After Softening"] = self.cohesionAfterSoftening
        attributes["Friction Coefficient"] = self.frictionCoefficient
        attributes["Friction Coefficient after Softening"] = (
            self.frictionAfterSoftening)
        attributes["Epsilon 1"] = self.epsilon1
        attributes["Epsilon 2"] = self.epsilon2
        header = "<table>"
        footer = "</table>"
        html = ""
        for key, val in attributes.items():
            html += '<tr><td style="text-align:left;">{0}</td><td style="text-align:left;">{1}</td></tr>'.format(key, val)

        return header + html + footer

    @property
    def cohesion(self):
        return self._cohesion

    @cohesion.setter
    def cohesion(self, value):
        self._cohesion = value
        self._cohesionAfterSoftening = value

    @property
    def cohesionAfterSoftening(self):
        return self._cohesionAfterSoftening

    @cohesionAfterSoftening.setter
    def cohesionAfterSoftening(self, value):
        if value:
            self._cohesionAfterSoftening = value
        else:
            self._cohesionAfterSoftening = self.cohesion

    @property
    def frictionCoefficient(self):
        return self._frictionCoefficient

    @frictionCoefficient.setter
    def frictionCoefficient(self, value):
        self._frictionCoefficient = value
        self._frictionAfterSoftening = value

    @property
    def frictionAfterSoftening(self):
        return self._frictionAfterSoftening

    @frictionAfterSoftening.setter
    def frictionAfterSoftening(self, value):
        if value:
            self._frictionAfterSoftening = value
        else:
            self._frictionAfterSoftening = self.frictionCoefficient

    def _frictionFn(self):
        if self.plasticStrain:
            friction = self.frictionWeakeningFn(
                self.plasticStrain,
                FrictionCoef=self.frictionCoefficient,
                FrictionCoefSw=self.frictionAfterSoftening,
                epsilon1=self.epsilon1,
                epsilon2=self.epsilon2)
        else:
            friction = fn.math.atan(nd(self.frictionCoefficient))
        return friction

    def _cohesionFn(self):
        if self.plasticStrain:
            cohesion = self.cohesionWeakeningFn(
                self.plasticStrain,
                Cohesion=nd(self.cohesion),
                CohesionSw=nd(self.cohesionAfterSoftening))
        else:
            cohesion = fn.misc.constant(self.cohesion)
        return cohesion

    def _get_yieldStress2D(self):
        f = self._frictionFn()
        C = self._cohesionFn()
        P = self.pressureField
        self.yieldStress = (C * fn.math.cos(f) + P * fn.math.sin(f))
        return self.yieldStress

    def _get_yieldStress3D(self):
        ## Drucker-Prgaer is calibrated to coincide with the triaxial compression
        ## Mohr Coulomb
        f = self._frictionFn()
        C = self._cohesionFn()
        P = self.pressureField
        self.yieldStress = 6.0 * C * fn.math.cos(f)
        self.yieldStress += 6.0 * fn.math.sin(f) * fn.misc.max(P, 0.0)
        self.yieldStress /= (fn.math.sqrt(3.0) * (3.0 - fn.math.sin(f)))
        return self.yieldStress


class VonMises(DruckerPrager):
    """ Von Mises Yield Criterion """

    def __init__(self, name=None, cohesion=None,
                 cohesionAfterSoftening=None,
                 epsilon1=0.0, epsilon2=0.2):

        super(VonMises, self).__init__(cohesion=cohesion,
                                       cohesionAfterSoftening=cohesionAfterSoftening,
                                       frictionCoefficient=0.,
                                       frictionAfterSoftening=0.,
                                       epsilon1=epsilon1, epsilon2=epsilon2)


class ConstantViscosity(Rheology):
    """The newtonian rheology Class."""

    def __init__(self, viscosity):
        """Newtonian Rheology.

        Parameters
        ----------
            Viscosity :
                newtonian viscosity of the material

        Returns
        -------

            An UWGeodynamics ConstantViscosity Class

        """
        super(ConstantViscosity, self).__init__()
        self.viscosity = viscosity
        self.name = "Constant ({0})".format(str(viscosity))

    @property
    def muEff(self):
        return self._effectiveViscosity()

    def _effectiveViscosity(self):
        return fn.Function.convert(nd(self.viscosity))


class ViscousCreep(Rheology):
    """ Viscous Creep Class """

    def __init__(self,
                 name=None,
                 preExponentialFactor=1.0,
                 stressExponent=1.0,
                 activationVolume=0.0,
                 activationEnergy=0.0,
                 waterFugacity=None,
                 grainSize=None,
                 meltFraction=None,
                 grainSizeExponent=0.0,
                 waterFugacityExponent=0.0,
                 meltFractionFactor=0.0,
                 f=1.0,
                 BurgersVectorLength=0.5e-9 * u.metre,
                 mineral="unspecified",
                 creep_type="unspecified"):
        """ Viscous Creep Rheology

        Deformation of materials on long timescale is predominantly achieved
        through viscous diffusion and dislocation creep. Those processes can be
        expressed using the following equation:

        .. math::

           \eta_{\text{eff}}^{vcreep} = \frac{1}{2}A^{\frac{-1}{n}}
           \dot{\epsilon}^{\frac{(1-n)}{n}}d^{\frac{m}{n}}\exp{\left(\frac{E + PV}{nRT}\right)}

        with `A` the prefactor, :math:`\dot{\epsilon}` the square root of the second invariant of the
        deviatoric strain rate tensor, `d` the grain size, `p` the grain size exponent, `E` the
        activation energy, `P` the pressure, `V` the activation volume, `n` the stress exponent,
        `R` the Gas Constant and `T` the temperature.

        Parameters
        ----------

            name : str
                Name of the Viscous Law
            preExponentialFactor :
                pre-exponential factor/ pre-factor
            stressExponent : float
                Stress exponent
            activationVolume :
                Activation Volume
            activationEnergy :
                Activation Energy
            waterFugacity :
                Water Fugacity
            grainSize :
                Grain Size
            meltFraction :
                Initial Fraction of Melt
            grainSizeExponent :
                Grain size eponent
            waterFugacityExponent :
                Water fugacity exponent
            meltFractionFactor :
                Melt Fraction
            f :
                Flow Law scaling factor
            BurgersVectorLength :
                Burger vector length
            mineral :
                Mineral associated to the flow law
            creep_type :
                "Dislocation, Diffusion"

        Returns
        -------

            An UWGeodynamics ViscousCreep Class.

        """

        super(ViscousCreep, self).__init__()

        self.name = name
        self.mineral = mineral
        self.creep_type = creep_type
        self.preExponentialFactor = preExponentialFactor
        self.stressExponent = stressExponent
        self.activationVolume = activationVolume
        self.activationEnergy = activationEnergy
        self.waterFugacity = waterFugacity
        self.grainSize = grainSize
        self.meltFraction = meltFraction
        self.grainSizeExponent = grainSizeExponent
        self.waterFugacityExponent = waterFugacityExponent
        self.meltFractionFactor = meltFractionFactor
        self.f = f
        self.constantGas = 8.3144621 * u.joule / u.mole / u.degK
        self.BurgersVectorLength = BurgersVectorLength

        self.onlinePDF = None
        self.citation = None

    def __mul__(self, other):
        self.f = other
        return self

    def __rmul__(self, other):
        self.f = other
        return self

    def __getitem__(self, name):
        return self.__dict__[name]

    def _repr_html_(self):
        attributes = OrderedDict()
        if isinstance(self.citation, str):
            attributes["Citation"] = self.citation
            if isinstance(self.onlinePDF, str):
                if self.onlinePDF.startswith("http"):
                    attributes["Citation"] = '<a href="{0}">{1}</a>'.format(
                        self.onlinePDF, self.citation)
        attributes["Mineral"] = self.mineral
        attributes["Pre-exponential factor"] = self.preExponentialFactor
        attributes["Stress Exponent"] = self.stressExponent
        attributes["Activation Volume"] = self.activationVolume
        attributes["Activation Energy"] = self.activationEnergy
        attributes["Factor"] = self.f
        attributes["Grain Size"] = self.grainSize
        attributes["Grain Size Exponent"] = self.grainSizeExponent
        attributes["Water Fugacity"] = self.waterFugacity
        attributes["Water Fugacity Exponent"] = self.waterFugacityExponent
        attributes["Melt Fraction"] = self.meltFraction
        attributes["Melt Fraction Factor"] = self.meltFractionFactor
        header = "<table>"
        footer = "</table>"
        html = """
        <tr>
            <th colspan="2" style="text-align:center;">{0}: {1}</th>
        </tr>""".format(self.creep_type, self.name)
        for key, val in attributes.items():
            if val is not None:
                html += '<tr><td style="text-align:left;width:20%;">{0}</td><td style="text-align:left;width:80%">{1}</td></tr>'.format(key, val)


        return header + html + footer

    @property
    def muEff(self):
        return self._effectiveViscosity()

    def _effectiveViscosity(self):
        """
        Return effective viscosity based on

        Power law creep equation
        viscosity = 0.5 * A^(-1/n) * edot_ii^((1-n)/n) * d^(m/n) * exp((E + P*V)/(nRT))

        Parameters:
        -----------

            A:
                prefactor
            I:
                square root of second invariant of deviatoric
                strain rate tensor,
            d:
                grain size,
            p:
                grain size exponent,
            E:
                activation energy,
            P:
                pressure,
            Va:
                activation volume,
            n:
                stress exponent,
            R:
                gas constant,
            T:
                temperature.
        """

        A = nd(self.preExponentialFactor)
        n = nd(self.stressExponent)
        P = self.pressureField
        T = self.temperatureField
        Q = nd(self.activationEnergy)
        Va = nd(self.activationVolume)
        p = nd(self.grainSizeExponent)
        d = nd(self.grainSize)
        r = nd(self.waterFugacityExponent)
        fH2O = nd(self.waterFugacity)
        I = self.strainRateInvariantField
        f = self.f
        #F = self.meltFraction
        #alpha = self.meltFractionFactor
        R = nd(self.constantGas)
        b = nd(self.BurgersVectorLength)

        mu_eff = f * 0.5 * A**(-1.0 / n)

        if np.abs(n - 1.0) > 1e-5:
            mu_eff *= I**((1.0 - n) / n)

        # Grain size dependency
        if p and d:
            mu_eff *= (d / b)**(p / n)

        # Water dependency
        if r and fH2O:
            mu_eff *= fH2O**(-r / n)

        #if F:
        #    mu_eff *= fn.math.exp(-1.0 * alpha * F / n)

        if T:
            mu_eff *= fn.math.exp((Q + P * Va) / (R * T * n))

        return mu_eff


class CompositeViscosity(Rheology):

    def __init__(self, viscosities):

        super(CompositeViscosity, self).__init__()

        if not isinstance(viscosities, (list, tuple)):
            raise ValueError('viscosities must be a list of viscosities')

        self.viscosities = viscosities

    @property
    def muEff(self):
        muEff = fn.misc.constant(0.0)
        for viscosity in self.viscosities:
            viscosity.pressureField = self.pressureField
            viscosity.strainRateInvariantField = self.strainRateInvariantField
            viscosity.temperatureField = self.temperatureField
            muEff += 1.0 / viscosity.muEff

        return 1.0 / muEff


class TemperatureAndDepthDependentViscosity(Rheology):

    def __init__(self, eta0, beta, gamma, reference, temperatureField=None):

        self._eta0 = nd(eta0)
        self._gamma = gamma
        self._beta = gamma
        self._reference = nd(reference)

    @property
    def muEff(self):
        coord = fn.input()
        return (self._eta0 * fn.math.exp(self._gamma *
                                         (coord[-1] - self._reference)))


class ViscousCreepRegistry(object):
    def __init__(self, filename=None):

        if not filename:
            import pkg_resources
            filename = pkg_resources.resource_filename(
                __name__, "ressources/ViscousRheologies.json")

        with open(filename, "r") as infile:
            self._viscousLaws = json.load(infile)

        for key in self._viscousLaws.keys():
            coefficients = self._viscousLaws[key]["coefficients"]
            for key2 in coefficients.keys():
                value = coefficients[key2]["value"]
                units = coefficients[key2]["units"]
                if units != "None":
                    coefficients[key2] = u.Quantity(value, units)
                else:
                    coefficients[key2] = value

        self._dir = {}
        for key in self._viscousLaws.keys():
            mineral = self._viscousLaws[key]["Mineral"]
            rh_type = self._viscousLaws[key]["Type"]
            name = key.replace(",", "").replace(".", "")
            name = [word.strip() for word in name.split()
                    if word.lower() not in ["viscous", "creep"]]
            name = "_".join(name)
            self._dir[name] = ViscousCreep(
                name=key, mineral=mineral, creep_type=rh_type,
                **self._viscousLaws[key]["coefficients"])

            try:
                self._dir[name].onlinePDF = self._viscousLaws[key]["onlinePDF"]
            except KeyError:
                pass

            try:
                self._dir[name].citation = self._viscousLaws[key]["citation"]
            except KeyError:
                pass

    def __dir__(self):
        # Make all the rheology available through autocompletion
        return list(self._dir.keys())

    def __getattr__(self, item):
        # Make sure to return a new instance of ViscousCreep
        return copy(self._dir[item])


class PlasticityRegistry(object):
    def __init__(self, filename=None):

        if not filename:
            import pkg_resources
            filename = pkg_resources.resource_filename(
                __name__, "ressources/PlasticRheologies.json")

        with open(filename, "r") as infile:
            _plasticLaws = json.load(infile)

        for key in _plasticLaws.keys():
            coefficients = _plasticLaws[key]["coefficients"]
            for key2 in coefficients.keys():
                value = coefficients[key2]["value"]
                units = coefficients[key2]["units"]
                if units != "None":
                    coefficients[key2] = u.Quantity(value, units)
                else:
                    coefficients[key2] = value

        self._dir = {}
        for key in _plasticLaws.keys():
            name = key.replace(" ", "_").replace(",", "").replace(".", "")
            name = name.replace(")", "").replace("(", "")
            self._dir[name] = DruckerPrager(
                name=key, **_plasticLaws[key]["coefficients"])

            try:
                self._dir[name].onlinePDF = _plasticLaws[key]["onlinePDF"]
            except KeyError:
                pass

            try:
                self._dir[name].citation = _plasticLaws[key]["citation"]
            except KeyError:
                pass

    def __dir__(self):
        # Make all the rheology available through autocompletion
        return list(self._dir.keys())

    def __getattr__(self, item):
        # Make sure to return a new instance of ViscousCreep
        return copy(self._dir[item])


class Elasticity(Rheology):

    def __init__(self, shear_modulus, observation_time):
        super(Elasticity, self).__init__()
        self.shear_modulus = shear_modulus
        self.observation_time = observation_time
        self.previousStress = None

    @property
    def muEff(self):
        return self._effectiveViscosity()

    def _effectiveViscosity(self):
        if not self.viscosity:
            raise ValueError("Can not find viscosity field")

        # Maxwell relaxation time
        alpha = self.viscosity / nd(self.shear_modulus)
        # observation time
        dt_e = nd(self.observation_time)
        # Calculate effective viscosity
        mu_eff = (self.viscosity * dt_e) / (alpha + dt_e)
        return mu_eff

    @property
    def elastic_stress(self):
        return self._elastic_stress()

    def _elastic_stress(self):
        # Check that the viscosity field has been properly
        # linked
        if not self.viscosity:
            raise ValueError("Can not find viscosity field")
        if not self.previousStress:
            raise ValueError("Can not find previous stress field")

        elasticStressFn = self.viscosity / (nd(self.shear_modulus) *
                                            nd(self.observation_time))
        elasticStressFn *= self.previousStress
        return elasticStressFn
