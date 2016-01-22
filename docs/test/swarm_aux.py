'''
This script contains auxiliary swarm related tests.
'''
import underworld as uw


def swarm_save_load():
    '''
    This test simply creates a swarm & variable, saves them, then loads it into another swarm and checks for equality.
    '''

    mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    swarm = uw.swarm.Swarm(mesh)
    swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
    svar = swarm.add_variable("int",1)
    
    # Write something to variable
    
    import numpy as np
    svar.data[:,0] = np.arange(swarm.particleLocalCount)

    # Save to a file:

    swarm.save("saved_swarm.h5")
    svar.save("saved_swarm_variable.h5")

    # Now let's try and reload. First create an empty swarm, and then load:

    clone_swarm = uw.swarm.Swarm(mesh)
    clone_swarm.load( "saved_swarm.h5" )
    clone_svar = clone_swarm.add_variable("int",1)
    clone_svar.load("saved_swarm_variable.h5")

    # Now check for equality:
    import numpy as np
    if np.allclose(swarm.particleCoordinates.data,clone_swarm.particleCoordinates.data) != True:
        raise RuntimeError("Loaded swarm does not appear to be identical to saved swarm.")
    if np.allclose(svar.data,clone_svar.data) != True:
        raise RuntimeError("Loaded swarm variable does not appear to be identical to saved swarm.")

    # Clean up:
    if uw.rank() == 0: import os; os.remove( "saved_swarm.h5" ); os.remove( "saved_swarm_variable.h5" );


if __name__ == '__main__':
    swarm_save_load()
