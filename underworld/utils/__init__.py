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
from ._meshvariable_projection import MeshVariable_Projection, SolveLinearSystem
from . import _io

def _run_from_ipython():
    """
    Small routine to check if running from ipy/jupyter.s
    """
    try:
        __IPYTHON__
        return True
    except NameError:
        return False

def matplotlib_inline():
    """
    This function simply enables Jupyter Notebook inlined matplotlib results.
    This function should be called at the start of your notebooks as a 
    replacement for the Jupyter Notebook *%matplotlib inline* magic. It provides
    the same functionality, however it allows notebooks to be converted to
    python without having to explicitly remove these calls.
    """
    if _run_from_ipython():
        get_ipython().magic(u'matplotlib inline')
