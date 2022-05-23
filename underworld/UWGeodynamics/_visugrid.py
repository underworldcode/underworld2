from __future__ import print_function,  absolute_import
import underworld as uw
from underworld.scaling import non_dimensionalise as nd
import numpy as np
from scipy import spatial


class Visugrid(object):

    def __init__(self, Model, elementRes, minCoord, maxCoord, velocityField):

        self.minCoord = minCoord
        self.maxCoord = maxCoord
        self.elementRes = elementRes
        self.velocityField = velocityField

        minCoord = tuple([nd(val) for val in self.minCoord])
        maxCoord = tuple([nd(val) for val in self.maxCoord])

        self.mesh = uw.mesh.FeMesh_Cartesian(elementType="Q1/dQ0",
                                             elementRes=self.elementRes,
                                             minCoord=minCoord,
                                             maxCoord=maxCoord)

        boundaryNodes = (Model.left_wall + Model.right_wall +
                         Model.top_wall + Model.bottom_wall)

        self.Model = Model

        # Build a KDTree to handle boundaries
        self.boundaries = boundaryNodes.data
        x = Model.mesh.data[self.boundaries, 0]
        y = Model.mesh.data[self.boundaries, 1]
        coords = np.zeros((x.size, 2))
        coords[:, 0] = x.ravel()
        coords[:, 1] = y.ravel()
        self.tree = spatial.cKDTree(coords)

    def advect(self, dt):

        with self.mesh.deform_mesh():
            for index, coords in enumerate(self.mesh.data):
                try:
                    self.mesh.data[index] += self.velocityField.evaluate(np.array([coords]))[0] * dt
                except:
                    _, loc = self.tree.query(self.mesh.data[index])
                    loc = self.boundaries[loc]
                    coords = self.Model.mesh.data[loc]
                    self.mesh.data[index] += self.velocityField.data[loc] * dt



