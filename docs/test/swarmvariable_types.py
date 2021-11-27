# ---
# jupyter:
#   jupytext:
#     text_representation:
#       extension: .py
#       format_name: light
#       format_version: '1.5'
#       jupytext_version: 1.13.1
#   kernelspec:
#     display_name: Python 3
#     language: python
#     name: python3
# ---

import underworld as uw
import numpy as np

mesh = uw.mesh.FeMesh_Cartesian()

swarm = uw.swarm.Swarm(mesh)
def addvars(swarm_inst):
    return (
            ( swarm_inst.add_variable('char'  ,1), np.iinfo(np.int8).max    , 'char'   ),
            ( swarm_inst.add_variable('short' ,1), np.iinfo(np.int16).max   , 'short'  ),
            ( swarm_inst.add_variable('int'   ,1), np.iinfo(np.int32).max   , 'int'    ),
            ( swarm_inst.add_variable('long'  ,1), np.iinfo(np.int64).max   , 'long'   ),
            ( swarm_inst.add_variable('float' ,1), np.finfo(np.float32).max , 'float'  ),
            ( swarm_inst.add_variable('double',1), np.finfo(np.float64).max , 'double' ) )
vars = addvars(swarm)

# add particles
swarm.populate_using_layout(uw.swarm.layouts.PerCellSpaceFillerLayout(swarm,20))
# initialise
for var in vars:
    var[0].data[:] = var[1]

# test evaluation
for var in vars:
    res = var[0].evaluate( (0.5,0.5) )
    if not (np.allclose( res, var[1])):
        strguy  = "There was an error evaluating a swarm variable of type '{}'.\n".format(var[2])
        strguy += "Expected value was '{}', but encountered value was '{}'".format(res,var[1])
        raise RuntimeError(strguy)

# test checkpointing
# first dump data to disk
swarm.save("test_swarm.h5")
for var in vars:
    var[0].save("test_swarvar_" + var[2] + ".h5")

# now create swarm which we will use to clone the original swarm
swarm2 = uw.swarm.Swarm(mesh)
vars2 = addvars(swarm2)
# load saved data onto new swarm
swarm2.load("test_swarm.h5")
for var in vars2:
    var[0].load("test_swarvar_" + var[2] + ".h5")

# now check data on new swarm is identical to old
for var in vars2:
    if not np.allclose(var[0].data, var[1] ):
        raise RuntimeError("Something went wrong with swarm variable object.")

# cleanup
if uw.mpi.rank == 0: 
    import os
    os.remove( "test_swarm.h5" )
    for var in vars:
        os.remove( "test_swarvar_" + var[2] + ".h5" ) 
