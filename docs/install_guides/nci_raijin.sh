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
RUN_MODS='pbs dot python/2.7.11 python/2.7.11-matplotlib openmpi/1.10.2 mpi4py/2.0.0'
module load gcc/5.2.0 hdf5/1.8.14p petsc/3.7.4 swig/3.0.12 $RUN_MODS # can use petsc/3.8.3 as well

# setup environment
H5PY_DIR=/apps/underworld/opt/h5py/2.7.1-python_2.7/lib/python2.7/site-packages/
export PYTHONPATH=$H5PY_DIR:$PYTHONPATH
export OPENGL_LIB=/apps/underworld/opt/mesa/17.1.5/build/linux-x86_64/gallium/targets/libgl-xlib/
export OPENGL_INC=/apps/underworld/opt/mesa/17.1.5/include/GL

# build and install code
cd libUnderworld
./configure.py --python-dir=$PYTHON_ROOT --with-debugging=0 --opengl-inc-dir=$OPENGL_INC --opengl-lib-dir=$OPENGL_LIB
./compile.py -j4
cd .. ; source updatePyPath.sh 

# some messages
echo "#############################################"
echo "Underworld2 built successfully.              "
echo "Remember to set the required environment     "
echo "before running Underworld2.                  "
echo "   source $UW_DIR/updatePyPath.sh            "
echo "   module load $RUN_MODS                     "
echo "   export PYTHONPATH=$H5PY_DIR:$PYTHONPATH   "
echo ""
echo "You will also need the following for         "
echo "glucifer output in your job script:          "
echo "  Xvfb :0 -screen 0 1600x1200x16&            "
echo "#############################################"
