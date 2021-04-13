#!/bin/bash

usage="
Usage:
  A script to install and run an Underworld software stack.

** To install **
Review script details: modules, paths, repository urls / branches etc.
 $ source <this_script_name>
 $ install_full_stack

** To run **
Add the line
 source /path/gadi.pbs
to your pbs file, make sure to include the absolute path.
"

while getopts ':h' option; do
  case "$option" in
    h) echo "$usage"
       # safe script exit for sourced script
       (return 0 2>/dev/null) && return 0 || exit 0
       ;;
    \?) # incorrect options
       echo "Error: Incorrect options"
       echo "$usage"
       (return 0 2>/dev/null) && return 0 || exit 0
       ;;
  esac
done


module purge
module load openmpi/4.0.3 hdf5/1.10.5p python3/3.7.4 petsc/3.12.2 

# set all compiler wrappers to mpicc/mpicxx
export CC='mpicc' CXX='mpicxx' F90='mpifort' F77='mpifort' LD='mpicc' LDSHARED='mpicc -shared'

export GROUP=m18
export USER=
export INSTALL_NAME=UWGeodynamics_2.10.2

export CODES_PATH=/g/data/$GROUP/$USER/codes/
export UW_OPT_DIR=$CODES_PATH/opt
export INSTALL_PATH=$CODES_PATH/$INSTALL_NAME

export OPENBLAS_NUM_THREADS=1 # disable numpy interal parallelisation
export OMPI_MCA_io=ompio      # preferred MPI IO implementation

# All above environment variables are require for the runtime environment
# Our practice is to add this script to gadi so users can `source` this file
# with the specific envrionment configuration

export CDIR=$PWD

#export SWIG_VERSION=3.0.12
#export SWIG_PATH=$UW_OPT_DIR/swig-$SWIG_VERSION
#export PATH=$SWIG_PATH/bin:$PATH

## disable when using the hdf5 modules ##
#export HDF5_VERSION=1.10.7
#export HDF5_DIR=$UW_OPT_DIR/hdf5-$HDF5_VERSION
#export PATH=$HDF5_DIR/bin:$PATH

install_swig() {
	tmp_dir=$(mktemp -d -t ci-XXXXXXXXXX)
	cd $tmp_dir
	wget http://prdownloads.sourceforge.net/swig/swig-$SWIG_VERSION.tar.gz
	tar -xvzf swig-$SWIG_VERSION.tar.gz
	cd swig-$SWIG_VERSION
	./configure --prefix=$SWIG_PATH
	make
	make install
	rm -rf $tmp_dir
	cd $CDIR
}


install_hdf5() {
	source $INSTALL_PATH/bin/activate
	tmp_dir=$(mktemp -d -t ci-XXXXXXXXXX)
	cd $tmp_dir
	wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/hdf5-$HDF5_VERSION/src/hdf5-$HDF5_VERSION.tar.gz
	tar -zxf hdf5-$HDF5_VERSION.tar.gz
	cd hdf5-$HDF5_VERSION
	./configure --prefix=$HDF5_DIR --enable-parallel --enable-build-mode=production
	make ; make install
	rm -rf $tmp_dir
	cd $CDIR
}

install_petsc(){
	source $INSTALL_PATH/bin/activate
        export PETSC_CONFIGURE_OPTIONS="--with-debugging=0 \
                --COPTFLAGS='-O3' --CXXOPTFLAGS='-O3' --FOPTFLAGS='-O3' \
                --with-zlib=1                   \
                --with-hdf5-dir=$HDF5_DIR       \
                --download-mumps=1              \
                --download-parmetis=1           \
                --download-metis=1              \
                --download-superlu=1            \
                --download-hypre=1              \
                --download-scalapack=1          \
                --download-superlu_dist=1       \
                --useThreads=0                  \
                --download-superlu=1            \
                --with-shared-libraries         \
                --with-cxx-dialect=C++11        \
		--prefix=$UW_OPT_DIR/petsc-3.12.3 \
                --with-make-np=4"

       pip3 install --no-binary :all: --no-cache-dir --no-build-isolation petsc==3.12.3 -vvv
}

install_python_dependencies(){
	source $INSTALL_PATH/bin/activate
	# we're using the gadi system version of these
	#pip3 install --upgrade pip --no-binary :all: --no-cache-dir --no-build-isolation Cython numpy==1.18.4 scipy==1.4.1
	pip install --upgrade pip --no-binary :all:
	pip install --no-binary :all: --no-cache-dir --no-build-isolation mpi4py
	LV_OSMESA=1 pip3 install --no-binary :all: --no-cache-dir lavavu==1.4.14
	export HDF5_VERSION=1.10.5
        HDF5_MPI="ON" pip3 install --no-binary :all: --no-cache-dir --no-build-isolation h5py==2.10

}

install_underworld(){
	#export PETSC_DIR=$UW_OPT_DIR/petsc-3.12.3 # disable when using petsc module

	source $INSTALL_PATH/bin/activate 
	pip3 install --no-binary :all: --no-cache-dir --no-build-isolation  underworld==2.10.1b0

	# tmp_dir=$(mktemp -d -t ci-XXXXXXXXXX)
	# cd $tmp_dir
        # git clone -b v2.10.0b https://github.com/underworldcode/underworld2.git $tmp_dir
        # pip3 install .
        # rm -rf $tmp_dir	
	# cd $CDIR
}

install_uwgeodynamics(){
	source $INSTALL_PATH/bin/activate
	pip install --no-binary :all: --no-cache-dir uwgeodynamics==2.10.2

	# tmp_dir=$(mktemp -d -t ci-XXXXXXXXXX)
	# cd $tmp_dir
        # git clone -b v2.10.0 https://github.com/underworldcode/uwgeodynamics.git $tmp_dir
        # pip3 install .
        # rm -rf $tmp_dir	
	# cd $CDIR
}

check_underworld_exists(){
	source $INSTALL_PATH/bin/activate
	return $(python3 -c "import underworld") 
}

check_uwgeodynamics_exists(){
	source $INSTALL_PATH/bin/activate
	return $(python3 -c "import UWGeodynamics") 
}

check_badlands_exists(){
	source $INSTALL_PATH/bin/activate
	return $(python3 -c "import badlands") 
}

install_badlands(){
       source $INSTALL_PATH/bin/activate
       pip3 install --no-binary :all: --no-cache-dir --no-build-isolation badlands
}

install_full_stack(){
    if ! command -v swig 2>/dev/null; then
           install_swig  
    else
           echo "Found swig"
    fi
    
    install_python_dependencies
    
    if ! check_underworld_exists; then
          install_underworld
    fi
    
    if ! check_uwgeodynamics_exists; then
          install_uwgeodynamics
    fi
    
    if ! check_badlands_exists; then
          install_badlands
    fi

    cp $(basename $BASH_SOURCE) $INSTALL_PATH.sh
}


if [ ! -d "$INSTALL_PATH" ]
then
    echo "Environment not found, creating a new one"
    mkdir -p $INSTALL_PATH
    python3 --version
    python3 -m venv --system-site-packages $INSTALL_PATH
else
    echo "Found Environment"
    source $INSTALL_PATH/bin/activate
fi
