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
import underworld.function
import libUnderworld
import libUnderworld.libUnderworldPy.Function as _cfn
from timeit import default_timer as timer
from mpi4py import MPI
import h5py
import numpy as np
import sys
import shutil
import os


class SuperconvergentPatchRecovery(object):
    """ SPR in the style of Zienkiewich & Zhu 1987, 1992, ...

        An SPR object is specific to a mesh because it requires
        knowing the element type and builing the relevant points.

        Note. this relies on fitting functions to local Superconvergent
        points which is different from REPself.

        Note 2. We use cKDTree to find nearby superconergent points and this
        is not quite the same as the standard SPR near boundaries. The recipe
        actually calls for all points contained in the element of a given node
        (modified for boundaries). This seems to be OK though.
    """

    def __init__(self, mesh):

        from scipy.spatial import cKDTree

        # Need error checking for these

        self.mesh = mesh

        if "Q1" in mesh.elementType:
            self._gaussPts = 1
            self._stencil_size=2**mesh.dim
            if mesh.dim == 2:
                self._fit_function = self._linear_fit_2D
            else:
                self._fit_function = self._linear_fit_3D

        else: # "Q2" in mesh.elementType:
            self._gaussPts = 2
            self._stencil_size=4**mesh.dim
            if mesh.dim == 2:
                self._fit_function = self._quadratic_fit_2D
            else:
                self._fit_function = self._quadratic_fit_3D


        # build the swarm

        self._gswarm = uw.swarm.Swarm(mesh, particleEscape=True)
        layout = uw.swarm.layouts.PerCellGaussLayout(self._gswarm, gaussPointCount=self._gaussPts)
        self._gswarm.populate_using_layout(layout)
        self._swarmGradientVar = self._gswarm.add_variable(dataType="double", count=1)

        self._gs_tree = cKDTree(self._gswarm.particleCoordinates.data)

        return


    def _linear_fit_2D(self, X, a, b, c, d):
        # (1+x) * (1+y) etc
        x,y = X
        fit = a + b * x + c * y + d * x * y
        return fit


    def _linear_fit_3D(self, X, a, b, c, d, e, f, g, h):
        # (1+x) * (1+y) * (1+z) etc
        x,y,z = X
        fit = a + b * x + c * y + d * z + e * x * y + f * x * z + g * y * z + h * x * y * z
        return fit

    def _quadratic_fit_2D(self, X, a, b, c, d, e, f):
        # (1+x+x**2)*(1+y+y**2)
        x,y = X
        fit = a + b * x + c * x**2 + d * y + e * y**2  + f * x * y
        return fit

    def _quadratic_fit_3D(self, X, a, b, c, d, e, f, g, h, i, j):
        # (1+x+x**2)*(1+y+y**2)*(1+y+y**2)
        x,y = X
        fit = a + b * x + c * x**2 + d * y + e * y**2  + f * z + g * z**2 + h * x * y + i * x * z + j * y * z
        return fit

    ## Z + Z recommend also including higher order terms from the tensor product (all possible quadradic terms)

    # def _quadratic_fit_full_2D(X, a, b, c, d, e, f, g, h, i):
    #     # (1+x+x**2)*(1+y+y**2)
    #     x,y = X
    #     fit = a + b * x + c * x**2 + d * y + e * x * y + f * x**2 * y + g * y**2 + h * y**2 * x + i * x**2 * y**2
    #     return fit

    def evaluate(self, GradVariable=None, nodesList=None):

        from scipy.optimize import curve_fit

        GstarSwarm = self._swarmGradientVar
        GstarNode = self.mesh.add_variable(nodeDofCount=1)

        GstarNode.data[:] = 0.0

        # List of nodes where the SPR is to be applied ... default to ALL if no list is given
        if nodesList is None:
            # Technically this would still work ... but ...
            nodesList = range(0, self.mesh.nodesLocal)

        for node in nodesList:
            try:
                d, patch_particles = self._gs_tree.query(self.mesh.data[node], k=self._stencil_size)
                X = self._gswarm.particleCoordinates.data[patch_particles].T
                data = GradVariable.evaluate(self._gswarm.particleCoordinates.data[patch_particles]).reshape(-1)
                popt, pcov = curve_fit(self._fit_function, X, data)
                GstarNode.data[node] = self._fit_function( self.mesh.data[node], *popt )

            except:
                GstarNode.data[node] = 0.0


        return GstarNode
