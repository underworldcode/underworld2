#!/bin/sh
# This script installs underworld on raijin.nci.org.au
# 
# Usage:
#  sh ./nci_raijin.sh <branch>
#
#  branch: 
#     for branch to checkout, ie 'master'(default), 'development', 'x.y.z'

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
RUN_MODS='pbs dot python3/3.6.7 openmpi/3.1.2 turbovnc/2.0.91'
module load hdf5/1.10.2p petsc/3.9.4 gcc/5.2.0 swig/3.0.12 scons/3.0.1 $RUN_MODS

# setup python environment with preinstalled packages
export PYTHONPATH=/projects/Underworld/opt/py3/:$PYTHONPATH
echo "*** New PYTHONPATH: $PYTHONPATH ***"

# build and install code
cd libUnderworld
CONFIG="./configure.py  --python-dir=`python3-config --prefix` --with-debugging=0"
echo "*** The module list is: ***"
module list -t
echo "*** The config line is: ***"
echo "$CONFIG"
$CONFIG
echo ""

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
echo "   export LD_PRELOAD=$OPENMPI_ROOT/lib/libmpi.so"
echo ""
echo "You will also need the following for         "
echo "glucifer output in your job script:          "
echo "  vncserver -localhost -nolisten tcp -depth 24 -geometry 1600x1000"
echo "  export DISPLAY=:1                          "
echo "#############################################"
