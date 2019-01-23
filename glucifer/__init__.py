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
import _glucifer
_uw.timing._add_timing_to_mod(_glucifer)
from _glucifer import Store
from _glucifer import Figure
from _glucifer import Viewer
from . import objects
_uw.timing._add_timing_to_mod(objects)

try:
    from _glucifer import lavavu
except:
    pass

_display = None

import os as _os
# disable collection of data if requested
if "GLUCIFER_USE_XVFB" in _os.environ:
    if _uw.rank() == 0:
        from pyvirtualdisplay import Display
        _display = Display(visible=0, size=(1600, 1200))
        _display.start()
        import atexit
        def all_done():
            _display.stop()
        atexit.register(all_done)
