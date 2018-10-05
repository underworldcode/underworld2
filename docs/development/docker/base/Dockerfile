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
	python3-setuptools \
        wget \
        gfortran  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/python python /usr/bin/python2.7 1
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.5 2

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
RUN pip3 install --no-cache-dir packaging \
        runipy \
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
        mpi4py

RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends python-minimal python-pip

WORKDIR /tmp/petsc-build
RUN wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.9.3.tar.gz
RUN tar zxf petsc-lite-3.9.3.tar.gz && cd petsc-3.9.3                         && \
    ./configure --with-debugging=0 --prefix=/usr                                 \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --download-cmake=1        \
                --download-fblaslapack=1  \
                --download-hdf5=1
RUN cd petsc-3.9.3                         && \
    make PETSC_DIR=/tmp/petsc-build/petsc-3.9.3 PETSC_ARCH=arch-linux2-c-opt all     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-3.9.3 PETSC_ARCH=arch-linux2-c-opt install && \
    cd /tmp && \
    rm -fr *

WORKDIR /tmp
RUN wget http://prdownloads.sourceforge.net/scons/scons-3.0.1.tar.gz
RUN tar zxf scons-3.0.1.tar.gz && cd scons-3.0.1 && \
    python setup.py install && \
    cd /tmp && \
    rm -rf *

ENV PYTHONPATH $PYTHONPATH:/usr/lib
RUN CC=mpicc HDF5_MPI="ON" pip3 install --no-cache-dir --no-binary=h5py h5py

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
