##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
Underworld2 is a python-friendly version of the Underworld geodynamics
code which provides a programmable and flexible front end to all the
functionality of the code running in a parallel HPC environment. This
gives signficant advantages to the user, with access to the power of
python libraries for setup of complex problems, analysis at runtime,
problem steering, and coupling of multiple problems.

Underworld2 is integrated with the literate programming environment of
the jupyter notebook system for tutorials and as a teaching tool for
solid Earth geoscience.

Underworld is an open-source, particle-in-cell finite element code
tuned for large-scale geodynamics simulations. The numerical algorithms
allow the tracking of history information through the high-strain
deformation associated with fluid flow (for example, transport of the
stress tensor in a viscoelastic, convecting medium, or the advection of
fine-scale damage parameters by the large-scale flow). The finite
element mesh can be static or dynamic, but it is not contrained to move
in lock-step with the evolving geometry of the fluid. This hybrid approach
is very well suited to complex fluids which is how the solid Earth behaves
on a geological timescale.
"""

__version__ = "2.7.0-dev"

# lets  set sys.path such that the project parent directory takes
# precedence
import sys as _sys
import os as _os
_sys.path.insert(0, _os.path.realpath(_os.path.dirname("..")))

from . import timing
import libUnderworld
from . import _stgermain

def _set_init_sig_as_sig(mod):
    '''
    This method replaces all constructor signatures with those defined
    by the class' __init__ method.  This is necessary for StgCompountComponent
    subclasses (the only type this method applies to) due to the metaclassing
    which results in the __call__ method's signature (as defined in _SetupClass)
    being the one inspect.signature() returns (which is *not* what the user
    should see).
    '''
    import inspect
    # first gather info
    mods = {}
    for guy in dir(mod):
        if guy[0] != "_":  # don't grab private guys
            obj = getattr(mod,guy)
            if inspect.ismodule(obj):
                mods[guy] = obj
            elif inspect.isclass(obj):
                if issubclass(obj, _stgermain.StgCompoundComponent):
                    # replace signature here
                    obj.__signature__ = inspect.signature(obj.__init__)
    for key in mods:
        # recurse into submodules
        _set_init_sig_as_sig(getattr(mod,key))


import underworld.container
_set_init_sig_as_sig(underworld.container)
timing._add_timing_to_mod(container)

import underworld.mesh
_set_init_sig_as_sig(underworld.mesh)
timing._add_timing_to_mod(underworld.mesh)

import underworld.conditions
_set_init_sig_as_sig(underworld.conditions)
timing._add_timing_to_mod(underworld.conditions)

import underworld.function
_set_init_sig_as_sig(underworld.function)
timing._add_timing_to_mod(underworld.function)

import underworld.swarm
_set_init_sig_as_sig(underworld.swarm)
timing._add_timing_to_mod(underworld.swarm)

import underworld.systems
_set_init_sig_as_sig(underworld.systems)
timing._add_timing_to_mod(underworld.systems)

import underworld.utils
_set_init_sig_as_sig(underworld.utils)
timing._add_timing_to_mod(underworld.utils)

# to allow our legacy doctest formats
try:
    import numpy as _np
    _np.set_printoptions(legacy='1.13')
except:
    pass

# Squelch these warnings as they are very noisey and not necessary
# https://stackoverflow.com/questions/40845304/runtimewarning-numpy-dtype-size-changed-may-indicate-binary-incompatibility
import warnings
warnings.filterwarnings("ignore", message="numpy.dtype size changed")
warnings.filterwarnings("ignore", message="numpy.ufunc size changed")
try:
    from ._uwid import uwid as _id
except:
    import uuid as _uuid
    _id = str(_uuid.uuid4())
from . import _net

# lets go right ahead and init now.  user can re-init if necessary.
_data =  libUnderworld.StGermain_Tools.StgInit( _sys.argv )

_stgermain.LoadModules( {"import":["StgDomain","StgFEM","PICellerator","Underworld","gLucifer","Solvers"]} )

def rank():
    """
    Returns the rank of the current process.

    Returns
    -------
    unsigned
        Rank of current process.
    """
    return _data.rank


def nProcs():
    """
    Returns the number of processes being utilised by the simulation.

    Returns
    -------
    unsigned
        Number of processors.
    """
    return _data.nProcs

def barrier():
    """
    Creates an MPI barrier. All processes wait here for others to catch up.

    """
    from mpi4py import MPI
    MPI.COMM_WORLD.Barrier()

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

# lets handle exceptions differently in parallel to ensure we call.
# add isinstance so that this acts correctly for Mocked classes used in sphinx docs generation
if isinstance(nProcs(), int) and nProcs() > 1:
    _origexcepthook = _sys.excepthook
    def _uw_uncaught_exception_handler(exctype, value, tb):
        print('An uncaught exception was encountered on processor {}.'.format(rank()))
        # pass through to original handler
        _origexcepthook(exctype, value, tb)
        libUnderworld.StGermain_Tools.StgAbort( _data )
    _sys.excepthook = _uw_uncaught_exception_handler

def _prepend_message_to_exception(e, message):
    """
    This function simply adds a message to an encountered exception.
    Currently it is not python 3 friendly.  Check here
    http://stackoverflow.com/questions/6062576/adding-information-to-a-python-exception
    """
#    raise type(e), type(e)(message + '\n' + e.message), _sys.exc_info()[2]
    raise          type(e)(message + '\n' + e.message).with_traceback(_sys.exc_info()[2])


# comment this out for now, as tear down in py3 is cleaner without it.
#
#class _del_uw_class:
#    """
#    This simple class simply facilitates calling StgFinalise on uw exit
#    Previous implementations used the 'atexit' module, but this called finalise
#    *before* python garbage collection which as problematic as objects were being
#    deleted after StgFinalise was called.
#    """
#    def __init__(self,delfunc, deldata):
#        self.delfunc = delfunc
#        self.deldata = deldata
#    def __del__(self):
#        # put this in a try loop to avoid errors during sphinx documentation generation
#        try:
#            self.delfunc(self.deldata)
#        except:
#            pass
#
#_delclassinstance = _del_uw_class(libUnderworld.StGermain_Tools.StgFinalise, _data)

def _in_doctest():
    """
    Returns true if running inside doctests run from docs/tests/doctest.py
    """
    import os
    return hasattr(_sys.modules['__main__'], '_SpoofOut') or "DOCTEST" in os.environ

# lets shoot off some usage metrics
# send metrics *only* if we are rank=0, and if we are not running inside a doctest.
if (rank() == 0) and not _in_doctest():
    def _sendData():
        import os
        # disable collection of data if requested
        if "UW_NO_USAGE_METRICS" not in os.environ:
            # get platform info
            import platform
            label  =        platform.system()
            label += "__" + platform.release()
            # check if docker
            import os.path
            if (os.path.isfile("/.dockerinit")):
                label += "__docker"

            # send info async
            import threading
            thread = threading.Thread( target=_net.PostGAEvent, args=( "runtime", "import", label, nProcs() ) )
            thread.daemon = True
            thread.start()

    try:
        _sendData()
    except: # continue quietly if something above failed
        pass

# lets also enable deprecation warnings
#import warnings as _warnings
#_warnings.simplefilter('always', DeprecationWarning)

# enable the following to force exceptions on warnings
#_warnings.simplefilter('error')
