# This image needs to be built from the top level project directory.
# Typically:
#    $  docker build -f docs/development/docker/underworld2/Dockerfile .

FROM underworldcode/base@sha256:f194ce40ea602305141a6a95fad640c90dd7ae5fae7532cd164dc566ec465edb as base_runtime
MAINTAINER https://github.com/underworldcode/
# install runtime requirements
USER root
ENV PYVER=3.8
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        libxml2 \
        libpython${PYVER}

FROM base_runtime AS build_base
# install build requirements
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        python3-dev \
        swig \
        libxml2-dev
RUN PYTHONPATH= /usr/bin/pip3 install --no-cache-dir setuptools scons 
# setup further virtualenv to avoid double copying back previous packages (h5py,mpi4py,etc)
RUN /usr/bin/python3 -m virtualenv --system-site-packages --python=/usr/bin/python3 ${VIRTUAL_ENV}
WORKDIR /tmp
COPY --chown=jovyan:users . /tmp/underworld2
WORKDIR /tmp/underworld2
RUN pip3 install -vvv . 
# clone UWGeodynamics, install 
WORKDIR /tmp
RUN git clone https://github.com/underworldcode/UWGeodynamics.git
RUN pip3 install --no-cache-dir UWGeodynamics/

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
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/docs/examples   $NB_WORK/UWGeodynamics/examples
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/docs/tutorials  $NB_WORK/UWGeodynamics/tutorials
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/docs/benchmarks $NB_WORK/UWGeodynamics/benchmarks
RUN  chown jovyan:users /home/jovyan/workspace /home/jovyan/UWGeodynamics
RUN jupyter serverextension enable --sys-prefix jupyter_server_proxy
USER $NB_USER
WORKDIR $NB_WORK
