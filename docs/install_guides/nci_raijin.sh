#!/bin/sh
# This script installs underworld on raijin.nci.org.au
# 
# Usage:
#  ./nci_raijin.sh <branch>
#
#  branch: 
#     for branch to checkout, ie 'master'(default), 'development', 'x.y.z'
#
# Tested successfully 11/2/2019
#

# exit when any command fails
set -e

#DATE=`date +%d%b%Y` # could be used to date checkout eg,
#UW_DIR=`pwd`/underworld-$DATE
UW_DIR=`pwd`/underworld
git clone https://github.com/underworldcode/underworld2.git $UW_DIR
cd $UW_DIR
git checkout $1  # checkout the requested version

# setup modules
module purge
RUN_MODS='pbs dot python3/3.6.7 openmpi/3.1.2'
module load hdf5/1.10.2p petsc/3.9.4 gcc/5.2.0 swig/3.0.12 scons/3.0.1 $RUN_MODS

# TODO: Need raijin admin to install mpi4py & h5py
echo "*** INSTALLING MPI4PY, LAVAVU & H5PY LOCALLY ***"
pip3 install --user  mpi4py lavavu
CC="mpicc" HDF5_MPI="ON" HDF5_VERSION=1.10.2 pip3 install --user --no-binary=h5py h5py

# setup environment
#H5PY_DIR=/apps/underworld/opt/h5py/2.8.0-python_2.7/lib/python2.7/site-packages
#LAVU_DIR=/apps/underworld/opt/lavavu/1.2.55
#MYPYPGK=$H5PY_DIR:$LAVU_DIR
#PYTHONPATH=$MYPYPGK:$PYTHONPATH

# build and install code
cd libUnderworld
./configure.py  --python-dir=`python3-config --prefix` --with-debugging=0
./compile.py -j4
cd .. ; source updatePyPath.sh 

# some messages
echo "#############################################"
echo "Underworld2 built successfully at:           "
echo "  $UW_DIR                                    "
echo "Remember to set the required environment     "
echo "before running Underworld2.                  "
echo "   module load $RUN_MODS                     "
echo "   export PYTHONPATH=$PYTHONPATH             "
echo ""
echo "You will also need the following for         "
echo "glucifer output in your job script:          "
echo "  Xvfb :0 -screen 0 1600x1200x16&            "
echo "#############################################"
