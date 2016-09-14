#!/bin/sh

# This script builds underworld2 on stampede
# To select the required branch to build, just specify the branch name:
#   $./xsede_stampede development
# Defaults to master branch.

# Tested successfully 14/9/2016

# ensure we bail on errors
set -e

# modules
module purge
module load intel/15.0.2
module load mvapich2/2.1
module load xalt/0.6
module load TACC
module load petsc/3.6
module load phdf5
module load swig
module load python/2.7.9
export HDF5_DIR=$TACC_HDF5_DIR

# underworld
git clone https://github.com/underworldcode/underworld2.git
cd underworld2/libUnderworld
git checkout $1
./configure.py --with-debugging=0 --python-dir=$TACC_PYTHON_DIR --cxxflags='-std=c++11\ -cxxlib=/opt/apps/gcc/4.9.1/' --numpy-dir=/opt/apps/intel15/python/2.7.9/lib/python2.7/site-packages/numpy/core --with-graphics=0
./compile.py

# some messages
echo "#############################################"
echo "Underworld2 built successfully.              "
echo "Remember to load necessary modules:           "
echo "   module load intel/15.0.2"
echo "   module load mvapich2/2.1"
echo "   module load xalt/0.6"
echo "   module load TACC"
echo "   module load petsc/3.6"
echo "   module load phdf5"
echo "   module load swig"
echo "   module load python/2.7.9 "
echo ""
echo "You will also want to set set the required   "
echo "environment variables:"
echo "   export PYTHONPATH=PATH_TO_YOUR_UW2_INSTALL:\$PYTHONPATH"
echo "   export LD_LIBRARY_PATH=/opt/apps/gcc/4.9.1/lib64:/opt/apps/gcc/4.9.1/lib:\$LD_LIBRARY_PATH"
echo "#############################################"
