# This image needs to be built from the top level project directory.
# Typically:
#    $  docker build -f docs/development/docker/underworld2/Dockerfile .

FROM underworldcode/base as base_runtime
MAINTAINER https://github.com/underworldcode/
# install runtime requirements
USER root
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        libxml2 

# install build requirements
FROM base_runtime AS build_base
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        python3-dev \
        swig \
        libxml2-dev
RUN pip3 install --no-cache-dir setuptools scons 
RUN python3 -m virtualenv --python=/usr/bin/python3 ${VIRTUAL_ENV} 
WORKDIR /tmp
COPY --chown=jovyan:users . /tmp/underworld2
WORKDIR /tmp/underworld2
RUN pip3 install -vvv . 
# clone UWGeodynamics, install 
WORKDIR /tmp
RUN git clone -b development https://github.com/underworldcode/UWGeodynamics.git 
RUN pip3 install --no-cache-dir UWGeodynamics/ 

FROM base_runtime 
COPY --from=build_base $VIRTUAL_ENV $VIRTUAL_ENV
# Record Python packages, but only record system packages! 
# Not venv packages, which will be copied directly in.
RUN /usr/bin/pip3 freeze >/opt/requirements.txt
# Record manually install apt packages.
RUN apt-mark showmanual >/opt/installed.txt
COPY --chown=jovyan:users ./docs/ $NB_WORK/
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/examples   $NB_WORK/UWGeodynamics/.
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/tutorials  $NB_WORK/UWGeodynamics/.
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/benchmarks $NB_WORK/UWGeodynamics/.
COPY --from=build_base --chown=jovyan:users /tmp/UWGeodynamics/docs       $NB_WORK/UWGeodynamics/.
USER $NB_USER
WORKDIR $NB_WORK

