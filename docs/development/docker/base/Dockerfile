FROM debian:latest
MAINTAINER https://github.com/underworldcode/

# install things
RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        build-essential \
        git \
        python-minimal \
        python-dev \
        python-pip \
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
        python-tk \
        rsync \
        vim \
        less \
        xauth \
        swig \
        gdb-minimal \
        python2.7-dbg \
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


# Install setuptools and wheel first, needed by plotly
RUN pip install --no-cache-dir setuptools wheel && \
    pip install --no-cache-dir packaging \
        appdirs \
        numpy \
        jupyter \
        plotly \
        matplotlib \
        runipy \
        pillow \
        pyvirtualdisplay \
        ipython==4.2.0 \
        ipyparallel \
        pint \
        sphinx \
        sphinx_rtd_theme \
        sphinxcontrib-napoleon \
        mock \
        scipy
# ^^^ Note we choose an older version of ipython because its tooltips work better.
#     Also, the system six is too old, so we upgrade for the pip version

WORKDIR /tmp/petsc-build
RUN wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.9.3.tar.gz
RUN tar zxf petsc-lite-3.9.3.tar.gz && cd petsc-3.9.3 && \
    ./configure --with-debugging=0 --prefix=/usr                                 \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --download-petsc4py=1     \
                --download-mpi4py=1       \
                --download-mumps=1        \
                --download-parmetis=1     \
                --download-metis=1        \
                --download-superlu=1      \
                --download-hypre=1        \
                --download-cmake=1        \
                --download-scalapack=1    \
                --download-superlu_dist=1 \
                --download-superlu=1      \
                --download-fblaslapack=1  \
                --download-hdf5=1                                                    && \
    make PETSC_DIR=/tmp/petsc-build/petsc-3.9.3 PETSC_ARCH=arch-linux2-c-opt all     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-3.9.3 PETSC_ARCH=arch-linux2-c-opt install && \
    cd /tmp && \
    rm -fr *

ENV PYTHONPATH $PYTHONPATH:/usr/lib
RUN CC=mpicc HDF5_MPI="ON" pip install --no-cache-dir --no-binary=h5py h5py

# Install Tini.. this is required because CMD (below) doesn't play nice with notebooks for some reason: https://github.com/ipython/ipython/issues/7062, https://github.com/jupyter/notebook/issues/334
RUN curl -L https://github.com/krallin/tini/releases/download/v0.10.0/tini > tini && \
    echo "1361527f39190a7338a0b434bd8c88ff7233ce7b9a4876f3315c22fce7eca1b0 *tini" | sha256sum -c - && \
    mv tini /usr/local/bin/tini && \
    chmod +x /usr/local/bin/tini

# environment variable will internally run xvfb when glucifer is imported,
#see /opt/underworld2/glucifer/__init__.py
ENV GLUCIFER_USE_XVFB 1

# Add a notebook profile.
RUN mkdir -p -m 700 /root/.jupyter/ && \
    echo "c.NotebookApp.ip = '*'" >> /root/.jupyter/jupyter_notebook_config.py

# Setup ipyparallel for mpi profile
WORKDIR /root/.jupyter
RUN ipcluster nbextension enable && \ 
    ipython profile create --parallel --profile=mpi && \
    echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> /root/.ipython/profile_mpi/ipcluster_config.py

WORKDIR /

# expose notebook port
EXPOSE 8888

# note we also use xvfb which is required for viz
ENTRYPOINT ["/usr/local/bin/tini", "--"]
