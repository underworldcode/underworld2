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
import _glucifer
from _glucifer import Store
from _glucifer import Figure
from _glucifer import Viewer
from _glucifer import lavavu
from . import objects

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
    _display = _xvfb_runner()
