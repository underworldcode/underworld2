## Underworld install on Magnus 


1) Setup environment

module swap PrgEnv-cray PrgEnv-gnu
module load cray-hdf5
module load numpy/1.9.2
module load mercurial
export MPI_DIR=$CRAY_MPICH2_DIR

2) Install petsc

wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.6.1.tar.gz
tar zxf petsc-lite-3.6.1.tar.gz
cd petsc-3.6.1
./configure --with-mpi-dir=$MPI_DIR --download-fblaslapack=1
make
export PETSC_DIR=/home/yourhome/petsc-3.6.1

3) Install Underworld

cd ~
hg clone https://www.bitbucket.org/underworldproject/underworld2
hg up -C newInterface
cd underworld2/libUnderworld
module load python/2.7.6
./configure.py
./scons.py
