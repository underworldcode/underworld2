from __future__ import print_function,  absolute_import
from ._utils import TransformedDict
from ._utils import u

COEFFICIENTS = TransformedDict()
COEFFICIENTS["[length]"] = 1.0 * u.meter
COEFFICIENTS["[mass]"] = 1.0 * u.kilogram 
COEFFICIENTS["[time]"] = 1.0 * u.year
COEFFICIENTS["[temperature]"] = 1.0 * u.degK
COEFFICIENTS["[substance]"] = 1.0 * u.mole
