#!/bin/sh
# This script installs underworld on gadi.nci.org.au
# Note, swig will need to be installed and in your path.
# Also, swig4 doesn't seem to work, so use swig3. 
#
#
# Usage:
#  sh ./nci_gadi.sh <branch>
#
#  branch (optional): 
#     branch name to checkout, i.e. 'master'(default), 'development', 'x.y.z'


# exit when any command fails
set -e

UW_DIR=`pwd`/underworld
if [ ! -d "$UW_DIR" ] ; then
    git clone -q https://github.com/underworldcode/underworld2.git $UW_DIR
fi
cd $UW_DIR
git checkout $1  # checkout the requested version

# setup modules
module purge
RUN_MODS='pbs openmpi/4.0.2 hdf5/1.10.5p python3/3.7.4 petsc/3.12.2'
module load scons/3.1.1 $RUN_MODS
echo "*** The module list is: ***"
module list -t

# The following are probably necessary, as via the hdf5 module it is possible
# to suck in ompi3 libraries instead of the required ompi4 libs. 
export LD_PRELOAD=/apps/openmpi-mofed4.7-pbs19.2/4.0.2/lib/libmpi_usempif08_GNU.so.40:/apps/openmpi-mofed4.7-pbs19.2/4.0.2/lib/libmpi_usempi_ignore_tkr_GNU.so.40:/apps/openmpi-mofed4.7-pbs19.2/4.0.2/lib/libmpi_cxx.so.40

pip3 install --user mpi4py

export OMPI_MCA_io=ompio
export HDF5_VERSION=1.10.5
CC=h5pcc HDF5_MPI="ON" pip3 install --user --no-cache-dir --global-option=build_ext --global-option="-L/apps/hdf5/1.10.5p/lib/ompi3/" --no-binary=h5py h5py

# lavavu not supported on Gadi currently. 
#pip3 install --user lavavu

# build and install underworld
pip3 install --user -vvv .

# some messages
echo "#####################################################################"
echo "Underworld2 built                                                    "
echo "Remember to set the required environment before running Underworld2. "
echo "   module load $RUN_MODS                                             "
echo "You will also need to set the following environment variables:       "
echo "   export OMPI_MCA_io=ompio                                          "
echo "   export LD_PRELOAD=${LD_PRELOAD}:/apps/openmpi-mofed4.7-pbs19.2/4.0.2/lib/libmpi_usempif08_GNU.so.40"
echo "   export LD_PRELOAD=${LD_PRELOAD}:/apps/openmpi-mofed4.7-pbs19.2/4.0.2/lib/libmpi_usempi_ignore_tkr_GNU.so.40"
echo "   export LD_PRELOAD=${LD_PRELOAD}:/apps/openmpi-mofed4.7-pbs19.2/4.0.2/lib/libmpi_cxx.so.40"
echo "#####################################################################"
