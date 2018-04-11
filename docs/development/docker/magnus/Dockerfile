FROM pawsey/mpi-base@sha256:3cd96b818c3a19cdbee3bbff542eeef8d6588b2aa8921dc76baa0a4a0c0d722b
MAINTAINER https://github.com/underworldcode/

# install things
RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        build-essential \
        git \
        python \
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
        rsync \
        vim \
        less \
        xauth \
        swig \
        gdb \
        python2.7-dbg && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


RUN pip install setuptools wheel && \
    pip install packaging \
		ipython==4.2.0 \
        appdirs \
        numpy \
        jupyter \
        matplotlib \
        runipy \
        pillow \
        pyvirtualdisplay \
        ipyparallel \
        scipy \
        pint
RUN pip install six --upgrade

# ^^^ Note we choose an older version of ipython because it's tooltips work better.
#     Also, the system six is too old, so we upgrade for the pip version

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

WORKDIR /workspace

# expose notebook port
EXPOSE 8888

# note we also use xvfb which is required for viz
ENTRYPOINT ["/usr/local/bin/tini", "--"]



RUN cd /tmp && \
    wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-3.8.3.tar.gz && \
    tar zxf petsc-lite-3.8.3.tar.gz && \
    cd petsc-3.8.3 && \
    ./configure --download-hdf5=1 --with-debugging=0 --prefix=/opt/petsc --download-fblaslapack=1 --COPTFLAGS="-g -O2" --CXXOPTFLAGS="-g -O2" --FOPTFLAGS="-g -O2" --download-petsc4py=1 --download-mumps=1 --download-parmetis=1 --download-metis=1 --download-superlu=1 --download-hypre=1 --download-cmake=1 --download-scalapack=1 && \
    make PETSC_DIR=/tmp/petsc-3.8.3 PETSC_ARCH=arch-linux2-c-opt all && \
    make PETSC_DIR=/tmp/petsc-3.8.3 PETSC_ARCH=arch-linux2-c-opt install && \
    cd /tmp && \
    rm -f petsc-lite-3.8.3.tar.gz && \
    rm -fr petsc-3.8.3

RUN cd /opt/ && \
    git clone --branch development --single-branch https://github.com/underworldcode/underworld2 && \
    cd underworld2/libUnderworld && \
    ./configure.py --with-debugging=0 --petsc-dir=/opt/petsc --hdf5-dir=/opt/petsc && \
    ./compile.py && \
    rm -fr h5py_ext              && \
    rm -f .sconsign.dblite       && \
    rm -fr .sconf_temp           && \
    cd build                     && \
    rm -fr libUnderworldPy       && \
    rm -fr StGermain             && \
    rm -fr gLucifer              && \
    rm -fr Underworld            && \
    rm -fr StgFEM                && \
    rm -fr StgDomain             && \
    rm -fr PICellerator          && \
    rm -fr Solvers               && \
    rm -fr h5py_ext && \
    cd .. && \
    find . -name \*.os |xargs rm -f && \
    git rev-parse --verify HEAD > build_commit.txt                   && \
    env > build_environment.txt                                      && \
    rm -fr .git

# for some reason the package kiwisolver (used by matplotlib) needs to be imported first, else it segfaults.. weird.
# also, let's get rid of python warnings.
RUN sed -i "1i import kiwisolver;import warnings;warnings.filterwarnings(\"ignore\")" /opt/underworld2/underworld/__init__.py

ENV PYTHONPATH $PYTHONPATH:/opt/underworld2:/opt/petsc/lib

