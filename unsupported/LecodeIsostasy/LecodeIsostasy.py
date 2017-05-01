import underworld as uw
import numpy as np
from mpi4py import MPI

comm = MPI.COMM_WORLD

def get_sep_velocities2D(mesh, velocityField):
    
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

    # If the local domain contains some of the top_ids, proceed:
    if top_ids:

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
    
    # 3-nodes mean average
    global_top_vy = (np.roll(global_top_vy, -1) + global_top_vy + np.roll(global_top_vy, 1)) / 3.0
    global_bot_vy = (np.roll(global_bot_vy, -1) + global_bot_vy + np.roll(global_bot_vy, 1)) / 3.0
    global_heights = (np.roll(global_heights, -1) + global_heights + np.roll(global_heights, 1)) / 3.0
    
    # Calculate and return sep velocities
    return global_top_vy - global_bot_vy, global_heights

def get_sep_velocities3D(mesh, velocityField):
    
    nx, ny, nz = mesh.elementRes
    GlobalIndices3d = np.arange(mesh.nodesGlobal).reshape(nz+1,ny+1,nx+1)
    
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

    # If the local domain contains some of the top_ids, proceed:
    if top_ids:

        # We must get rid of the shadow nodes
        top_ids = top_ids.data[top_ids.data < mesh.nodesLocal]

        # Get the cooresponding global ids
        node_gids = mesh.data_nodegId[top_ids]

        # Get y-velocities at the top
        topVelocities_nodes = velocityField.data[top_ids,1]
        heights_nodes = mesh.data[top_ids,1]

        # Get an I,J representation of the node coordinates
        Ipositions = [int(np.where(GlobalIndices3d == i)[2]) for i in top_ids]
        Jpositions = [int(np.where(GlobalIndices3d == i)[1]) for i in top_ids]
        Kpositions = [int(np.where(GlobalIndices3d == i)[0]) for i in top_ids]

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
        Ipositions = [int(np.where(GlobalIndices3d == i)[2]) for i in bot_ids]
        Jpositions = [int(np.where(GlobalIndices3d == i)[1]) for i in bot_ids]
        Kpositions = [int(np.where(GlobalIndices3d == i)[0]) for i in bot_ids]

        # Load the top-velocities in the local array with global dimensions.
        k = 0
        for i, j in zip(Ipositions, Jpositions):
            local_top_vy[j, i] = topVelocities_nodes[k]
            local_heights[j, i] += heights_nodes[k]
            k+=1
    
    # reduce local arrays into global_array
    comm.Allreduce(local_top_vy, global_top_vy)
    comm.Allreduce(local_bot_vy, global_bot_vy)
    comm.Allreduce(local_heights, global_heights)
        
    # Calculate and return sep velocities
    return global_top_vy - global_bot_vy, global_heights

def get_average_densities2D(mesh, DensityVar, MaterialVar, reference_mat):
    
    ncol, nrow = mesh.elementRes
    
    # Create some work arrays.
    global_densities = np.zeros((nrow+1)*(ncol+1))
    local_densities = np.zeros((nrow+1)*(ncol+1))
    local_materials = np.zeros((nrow+1)*(ncol+1))
    global_materials = np.zeros((nrow+1)*(ncol+1))

    # Load the densities and material into the local_densities arrays
    local_densities[mesh.data_nodegId] = DensityVar.data
    local_materials[mesh.data_nodegId] = MaterialVar.data

    # Reduce local_densities arrays to global_densities
    comm.Allreduce(local_densities, global_densities)
    comm.Allreduce(local_materials, global_materials)

    # Reshape the arrays
    global_densities = global_densities.reshape(((nrow+1),(ncol+1)))
    global_materials = global_materials.reshape(((nrow+1),(ncol+1)))
    # Convert material values to closest integers.
    global_materials = global_materials.astype("int")

    # Calculate Mean densities at the bottom
    botMeanDensities = np.mean(global_densities,0)
    
    # Calculate Mean densities at the bottom (reference_mat only)
    botMeanDensities0 = np.mean(np.ma.masked_where(global_materials != reference_mat, global_densities), 0)
    botMeanDensities0 = np.array(botMeanDensities0)

    botMeanDensities = (np.roll(botMeanDensities, -1) + botMeanDensities + np.roll(botMeanDensities, 1)) / 3.0
    botMeanDensities0 = (np.roll(botMeanDensities0, -1) + botMeanDensities0 + np.roll(botMeanDensities0, 1)) / 3.0    
    
    # return bottom Densities and Densities0
    return botMeanDensities, botMeanDensities0

def get_average_densities3D(mesh, DensityVar, MaterialVar, reference_mat):
    
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

    # Reshape the arrays
    global_densities = global_densities.reshape(((nz+1),(ny+1),(nx+1)))
    global_materials = global_materials.reshape(((nz+1),(ny+1),(nx+1)))
    # Convert material values to closest integers.
    global_materials = np.rint(global_materials).astype("int")

    # Calculate Mean densities at the bottom
    botMeanDensities = np.mean(global_densities,0)
    # Calculate Mean densities at the bottom (reference_mat only)
    botMeanDensities0 = np.ma.array(global_densities, mask=(global_materials != reference_mat))
    botMeanDensities0 = botMeanDensities0.mean(axis=0)
    botMeanDensities0 = np.array(botMeanDensities0)   
    
    # return bottom Densities and Densities0
    return botMeanDensities, botMeanDensities0  

def lecode_tools_isostasy2D(mesh, swarm, velocityField, densityFn,
        materialIndexField, reference_mat, average=False):

    MaterialIndexFieldFloat = swarm.add_variable( dataType="double", count=1 )
    DensityVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)
    MaterialVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)

    MaterialIndexFieldFloat.data[...] = np.rint(materialIndexField.data.astype("float"))
    
    projectorDensity = uw.utils.MeshVariable_Projection( DensityVar, densityFn, type=0 )
    projectorMaterial = uw.utils.MeshVariable_Projection( MaterialVar, MaterialIndexFieldFloat, type=0 )
    
    projectorDensity.solve()
    projectorMaterial.solve()

    sep_velocities_nodes, heights = get_sep_velocities2D(mesh, velocityField)
    botMeanDensities, botMeanDensities0 = get_average_densities2D(mesh, DensityVar,
                                                                MaterialVar, reference_mat)
    
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
        materialIndexField, reference_mat, average=False):
    
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
        materialIndexField, reference_mat, average=False):

    if mesh.dim == 2:
        lecode_tools_isostasy2D(mesh, swarm, velocityField, densityFn, 
                                materialIndexField, reference_mat, average)

    if mesh.dim == 3:
        lecode_tools_isostasy3D(mesh, swarm, velocityField, densityFn, 
                                materialIndexField, reference_mat, average)

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
  
