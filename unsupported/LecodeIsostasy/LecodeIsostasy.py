import underworld as uw
import numpy as np
from mpi4py import MPI
from scipy.interpolate import interp1d, interp2d

comm = MPI.COMM_WORLD
rank = comm.Get_rank()

def get_surface_nodes(mesh, surface):
    """ Get the closest mesh nodes to a surface 
        The function returns the global ids by default
    """

    procCount = comm.allgather(surface.particleLocalCount)
    particleGlobalCount = np.sum(procCount)

    offset = 0
    for i in xrange(rank):
        offset += procCount[i]

    if mesh.dim == 2:

        localSurface = np.zeros((particleGlobalCount, 2), surface.particleCoordinates.data.dtype)
        globalSurface = np.zeros((particleGlobalCount, 2), surface.particleCoordinates.data.dtype)

        if surface.particleLocalCount > 0:
            localSurface[offset:offset+surface.particleLocalCount] = surface.particleCoordinates.data[:]

        comm.Allreduce(localSurface, globalSurface)
        comm.Barrier()

        surface = globalSurface
        surface.sort(axis=0)
        
        
        # Build linearly interpolated function from surface points
        f = interp1d(surface[:,0], surface[:,1], bounds_error=False, fill_value="extrapolate")
        # Get y positions on the surface
        ypos = f(mesh.data[:,0])
        dy = np.abs((mesh.maxCoord[1] - mesh.minCoord[1]) / mesh.elementRes[1])
        localIds = np.arange(mesh.nodesDomain)[np.abs(mesh.data[:,1] - ypos) < (dy / 2.0)]
        globalIds = mesh.data_nodegId[localIds]
        return localIds, globalIds

    if mesh.dim == 3:
        
        localSurface = np.zeros((particleGlobalCount, 3), surface.particleCoordinates.data.dtype)
        globalSurface = np.zeros((particleGlobalCount, 3), surface.particleCoordinates.data.dtype)

        if surface.particleLocalCount > 0:
            localSurface[offset:offset+surface.particleLocalCount] = surface.particleCoordinates.data[:]

        comm.Allreduce(localSurface, globalSurface)
        comm.Barrier()

        surface = globalSurface
        # Sort the array
        surface = surface[np.lexsort(np.transpose(surface)[::-1])]

        f = interp2d(surface[:,0], surface[:,1], surface[:,2])
        # Get y positions on the surface
        zpos = f(mesh.data[:,0], mesh.data[:,1])
        dz = np.abs((mesh.maxCoord[2] - mesh.minCoord[2]) / mesh.elementRes[2])
        localIds = np.arange(mesh.nodesLocal)[np.abs(mesh.data[:,2] - zpos) < dz / 2.0]
        globalIds = mesh.data_nodegId[localIds]
        return localIds, globalIds


def get_sep_velocities2D(mesh, velocityField, surface=None):
    
    ncol, nrow = mesh.elementRes
    
    # Create some work arrays.
    local_top_vy = np.zeros((ncol+1,))
    local_bot_vy = np.zeros((ncol+1,))
    global_top_vy = np.zeros((ncol+1,))
    global_bot_vy = np.zeros((ncol+1,))
    
    local_heights = np.zeros((ncol+1,))
    global_heights = np.zeros((ncol+1,))

    # Get the ids of the nodes belonging to the top and bottom of the
    # global domain.
    top_ids = mesh.specialSets["MaxJ_VertexSet"]
    bot_ids = mesh.specialSets["MinJ_VertexSet"]

    if surface is not None:

        surfLocals, surfGlobals = get_surface_nodes(mesh, surface)
        if surfLocals.size > 0:
            # Get an I,J representation of the node coordinates
            Jpositions = (surfGlobals.astype("float") / (ncol + 1)).astype("int")
            Ipositions = surfGlobals - Jpositions * (ncol + 1)

            # Load the top-velocities in the local array with global dimensions.
            local_top_vy[Ipositions.flatten()] = velocityField.data[surfLocals, 1]
            local_heights[Ipositions.flatten()] += mesh.data[surfLocals][:,1]

    # If the local domain contains some of the top_ids, proceed:
    elif top_ids:

        # We must get rid of the shadow nodes
        top_ids = top_ids.data[top_ids.data < mesh.nodesLocal]

        # Get the cooresponding global ids
        node_gids = mesh.data_nodegId[top_ids]

        # Get y-velocities at the top
        topVelocities_nodes = velocityField.data[top_ids,1]
        heights_nodes = mesh.data[top_ids,1]

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
        bot_ids = bot_ids.data[bot_ids.data < mesh.nodesLocal]
        
        # Get the cooresponding global ids
        node_gids = mesh.data_nodegId[bot_ids]

        # Get y-velocities at the bottom
        botVelocities_nodes = velocityField.data[bot_ids,1] 
        heights_nodes = mesh.data[bot_ids,1]

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

