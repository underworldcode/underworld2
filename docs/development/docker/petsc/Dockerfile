FROM ubuntu:22.04 as base_runtime
LABEL maintainer="https://github.com/underworldcode/"
ENV LANG=C.UTF-8
ENV PYVER=3.10
# Setup some things in anticipation of virtualenvs
ENV VIRTUAL_ENV=/opt/venv
# The following ensures that the venv takes precedence if available
ENV PATH=${VIRTUAL_ENV}/bin:$PATH

# The following ensures venv packages are available when using system python (such as from jupyter)
ENV PYTHONPATH=${PYTHONPATH}:${VIRTUAL_ENV}/lib/python${PYVER}/site-packages
# add joyvan user, volume mount and expose port 8888
EXPOSE 8888
ENV NB_USER jovyan
ENV NB_WORK /home/$NB_USER
RUN useradd -m -s /bin/bash -N $NB_USER -g users \
&&  mkdir -p /$NB_WORK/workspace \
&&  chown -R $NB_USER:users $NB_WORK
VOLUME $NB_WORK/workspace

# make virtualenv directory and set permissions 
RUN mkdir ${VIRTUAL_ENV} \
&&  chmod ugo+rwx ${VIRTUAL_ENV}

# install runtime requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        python3-minimal \
        python3-virtualenv \
        python3-pip \
        python3-numpy \
        libopenblas-base \
        gcc python3-dev \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*

RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        jupyter \
        ipyparallel 

FROM base_runtime AS build_base
# install build requirements
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        cmake \
        wget \
        gfortran \
        python3-dev \
        libopenblas-dev \
        libz-dev \
        cmake \
        git
# build mpi
WORKDIR /tmp/mpich-build
ARG MPICH_VERSION="4.0.2"
RUN wget http://www.mpich.org/static/downloads/${MPICH_VERSION}/mpich-${MPICH_VERSION}.tar.gz 
RUN  tar xvzf mpich-${MPICH_VERSION}.tar.gz 
WORKDIR /tmp/mpich-build/mpich-${MPICH_VERSION}              
ARG MPICH_CONFIGURE_OPTIONS="--prefix=/usr/local --enable-g=option=none --disable-debuginfo --enable-fast=O3,ndebug --without-timing --without-mpit-pvars --with-device=ch3 FFLAGS=-fallow-argument-mismatch FCFLAGS=-fallow-argument-mismatch"
ARG MPICH_MAKE_OPTIONS="-j8"
RUN ./configure ${MPICH_CONFIGURE_OPTIONS} 
RUN make ${MPICH_MAKE_OPTIONS}             
RUN make install                           
RUN ldconfig

# create venv now for forthcoming python packages
USER $NB_USER
RUN /usr/bin/python3 -m virtualenv --system-site-packages --python=/usr/bin/python3 ${VIRTUAL_ENV}

USER root
# build petsc
WORKDIR /tmp/petsc-build
ARG PETSC_VERSION="3.17.1"
RUN wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-${PETSC_VERSION}.tar.gz
RUN tar zxf petsc-lite-${PETSC_VERSION}.tar.gz
WORKDIR /tmp/petsc-build/petsc-${PETSC_VERSION}
RUN python3 ./configure --with-debugging=0 --prefix=/usr/local \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --with-petsc4py=1               \
                --with-zlib=1                   \
                --download-hdf5=1               \
                --download-mumps=1              \
                --download-parmetis=1           \
                --download-metis=1              \
                --download-superlu=1            \
                --download-hypre=1              \
                --download-scalapack=1          \
                --download-superlu_dist=1       \
                --download-ctetgen              \
                --download-eigen                \
                --download-triangle             \
                --useThreads=0                  \
                --download-superlu=1            \
                --with-shared-libraries         \
                --with-cxx-dialect=C++11        \
                --with-make-np=8
RUN make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt all
RUN make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt install
# these aren't needed
RUN rm -fr /usr/local/share/petsc 
# build petsc4py
ENV PETSC_DIR=/usr/local
USER $NB_USER
RUN CC=h5pcc HDF5_MPI="ON" HDF5_DIR=${PETSC_DIR} pip3 install --no-cache-dir --no-binary=h5py h5py

FROM base_runtime AS minimal
COPY --from=build_base $VIRTUAL_ENV $VIRTUAL_ENV
COPY --from=build_base /usr/local /usr/local
# Record Python packages, but only record system packages! 
# Not venv packages, which will be copied directly in.
RUN PYTHONPATH= /usr/bin/pip3 freeze >/opt/requirements.txt
# Record manually install apt packages.
RUN apt-mark showmanual >/opt/installed.txt

# Add user environment
FROM minimal
USER $NB_USER
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]