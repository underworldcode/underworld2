from __future__ import print_function,  absolute_import
import underworld.function as fn
from underworld import non_dimensionalise as nd
from underworld import UnitRegistry as u


class Density(object):

    def __init__(self):
        self.temperatureField = None
        self.pressureField = None
        self.name = None


class ConstantDensity(Density):

    def __init__(self, reference_density):
        """Constant density function

        Parameters
        ----------

        reference_density : density

        Returns
        -------

        An UWGeodynamics Constant Density object
        """
        self.reference_density = reference_density
        self._density = nd(reference_density)
        self.name = "Constant ({0})".format(str(reference_density))

    def effective_density(self):
        return fn.Function.convert(self._density)


class LinearDensity(Density):

    def __init__(self, reference_density, thermalExpansivity=3e-5 / u.kelvin,
                 reference_temperature=273.15 * u.degK, beta=0. / u.pascal,
                 reference_pressure=0. * u.pascal):
        """ The LinearDensity function calculates:
            density = rho0 * (1 + (beta * deltaP) - (alpha * deltaT))
            where deltaP is the difference between P and the reference P,
            and deltaT is the difference between T and the reference T

        Parameters
        ----------

        reference_density : reference density
        thermalExpansivity : thermal expansivity of the material at the
                             temperature of reference.
        reference_temperature : reference temperature
        beta : coefficient of compressibility
        reference_pressure : reference pressure

        Returns
        -------

        An UWGeodynamics Linear Density object.
        """

        super(LinearDensity, self).__init__()

        self.name = "Linear (ref: {0})".format(str(reference_density))
        self.reference_density = reference_density
        self.reference_temperature = reference_temperature
        self.thermalExpansivity = thermalExpansivity
        self.reference_pressure = reference_pressure
        self._alpha = nd(thermalExpansivity)
        self._beta = nd(beta)
        self._Tref = nd(reference_temperature)
        self._Pref = nd(reference_pressure)

    def effective_density(self):
        """calculate effective_density based
           on PT conditions"""

        density = nd(self.reference_density)

        # Temperature dependency
        if not self.temperatureField:
            raise RuntimeError("No temperatureField found!")

        t_term = self._alpha * (self.temperatureField - self._Tref)

        # Pressure dependency
        if not self.pressureField:
            raise RuntimeError("No pressureField found!")

        p_term = self._beta * (self.pressureField - self._Pref)

        return density * (1.0 + p_term - t_term)