def get_sep_velocities3D(mesh, velocityField, surface=None):
    
    nx, ny, nz = mesh.elementRes
    GlobalIndices3d = np.indices((nz+1,ny+1,nx+1))

    # Create some work arrays.
    local_top_vy = np.zeros(((ny+1),(nx+1)))
    local_bot_vy = np.zeros(((ny+1),(nx+1)))
    global_top_vy = np.zeros(((ny+1),(nx+1)))
    global_bot_vy = np.zeros(((ny+1),(nx+1)))
    
    local_heights = np.zeros(((ny+1),(nx+1)))
    global_heights = np.zeros(((ny+1),(nx+1)))

    # Get the ids of the nodes belonging to the top and bottom of the
    # global domain.
    top_ids = mesh.specialSets["MaxK_VertexSet"]
    bot_ids = mesh.specialSets["MinK_VertexSet"]


    if surface is not None:

        _, surfaceNodes = get_surface_nodes(mesh, surface)
        if surfaceNodes.size > 0:
            ix = np.in1d(np.arange(mesh.nodesGlobal), node_gids)
            Ipositions = GlobalIndices3d[2].flatten()[ix]
            Jpositions = GlobalIndices3d[1].flatten()[ix]
            Kpositions = GlobalIndices3d[0].flatten()[ix]

            # Load the top-velocities in the local array with global dimensions.
            k = 0
            for i, j in zip(Ipositions, Jpositions):
                local_top_vy[j, i] = velocityField.data[surfaceNodes][k]
                local_heights[j, i] += mesh.data[surfaceNodes,2][k]
                k+=1

    # If the local domain contains some of the top_ids, proceed:
    elif top_ids:

        # We must get rid of the shadow nodes
        top_ids = top_ids.data[top_ids.data < mesh.nodesLocal]

        # Get the cooresponding global ids
        node_gids = mesh.data_nodegId[top_ids]

        # Get y-velocities at the top
        topVelocities_nodes = velocityField.data[top_ids,1]
        heights_nodes = mesh.data[top_ids,1]

        # Get an I,J representation of the node coordinates

        # FROM Numpy 1.13> ... more efficient ?
        #ix = np.isin(GlobalIndices3d, node_gids)
        #Kpositions, Jposition, Iposition = np.where(ix)
        ix = np.in1d(np.arange(mesh.nodesGlobal), node_gids)
        Ipositions = GlobalIndices3d[2].flatten()[ix]
        Jpositions = GlobalIndices3d[1].flatten()[ix]
        Kpositions = GlobalIndices3d[0].flatten()[ix]

        # Load the top-velocities in the local array with global dimensions.
        k = 0
        for i, j in zip(Ipositions, Jpositions):
            local_top_vy[j, i] = topVelocities_nodes[k]
            local_heights[j, i] += heights_nodes[k]
            k+=1

    # If the local domain contains some of the bot_ids, proceed:
    if bot_ids:

        # We must get rid of the shadow nodes
        bot_ids = bot_ids.data[bot_ids.data < mesh.nodesLocal]
        
        # Get the cooresponding global ids
        node_gids = mesh.data_nodegId[bot_ids]

        # Get y-velocities at the bottom
        botVelocities_nodes = velocityField.data[bot_ids,1] 
        heights_nodes = mesh.data[bot_ids,1]

        # Get an I,J representation of the node coordinates
        ix = np.in1d(np.arange(mesh.nodesGlobal), node_gids)
        Ipositions = GlobalIndices3d[2].flatten()[ix]
        Jpositions = GlobalIndices3d[1].flatten()[ix]
        Kpositions = GlobalIndices3d[0].flatten()[ix]

        # Load the top-velocities in the local array with global dimensions.
        k = 0
        for i, j in zip(Ipositions, Jpositions):
            local_bot_vy[j, i] = botVelocities_nodes[k]
            local_heights[j, i] -= heights_nodes[k]
            k+=1
    
    # reduce local arrays into global_array
    comm.Allreduce(local_top_vy, global_top_vy)
    comm.Allreduce(local_bot_vy, global_bot_vy)
    comm.Allreduce(local_heights, global_heights)
    comm.Barrier()  

    # Calculate and return sep velocities
    return global_top_vy - global_bot_vy, global_heights

