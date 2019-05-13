# This image needs to be built from the top level project directory.
# Typically:
#    $  docker build -f docs/development/docker/underworld2/Dockerfile .


FROM underworldcode/base@sha256:e022c04b6bbaddf7447b89dd187bdd31958d403db3677adc3b21c7ca6560202e
MAINTAINER https://github.com/underworldcode/

USER root

# install underworld under /tmp.
WORKDIR /tmp
ENV UW2_TMP /tmp/underworld2
ENV UW2_DIR /usr/local/underworld2
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR:$UW2_DIR/lib
RUN mkdir -p $UW2_TMP 

# install lavavu
RUN pip3 install --no-cache-dir lavavu

# COPY UW - unfortunately, the version of docker at docker cloud does not support chown yet.
COPY --chown=jovyan:users . $UW2_DIR
COPY --chown=jovyan:users ./docs/ $NB_WORK/

# get underworld, compile and install underworld libs
RUN cd $UW2_DIR/libUnderworld && \
    ./configure.py --with-debugging=0  && \
    ./compile.py                 && \
    rm -fr h5py_ext              && \
    rm .sconsign.dblite          && \
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
    cd $UW2_DIR                  && \
    find . -name \*.os |xargs rm -f                                  && \
    env > build_environment.txt                                      && \
    rm -fr .git                                                      && \
    chown -R $NB_USER:users $UW2_DIR

# clone UWGeodyanamics, install 
RUN git clone https://github.com/underworldcode/UWGeodynamics.git && \
    pip3 install --no-cache-dir UWGeodynamics/ && \
    mkdir $NB_WORK/UWGeodynamics && \
    mv ./UWGeodynamics/examples   $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/tutorials  $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/benchmarks $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/docs       $NB_WORK/UWGeodynamics/  && \
    rm -rf UWGeodynamics                                   && \
    chown -R $NB_USER:users $NB_WORK/UWGeodynamics

# expose glucifer port
EXPOSE 9999

# environment variable will internally run xvfb when glucifer is imported,
# see /opt/underworld2/glucifer/__init__.py
ENV GLUCIFER_USE_XVFB 1

USER $NB_USER
WORKDIR $NB_WORK

