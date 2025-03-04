from __future__ import print_function,  absolute_import
import underworld as uw
import numpy as np
from scipy.interpolate import interp1d, interp2d

comm = uw.mpi.comm
rank = uw.mpi.rank


class LecodeIsostasy(object):
    """LecodeIsostasy"""

    def __init__(self, reference_mat=None, average=False,
                 surface=None, maskedMat=None,
                 vertical_walls_conditions=None):
        """LecodeIsostasy Init Method

        Parameters
        ----------

        reference_mat : reference material at the base of the Model
                        (The Material that enters or leaves the Model
                         to account for isostasy)
        average : (bool) if True, calculate an average velocity at the base of the Model.
        surface : (optional) Initial position of the surface of the material columns. The
                  top of the Model is used by default and should be fine in most cases.
        maskedMat : (optional) Mask a material or a list of materials.
                    The masked material will not interfere in the calculation of the densities.
        vertical_walls_conditions : (optional) Existing boundary conditions on the
                                    lateral walls.

        Returns
        -------

        GEO.LecodeIsostasy object.

        """

        self.mesh = None
        self.swarm = None
        self._mesh_advector = None
        self.velocityField = None
        self._densityFn = None
        self.materialIndexField = None
        self.reference_mat = reference_mat
        self.average = average
        self.surface = surface
        self.maskedMat = list(maskedMat) if maskedMat else list()
        self._maskedMatIndices = [mat.index for mat in self.maskedMat]
        self.initialized = False

        # Make the class aware of the conditions on the vertical walls
        if vertical_walls_conditions:
            if not isinstance(vertical_walls_conditions, dict):
                raise TypeError(
                    """vertical_walls_conditions must be of type
                       'dict'""")
            options = ["left", "right", "front", "back"]
            for key in vertical_walls_conditions.keys():
                if key not in options:
                    raise KeyError("""
                                   Wrong dictionary key, options are:
                                   {0}""".format(options))
            self.vertical_walls_conditions = vertical_walls_conditions

    def solve(self):

        if not self.initialized:
            self._check_all_defined()

        if self.mesh.dim == 2:
            self._lecode_tools_isostasy2D()

        if self.mesh.dim == 3:
            self._lecode_tools_isostasy3D()

    def _check_all_defined(self):
        if not self.mesh:
            raise ValueError("Please link a Mesh to the Isostasy solver")
        if not self.swarm:
            raise ValueError("Please link a Swarm to the Isostasy solver")
        if not self.velocityField:
            raise ValueError("""
                Please link a Velocity Field to the Isostasy solver""")
        if not self.materialIndexField:
            raise ValueError("""
                Please link a Material Field to the Isostasy solver""")
        if not self.materialIndexField:
            raise ValueError("""
                Please link a Material Field to the Isostasy solver""")
        if not self._densityFn:
            raise ValueError("""
                Please link a Density Field to the Isostasy solver""")
        if not self.reference_mat:
            raise ValueError("""
                Please define a reference Material for the Isostasy solver""")

        # Create utilities
        self.MaterialIndexFieldFloat = self.swarm.add_variable(
            dataType="double", count=1)
        self.DensityVar = uw.mesh.MeshVariable(self.mesh, nodeDofCount=1)
        self.MaterialVar = uw.mesh.MeshVariable(self.mesh, nodeDofCount=1)
        self.projectorDensity = uw.utils.MeshVariable_Projection(
            self.DensityVar,
            self._densityFn,
            type=0)
        self.projectorMaterial = uw.utils.MeshVariable_Projection(
            self.MaterialVar, self.MaterialIndexFieldFloat, type=0)

        if not self.mesh._cself.isRegular:
            if self._mesh_advector:
                if self._mesh_advector.axis > 0:
                    raise TypeError("""You are using an irregular mesh: \
                                    isostasy module only works with regular
                                    meshes""")
            else:
                raise TypeError("""You are using an irregular mesh: \
                                isostasy module only works with regular
                                meshes""")

        if (self.surface is not None and
           not isinstance(self.surface, uw.swarm._swarm.Swarm)):
            raise TypeError("'surface' must be a tuple'")

        self.initialized = True

    def _lecode_tools_isostasy2D(self):

        self.MaterialIndexFieldFloat.data[...] = (
            np.rint(self.materialIndexField.data.astype("float"))
        )

        self.projectorDensity.solve()
        self.projectorMaterial.solve()

        sep_velocities_nodes, _ = self._get_sep_velocities2D()
        botMeanDensities, botMeanDensities0 = self._get_average_densities2D()

        basal_velocities = -1.0 * botMeanDensities * sep_velocities_nodes / botMeanDensities0

        if self.average:
            basal_velocities = np.ones((basal_velocities.shape)) * np.mean(basal_velocities)

        base = self.mesh.specialSets["MinJ_VertexSet"]

        if base:

            # Look at the vertical conditions on the vertical walls
            # if one has been defined, remove the corners from the base
            # and do not change the velocity on those nodes.
            if self.vertical_walls_conditions:
                if self.vertical_walls_conditions["left"]:
                    if self.vertical_walls_conditions["left"][-1] is not None:
                        left = self.mesh.specialSets["MinI_VertexSet"]
                        if left:
                            base -= left
                if self.vertical_walls_conditions["right"]:
                    if self.vertical_walls_conditions["right"][-1] is not None:
                        right = self.mesh.specialSets["MaxI_VertexSet"]
                        if right:
                            base -= right

            bot_ids = base.data[base.data < self.mesh.nodesLocal]
            node_gids = self.mesh.data_nodegId[bot_ids].flatten()
            self.velocityField.data[bot_ids, 1] = basal_velocities[node_gids]

        self.velocityField.syncronise()

    def _lecode_tools_isostasy3D(self):

        self.MaterialIndexFieldFloat.data[...] = (
            np.rint(self.materialIndexField.data.astype("float"))
        )

        self.projectorDensity.solve()
        self.projectorMaterial.solve()

        sep_velocities_nodes, _ = self._get_sep_velocities3D()
        botMeanDensities, botMeanDensities0 = self._get_average_densities3D()

        basal_velocities = -1.0 * botMeanDensities * sep_velocities_nodes / botMeanDensities0

        if self.average:
            basal_velocities = np.ones((basal_velocities.shape)) * np.mean(basal_velocities)

        base = self.mesh.specialSets["MinK_VertexSet"]
        if base:

            # Look at the vertical conditions on the vertical walls
            # if one has been defined, remove the corners from the base
            # and do not change the velocity on those nodes.
            if self.vertical_walls_conditions:
                if self.vertical_walls_conditions["left"]:
                    if self.vertical_walls_conditions["left"][-1] is not None:
                        left = self.mesh.specialSets["MinI_VertexSet"]
                        if left:
                            base -= left
                if self.vertical_walls_conditions["right"]:
                    if self.vertical_walls_conditions["right"][-1] is not None:
                        right = self.mesh.specialSets["MaxI_VertexSet"]
                        if right:
                            base -= right
                if self.vertical_walls_conditions["front"]:
                    if self.vertical_walls_conditions["front"][-1] is not None:
                        front = self.mesh.specialSets["MinJ_VertexSet"]
                        if front:
                            base -= front
                if self.vertical_walls_conditions["back"]:
                    if self.vertical_walls_conditions["back"][-1] is not None:
                        back = self.mesh.specialSets["MaxJ_VertexSet"]
                        if back:
                            base -= back
            bot_ids = base.data[base.data < self.mesh.nodesLocal]
            node_gids = self.mesh.data_nodegId[bot_ids].flatten()
            self.velocityField.data[bot_ids, -1] = basal_velocities.flatten()[node_gids]

        self.velocityField.syncronise()

    def _get_sep_velocities2D(self):

        ncol, nrow = self.mesh.elementRes
        if self.mesh.elementType == "Q2":
            fact = 2
        elif self.mesh.elementType == "Q1":
            fact = 1
        ncol *= fact
        nrow *= fact

        # Create some work arrays.
        local_top_vy = np.zeros((ncol + 1,))
        local_bot_vy = np.zeros((ncol + 1,))
        global_top_vy = np.zeros((ncol + 1,))
        global_bot_vy = np.zeros((ncol + 1,))

        local_heights = np.zeros((ncol + 1,))
        global_heights = np.zeros((ncol + 1,))

        # Get the ids of the nodes belonging to the top and bottom of the
        # global domain.
        top_ids = self.mesh.specialSets["MaxJ_VertexSet"]
        bot_ids = self.mesh.specialSets["MinJ_VertexSet"]

        if self.surface is not None:

            surfLocals, surfGlobals = self._get_surface_nodes()
            if surfLocals.size > 0:
                # Get an I,J representation of the node coordinates
                Jpositions = (surfGlobals.astype("float") / (ncol + 1)).astype("int")
                Ipositions = surfGlobals - Jpositions * (ncol + 1)

                # Load the top-velocities in the local array with global dimensions.
                local_top_vy[Ipositions.flatten()] = self.velocityField.data[surfLocals, 1]
                local_heights[Ipositions.flatten()] += self.mesh.data[surfLocals][:,1]

        # If the local domain contains some of the top_ids, proceed:
        elif top_ids:

            # We must get rid of the shadow nodes
            top_ids = top_ids.data[top_ids.data < self.mesh.nodesLocal]

            # Get the cooresponding global ids
            node_gids = self.mesh.data_nodegId[top_ids]

            # Get y-velocities at the top
            topVelocities_nodes = self.velocityField.data[top_ids,1]
            heights_nodes = self.mesh.data[top_ids,1]

            # Get an I,J representation of the node coordinates
            Jpositions = (node_gids.astype("float") / (ncol + 1)).astype("int")
            Ipositions = node_gids - Jpositions * (ncol + 1)

            # Load the top-velocities in the local array with global dimensions.
            j = 0
            for pos in Ipositions:
                local_top_vy[pos] = topVelocities_nodes[j]
                local_heights[pos] += heights_nodes[j]
                j+=1

        # If the local domain contains some of the bot_ids, proceed:
        if bot_ids:

            # We must get rid of the shadow nodes
            bot_ids = bot_ids.data[bot_ids.data < self.mesh.nodesLocal]

            # Get the cooresponding global ids
            node_gids = self.mesh.data_nodegId[bot_ids]

            # Get y-velocities at the bottom
            botVelocities_nodes = self.velocityField.data[bot_ids,1]
            heights_nodes = self.mesh.data[bot_ids,1]

            # Get an I,J representation of the node coordinates
            Jpositions = (node_gids.astype("float") / (ncol + 1)).astype("int")
            Ipositions = node_gids - Jpositions * (ncol + 1)

            # Load the bottom-velocities in the local array with global dimensions.
            j = 0
            for pos in Ipositions:
                local_bot_vy[pos] = botVelocities_nodes[j]
                local_heights[pos] -= heights_nodes[j]
                j+=1

        # reduce local arrays into global_array
        comm.Allreduce(local_top_vy, global_top_vy)
        comm.Allreduce(local_bot_vy, global_bot_vy)
        comm.Allreduce(local_heights, global_heights)
        comm.Barrier()

        # 3-nodes mean average
        global_top_vy = (np.roll(global_top_vy, -1) + global_top_vy + np.roll(global_top_vy, 1)) / 3.0
        global_bot_vy = (np.roll(global_bot_vy, -1) + global_bot_vy + np.roll(global_bot_vy, 1)) / 3.0
        global_heights = (np.roll(global_heights, -1) + global_heights + np.roll(global_heights, 1)) / 3.0

        # Calculate and return sep velocities
        return global_top_vy - global_bot_vy, global_heights

    def _get_sep_velocities3D(self):

        nx, ny, nz = self.mesh.elementRes
        if self.mesh.elementType == "Q2":
            fact = 2
        elif self.mesh.elementType == "Q1":
            fact = 1
        nx *= fact
        ny *= fact
        nz *= fact
        GlobalIndices3d = np.indices((nz + 1, ny + 1, nx + 1))

        # Create some work arrays.
        local_top_vy = np.zeros(((ny + 1), (nx + 1)))
        local_bot_vy = np.zeros(((ny + 1), (nx + 1)))
        global_top_vy = np.zeros(((ny + 1), (nx + 1)))
        global_bot_vy = np.zeros(((ny + 1), (nx + 1)))

        local_heights = np.zeros(((ny + 1), (nx + 1)))
        global_heights = np.zeros(((ny + 1), (nx + 1)))

        # Get the ids of the nodes belonging to the top and bottom of the
        # global domain.
        top_ids = self.mesh.specialSets["MaxK_VertexSet"]
        bot_ids = self.mesh.specialSets["MinK_VertexSet"]

        if self.surface is not None:

            _, surfaceNodes = self._get_surface_nodes()
            if surfaceNodes.size > 0:
                ix = np.in1d(np.arange(self.mesh.nodesGlobal), surfaceNodes)
                Ipositions = GlobalIndices3d[2].flatten()[ix]
                Jpositions = GlobalIndices3d[1].flatten()[ix]

                # Load the top-velocities in the local array with global dimensions.
                k = 0
                for i, j in zip(Ipositions, Jpositions):
                    local_top_vy[j, i] = self.velocityField.data[surfaceNodes][k]
                    local_heights[j, i] += self.mesh.data[surfaceNodes,2][k]
                    k += 1

        # If the local domain contains some of the top_ids, proceed:
        elif top_ids:

            # We must get rid of the shadow nodes
            top_ids = top_ids.data[top_ids.data < self.mesh.nodesLocal]

            # Get the cooresponding global ids
            node_gids = self.mesh.data_nodegId[top_ids]

            # Get y-velocities at the top
            topVelocities_nodes = self.velocityField.data[top_ids, -1]
            heights_nodes = self.mesh.data[top_ids, -1]

            # Get an I,J representation of the node coordinates

            # FROM Numpy 1.13> ... more efficient ?
            #ix = np.isin(GlobalIndices3d, node_gids)
            #Kpositions, Jposition, Iposition = np.where(ix)
            ix = np.in1d(np.arange(self.mesh.nodesGlobal), node_gids)
            Ipositions = GlobalIndices3d[2].flatten()[ix]
            Jpositions = GlobalIndices3d[1].flatten()[ix]

            # Load the top-velocities in the local array with global dimensions.
            k = 0
            for i, j in zip(Ipositions, Jpositions):
                local_top_vy[j, i] = topVelocities_nodes[k]
                local_heights[j, i] += heights_nodes[k]
                k += 1

        # If the local domain contains some of the bot_ids, proceed:
        if bot_ids:

            # We must get rid of the shadow nodes
            bot_ids = bot_ids.data[bot_ids.data < self.mesh.nodesLocal]

            # Get the cooresponding global ids
            node_gids = self.mesh.data_nodegId[bot_ids]

            # Get y-velocities at the bottom
            botVelocities_nodes = self.velocityField.data[bot_ids,-1]
            heights_nodes = self.mesh.data[bot_ids,-1]

            # Get an I,J representation of the node coordinates
            ix = np.in1d(np.arange(self.mesh.nodesGlobal), node_gids)
            Ipositions = GlobalIndices3d[2].flatten()[ix]
            Jpositions = GlobalIndices3d[1].flatten()[ix]

            # Load the top-velocities in the local array with global dimensions.
            k = 0
            for i, j in zip(Ipositions, Jpositions):
                local_bot_vy[j, i] = botVelocities_nodes[k]
                local_heights[j, i] -= heights_nodes[k]
                k += 1

        # reduce local arrays into global_array
        comm.Allreduce(local_top_vy, global_top_vy)
        comm.Allreduce(local_bot_vy, global_bot_vy)
        comm.Allreduce(local_heights, global_heights)
        comm.Barrier()

        # Calculate and return sep velocities
        return global_top_vy - global_bot_vy, global_heights

    def _get_surface_nodes(self):
        """ Get the closest mesh nodes to a surface
            The function returns the global ids by default
        """

        procCount = comm.allgather(self.surface.particleLocalCount)
        particleGlobalCount = np.sum(procCount)

        offset = 0
        for i in range(rank):
            offset += procCount[i]

        if self.mesh.dim == 2:

            localSurface = np.zeros((particleGlobalCount, 2), self.surface.particleCoordinates.data.dtype)
            globalSurface = np.zeros((particleGlobalCount, 2), self.surface.particleCoordinates.data.dtype)

            if self.surface.particleLocalCount > 0:
                localSurface[offset:offset + self.surface.particleLocalCount] = self.surface.particleCoordinates.data[:]

            comm.Allreduce(localSurface, globalSurface)
            comm.Barrier()

            surface = globalSurface
            surface.sort(axis=0)

            # Build linearly interpolated function from surface points
            f = interp1d(surface[:, 0], surface[:, 1], bounds_error=False, fill_value="extrapolate")
            # Get y positions on the surface
            ypos = f(self.mesh.data[:, 0])
            dy = np.abs((self.mesh.maxCoord[1] - self.mesh.minCoord[1]) / self.mesh.elementRes[1])
            localIds = np.arange(self.mesh.nodesDomain)[np.abs(self.mesh.data[:, 1] - ypos) < (dy / 2.0)]
            globalIds = self.mesh.data_nodegId[localIds]
            return localIds, globalIds

        if self.mesh.dim == 3:

            localSurface = np.zeros((particleGlobalCount, 3), self.surface.particleCoordinates.data.dtype)
            globalSurface = np.zeros((particleGlobalCount, 3), self.surface.particleCoordinates.data.dtype)

            if surface.particleLocalCount > 0:
                localSurface[offset:offset + surface.particleLocalCount] = self.surface.particleCoordinates.data[:]

            comm.Allreduce(localSurface, globalSurface)
            comm.Barrier()

            surface = globalSurface
            # Sort the array
            surface = surface[np.lexsort(np.transpose(surface)[::-1])]

            f = interp2d(surface[:, 0], surface[:, 1], surface[:, 2])
            # Get y positions on the surface
            zpos = f(self.mesh.data[:, 0], self.mesh.data[:, 1])
            dz = np.abs((self.mesh.maxCoord[2] - self.mesh.minCoord[2]) / self.mesh.elementRes[2])
            localIds = np.arange(self.mesh.nodesLocal)[np.abs(self.mesh.data[:, 2] - zpos) < dz / 2.0]
            globalIds = self.mesh.data_nodegId[localIds]
            return localIds, globalIds

    def _get_average_densities2D(self):

        ncol, nrow = self.mesh.elementRes
        if self.mesh.elementType == "Q2":
            fact = 2
        elif self.mesh.elementType == "Q1":
            fact = 1
        ncol *= fact
        nrow *= fact

        # Create some work arrays.
        global_densities = np.zeros((nrow + 1) * (ncol + 1))
        local_densities = np.zeros((nrow + 1) * (ncol + 1))
        local_materials = np.zeros((nrow + 1) * (ncol + 1))
        global_materials = np.zeros((nrow + 1) * (ncol + 1))

        # Load the densities and material into the local_densities arrays
        # data_nodegId as domain size (local+node) so we need to only take
        # the local nodes otherwise the reduce operation will results in higher
        # values where domains overlaps...
        local_densities[self.mesh.data_nodegId[:self.mesh.nodesLocal]] = self.DensityVar.data[:self.mesh.nodesLocal, 0]
        local_materials[self.mesh.data_nodegId[:self.mesh.nodesLocal]] = self.MaterialVar.data[:self.mesh.nodesLocal, 0]

        # Reduce local_densities arrays to global_densities
        comm.Allreduce(local_densities, global_densities)
        comm.Allreduce(local_materials, global_materials)
        # It seems that the MPI implementation to not systematically impose a
        # barrier inside a reduce operation. That is weird but just to be safe:
        comm.Barrier()

        # Reshape the arrays
        global_densities = global_densities.reshape(((nrow + 1), (ncol + 1)))
        global_materials = global_materials.reshape(((nrow + 1), (ncol + 1)))
        # Convert material values to closest integers.
        global_materials = global_materials.astype("int")

        # Calculate Mean densities at the bottom
        if self.maskedMat:
            mask = np.in1d(global_materials, self.maskedMatIndices)
            maskedArray = np.ma.masked_array(global_densities, mask)
            botMeanDensities = maskedArray.mean(axis=0)
        else:
            botMeanDensities = np.mean(global_densities, 0)

        # Calculate Mean densities at the bottom (reference_mat only)
        botMeanDensities0 = np.ma.array(global_densities, mask=(global_materials != self.reference_mat.index))
        if botMeanDensities0.count() == 0:
            raise ValueError("""I am trying hard here but it looks like you
                             don't have any material with index {0} in your model.
                             Please check your set up. The resolution might
                             also be insufficient to resolve the
                             material...""".format(self.reference_mat.name))

        botMeanDensities0 = botMeanDensities0.mean(axis=0)
        botMeanDensities0 = np.array(botMeanDensities0)

        botMeanDensities = (np.roll(botMeanDensities, -1) + botMeanDensities + np.roll(botMeanDensities, 1)) / 3.0
        botMeanDensities0 = (np.roll(botMeanDensities0, -1) + botMeanDensities0 + np.roll(botMeanDensities0, 1)) / 3.0

        # return bottom Densities and Densities0
        return botMeanDensities, botMeanDensities0

    def _get_average_densities3D(self):

        nx, ny, nz = self.mesh.elementRes
        if self.mesh.elementType == "Q2":
            fact = 2
        elif self.mesh.elementType == "Q1":
            fact = 1
        nx *= fact
        ny *= fact
        nz *= fact

        # Create some work arrays.
        global_densities = np.zeros((nx+1)*(ny+1)*(nz+1))
        local_densities = np.zeros((nx+1)*(ny+1)*(nz+1))
        local_materials = np.zeros((nx+1)*(ny+1)*(nz+1))
        global_materials = np.zeros((nx+1)*(ny+1)*(nz+1))

        # Load the densities and material into the local_densities arrays
        local_densities[self.mesh.data_nodegId[:self.mesh.nodesLocal]] = self.DensityVar.data[:self.mesh.nodesLocal, 0]
        local_materials[self.mesh.data_nodegId[:self.mesh.nodesLocal]] = self.MaterialVar.data[:self.mesh.nodesLocal, 0]

        # Reduce local_densities arrays to global_densities
        comm.Allreduce(local_densities, global_densities)
        comm.Allreduce(local_materials, global_materials)
        comm.Barrier()

        # Reshape the arrays
        global_densities = global_densities.reshape(((nz + 1), (ny + 1), (nx + 1)))
        global_materials = global_materials.reshape(((nz + 1), (ny + 1), (nx + 1)))
        # Convert material values to closest integers.
        global_materials = np.rint(global_materials).astype("int")

        # Calculate Mean densities at the bottom
        if self.maskedMat:
            mask = np.in1d(global_materials, self.maskedMatIndices)
            maskedArray = np.ma.masked_array(global_densities, mask)
            botMeanDensities = maskedArray.mean(axis=0)
        else:
            botMeanDensities = np.mean(global_densities, 0)

        # Calculate Mean densities at the bottom (reference_mat only)
        botMeanDensities0 = np.ma.array(global_densities, mask=(global_materials != self.reference_mat.index))
        if botMeanDensities0.count() == 0:
            raise ValueError("""I am trying hard here but it looks like you
                             don't have any material with index {0} in your model.
                             Please check your set up. The resolution might
                             also be insufficient to resolve the
                             material...""".format(self.reference_mat.name))
        botMeanDensities0 = botMeanDensities0.mean(axis=0)
        botMeanDensities0 = np.array(botMeanDensities0)

        # return bottom Densities and Densities0
        return botMeanDensities, botMeanDensities0

