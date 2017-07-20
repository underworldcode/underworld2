# Romain Beucher romain.beucher@unimelb.edu.au

import underworld as uw
import numpy as np
from mpi4py import MPI

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

def lithoPressure(mesh, densityFn, gravity):
    """Function that calculates the lithostatic pressure field based on 
    material densities.

    inputs

    mesh: regular mesh
    densityFn: underworld function, shall return material densities
    gravity

    outputs:

    tuple: lithostatic pressure field, pressure field at the bottom of the
    model
    """

    DensityVar = uw.mesh.MeshVariable(mesh, nodeDofCount=1)
    projectorDensity = uw.utils.MeshVariable_Projection( DensityVar, densityFn, type=0 )
    projectorDensity.solve() 

    # 2D case
    if mesh.dim == 2:
        return lithoPressure2D(mesh, DensityVar, gravity)
    # 3D case
    if mesh.dim == 3:
        return lithoPressure3D(mesh, DensityVar, gravity)

def lithoPressure2D(mesh, DensityVar, gravity): 
    
    # Get Dimension of the global domain
    ncol, nrow = mesh.elementRes
    
    # Create some work arrays.
    local_y  = np.zeros((nrow+1, ncol+1))
    global_y = np.zeros((nrow+1, ncol+1))
    local_density  = np.zeros((nrow+1, ncol+1))
    global_density = np.zeros((nrow+1, ncol+1))
    local_pressure  = np.zeros((nrow, ncol))
    global_pressure = np.zeros((nrow, ncol))

    # Now we need to get the dimension as well as the location of the local domain
    # in the global domain.
    
    # Get the global ids, note that we must get rid of the shadow nodes
    all_nodes = mesh.data_elgId # local + shadow
    node_gids = mesh.data_nodegId[:mesh.nodesLocal]
   
    # Get an I,J representation of the node coordinates
    Jpositions = (node_gids.astype("float") / (ncol + 1)).astype("int")
    Ipositions = node_gids - Jpositions * (ncol + 1)
   
    # Get local domain data
    local_y[Jpositions, Ipositions] = mesh.data[:mesh.nodesLocal,1][:,np.newaxis]   
    local_density[Jpositions, Ipositions] = DensityVar.data[:mesh.nodesLocal]
    
    comm.Allreduce(local_y, global_y)
    comm.Allreduce(local_density, global_density)

    # Remember that the nodes coordinates start from the bottom left so that the first
    # row in the numpy array is actually the bottom of the mesh.
    # Top Left - Bottom Left
    dyleft = np.abs(global_y[1:,:-1] - global_y[:-1,:-1])
    # Top Right - Bottom Right
    dyright = np.abs(global_y[1:,1:] - global_y[:-1,1:])
    dy = (dyleft + dyright) / 2.

    # Calculate pressure from the top half of the element.
    # (Takes the average density of the 2 top nodes)
    EpressureTop = gravity * dy / 2.0 * (global_density[1:,:-1] + global_density[1:,1:]) / 2.0
    # Calculate pressure from the bottom half of the element.
    # (Takes the average density of the 2 bottom nodes)
    EpressureBot = gravity * dy / 2.0 * (global_density[:-1,:-1] + global_density[:-1,1:]) / 2.0 

    # Add pressure from element above except at the top.
    pressure = np.copy(EpressureTop)
    pressure[:-1,:] += EpressureBot[1:,:]
    residual = EpressureBot[0,:]
    
    # Flip the array upside down, do a cumul sum, flip it back.
    Tpressure = np.cumsum(pressure[::-1,:], axis=0)[::-1,:]
    bottom = Tpressure[0,:] + residual
     
    local_pressure = Tpressure.flatten()[all_nodes]
    local_pressure = local_pressure.reshape((local_pressure.size,1))
    
    return local_pressure, bottom

def lithoPressure3D(mesh, DensityVar, gravity):   
    
    # Get Dimension of the global domain
    nx, ny, nz = mesh.elementRes
    
    # Create some work arrays.
    local_z  = np.zeros((nz+1, ny+1, nx+1))
    global_z = np.zeros((nz+1, ny+1, nx+1))
    local_density  = np.zeros((nz+1, ny+1, nx+1))
    global_density = np.zeros((nz+1, ny+1, nx+1))
    local_pressure  = np.zeros((nz, ny, nx))
    global_pressure = np.zeros((nz, ny, nx))

    # Get the global ids, note that we must get rid of the shadow nodes
    all_nodes = mesh.data_elgId
    node_gids = mesh.data_nodegId[:mesh.nodesLocal] 
    
    GlobalIndices3d = np.arange(mesh.nodesGlobal).reshape(nz+1,ny+1,nx+1)  
    # Get an I,J representation of the node coordinates
    Ipositions = np.array([int(np.where(GlobalIndices3d == i)[2]) for i in node_gids])
    Jpositions = np.array([int(np.where(GlobalIndices3d == i)[1]) for i in node_gids])
    Kpositions = np.array([int(np.where(GlobalIndices3d == i)[0]) for i in node_gids])
    
     # Get local domain data
    local_z[Kpositions, Jpositions, Ipositions] = mesh.data[:mesh.nodesLocal,2]  
    local_density[Kpositions, Jpositions, Ipositions] = DensityVar.data[:mesh.nodesLocal].flatten()
    
    comm.Allreduce(local_z, global_z)
    comm.Allreduce(local_density, global_density)   

    # Remember that the nodes coordinates start from the bottom left so that the first
    # row in the numpy array is actually the bottom of the mesh.
    # Top Left - Bottom Left
    top1 = global_z[1:,:-1,:-1]
    top2 = global_z[1:,1:,:-1]
    top3 = global_z[1:,1:,1:]
    top4 = global_z[1:,:-1,1:]

    top = (top1 + top2 + top3 + top4) / 4.0
    
    bot1 = global_z[:-1,:-1,:-1]
    bot2 = global_z[:-1,1:,:-1]
    bot3 = global_z[:-1,1:,1:]
    bot4 = global_z[:-1,:-1,1:]
    
    bot = (bot1 + bot2 + bot3 + bot4) / 4.0
    
    dy = top - bot

    # Calculate pressure from the top half of the element.
    # (Takes the average density of the 2 top nodes)
    EpressureTop = (gravity * dy / 2.0 *
                   (global_density[1:,:-1,:-1] + 
                    global_density[1:,1:,:-1] + 
                    global_density[1:,1:,1:] + 
                    global_density[1:,:-1,1:]) / 4.0)
    # Calculate pressure from the bottom half of the element.
    # (Takes the average density of the 2 bottom nodes)
    EpressureBot = (gravity * dy / 2.0 * 
                   (global_density[:-1,:-1,:-1] + 
                    global_density[:-1,1:,:-1] + 
                    global_density[:-1,1:,1:] + 
                    global_density[:-1,:-1,1:]) / 4.0)
    
    # Add pressure from element above except at the top.
    pressure = np.copy(EpressureTop)
    pressure[:-1,:,:] += EpressureBot[1:,:,:]
    residual = EpressureBot[0,:,:]

    # Flip the array upside down, do a cumul sum, flip it back.
    Tpressure = np.cumsum(pressure[::-1,:,:], axis=0)[::-1,:,:]
    bottom = Tpressure[0,:,:] + residual
    
    local_pressure = Tpressure.flatten()[all_nodes]
    local_pressure = local_pressure.reshape((local_pressure.size,1))
    
    return local_pressure, bottom


