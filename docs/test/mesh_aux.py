'''
This script contains auxiliary mesh related tests.

We test saving and loading of mesh and mesh variable objects.
We also tests creation/load/save of non-partitioned mesh and
variables. I'll leave this in for now, but I'm not sure if
load/save of non-partitioned mesh is useful or even a good
idea. Effectively, in parallel, we're saying that each process
has its own distinct mesh object, but then we're saving it to
a single global file. Sequentially this is fine (and each proc
*could* specify its own filename), but collectively this doesn't
make sense as we use COMM_WORLD but the mesh only is aware of
COMM_SELF. Note that the test passes, but I've disabled it in
any case as it doesn't make sense.
'''
import underworld as uw
import numpy as np
import h5py
from underworld.scaling import get_coefficients
from underworld.scaling import units as u
from underworld.scaling import non_dimensionalise
from underworld.scaling import dimensionalise

def meshtest(res, partitioned):
    mesh = uw.mesh.FeMesh_Cartesian(elementRes=(res,res),partitioned=partitioned)

    resp1 = res + 1
    if not partitioned:
        if (len(mesh.data) != (resp1*resp1)):
            raise RuntimeError("A non-partitioned mesh should report identical vertex count "\
                               "independent of processor count.")
    # test save/load of mesh
    with mesh.deform_mesh():
        mesh.data[:] *= 2.

    cpy = mesh.data.copy()
    mesh.save('temp.h5', units=u.m, attr1_name="attr1_val", attr2_name="attr2_val")

    # Check that swarm attributes are properly saved to the h5 file.
    attrs_list = ["units", "attr1_name", "attr2_name"]
    attrs_vals = ["meter", "attr1_val", "attr2_val"]
    with h5py.File("temp.h5") as f:
        for ix, attr in enumerate(attrs_list):
            if attr not in f.attrs:
                raise RuntimeError("Attribute {} does not appear to have been saved to {}".format(attr, outFile))
            if f.attrs[attr] != attrs_vals[ix]:
                raise RuntimeError('Expecting value {} for attribute {} in {}, got {} instead'.format(attrs_vals[ix], attr, outFile, f.attrs[attr]))

    mesh.reset()
    if np.allclose(mesh.data, cpy):
        raise RuntimeError("These arrays should be different.")
    mesh.load('temp.h5')
    if not np.allclose(mesh.data, cpy):
        raise RuntimeError("These arrays should be identical.")

    # test save/load of meshvariable
    var = uw.mesh.MeshVariable( mesh, nodeDofCount=2 )
    for ind, coord in enumerate(mesh.data):
        var.data[ind] = [coord[1]+5., coord[0]*-2.]

    var.syncronise()
    cpy = var.data.copy()
    var.save('temp2.h5')
    var.data[:] = 0.
    if np.allclose(var.data, cpy):
        raise RuntimeError("These arrays should be different.")
    var.load('temp2.h5')
    if not np.allclose(var.data, cpy):
        if uw.mpi.rank==0:
            print("VAR")
            print(var.data[:])
            print("CPY")
            print(cpy)
        raise RuntimeError("These arrays should be identical.")

if __name__ == '__main__':
    import underworld as uw
    uw.utils._io.PATTERN=1 # sequential
    meshtest(16,True)
    meshtest(8, True)  # second run to make sure we're deleting datasets where different sizes
    meshtest(16,False)
    uw.utils._io.PATTERN=2 # collective
    meshtest(16,True)
    meshtest(8, True)
    # meshtest(16,False)  # this isn't a good idea, so we shouldn't do it.
    if uw.mpi.rank==0:
        import os
        os.remove('temp.h5')
        os.remove('temp2.h5')