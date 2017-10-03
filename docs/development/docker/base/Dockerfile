FROM debian:jessie
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
        libpng12-dev \
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
        swig && \
        gdb-minimal && \
        python2.7-dbg && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install python dependencies. Note we pin ipython to 5.5.0 to get py2.7
# compatibility.
RUN pip install --upgrade pip && \
    pip install \
        packaging \
        appdirs \
        numpy \
        ipython==5.5.0 \
        notebook \
        plotly \
        mpi4py \
        matplotlib \
        runipy \
        pillow \
        pyvirtualdisplay \
        ipyparallel \
        pint \
        sphinx \
        sphinx_rtd_theme \
        sphinxcontrib-napoleon \
        mock \
        scipy && \
    pip install six --upgrade

# ^^^ Note we choose an older version of ipython because it's tooltips work better.
#     Also, the system six is too old, so we upgrade for the pip version
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

