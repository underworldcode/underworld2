#####################################################################
# Multi stage Dockerfile structure:
# 1. runtime 
# 2. build
# 3. final == runtime + min. build
#
# It begins with layers for runtime execution. 
# The runtime environment (packages, permissions, ENV vars.) 
# are consistent accross all layer of this Dockerfile. 
# The build layer takes the runtime layer and builds the software
# stack in /usr/local.
# The final image is a composite of the runtime layer and 
# minimal sections of the build layer.
#####################################################################

# PETSC_VERSION passed in via --build-args
# Used for github actions on the underworld repo
# Must go before the 1st FROM see
# https://docs.docker.com/engine/reference/builder/#understand-how-arg-and-from-interact
ARG PETSC_VERSION=3.18.1
# 'petsc-image' will be used later on in build stage COPY command 
FROM julesg/petsc:${PETSC_VERSION} as petsc-image

FROM ubuntu:22.04 as runtime
LABEL maintainer="https://github.com/underworldcode/"

#### Dockerfile ENV vars - for all image stages
ENV LANG=C.UTF-8
# mpi lib will be install at /usr/local/lib
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# python env vars. 
# prepappending on PATH means all pip install will goto the PYOPT
ENV PYVER=3.10
ENV PYOPT=/opt/venv
ENV PATH=$PYOPT/bin:$PATH
ENV PYTHONPATH=$PYTHONPATH:$PYOPT/lib/python${PYVER}/site-packages
# taken from PETSC images
ENV PETSC_DIR=/usr/local
ENV PYTHONPATH=$PYTHONPATH:$PETSC_DIR/lib

# add user jovyan
ENV NB_USER jovyan
ENV NB_HOME /home/$NB_USER
RUN useradd -m -s /bin/bash -N $NB_USER

# runtime packages
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        ssh \
        python3-venv \
        python3-dev \
        libopenblas-base \
        python3-venv \
        libxml2 \
        vim \
        git \
        gdb \
        libpng16-16 \
        libjpeg8 \
        libtiff5 \ 
        libglu1-mesa \
        libosmesa6 \
        libavcodec58 \
        libavformat58 \
        libavutil56 \
        libswscale5 \
        zlib1g \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*

# start a virtual environment install runtime modules in 'build' as some
# packages need gcc
RUN python3 -m venv $PYOPT \
&&  chmod ugo+rwx $PYOPT

FROM runtime as build

# install build packages
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        swig \
        cmake \
        make \
        gfortran \
        python3-dev \
        pkg-config \
        libxml2-dev \
        ninja-build \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*

# 
RUN pip3 install -U setuptools \
&&  pip3 install --no-cache-dir \
         matplotlib \
         scipy \
         pint \
         rise \
         pytest \
         nbmake \
         gFlex \
         tabulate 


# Lets grab lavavu & requirements
COPY --from=underworldcode/lavavu /opt       /opt
COPY --from=underworldcode/lavavu /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN pip3 install -r /opt/requirements.txt

# mpi, petsc, mpi4py, petsc4py, h5py
COPY --from=petsc-image /opt       /opt
COPY --from=petsc-image /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*
RUN pip3 install -r /opt/requirements.txt

WORKDIR /tmp
COPY --chown=$NB_USER:users . /tmp/underworld2
WORKDIR /tmp/underworld2
RUN pip3 install -vvv .
RUN pip3 install --no-cache-dir \
        git+https://github.com/drufat/triangle.git \
        badlands

RUN pip3 freeze >/opt/requirements.txt
# Record manually install apt packages.
RUN apt-mark showmanual >/opt/installed.txt


# Build the final image, a combination of the runtime and build stages
FROM runtime as final

COPY --from=build --chown=$NB_USER:users /opt /opt
COPY --from=build --chown=$NB_USER:users /usr/local /usr/local

# Record Python packages, but only record system packages!
# Not venv packages, which will be copied directly in.
# Copy in examples, tests, etc.
COPY --chown=jovyan:users ./docs/examples   $NB_HOME/Underworld/examples
COPY --chown=jovyan:users ./docs/cheatsheet $NB_HOME/Underworld/cheatsheet
COPY --chown=jovyan:users ./docs/user_guide $NB_HOME/Underworld/user_guide
COPY --chown=jovyan:users ./docs/test       $NB_HOME/Underworld/test
COPY --chown=jovyan:users ./docs/UWGeodynamics/examples  $NB_HOME/Underworld/UWGeodynamics/examples
COPY --chown=jovyan:users ./docs/UWGeodynamics/benchmarks  $NB_HOME/Underworld/UWGeodynamics/benchmarks
COPY --chown=jovyan:users ./docs/UWGeodynamics/tutorials  $NB_HOME/Underworld/UWGeodynamics/tutorials
RUN mkdir -p $NB_HOME/workspace \
&&  chown jovyan:users /home/jovyan/workspace
RUN jupyter serverextension enable --sys-prefix jupyter_server_proxy

EXPOSE 8888
USER $NB_USER
WORKDIR $NB_HOME

CMD ["jupyter-lab", "--no-browser", "--ip='0.0.0.0'"]
