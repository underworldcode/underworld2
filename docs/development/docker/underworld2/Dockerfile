# This image needs to be built from the top level project directory.
# Typically:
#    $  docker build -f docs/development/docker/underworld2/Dockerfile .

FROM underworldcode/base@sha256:ee52e70411e64a41ee1b0cd6853235c5937dbe3ab34325763dde9df0a2dfda61
MAINTAINER https://github.com/underworldcode/

USER root

# install underworld under /tmp.
WORKDIR /tmp
ENV UW2_TMP /tmp/underworld2
RUN mkdir -p $UW2_TMP 

# COPY UW
COPY --chown=jovyan:users . $UW2_TMP
COPY --chown=jovyan:users ./docs/ $NB_WORK/

# get underworld, compile and install underworld libs
RUN cd $UW2_TMP && \
    pip3 install -vvv . && \ 
    cd /tmp                           && \
    rm -rf *

# clone UWGeodynamics, install 
RUN git clone -b development https://github.com/underworldcode/UWGeodynamics.git && \
    pip3 install --no-cache-dir UWGeodynamics/ && \
    mkdir $NB_WORK/UWGeodynamics && \
    mv ./UWGeodynamics/examples   $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/tutorials  $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/benchmarks $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/docs       $NB_WORK/UWGeodynamics/  && \
    rm -rf UWGeodynamics                                   && \
    chown -R $NB_USER:users $NB_WORK/UWGeodynamics

# DISABLE FOLLOWING. We've reverted xvfb via entrypoint.
# environment variable will internally run xvfb when uw.visualisation is imported,
# see /opt/underworld2/visualisation/__init__.py
# ENV UW_USE_XVFB 1

USER $NB_USER
WORKDIR $NB_WORK

