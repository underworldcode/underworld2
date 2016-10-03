# Magnus installation guide 
git clone git clone http://github.com/underworldcode/underworld2.git your_uw_directory 
cd your_uw_directory

# get subrepos now. normally this is executed automatically when you running
# `configure.py`, but we'll be running `configure.py` on an interactive node,
# and it appears the network is limited, so do this now instead.  
git submodule update --init

# now login to interactive queue & rerun configure 
salloc -p debugq --nodes=1

# Environment
# setup the environment for building and running Underworld on magnus.pawsey.org.au 
module swap PrgEnv-cray PrgEnv-gnu
module load cray-hdf5-parallel/1.8.13 python/2.7.10 swig numpy scipy cython mpi4py cmake pkgconfig  

# python things
export PYTHONPATH=/ivec/cle52/magnus/python/2.7.6/numpy/1.6.2/lib/python2.7/site-packages:$PYTHONPATH
export PYTHONPATH=$PYTHONPATH:/ivec/cle52/magnus/python/2.7.6/six/1.9.0/lib/python2.7/site-packages/

# Petsc installed in shared space /group/m18/Apps
export MPI_DIR=$CRAY_MPICH2_DIR
export PETSC_DIR=/group/m18/Apps/petsc-3.6.3
export PETSC_ARCH=arch-linux2-c-opt

# Configure
cd libUnderworld
aprun -n 1 -N 1 python configure.py --with-graphics=0 --with-debugging=0
 
# Compile
aprun -n 1 -N 1 python compile.py

# Test
cd ..
export PYTHONPATH=$PYTHONPATH:$PWD
cd utils
aprun -n 1 -N 1 