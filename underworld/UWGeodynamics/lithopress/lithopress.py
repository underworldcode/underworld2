from __future__ import print_function,  absolute_import

import underworld as uw
import underworld.function as fn
import numpy as np
from mpi4py import MPI

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

supported_elem_mesh = ["Q1", "Q2"]
supported_elem_subMesh = ["DQ1", "DQ0", "DPC1"]


class Lithostatic_pressure(fn.Function):
    """Class that calculates the lithostatic pressure field based on
    material densities.

    inputs

    mesh: regular mesh
    densityFn: underworld function, shall return material densities
    gravity

    outputs:

    tuple: lithostatic pressure field, pressure at the bottom of the
    model
    """

    def __init__(self, mesh, densityFn, gravity):

        self.mesh = mesh
        self._densityFn = fn.Function.convert(densityFn)
        self.gravity = fn.Function.convert(gravity)

        self.lithostatic_field_nodes = uw.mesh.MeshVariable(self.mesh, nodeDofCount=1)
        self.lithostatic_field = uw.mesh.MeshVariable(self.mesh.subMesh, nodeDofCount=1)
        self.Cell2Nodes = uw.utils.MeshVariable_Projection(
            self.lithostatic_field_nodes, self.lithostatic_field, type=0)
        # Create Utilities
        self.DensityVar = uw.mesh.MeshVariable(self.mesh, nodeDofCount=1)
        self.projectorDensity = uw.utils.MeshVariable_Projection(self.DensityVar, self._densityFn, type=0 )

        if not self.mesh.elementType.upper() in supported_elem_mesh:
            raise ValueError("Unsupported element: {0}".format(self.mesh.elementType))

        if not self.mesh.subMesh.elementType.upper() in supported_elem_subMesh:
            raise ValueError("Unsupported element: {0}".format(self.mesh.subMesh.elementType))

        super(Lithostatic_pressure, self).__init__(
            argument_fns=[self._densityFn,
                          self.gravity])
        # 2D case
        if self.mesh.dim == 2:
            self._fn = self._lithoPressure2D()
        # 3D case
        if self.mesh.dim == 3:
            self._fn = self._lithoPressure3D()

        self._fncself = self._fn._fncself

    def _lithoPressure2D(self):

        self.projectorDensity.solve()

        # Get Dimension of the global domain
        ncol, nrow = self.mesh.elementRes
        if self.mesh.elementType == "Q2":
            fact = 2
        elif self.mesh.elementType == "Q1":
            fact = 1

        ncol *= fact
        nrow *= fact

        # Create some work arrays.
        local_y = np.zeros((nrow + 1, ncol + 1))
        global_y = np.zeros((nrow + 1, ncol + 1))
        local_density = np.zeros((nrow + 1, ncol + 1))
        global_density = np.zeros((nrow + 1, ncol + 1))
        local_pressure = np.zeros((nrow, ncol))

        # Now we need to get the dimension as well as
        # the location of the local domain
        # in the global domain.

        # Get the global ids, note that we must get rid of the shadow nodes
        all_nodes = self.mesh.subMesh.data_nodegId  # local + shadow
        node_gids = self.mesh.data_nodegId[:self.mesh.nodesLocal]

        # Get an I,J representation of the node coordinates
        Jpositions = (node_gids.astype("float") / (ncol + 1)).astype("int")
        Ipositions = node_gids - Jpositions * (ncol + 1)

        # Get local domain data
        local_y[Jpositions, Ipositions] = self.mesh.data[:self.mesh.nodesLocal, 1]
        local_density[Jpositions, Ipositions] = self.DensityVar.data[:self.mesh.nodesLocal, 0]

        comm.Allreduce(local_y, global_y)
        comm.Allreduce(local_density, global_density)

        # Remember that the nodes coordinates start from the bottom left so that the first
        # row in the numpy array is actually the bottom of the mesh.
        # Top Left - Bottom Left
        dyleft = np.abs(global_y[1:, :-1] - global_y[:-1, :-1])
        # Top Right - Bottom Right
        dyright = np.abs(global_y[1:, 1:] - global_y[:-1, 1:])
        dy = (dyleft + dyright) / 2.

        # Calculate pressure from the top half of the element.
        # (Takes the average density of the 2 top nodes)
        EpressureTop = self.gravity.value * dy / 2.0 * (global_density[1:, :-1] + global_density[1:, 1:]) / 2.0
        # Calculate pressure from the bottom half of the element.
        # (Takes the average density of the 2 bottom nodes)
        EpressureBot = self.gravity.value * dy / 2.0 * (global_density[:-1, :-1] + global_density[:-1, 1:]) / 2.0

        # Add pressure from element above except at the top.
        pressure = np.copy(EpressureTop)
        pressure[:-1, :] += EpressureBot[1:, :]
        residual = EpressureBot[0, :]

        # Flip the array upside down, do a cumul sum, flip it back.
        Tpressure = np.cumsum(pressure[::-1, :], axis=0)[::-1, :]
        bottom = Tpressure[0, :] + residual

        local_pressure = Tpressure.flatten()[all_nodes]

        self.lithostatic_field.data[:, 0] = local_pressure
        self.lithostatic_field.syncronise()

        self.Cell2Nodes.solve()

        return self.lithostatic_field_nodes

    def _lithoPressure3D(self):

        self.projectorDensity.solve()

        # Get Dimension of the global domain
        nx, ny, nz = self.mesh.elementRes
        if self.mesh.elementType == "Q2":
            fact = 2
        elif self.mesh.elementType == "Q1":
            fact = 1

        nx *= fact
        ny *= fact
        nz *= fact

        # Create some work arrays.
        local_z  = np.zeros((nz + 1, ny + 1, nx + 1))
        global_z = np.zeros((nz + 1, ny + 1, nx + 1))
        local_density  = np.zeros((nz + 1, ny + 1, nx + 1))
        global_density = np.zeros((nz + 1, ny + 1, nx + 1))
        local_pressure  = np.zeros((nz, ny, nx))

        # Get the global ids, note that we must get rid of the shadow nodes
        all_nodes = self.mesh.subMesh.data_nodegId # local + shadow
        node_gids = self.mesh.data_nodegId[:self.mesh.nodesLocal]

        GlobalIndices3d = np.arange(self.mesh.nodesGlobal).reshape(nz + 1, ny + 1, nx + 1)
        # Get an I,J representation of the node coordinates
        Ipositions = np.array([int(np.where(GlobalIndices3d == i)[2]) for i in node_gids])
        Jpositions = np.array([int(np.where(GlobalIndices3d == i)[1]) for i in node_gids])
        Kpositions = np.array([int(np.where(GlobalIndices3d == i)[0]) for i in node_gids])

        # Get local domain data
        local_z[Kpositions, Jpositions, Ipositions] = self.mesh.data[:self.mesh.nodesLocal, 2]
        local_density[Kpositions, Jpositions, Ipositions] = self.DensityVar.data[:self.mesh.nodesLocal].flatten()

        comm.Allreduce(local_z, global_z)
        comm.Allreduce(local_density, global_density)

        # Remember that the nodes coordinates start from the bottom left so that the first
        # row in the numpy array is actually the bottom of the mesh.
        # Top Left - Bottom Left
        top1 = global_z[1:, :-1, :-1]
        top2 = global_z[1:, 1:, :-1]
        top3 = global_z[1:, 1:, 1:]
        top4 = global_z[1:, :-1, 1:]

        top = (top1 + top2 + top3 + top4) / 4.0

        bot1 = global_z[:-1, :-1, :-1]
        bot2 = global_z[:-1, 1:, :-1]
        bot3 = global_z[:-1, 1:, 1:]
        bot4 = global_z[:-1, :-1, 1:]

        bot = (bot1 + bot2 + bot3 + bot4) / 4.0

        dy = top - bot

        # Calculate pressure from the top half of the element.
        # (Takes the average density of the 2 top nodes)
        EpressureTop = (self.gravity.value * dy / 2.0 *
                       (global_density[1:, :-1, :-1] +
                        global_density[1:, 1:, :-1] +
                        global_density[1:, 1:, 1:] +
                        global_density[1:, :-1, 1:]) / 4.0)
        # Calculate pressure from the bottom half of the element.
        # (Takes the average density of the 2 bottom nodes)
        EpressureBot = (self.gravity.value * dy / 2.0 *
                       (global_density[:-1, :-1, :-1] +
                        global_density[:-1, 1:, :-1] +
                        global_density[:-1, 1:, 1:] +
                        global_density[:-1, :-1, 1:]) / 4.0)

        # Add pressure from element above except at the top.
        pressure = np.copy(EpressureTop)
        pressure[:-1, :, :] += EpressureBot[1:, :, :]
        residual = EpressureBot[0, :, :]

        # Flip the array upside down, do a cumul sum, flip it back.
        Tpressure = np.cumsum(pressure[::-1, :, :], axis=0)[::-1, :, :]
        bottom = Tpressure[0, :, :] + residual

        local_pressure = Tpressure.flatten()[all_nodes]

        self.lithostatic_field.data[:, 0] = local_pressure
        self.lithostatic_field.syncronise()

        self.Cell2Nodes.solve()

        return self.lithostatic_field_nodes
