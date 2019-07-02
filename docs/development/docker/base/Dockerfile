FROM debian:latest
MAINTAINER https://github.com/underworldcode/

# install things
RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        build-essential \
        git \
        python3-minimal \
        python3-dev \
        python3-pip \
        libxml2-dev \
        xorg-dev \
        ssh \
        curl \
        libfreetype6-dev \
        libpng-dev \
        libxft-dev \
        xvfb \
        freeglut3 \
        freeglut3-dev \
        libgl1-mesa-dri \
        libgl1-mesa-glx \
        python3-tk \
        rsync \
        vim \
        less \
        xauth \
        swig \
        gdb-minimal \
        python3-dbg \
        cmake \
        python3-setuptools \
        wget \
        gfortran  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ARG MPICH_VERSION="3.1.4"
ARG MPICH_CONFIGURE_OPTIONS="--enable-fast=all,O3 --prefix=/usr"
ARG MPICH_MAKE_OPTIONS="-j4"
WORKDIR /tmp/mpich-build
RUN wget http://www.mpich.org/static/downloads/${MPICH_VERSION}/mpich-${MPICH_VERSION}.tar.gz && \
    tar xvzf mpich-${MPICH_VERSION}.tar.gz && \
    cd mpich-${MPICH_VERSION}              && \
    ./configure ${MPICH_CONFIGURE_OPTIONS} && \
    make ${MPICH_MAKE_OPTIONS}             && \
    make install                           && \
    ldconfig                               && \
    cd /tmp                                && \
    rm -fr *

ENV LANG=C.UTF-8
# Install setuptools and wheel first, needed by plotly
RUN pip3 install -U setuptools  && \
    pip3 install -U wheel       && \
    pip3 install --no-cache-dir packaging \
        appdirs \
        numpy \
        jupyter \
        plotly \
        matplotlib \
        pillow \
        pyvirtualdisplay \
        ipython \
        ipyparallel \
        pint \
        sphinx \
        sphinx_rtd_theme \
        sphinxcontrib-napoleon \
        mock \
        scipy \ 
        tabulate \
        mpi4py \
        scons

# update python to py3
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.5 1

WORKDIR /tmp/petsc-build
ARG PETSC_VERSION="3.11.2"
RUN wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-${PETSC_VERSION}.tar.gz && \
    tar zxf petsc-lite-${PETSC_VERSION}.tar.gz && cd petsc-${PETSC_VERSION}   && \
    ./configure --with-debugging=0 --prefix=/usr/local/petsc                     \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --with-zlib=1                   \
                --download-fblaslapack=1        \
                --download-ctetgen=1            \
                --download-triangle=1           \
                --download-hdf5=1               \
                --download-mumps=1              \
                --download-parmetis=1           \
                --download-metis=1              \
                --download-superlu=1            \
                --download-hypre=1              \
                --download-scalapack=1          \
                --download-superlu_dist=1       \
                --useThreads=1                  \
                --download-superlu=1            \
                --with-shared-libraries         \
                --with-cxx-dialect=C++11     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt all     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt install && \
    make PETSC_DIR=/usr/local/petsc PETSC_ARCH="" test && \
    cd /tmp && \
    rm -fr *

ENV PETSC_DIR=/usr/local/petsc
ENV PATH=${PETSC_DIR}/bin:$PATH

# Hack for scons.. should be able to remove in future.
ENV SCONS_LIB_DIR /usr/local/lib/python3.5/dist-packages/scons-3.0.1

ENV PYTHONPATH $PYTHONPATH:/usr/lib
RUN CC=h5pcc HDF5_MPI="ON" HDF5_DIR=${PETSC_DIR} pip3 install --no-cache-dir --no-binary=h5py h5py && \
    pip3 install --no-cache-dir petsc4py

# expose notebook port
EXPOSE 8888

# Setup ipyparallel for mpi profile
RUN ipcluster nbextension enable

# script for xvfb-run.  all docker commands will effectively run under this via the entrypoint
RUN printf "#\041/bin/sh \n rm -f /tmp/.X99-lock && xvfb-run -s '-screen 0 1600x1200x16' \$@" >> /usr/local/bin/xvfbrun.sh && \
chmod +x /usr/local/bin/xvfbrun.sh

# Install Tini.. sedd https://github.com/ipython/ipython/issues/7062, https://github.com/jupyter/notebook/issues/334
ENV TINI_VERSION v0.18.0
ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini
ENTRYPOINT ["/tini", "--", "/usr/local/bin/xvfbrun.sh"]

# add default user jovyan and change permissions on NB_WORK
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan
USER $NB_USER
ENV NB_WORK /home/$NB_USER

RUN ipython profile create --parallel --profile=mpi && \
    echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> $NB_WORK/.ipython/profile_mpi/ipcluster_config.py

# copy this file over so that no password is required
COPY --chown=jovyan:users ./docs/development/docker/base/jupyter_notebook_config.json $NB_WORK/.jupyter/jupyter_notebook_config.json

# create a volume
VOLUME $NB_WORK/workspace
WORKDIR $NB_WORK

# launch notebook
CMD ["jupyter", "notebook", "--ip='0.0.0.0'", "--no-browser"]

