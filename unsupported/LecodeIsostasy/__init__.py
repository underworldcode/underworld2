__version__ = "0.1"

from underworld import rank
import warnings

if rank() == 0:
    warnings.warn(
    """\n
    The LecodeIsostasy module is not supported.\n
    Questions should be addressed to romain.beucher@unimelb.edu.au \n """
    )

from LecodeIsostasy import LecodeIsostasy

