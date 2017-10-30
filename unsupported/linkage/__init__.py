__version__ = "0.1"

from underworld import rank
import warnings

if rank() == 0: 
    warnings.warn(
    """\n
    The 'linkage' module is not supported.\n
    It requires 'badlands' as a dependency.\n
    Questions should be addressed to romain.beucher@unimelb.edu.au \n """
    )

from linkage import *
from linkage2 import *