def get_average_densities2D(mesh, DensityVar, MaterialVar, reference_mat,
                            maskedMat=[]):
    
    ncol, nrow = mesh.elementRes
    
    # Create some work arrays.
    global_densities = np.zeros((nrow+1)*(ncol+1))
    local_densities = np.zeros((nrow+1)*(ncol+1))
    local_materials = np.zeros((nrow+1)*(ncol+1))
    global_materials = np.zeros((nrow+1)*(ncol+1))

    # Load the densities and material into the local_densities arrays
    # data_nodegId as domain size (local+node) so we need to only take
    # the local nodes otherwise the reduce operation will results in higher
    # values where domains overlaps...
    local_densities[mesh.data_nodegId[:mesh.nodesLocal]] = DensityVar.data
    local_materials[mesh.data_nodegId[:mesh.nodesLocal]] = MaterialVar.data

    # Reduce local_densities arrays to global_densities
    comm.Allreduce(local_densities, global_densities)
    comm.Allreduce(local_materials, global_materials)
    # It seems that the MPI implementation to not systematically impose a
    # barrier inside a reduce operation. That is weird but just to be safe:
    comm.Barrier()

    # Reshape the arrays
    global_densities = global_densities.reshape(((nrow+1),(ncol+1)))
    global_materials = global_materials.reshape(((nrow+1),(ncol+1)))
    # Convert material values to closest integers.
    global_materials = global_materials.astype("int")

    # Calculate Mean densities at the bottom
    if maskedMat:
        mask = np.in1d(global_materials, maskedMat)
        maskedArray = np.ma.masked_array(global_densities, mask)
        botMeanDensities = maskedArray.mean(axis=0)
    else:
        botMeanDensities = np.mean(global_densities,0)
    
    # Calculate Mean densities at the bottom (reference_mat only)
    botMeanDensities0 = np.ma.array(global_densities, mask=(global_materials != reference_mat))
    botMeanDensities0 = botMeanDensities0.mean(axis=0)
    botMeanDensities0 = np.array(botMeanDensities0)   

    botMeanDensities = (np.roll(botMeanDensities, -1) + botMeanDensities + np.roll(botMeanDensities, 1)) / 3.0
    botMeanDensities0 = (np.roll(botMeanDensities0, -1) + botMeanDensities0 + np.roll(botMeanDensities0, 1)) / 3.0    
    
    # return bottom Densities and Densities0
    return botMeanDensities, botMeanDensities0

def get_average_densities3D(mesh, DensityVar, MaterialVar, reference_mat,
                            maskedMat):
    
    nx, ny, nz = mesh.elementRes
    
    # Create some work arrays.
    global_densities = np.zeros((nx+1)*(ny+1)*(nz+1))
    local_densities = np.zeros((nx+1)*(ny+1)*(nz+1))
    local_materials = np.zeros((nx+1)*(ny+1)*(nz+1))
    global_materials = np.zeros((nx+1)*(ny+1)*(nz+1))

    # Load the densities and material into the local_densities arrays
    local_densities[mesh.data_nodegId[:mesh.nodesLocal]] = DensityVar.data[:mesh.nodesLocal]
    local_materials[mesh.data_nodegId[:mesh.nodesLocal]] = MaterialVar.data[:mesh.nodesLocal]

    # Reduce local_densities arrays to global_densities
    comm.Allreduce(local_densities, global_densities)
    comm.Allreduce(local_materials, global_materials)
    comm.Barrier()

    # Reshape the arrays
    global_densities = global_densities.reshape(((nz+1),(ny+1),(nx+1)))
    global_materials = global_materials.reshape(((nz+1),(ny+1),(nx+1)))
    # Convert material values to closest integers.
    global_materials = np.rint(global_materials).astype("int")

    # Calculate Mean densities at the bottom
    if maskedMat:
        mask = np.in1d(global_materials, maskedMat)
        maskedArray = np.ma.masked_array(global_densities, mask)
        botMeanDensities = maskedArray.mean(axis=0)
    else:
        botMeanDensities = np.mean(global_densities,0)

    # Calculate Mean densities at the bottom (reference_mat only)
    botMeanDensities0 = np.ma.array(global_densities, mask=(global_materials != reference_mat))
    botMeanDensities0 = botMeanDensities0.mean(axis=0)
    botMeanDensities0 = np.array(botMeanDensities0)   
    
    # return bottom Densities and Densities0
    return botMeanDensities, botMeanDensities0  

