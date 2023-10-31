#!/bin/bash -l
 
## User required input
#SBATCH --account=pawsey0407
#SBATCH --job-name=bobthejob
#SBATCH --ntasks=3
#SBATCH --time=00:20:00

## Setup job conditions and run environment
#SBATCH --ntasks-per-node=64  # found this is needed ~Apr2023
#SBATCH --cpus-per-task=1     # OMP_NUM_THREADS equivalent

# Note we avoid any inadvertent OpenMP threading by setting
export OMP_NUM_THREADS=1

# load system packages: py39, mpi, hdf5 
module load python/3.9.15 py-mpi4py/3.1.2-py3.9.15 py-numpy/1.20.3 py-h5py/3.4.0 py-cython/0.29.24 cmake/3.21.4

# add custom virtual environment and underworld
export OPT_DIR=/software/projects/pawsey0407/setonix/
export PYTHONPATH=$OPT_DIR/py39/lib/python3.9/site-packages/:$PYTHONPATH
export PYTHONPATH=$OPT_DIR/underworld/2.14.2/lib/python3.9/site-packages:$PYTHONPATH

# load custom petsc
export PETSC_DIR=$OPT_DIR/petsc-3.19.0
export PYTHONPATH=$PETSC_DIR/lib:$PYTHONPATH

## model name and execution
export model="mymod.py"
srun -n ${SLURM_NTASKS} python3 $model
