import underworld as uw
import scaling as sca
from scaling import nonDimensionalize as nd
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

        boundaryNodes = (Model.leftWall + Model.rightWall +
                         Model.topWall + Model.bottomWall)
       
        self.Model = Model
        # Build a KDTree to handle boundaries
        self.boundaries = boundaryNodes.data
        x = Model.mesh.data[self.boundaries,0]
        y = Model.mesh.data[self.boundaries,1]
        self.tree = spatial.KDTree(zip(x.ravel(), y.ravel()))

    def advect(self, dt):

        with self.mesh.deform_mesh():
            for index, coords in enumerate(self.mesh.data):
                try:
                    self.mesh.data[index] += self.velocityField.evaluate(np.array([coords]))[0] * dt
                except:
                    d, loc = self.tree.query(self.mesh.data[index])
                    loc = self.boundaries[loc]
                    coords = self.Model.mesh.data[loc]
                    self.mesh.data[index] += self.velocityField.data[loc] * dt

