'''
This script contains auxiliary mesh related tests. 
'''
import underworld as uw
import numpy as np



# Simple test to check non-parallel decomposed meshing is working.
# Note that we are only really testing this parameter when we
# run this test in parallel.
mesh1 = uw.mesh.FeMesh_Cartesian(elementRes=(4,4),partitioned=False)

if len(mesh1.data) != 25:
    raise RuntimeError("A non-partitioned mesh should report identical vertex count \
                        independent of processor count.")

mesh2 = uw.mesh.FeMesh_Cartesian(elementRes=(10,10),partitioned=True)
var   = uw.mesh.MeshVariable( mesh2, nodeDofCount=2 )
var2   = uw.mesh.MeshVariable( mesh2, nodeDofCount=2 )

for ind, coord in enumerate(mesh2.data):
    var.data[ind] = [coord[1]+5., coord[0]*-2.]

var.save('temp.h5')
var2.load('temp.h5')

checkpoint_pass = np.allclose(var.data, var2.data)

if( not checkpoint_pass ):
  raise RuntimeError("Error in a checkpoint test")
