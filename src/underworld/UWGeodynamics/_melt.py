from __future__ import print_function,  absolute_import
from underworld.scaling import non_dimensionalise as nd
from underworld.scaling import units as u
from underworld.scaling import dimensionalise
import json
from copy import copy


class _Polynom(object):

    def __init__(self, A1, A2=0., A3=0., A4=0.):

        self.A1 = A1
        self.A2 = A2
        self.A3 = A3
        self.A4 = A4

    def temperature(self, pressure):
        T = nd(self.A1)
        T += nd(self.A2) * pressure
        T += nd(self.A3) * pressure**2
        T += nd(self.A4) * pressure**3
        return T

    def plot(self, pressure):
        import pylab as plt
        temperature = dimensionalise(self.temperature(pressure), u.kelvin).m
        pressure = dimensionalise(pressure, u.pascal).m
        plt.plot(temperature, pressure)
        plt.gca().invert_yaxis()
        plt.show()


class Solidus(_Polynom):
    """ This class defines a solidus using the
    form suggested by Hirshmann, 2000"""

    def __init__(self, A1, A2=0., A3=0., A4=0.):
        super(Solidus, self).__init__(A1, A2, A3, A4)


class Liquidus(_Polynom):
    """ This class defines a liquidus using the
    form suggested by Hirshmann, 2000"""

    def __init__(self, A1, A2=0., A3=0., A4=0.):
        super(Liquidus, self).__init__(A1, A2, A3, A4)


class SolidusRegistry(object):
    """SolidusRegistry Class"""
    def __init__(self, filename=None):
        """Create a regustry of Solidus polynomials

        Parameters
        ----------

        filename : Name of the json file database

        Returns
        -------
        """

        if not filename:
            import pkg_resources
            filename = pkg_resources.resource_filename(
                __name__, "resources/Solidus.json")

        with open(filename, "r") as infile:
            _solidii = json.load(infile)

        for key in _solidii.keys():
            coefficients = _solidii[key]["coefficients"]
            for key2 in coefficients.keys():
                value = coefficients[key2]["value"]
                units = coefficients[key2]["units"]
                if units != "None":
                    coefficients[key2] = u.Quantity(value, units)
                else:
                    coefficients[key2] = value

        self._dir = {}
        for key in _solidii.keys():
            name = key.replace(" ", "_").replace(",", "").replace(".", "")
            name = name.replace(")", "").replace("(", "")
            self._dir[name] = Solidus(**_solidii[key]["coefficients"])

    def __dir__(self):
        # Make all the rheology available through autocompletion
        return list(self._dir.keys())

    def __getattr__(self, item):
        # Make sure to return a new instance of ViscousCreep
        return copy(self._dir[item])


class LiquidusRegistry(object):
    """LiquidusRegistry class"""
    def __init__(self, filename=None):
        """Create a regustry of Liquidus polynomials

        Parameters
        ----------

        filename : Name of the json file database

        Returns
        -------
        """

        if not filename:
            import pkg_resources
            filename = pkg_resources.resource_filename(
                __name__, "resources/Liquidus.json")

        with open(filename, "r") as infile:
            _liquidii = json.load(infile)

        for key in _liquidii.keys():
            coefficients = _liquidii[key]["coefficients"]
            for key2 in coefficients.keys():
                value = coefficients[key2]["value"]
                units = coefficients[key2]["units"]
                if units != "None":
                    coefficients[key2] = u.Quantity(value, units)
                else:
                    coefficients[key2] = value

        self._dir = {}
        for key in _liquidii.keys():
            name = key.replace(" ", "_").replace(",", "").replace(".", "")
            name = name.replace(")", "").replace("(", "")
            self._dir[name] = Liquidus(**_liquidii[key]["coefficients"])

    def __dir__(self):
        # Make all the rheology available through autocompletion
        return list(self._dir.keys())

    def __getattr__(self, item):
        # Make sure to return a new instance of ViscousCreep
        return copy(self._dir[item])
