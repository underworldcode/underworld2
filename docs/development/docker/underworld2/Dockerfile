# This image needs to be built from the top level project directory.
# Typically:
#    $  docker build -f docs/development/docker/underworld2/Dockerfile .

FROM underworldcode/base --platform=$BUILDPLATFORM as base_runtime
LABEL maintainer="https://github.com/underworldcode/"
# install runtime requirements
USER root
ENV PYVER=3.10
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        libxml2 \
        libpython${PYVER}
        

FROM base_runtime AS build_base
# install build requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        pkg-config \
        python3-dev \
        cmake \
        ninja-build \
        make \
        swig \
        libxml2-dev \
        g++ \
        pkg-config
# setup further virtualenv to avoid double copying back previous packages (h5py,mpi4py,etc)
USER $NB_USER
RUN /usr/bin/python3 -m venv --system-site-packages ${VIRTUAL_ENV}
WORKDIR /tmp
COPY --chown=jovyan:users . /tmp/underworld2
WORKDIR /tmp/underworld2
RUN pip3 install -vvv . 

FROM base_runtime
COPY --from=build_base ${VIRTUAL_ENV} ${VIRTUAL_ENV}
# Record Python packages, but only record system packages! 
# Not venv packages, which will be copied directly in.
RUN PYTHONPATH= /usr/bin/pip3 freeze >/opt/requirements.txt
# Record manually install apt packages.
RUN apt-mark showmanual >/opt/installed.txt
# Copy in examples, tests, etc. 
COPY --chown=jovyan:users ./docs/examples   $NB_WORK/Underworld/examples
COPY --chown=jovyan:users ./docs/cheatsheet $NB_WORK/Underworld/cheatsheet
COPY --chown=jovyan:users ./docs/user_guide $NB_WORK/Underworld/user_guide
COPY --chown=jovyan:users ./docs/test       $NB_WORK/Underworld/test
COPY --chown=jovyan:users ./docs/UWGeodynamics/examples  $NB_WORK/Underworld/UWGeodynamics/examples
COPY --chown=jovyan:users ./docs/UWGeodynamics/benchmarks  $NB_WORK/Underworld/UWGeodynamics/benchmarks
COPY --chown=jovyan:users ./docs/UWGeodynamics/tutorials  $NB_WORK/Underworld/UWGeodynamics/tutorials
RUN chown jovyan:users /home/jovyan/workspace
RUN jupyter serverextension enable --sys-prefix jupyter_server_proxy
USER $NB_USER
WORKDIR $NB_WORK
