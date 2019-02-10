#!/bin/sh
# This script installs underworld on raijin.nci.org.au
# 
# Usage:
#  ./nci_raijin.sh <branch>
#
#  branch: 
#     for branch to checkout, ie 'master'(default), 'development', 'x.y.z'
#
# Tested successfully 30/1/2018
#

# exit when any command fails
set -e

# DATE=`date +%d%b%Y` # could be used to date checkout eg,
# UW_DIR=`pwd`/underworld-$DATE
UW_DIR=`pwd`/underworld
git clone https://github.com/underworldcode/underworld2.git $UW_DIR
cd $UW_DIR
git checkout $1  # checkout the requested version

# setup modules
module purge
RUN_MODS='pbs dot python/2.7.11 python/2.7.11-matplotlib openmpi/3.1.2 mpi4py/3.0.0-py2-ompi3'
module load hdf5/1.10.2p petsc/3.9.4 gcc/5.2.0 swig/3.0.12 $RUN_MODS

# setup environment
H5PY_DIR=/apps/underworld/opt/h5py/2.8.0-python_2.7/lib/python2.7/site-packages
LAVU_DIR=/apps/underworld/opt/lavavu/
MYPYPGK=$H5PY:$LAVU_DIR
export PYTHONPATH=$MYPYPG:$PYTHONPATH

# build and install code
cd libUnderworld
./configure.py --python-dir=$PYTHON_ROOT --with-debugging=0
./compile.py -j4
cd .. ; source updatePyPath.sh 

# some messages
echo "#############################################"
echo "Underworld2 built successfully.              "
echo "Remember to set the required environment     "
echo "before running Underworld2.                  "
echo "   source $UW_DIR/updatePyPath.sh            "
echo "   module load $RUN_MODS                     "
echo "   export PYTHONPATH=$MYPYPG:$PYTHONPATH     "
echo ""
echo "You will also need the following for         "
echo "glucifer output in your job script:          "
echo "  Xvfb :0 -screen 0 1600x1200x16&            "
echo "#############################################"
