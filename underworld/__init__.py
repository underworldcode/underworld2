##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
this is a docstring
"""

# ok, first need to change default python dlopen flags to global
# this is because when python imports the module, the shared libraries are loaded as RTLD_LOCAL
# and then when MPI_Init is called, OpenMPI tries to dlopen its plugin, they are unable to
# link to the openmpi libraries as they are private
import sys as _sys
import ctypes as _ctypes
_oldflags = _sys.getdlopenflags()
_sys.setdlopenflags( _oldflags | _ctypes.RTLD_GLOBAL )

import libUnderworld
import container
import mesh
import fevariable
import conditions
import function
import swarm
import systems
import utils

#import threading
#def go():
#    import urllib2
#    try:
#        print("Hottub time!!")
#        response = urllib2.urlopen('http://github.moresi.info')
#        html = response.read()
#        print("Hottub time!!")
#    except:
#        pass
#    return
#    #print("Nein!!")
#thread = threading.Thread( target=go )
#thread.start()
# ok, now set this back to the original value
_sys.setdlopenflags( _oldflags )

# lets go right ahead and init now.  user can re-init if necessary.
import _stgermain
_data =  libUnderworld.StGermain_Tools.StgInit( [] )

_stgermain.LoadModules( {"import":["StgDomain","StgFEM","PICellerator","Underworld","gLucifer","Solvers"]} )

def rank():
    """
    Returns the rank of the current processors.

    Parameters
    ----------
        None
    Returns:
        rank (unsigned) : Rank of current processor.
    """
    return _data.rank


def nProcs():
    """
    Returns the number of processors being utilised by the simulation.

    Parameters
    ----------
        None
    Returns:
        nProcs (unsigned) : Number of processors.
    """
    return _data.nProcs

# lets handle exceptions differently in parallel to ensure we call 
if nProcs() > 1:
    origexcepthook = _sys.excepthook
    def uw_uncaught_exception_handler(exctype, value, tb):
        print('\n###########################################################################################')
        print('###########################################################################################')
        print('An uncaught exception was encountered on processor {}.'.format(rank()))
        # pass through to original handler
        origexcepthook(exctype, value, tb)
        print('###########################################################################################')
        print('###########################################################################################')
        libUnderworld.StGermain_Tools.StgAbort( _data )
    _sys.excepthook = uw_uncaught_exception_handler

def _prepend_message_to_exception(e, message):
    """
    This function simply adds a message to an encountered exception. 
    Currently it is not python 3 friendly.  Check here
    http://stackoverflow.com/questions/6062576/adding-information-to-a-python-exception
    """
    raise type(e), type(e)(message + '\n' + e.message), _sys.exc_info()[2]














