# This image needs to be built from the top level project directory.
# Typically:
#    $  docker build -f docs/development/docker/underworld2/Dockerfile .


FROM underworldcode/base@sha256:a49b6fd56575be1a4afbb5d4aa00c4fffeb7201806aed7b7d368bc4ca474ced4
MAINTAINER https://github.com/underworldcode/

USER root

# install underworld under /tmp.
WORKDIR /tmp
ENV UW2_TMP /tmp/underworld2
ENV UW2_DIR /usr/local/underworld2
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR/lib
RUN mkdir -p $UW2_TMP 

# install lavavu
RUN pip3 install --no-cache-dir lavavu

# COPY UW
COPY --chown=jovyan:users . $UW2_TMP
COPY --chown=jovyan:users ./docs/ $NB_WORK/

# get underworld, compile and install underworld libs
RUN cd $UW2_TMP/libUnderworld && \
    ./configure.py --with-debugging=0 --prefix=$UW2_DIR && \
    ./compile.py                      && \
    scons install                     && \
    env > build_environment.txt       && \
    cd /tmp                           && \
    rm -rf *                          && \
    python3 -c "import underworld"     && \
    python3 -c "import glucifer"       && \
    chown -R $NB_USER:users $UW2_DIR

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

# expose glucifer port
EXPOSE 9999

# DISABLE FOLLOWING. We've reverted xvfb via entrypoint.
# environment variable will internally run xvfb when glucifer is imported,
# see /opt/underworld2/glucifer/__init__.py
# ENV GLUCIFER_USE_XVFB 1

USER $NB_USER
WORKDIR $NB_WORK

