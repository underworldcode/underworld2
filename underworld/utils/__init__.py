##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##

"""
Various utility classes & functions.

"""

from ._utils import Integral
from ._utils import ProgressBar as _ProgressBar
from ._utils import SavedFileData
from ._utils import _createMeshName, _spacetimeschema, _fieldschema, _swarmspacetimeschema, _swarmvarschema, _xdmfheader, _xdmffooter, _nps_2norm
from ._utils import is_kernel
from ._SPR   import SuperconvergentPatchRecovery
from ._meshvariable_projection import MeshVariable_Projection, SolveLinearSystem
from . import _io