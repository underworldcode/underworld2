FROM debian:latest
MAINTAINER https://github.com/underworldcode/

# install things
RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        build-essential \
        git \
        python \
        python-dev \
        petsc-dev \
        libhdf5-openmpi-dev \
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
        python-tk \
        swig \
        gdb-minimal \
        python2.7-dbg && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install setuptools and wheel first, needed by plotly
RUN pip install --no-cache-dir setuptools wheel && \
    pip install --no-cache-dir packaging \
        appdirs \
        numpy \
        jupyter \
        plotly \
        mpi4py \
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
        scipy && \
    CC=mpicc HDF5_MPI="ON" HDF5_DIR=/usr/lib/x86_64-linux-gnu/hdf5/openmpi/ pip install --no-cache-dir --no-binary=h5py h5py

# ^^^ Note we choose an older version of ipython because its tooltips work better.
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

WORKDIR /

# expose notebook port
EXPOSE 8888



# note we also use xvfb which is required for viz
ENTRYPOINT ["/usr/local/bin/tini", "--"]

