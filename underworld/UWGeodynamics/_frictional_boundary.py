from __future__ import print_function,  absolute_import
import numpy as np
import underworld as uw
import underworld.function as fn


class FrictionBoundaries(object):
    """ This class flags elements at the boundaries

    """
    def __init__(self, Model, rightFriction=None, leftFriction=None,
                 topFriction=None, bottomFriction=None, frontFriction=None,
                 backFriction=None, thickness=2):
        """Frictional boundaries processor_

        Parameters
        ----------

        Model : UWGeodynamics Model
        rightFriction : coefficient of friction on the right wall
                        (tangent of the friction angle in radians)

        leftFriction : coefficient of friction on the left wall
                        (tangent of the friction angle in radians)

        topFriction : coefficient of friction on the top wall
                        (tangent of the friction angle in radians)

        bottomFriction : coefficient of friction on the bottom wall
                        (tangent of the friction angle in radians)

        frontFriction : coefficient of friction on the front wall
                        (tangent of the friction angle in radians)

        backFriction : coefficient of friction on the back wall
                        (tangent of the friction angle in radians)

        thickness : Thickness of the friction layer.

        Returns
        -------
        """

        self.Model = Model
        self.thickness = thickness

        # Build borders
        globalIndices = np.arange(np.prod(Model.mesh.elementRes))
        globalIndices = globalIndices.reshape((Model.mesh.elementRes[::-1]))

        self.bottomFriction = bottomFriction
        self.rightFriction = rightFriction
        self.leftFriction = leftFriction
        self.topFriction = topFriction
        self.frontFriction = frontFriction
        self.backFriction = backFriction

        self.subMesh = Model.mesh.subMesh
        conditions = list()

        self._mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
        self._mask.data[:] = 0

        if self.rightFriction:
            self._right_mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
            self._right_mask.data[:] = 0

            if Model.mesh.dim < 3:
                right = globalIndices[:, -thickness:].ravel()
            else:
                right = globalIndices[:, :, -thickness:].ravel()

            intersect = np.intersect1d(self.subMesh.data_nodegId.ravel(), right)
            mask = np.in1d(self.subMesh.data_nodegId.ravel(), intersect)
            self._right_mask.data[mask, 0] = 1
            self._mask.data[mask, 0] = 1
            conditions.append((self._right_mask > 0., self.rightFriction))

        if self.leftFriction:
            self._left_mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
            self._left_mask.data[:] = 0

            if Model.mesh.dim < 3:
                left = globalIndices[:, :thickness].ravel()
            else:
                left = globalIndices[:, :, :thickness].ravel()

            intersect = np.intersect1d(self.subMesh.data_nodegId.ravel(), left)
            mask = np.in1d(self.subMesh.data_nodegId.ravel(), intersect)
            self._left_mask.data[mask, 0] = 1
            self._mask.data[mask, 0] = 1
            conditions.append((self._left_mask > 0., self.leftFriction))

        if self.frontFriction:
            self._front_mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
            self._front_mask.data[:] = 0

            if Model.mesh.dim > 2:
                front = globalIndices[:, :thickness, :].ravel()
            else:
                raise ValueError("Mesh is 2D")

            intersect = np.intersect1d(self.subMesh.data_nodegId.ravel(), front)
            mask = np.in1d(self.subMesh.data_nodegId.ravel(), intersect)
            self._front_mask.data[mask, 0] = 1
            self._mask.data[mask, 0] = 1
            conditions.append((self._front_mask > 0., self.frontFriction))

        if self.backFriction:
            self._back_mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
            self._back_mask.data[:] = 0

            if Model.mesh.dim > 2:
                back = globalIndices[:, -thickness:, :].ravel()
            else:
                raise ValueError("Mesh is 2D")

            intersect = np.intersect1d(self.subMesh.data_nodegId.ravel(), back)
            mask = np.in1d(self.subMesh.data_nodegId.ravel(), intersect)
            self._back_mask.data[mask, 0] = 1
            self._mask.data[mask, 0] = 1
            conditions.append((self._back_mask > 0., self.backFriction))

        if self.bottomFriction:
            self._bottom_mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
            self._bottom_mask.data[:] = 0

            bottom = globalIndices[:thickness].ravel()
            # Take the intersection between the globalID and the boundaries where
            # friction is to be applied
            intersect = np.intersect1d(self.subMesh.data_nodegId.ravel(), bottom)
            # Create a mask to highlight those elements in the local domain
            mask = np.in1d(self.subMesh.data_nodegId.ravel(), intersect)
            self._bottom_mask.data[mask, 0] = 1
            self._mask.data[mask, 0] = 1
            conditions.append((self._bottom_mask > 0., self.bottomFriction))

        if self.topFriction:
            self._top_mask = uw.mesh.MeshVariable(mesh=self.subMesh, nodeDofCount=1)
            self._top_mask.data[:] = 0

            top = globalIndices[-thickness:].ravel()
            intersect = np.intersect1d(self.subMesh.data_nodegId.ravel(), top)
            mask = np.in1d(self.subMesh.data_nodegId.ravel(), intersect)
            self._top_mask.data[mask, 0] = 1
            self._mask.data[mask, 0] = 1
            conditions.append((self._top_mask > 0., self.topFriction))

        conditions.append((True, -1.0))
        self.friction = fn.branching.conditional(conditions)
