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
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*  && \
    pip install \
        packaging \
        appdirs \
        numpy \
        jupyter \
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
    pip install 'ipython==4.2.0' --force-reinstall && \
    pip install six --upgrade

# ^^^ Note we choose an older version of ipython because it's tooltips work better.
#     Also, the system six is too old, so we upgrade for the pip version


# Install Tini.. this is required because CMD (below) doesn't play nice with notebooks for some reason: https://github.com/ipython/ipython/issues/7062, https://github.com/jupyter/notebook/issues/334
RUN curl -L https://github.com/krallin/tini/releases/download/v0.6.0/tini > tini && \
    echo "d5ed732199c36a1189320e6c4859f0169e950692f451c03e7854243b95f4234b *tini" | sha256sum -c - && \
    mv tini /usr/local/bin/tini && \
    chmod +x /usr/local/bin/tini

# script for xvfb-run.  all docker commands will effectively run under this via the entrypoint
RUN printf "#\041/bin/sh \n rm -f /tmp/.X99-lock && xvfb-run -s '-screen 0 1600x1200x16' \$@" >> /usr/local/bin/xvfbrun.sh && \
    chmod +x /usr/local/bin/xvfbrun.sh

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
ENTRYPOINT ["/usr/local/bin/tini", "--", "xvfbrun.sh"]

