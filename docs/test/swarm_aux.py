'''
This script contains auxiliary swarm related tests.
'''
import underworld as uw
import numpy as np

def swarm_save_load(swarmtype):
    '''
    This test simply creates a swarm & variable, saves them, then loads it into another swarm and checks for equality.
    '''

    mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    swarm = uw.swarm.Swarm(mesh)
    if swarmtype=='global':
        swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
    elif swarmtype=='passivetracer':
        arr = np.zeros((1,2))
        arr[0] = [0.1,0.1]
        swarm.add_particles_with_coordinates(arr)
    else:
        raise RuntimeError("Must be 'global' or 'passivetracer'")
    svar1 = swarm.add_variable("double",2)
    svar2 = swarm.add_variable("int",1)

    # Write the positions to the variable
    svar1.data[:] = swarm.particleCoordinates.data[:]
    
    # write the rounded particle coords
    svar2.data[:,0] = (1000.*swarm.particleCoordinates.data[:,0]).astype(int)

    # Save to a file:
    swarm.save("saved_swarm.h5")
    svar1.save("saved_swarm_variable1.h5")
    svar2.save("saved_swarm_variable2.h5")
    partcount = swarm.particleGlobalCount
    
    # Now let's try and reload. First create an empty swarm, and then load:
    clone_swarm = uw.swarm.Swarm(mesh)
    clone_swarm.load( "saved_swarm.h5" )
    # check it has required particle count
    globcount = clone_swarm.particleGlobalCount
    if globcount != partcount:
        raise RuntimeError("Reloaded swarm appears has {} particles but {} were expected.".format(globcount,partcount))

    # reload recorded positions var
    clone_svar1 = clone_swarm.add_variable("double",2)
    clone_svar1.load("saved_swarm_variable1.h5")
    if np.allclose(clone_swarm.particleCoordinates.data,clone_svar1.data) != True:
        raise RuntimeError("Loaded swarm variable1 does not appear to contain the correct data.")

    clone_svar2 = clone_swarm.add_variable("int",1)
    clone_svar2.load("saved_swarm_variable2.h5")

    if np.allclose(clone_svar2.data[:,0],(1000.*clone_swarm.particleCoordinates.data[:,0]).astype(int)) != True:
        raise RuntimeError("Loaded swarm variable2 does not appear to contain correct data.")

    # Clean up:
    if uw.rank() == 0: import os; os.remove( "saved_swarm.h5" ); os.remove( "saved_swarm_variable1.h5" ); os.remove( "saved_swarm_variable2.h5" );


if __name__ == '__main__':
    swarm_save_load('global')
    swarm_save_load('passivetracer')
