FROM debian:latest as base_runtime
ENV LANG=C.UTF-8
# Put this here in anticipation of virtualenvs
ENV VIRTUAL_ENV=/opt/venv
ENV PATH=${VIRTUAL_ENV}/bin:$PATH
ENV PYTHONPATH=$PYTHONPATH:/usr/lib/python3.7/site-packages
# install runtime requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        python3-minimal \
        python3-virtualenv \
        python3-pip \
        libopenblas-base
# Install setuptools and wheel first, needed by plotly
RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        numpy \
        jupyter \
        ipyparallel 

# install build requirements
FROM base_runtime AS build_base
RUN python3 -m virtualenv --python=/usr/bin/python3 ${VIRTUAL_ENV} 
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        cmake \
        wget \
        gfortran \
        python3-dev \
        libopenblas-dev \
        cmake \ 
        libz-dev
# build mpi
WORKDIR /tmp/mpich-build
ARG MPICH_VERSION="3.1.4"
RUN wget http://www.mpich.org/static/downloads/${MPICH_VERSION}/mpich-${MPICH_VERSION}.tar.gz 
RUN  tar xvzf mpich-${MPICH_VERSION}.tar.gz 
WORKDIR /tmp/mpich-build/mpich-${MPICH_VERSION}              
ARG MPICH_CONFIGURE_OPTIONS="--prefix=/usr/local --enable-g=option=none --disable-debuginfo --enable-fast=O3,ndebug --without-timing --without-mpit-pvars"
ARG MPICH_MAKE_OPTIONS="-j8"
RUN ./configure ${MPICH_CONFIGURE_OPTIONS} 
RUN make ${MPICH_MAKE_OPTIONS}             
RUN make install                           
RUN ldconfig
RUN pip3 install --no-cache-dir mpi4py
# build petsc
WORKDIR /tmp/petsc-build
ARG PETSC_VERSION="3.12.1"
RUN wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-${PETSC_VERSION}.tar.gz
RUN tar zxf petsc-lite-${PETSC_VERSION}.tar.gz
WORKDIR /tmp/petsc-build/petsc-${PETSC_VERSION}
RUN python3 ./configure --with-debugging=0 --prefix=/usr/local \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --with-zlib=1                   \
                --download-hdf5=1               \
                --download-mumps=1              \
                --download-parmetis=1           \
                --download-metis=1              \
                --download-superlu=1            \
                # --download-hypre=1              \ # issue with this.. something about difficulty inferring version number
                --download-scalapack=1          \
                --download-superlu_dist=1       \
                --useThreads=0                  \
                --download-superlu=1            \
                --with-shared-libraries         \
                --with-cxx-dialect=C++11        \
                --with-make-np=8
RUN make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt all
RUN make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt install
RUN pip3 install --no-cache-dir petsc4py
# I don't think the petsc py package is needed. 
RUN pip3 uninstall -y petsc  
RUN CC=h5pcc HDF5_MPI="ON" HDF5_DIR=${PETSC_DIR} pip3 install --no-cache-dir --no-binary=h5py h5py

FROM base_runtime AS minimal
COPY --from=build_base $VIRTUAL_ENV $VIRTUAL_ENV
COPY --from=build_base /usr/local /usr/local
# Record Python packages, but only record system packages! 
# Not venv packages, which will be copied directly in.
RUN /usr/bin/pip3 freeze >/opt/requirements.txt
# Record manually install apt packages.
RUN apt-mark showmanual >/opt/installed.txt

FROM minimal
EXPOSE 8888
RUN ipcluster nbextension enable
ADD https://github.com/krallin/tini/releases/download/v0.18.0/tini /tini
RUN chmod +x /tini
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan
USER $NB_USER
ENV NB_WORK /home/$NB_USER
RUN ipython profile create --parallel --profile=mpi && \
    echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> $NB_WORK/.ipython/profile_mpi/ipcluster_config.py
VOLUME $NB_WORK/workspace
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]