##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##

"""
   Implementation relating to meshing.

"""

from ._mesh import FeMesh, FeMesh_Cartesian, FeMesh_IndexSet
from ._spherical_mesh import _FeMesh_Regional, FeMesh_SRegion, _FeMesh_Annulus,  FeMesh_Annulus
from ._spherical_mesh import  FeMesh_SphericalCap, FeMesh_Cylinder, FeMesh_CubedSphere, FeMesh_SquaredCircle
from ._meshvariable import MeshVariable
