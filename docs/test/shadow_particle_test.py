import underworld as uw
import numpy as np

mesh = uw.mesh.FeMesh_Cartesian(elementRes=(128,128))

swarm = uw.swarm.Swarm(mesh)

# create some variables to track
origOwningEl = swarm.add_variable('int',1)
origCreatingProc = swarm.add_variable('int',1)
origParticleIndex = swarm.add_variable('int',1)
randomNumber = swarm.add_variable('int',1)

swarm.populate_using_layout(uw.swarm.layouts.PerCellSpaceFillerLayout(swarm,20))

# init variables
origOwningEl.data[:] = mesh.data_elgId[swarm.owningCell.data[:,0]] # global elementId where created
origCreatingProc.data[:] = uw.mpi.rank                               # rank where created
origParticleIndex.data[:,0] = range(swarm.particleLocalCount)      # local index where created
from random import randint
for index in range(0,swarm.particleLocalCount):                    # add random numbers to this variable
    randomNumber.data[index] = randint(0,9999999)


# get max local particlecount across all procs
from mpi4py import MPI
import numpy as np
comm = MPI.COMM_WORLD
inguy = np.zeros(1)
outguy = np.zeros(1)
inguy[:] = swarm.particleLocalCount
comm.Allreduce(inguy, outguy, op=MPI.MAX)

# create h5 array for players to write primary data into
import h5py
f = h5py.File('primarydata.hdf5', 'w', driver='mpio', comm=MPI.COMM_WORLD)
dset_data = f.create_dataset('randomdata', (comm.Get_size(),outguy[0]), dtype='i')
# write primary data parallel array
dset_data[uw.mpi.rank,origParticleIndex.data[:,0]] = randomNumber.data[:,0]

# also create one to write particle element counts
dset_counts = f.create_dataset('counts', (mesh.elementsGlobal,), dtype='i')
# get counts
el_index, counts = np.unique(origOwningEl.data[:,0],return_counts=True)
for element_gId, el_count in zip (el_index,counts):
    dset_counts[element_gId] = el_count


if len(origCreatingProc.data_shadow) != 0:
    raise RuntimeError("The shadow data should be empty at this stage, but isn't. Hmm...")

# get shadow particles!!
swarm.shadow_particles_fetch()

if len(origCreatingProc.data_shadow) == 0 and (uw.mpi.size>1):
    raise RuntimeError("The shadow data should be populated at this stage, but isn't. Hmm...")


# now check that communicated particles contain required data.
# first create local numpy copies of primary data in memory,
# as h5py has limitations in the way you can index its arrays
dset_numpy_data = np.array(dset_data)
if not (dset_numpy_data[origCreatingProc.data_shadow[:,0], origParticleIndex.data_shadow[:,0]] == randomNumber.data_shadow[:,0]).all():
    raise RuntimeError("Shadow particle data does not appear to be correct.")

# also check that we have the correct particle counts
# get counts
el_index, counts = np.unique(origOwningEl.data_shadow[:,0],return_counts=True)
# again create copy for indexing ease
dset_numpy_counts = np.array(dset_counts)
if not (dset_numpy_counts[el_index] == counts[:]).all():
    raise RuntimeError("Shadow data particle counts do not appear to be correct.")


# close and cleaup
f.close()
import os
if uw.mpi.rank==0:
    os.remove('primarydata.hdf5')

