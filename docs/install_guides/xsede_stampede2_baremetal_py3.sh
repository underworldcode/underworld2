#!/bin/sh

# This script is for UW versions 2.8.0b and newer, as these versions 
# of UW used Python3. 
# Don't forget to update your PYTHONPATH as necessary after the build is 
# complete. You can do this by running `source updatePyPath.sh` from the 
# top level UW directory.

git clone https://github.com/underworldcode/underworld2.git
cd underworld2/libUnderworld 
module load petsc/3.11-nohdf5
module load phdf5/1.10.4
module load python3
pip3 install --user scons
pip3 install --user pint
export PATH=$PATH:$HOME/.local/bin
export HDF5_DIR=/opt/apps/intel18/impi18_0/phdf5/1.10.4/x86_64
export PYTHONPATH=$HOME/.local/lib/python3.7/site-packages:$PYTHONPATH
CC=h5pcc HDF5_MPI="ON" pip3 install --no-binary=h5py --user --force h5py
./configure.py --python-dir=/opt/apps/intel18/python3/3.7.0 --with-debugging=0
module load swig
./compile.py
