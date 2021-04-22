'''
This script contains auxiliary swarm related tests.
'''
import underworld as uw
import numpy as np
import h5py 
from underworld.scaling import get_coefficients
from underworld.scaling import units as u
from underworld.scaling import non_dimensionalise
from underworld.scaling import dimensionalise

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
    svar3 = swarm.add_variable("double",2)

    # Write the positions to the variable
    svar1.data[:] = swarm.particleCoordinates.data[:]
    fact = dimensionalise(1.0, units=u.m).magnitude
    svar3.data[:] = swarm.particleCoordinates.data[:] * fact
    
    # write the rounded particle coords
    svar2.data[:,0] = (1000.*swarm.particleCoordinates.data[:,0]).astype(int)

    # Save to a file:
    swarm.save("saved_swarm.h5", units=u.m, attr1_name="attr1_val", attr2_name="attr2_val")
    svar1.save("saved_swarm_variable1.h5")
    svar2.save("saved_swarm_variable2.h5")
    svar3.save("saved_swarm_variable3.h5", units=u.m, attr1_name="attr1_val", attr2_name="attr2_val")
    partcount = swarm.particleGlobalCount

    # Check that swarm attributes are properly saved to the h5 file.
    attrs_list = ["units", "attr1_name", "attr2_name"]
    attrs_vals = ["meter", "attr1_val", "attr2_val"]

    for outFile in ["saved_swarm.h5", "saved_swarm_variable3.h5"]:
        with h5py.File(outFile) as f:
            for ix, attr in enumerate(attrs_list):
                if attr not in f.attrs:
                    raise RuntimeError("Attribute {} does not appear to have been saved to {}".format(attr, outFile))
                if f.attrs[attr] != attrs_vals[ix]:
                    raise RuntimeError('Expecting value {} for attribute {} in {}, got {} instead'.format(attrs_vals[ix], attr, outFile, f.attrs[attr]))
    
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
    
    clone_svar3 = clone_swarm.add_variable("double",2)
    clone_svar3.load("saved_swarm_variable3.h5")
    with h5py.File("saved_swarm_variable3.h5") as f:
        fact = dimensionalise(1.0, units=u.Quantity(f.attrs["units"]).units).magnitude
    if np.allclose(clone_swarm.particleCoordinates.data * fact, clone_svar3.data) != True:
        raise RuntimeError("Loaded swarm variable1 does not appear to contain the correct data.")

    # Clean up:
    if uw.mpi.rank == 0: 
        import os
        os.remove( "saved_swarm.h5" )
        os.remove( "saved_swarm_variable1.h5" )
        os.remove( "saved_swarm_variable2.h5" )
        os.remove( "saved_swarm_variable3.h5" )


if __name__ == '__main__':
    import underworld as uw
    uw.utils._io.PATTERN=1 # sequential
    swarm_save_load('global')
    swarm_save_load('passivetracer')
    uw.utils._io.PATTERN=2 # collective
    swarm_save_load('global')
    swarm_save_load('passivetracer')