def lecode_tools_isostasy2D(mesh, swarm, velocityField, densityFn,
        materialIndexField, reference_mat, average=False, surface=None,
        maskedMat=[]):

    MaterialIndexFieldFloat = swarm.add_variable( dataType="double", count=1 )
    DensityVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)
    MaterialVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)

    MaterialIndexFieldFloat.data[...] = np.rint(materialIndexField.data.astype("float"))
    
    projectorDensity = uw.utils.MeshVariable_Projection( DensityVar, densityFn, type=0 )
    projectorMaterial = uw.utils.MeshVariable_Projection( MaterialVar, MaterialIndexFieldFloat, type=0 )
    
    projectorDensity.solve()
    projectorMaterial.solve()

    sep_velocities_nodes, heights = get_sep_velocities2D(mesh, velocityField,
                                                         surface=surface)
    botMeanDensities, botMeanDensities0 = get_average_densities2D(mesh, DensityVar,
                                                                MaterialVar,
                                                                reference_mat,
                                                                maskedMat)
    
    basal_velocities = -1.0 * botMeanDensities * sep_velocities_nodes / botMeanDensities0
    
    if average:
        basal_velocities = np.ones((basal_velocities.shape)) * np.mean(basal_velocities)

    base   = mesh.specialSets["MinJ_VertexSet"]
    if base:
        bot_ids = base.data[base.data < mesh.nodesLocal]
        node_gids = mesh.data_nodegId[bot_ids].flatten()
        velocityField.data[bot_ids,1] = basal_velocities[node_gids]

    velocityField.syncronise()

def lecode_tools_isostasy3D(mesh, swarm, velocityField, densityFn,
        materialIndexField, reference_mat, average=False, surface=None,
        maskedMat=[]):
    
    MaterialIndexFieldFloat = swarm.add_variable( dataType="double", count=1 )
    DensityVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)
    MaterialVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)

    MaterialIndexFieldFloat.data[...] = np.rint(materialIndexField.data.astype("float"))
    
    projectorDensity = uw.utils.MeshVariable_Projection( DensityVar, densityFn, type=0 )
    projectorMaterial = uw.utils.MeshVariable_Projection( MaterialVar, MaterialIndexFieldFloat, type=0 )
    
    projectorDensity.solve()
    projectorMaterial.solve()
    
    sep_velocities_nodes, heights = get_sep_velocities3D(mesh, velocityField)
    botMeanDensities, botMeanDensities0 = get_average_densities3D(mesh, DensityVar,
                                                                MaterialVar, reference_mat)
    
    basal_velocities = -1.0 * botMeanDensities * sep_velocities_nodes / botMeanDensities0
    
    if average:
        basal_velocities = np.ones((basal_velocities.shape)) * np.mean(basal_velocities)

    base   = mesh.specialSets["MinK_VertexSet"]
    if base:
        bot_ids = base.data[base.data < mesh.nodesLocal]
        node_gids = mesh.data_nodegId[bot_ids].flatten()
        velocityField.data[bot_ids,1] = basal_velocities.flatten()[node_gids]

    velocityField.syncronise()

def lecode_tools_isostasy(mesh, swarm, velocityField, densityFn,
        materialIndexField, reference_mat, average=False, surface=None,
        maskedMat=[]):
    
    if not mesh._cself.isRegular:
        raise TypeError("You are using an irregular mesh: \
                         isostasy module only works with regular meshes")

    if surface is not None and not isinstance(surface, uw.swarm._swarm.Swarm):
        raise TypeError("'surface' must be a tuple'")

    if mesh.dim == 2:
        lecode_tools_isostasy2D(mesh, swarm, velocityField, densityFn, 
                                materialIndexField, reference_mat, average,
                                surface=surface, maskedMat=maskedMat)

    if mesh.dim == 3:
        lecode_tools_isostasy3D(mesh, swarm, velocityField, densityFn, 
                                materialIndexField, reference_mat, average,
                                surface=surface, maskedMat=maskedMat)

#def get_phi():
#    
#    ncol, nrow = mesh.elementRes
#    
#    # Create some work arrays.
#    global_TdotField = np.zeros((nrow+1)*(ncol+1))
#    local_TdotField = np.zeros((nrow+1)*(ncol+1))
#    
#    local_TdotField[mesh.data_nodegId] = temperatureDotField.data
#    comm.Allreduce(local_TdotField, global_TdotField)
#    
#    global_TdotField = global_TdotField.reshape((nrow+1)*(ncol+1))
#    botMeanTdotField = np.mean(global_TdotField,0)
#    
#    botMeanTdotField = (np.roll(botMeanTdotField, -1) + botMeanTdotField + np.roll(botMeanTdotField, 1)) / 3.0
#    
#    return botMeanTdotField
  
