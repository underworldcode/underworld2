# setup the environment for building and running Underworld on raijin.nci.org 
module purge
module load dot pbs gcc/5.2.0 hdf5/1.8.14p python/2.7.11 python/2.7.11-matplotlib openmpi/1.8.4

# Petsc and python modules installed in shared space /project/m18/opt
#  'Local' pip used to install mpi4py
#  Swig, h5py and petsc-3.6.3 are all installed manually using the cluster environment as per the 'module load' above
#    Trick to installing swig. I had to create a directory called ‘tmp/’ in my home directory for its configure system to work.

export HDF5_VERSION=1.8.14
export PETSC_DIR=/projects/m18/opt/petsc-3.6.3
export PYTHONPATH=/projects/m18/opt:$PYTHONPATH
export PATH=/projects/m18/opt/swig-3.0.10/bin:$PATH

./configure.py --python-dir=$PYTHON_ROOT
./compile.py

export PYTHONPATH=$PYTHONPATH:/path-to-Underworld2/underworld2
