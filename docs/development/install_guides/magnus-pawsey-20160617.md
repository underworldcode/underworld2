# Magnus installation guide 

# Environment
# setup the environment for building and running Underworld on magnus.pawsey.org.au 
module swap PrgEnv-cray PrgEnv-gnu
module load cray-hdf5-parallel python/2.7.10 swig numpyi scipy cython mpi4py cmake pkgconfig  

# Petsc installed in shared space /group/m18/Apps
export MPI_DIR=$CRAY_MPICH2_DIR
export PETSC_DIR=/group/m18/Apps/petsc-3.6.3
export PETSC_ARCH=arch-linux2-c-opt

# Configure
# login to interactive queue & configure
salloc -p debugq --nodes=1
cd libUnderworld
aprun -n 1 -N 1 python configure.py
 
# If you have trouble cloning LavaVu or h5py_ext run 
# ./configure.py without aprun, then repeat the aprun configure

# Compile
aprun -n 1 -N 1 python compile.py

# Test
export PYTHONPATH=$PYTHONPATH:/path-to-Underworld2/underworld2
aprun python -c "import underworld as uw ; mesh = uw.mesh.FeMesh_Cartesian()"
