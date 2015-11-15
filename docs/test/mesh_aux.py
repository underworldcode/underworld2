'''
This script contains auxiliary mesh related tests. 
'''
import underworld as uw



# Simple test to check non-parallel decomposed meshing is working.
# Note that we are only really testing this parameter when we
# run this test in parallel.
mesh1 = uw.mesh.FeMesh_Cartesian(elementRes=(4,4),partitioned=False)

if len(mesh1.data) != 25:
    raise RuntimeError("A non-partitioned mesh should report identical vertex count \
                        independent of processor count.")
