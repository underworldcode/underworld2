#!/bin/sh
# This script installs underworld on gadi.nci.org.au
# Note, swig will need to be installed and in your path.
# Also, swig4 doesn't seem to work, so use swig3. 
# 
# Usage:
#  sh ./nci_gadi.sh <branch>
#
#  branch (optional): 
#     branch name to checkout, i.e. 'master'(default), 'development', 'x.y.z'


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
RUN_MODS='pbs openmpi/3.1.4 hdf5/1.10.5p python3/3.7.4 petsc/3.12.2'
module load scons/3.1.1 $RUN_MODS
echo "*** The module list is: ***"
module list -t

# build mpi4py
pip3 install --user mpi4py

# build h5py
export HDF5_VERSION=1.10.5
CC=h5pcc HDF5_MPI="ON" pip3 install --user --no-cache-dir --global-option=build_ext --global-option="-L/apps/hdf5/1.10.5p/lib/ompi3/" --no-binary=h5py h5py

# build and install code
cd underworld
pip3 install --user -vvv .

# some messages
echo "#####################################################################"
echo "Underworld2 built                                                    "
echo "Remember to set the required environment before running Underworld2. "
echo "   module load $RUN_MODS                                             "
echo "#####################################################################"
