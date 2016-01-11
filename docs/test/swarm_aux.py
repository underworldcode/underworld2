'''
This script contains auxiliary swarm related tests.
'''
import underworld as uw


def swarm_save_load():
    '''
    This test simply creates a mesh, saves it, then loads it into another mesh and checks for equality.
    '''

    mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    swarm = uw.swarm.Swarm(mesh)
    swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))

    # Save to a file:

    swarm.save("blat3/saved_swarm.h5")

    # Now let's try and reload. First create an empty swarm, and then load:

    clone_swarm = uw.swarm.Swarm(mesh)
    clone_swarm.load( "blat3/saved_swarm.h5" )

    # Now check for equality:

    import numpy as np
    if np.allclose(swarm.particleCoordinates.data,clone_swarm.particleCoordinates.data) != True:
        raise RuntimeError("Loaded swarm does not appear to be identical to saved swarm")

    # Clean up:
    if uw.rank == 0: import os; os.remove( "saved_swarm.h5" )


if __name__ == '__main__':
    swarm_save_load()