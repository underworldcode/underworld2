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
        mpi4py  && \
    pip3 install scons

WORKDIR /tmp/petsc-build
RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
    python-minimal python-pip         &&\
    wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.9.3.tar.gz && \
    tar zxf petsc-lite-3.9.3.tar.gz && cd petsc-3.9.3                         && \
    ./configure --with-debugging=0 --prefix=/usr                                 \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --download-fblaslapack=1        \
                --download-hdf5=1               \
                --download-mumps=1              \
                --download-parmetis=1           \
                --download-metis=1              \
                --download-superlu=1            \
                --download-hypre=1              \
                --download-scalapack=1          \
                --download-superlu_dist=1       \
                --download-superlu=1         && \
    make PETSC_DIR=/tmp/petsc-build/petsc-3.9.3 PETSC_ARCH=arch-linux2-c-opt all     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-3.9.3 PETSC_ARCH=arch-linux2-c-opt install && \
    cd /tmp && \
    rm -fr *  && \
    apt-get remove -yq python-minimal python-pip  && \
    apt autoremove -yq  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Hack for scons.. should be able to remove in future.
ENV SCONS_LIB_DIR /usr/local/lib/python3.5/dist-packages/scons-3.0.1

ENV PYTHONPATH $PYTHONPATH:/usr/lib
RUN CC=mpicc HDF5_MPI="ON" pip3 install --no-cache-dir --no-binary=h5py h5py

# Install Tini.. this is required because CMD (below) doesn't play nice with notebooks for some reason: https://github.com/ipython/ipython/issues/7062, https://github.com/jupyter/notebook/issues/334
RUN curl -L https://github.com/krallin/tini/releases/download/v0.10.0/tini > tini && \
    echo "1361527f39190a7338a0b434bd8c88ff7233ce7b9a4876f3315c22fce7eca1b0 *tini" | sha256sum -c - && \
    mv tini /usr/local/bin/tini && \
    chmod +x /usr/local/bin/tini

# expose notebook port
EXPOSE 8888

# Setup ipyparallel for mpi profile
RUN ipcluster nbextension enable

# add default user jovyan and change permissions on NB_WORK
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan
USER $NB_USER
ENV NB_WORK /workspace

RUN ipython profile create --parallel --profile=mpi && \
    echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> /home/$NB_USER/.ipython/profile_mpi/ipcluster_config.py

# note we also use xvfb which is required for viz
ENTRYPOINT ["/usr/local/bin/tini", "--"]

# copy this file over so that no password is required
COPY jupyter_notebook_config.json /home/$NB_USER/.jupyter/jupyter_notebook_config.json

# setup symlink for terminal convenience
RUN ln -s $NB_WORK /home/$NB_USER/

# create a volume
VOLUME $NB_WORK/user_data
WORKDIR $NB_WORK

# launch notebook
CMD ["jupyter", "notebook", "--ip='0.0.0.0'", "--no-browser"]

