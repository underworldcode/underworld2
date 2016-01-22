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
from _glucifer import Figure
from . import objects