#!/bin/sh
# This script installs underworld on raijin.nci.org.au
# 
# Usage:
#  sh ./nci_raijin.sh <branch>
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
RUN_MODS='pbs dot mpi4py/3.0.2-py36-ompi3'
module load hdf5/1.10.2p petsc/3.9.4 gcc/5.2.0 mesa/11.2.2 swig/3.0.12 scons/3.0.1 $RUN_MODS
echo "*** The module list is: ***"
module list -t

# setup python environment with preinstalled packages (h5py, lavavu, pint)
export PYTHONPATH=/apps/underworld/opt/h5py/2.9.0-py36-ompi3/lib/python3.6/site-packages/h5py-2.9.0-py3.6-linux-x86_64.egg/:/apps/underworld/opt/lavavu/1.4.1_rc/:/apps/underworld/opt/pint/0.9_py36/lib/python3.6/site-packages/:$PYTHONPATH
echo "*** New PYTHONPATH: $PYTHONPATH ***"

# build and install code
cd underworld/libUnderworld
CONFIG="./configure.py  --python-dir=`python3-config --prefix` --with-debugging=0"
echo "*** The config line is: ***"
echo "$CONFIG"
echo ""

$CONFIG
./compile.py -j4
cd ../.. ; source updatePyPath.sh 

# some messages
echo "#####################################################################"
echo "Underworld2 built successfully at:                                   "
echo "  $UW_DIR                                                            "
echo "Remember to set the required environment before running Underworld2. "
echo "   module load $RUN_MODS                                             "
echo "   export PYTHONPATH=$UW_DIR:$PYTHONPATH                             "
echo "   export LD_PRELOAD=$OPENMPI_ROOT/lib/libmpi.so                     "
echo "#####################################################################"
