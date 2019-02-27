##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##

"""
The glucifer module provides visualisation algorithms for Underworld. 

Visualisation data is generated in parallel, with each processes generating the 
necessary data for its part of the domain. This data is written into a data file. 

Actual rendering is performed in serial using the LavaVu rendering engine.

glucifer provides many flexible rendering options, including client-server based
operation for remote usage. Users may choose to renderer outputs to raster images, 
or save a database file for later rendering. For those working in the Jupyter 
environment, glucifer will inline rendered images or even interactive webgl 
frames (still experimental).   

"""
import underworld as _uw
from . import _glucifer
_uw.timing._add_timing_to_mod(_glucifer)
from ._glucifer import Store
_uw._set_init_sig_as_sig(Store)

from ._glucifer import Figure
_uw._set_init_sig_as_sig(Figure)

from ._glucifer import Viewer
_uw._set_init_sig_as_sig(Viewer)

from . import objects
_uw._set_init_sig_as_sig(objects)
_uw.timing._add_timing_to_mod(objects)

try:
    from ._glucifer import lavavu
except:
    pass

_display = None

class _xvfb_runner(object):
    """
    This class will initialise the X virtual framebuffer (Xvfb).
    Xvfb is useful on headless systems. Note that xvfb will need to be 
    installed, as will pyvirtualdisplay.

    This class also manages the lifetime of the virtual display driver. When
    the object is garbage collected, the driver is stopped.
    """
    def __init__(self):
        from pyvirtualdisplay import Display
        self._xvfb = Display(visible=0, size=(1600, 1200))
        self._xvfb.start()

    def __del__(self):
        if not self._xvfb is None :
            self._xvfb.stop()

import os as _os
# disable collection of data if requested
if "GLUCIFER_USE_XVFB" in _os.environ:
    if _uw.mpi.rank == 0:
        _display = _xvfb_runner()
