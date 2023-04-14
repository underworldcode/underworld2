#!/bin/bash -l

# The following assumes read access to 
#   /software/projects/pawsey0407/setonix/

module load python/3.9.15 py-mpi4py/3.1.2-py3.9.15 py-numpy/1.20.3 py-h5py/3.4.0 py-cython/0.29.24

export OPT_DIR=/software/projects/pawsey0407/setonix/

## for modifying the venv
# source $OPT_DIR/py39/bin/activate

## For development only
# export PETSC_DIR=$OPT_DIR/petsc-3.18.1
# export PYTHONPATH=$PETSC_DIR/:$PYTHONPATH

# for using the venv
export PYTHONPATH=$OPT_DIR/py39/lib/python3.9/site-packages:$PYTHONPATH
export PYTHONPATH=$OPT_DIR/underworld/2.14.2/lib/python3.9/site-packages/:$PYTHONPATH
