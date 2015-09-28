##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld as uw
import underworld._stgermain as _stgermain
import underworld.mesh as mesh
import weakref
from libUnderworld import *
from underworld.swarm._iswarm import *
from underworld.swarm._swarm import *

mesh=uw.mesh.FeMesh_Cartesian("linear", minCoord=(0,0), maxCoord=(3,3), elementRes=(35,35))
genswarm=Swarm(mesh)
layout=uw.swarm.layouts.GlobalSpaceFillerLayout(genswarm, 37)
genswarm.PopulateUsingLayout(layout)
genswarm.particleLocalCount

weights = uw.swarm._weights.PCDVC(genswarm)
iswarm=IntSwarm(mesh, genswarm, weights)
