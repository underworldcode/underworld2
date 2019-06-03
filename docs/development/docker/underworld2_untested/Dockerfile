FROM underworldcode/base@sha256:0f9b00fe918df29d9e2c9cc6d10bb2a8b42060d1d75fb8800646be9cad748baf
MAINTAINER https://github.com/underworldcode/

# set working directory to /opt, and install underworld files there.
WORKDIR /opt
ENV UW2_DIR /opt/underworld2
RUN mkdir $UW2_DIR
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR
ENV NB_WORK /workspace

# add default user jovyan and change permissions on NB_WORK
ENV NB_USER jovyan
ENV NB_UID 1000
#RUN useradd -m -s /bin/bash -N jovyan
RUN adduser --disabled-password --gecos "Default user" --uid ${NB_UID} ${NB_USER}

# copy this file over so that no password is required
COPY docs/development/docker/underworld2_untested/jupyter_notebook_config.json /home/$NB_USER/.jupyter/jupyter_notebook_config.json

# install lavavu
RUN pip install --no-cache-dir lavavu

# COPY UW
COPY --chown=jovyan:users . $UW2_DIR/
#COPY . $UW2_DIR/

# get underworld, compile, delete some unnecessary files, trust notebooks, copy to workspace
RUN cd underworld2/libUnderworld && \
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
    find $UW2_DIR/docs -name \*.ipynb  -print0 | xargs -0 jupyter trust && \
    mkdir $NB_WORK                                                   && \
    rsync -av $UW2_DIR/docs/. $NB_WORK                               && \
    cd /opt/underworld2                                              && \
    find . -name \*.os |xargs rm -f                                  && \
    cat .git/refs/heads/* > build_commit.txt                         && \
    env > build_environment.txt                                      && \
    rm -fr .git                                                      && \
    chown -R $NB_USER:users $NB_WORK $UW2_DIR /home/$NB_USER

RUN git clone https://github.com/underworldcode/UWGeodynamics.git && \
    pip install --no-cache-dir UWGeodynamics/ && \
    mkdir /workspace/UWGeodynamics && \
    mv ./UWGeodynamics/examples /workspace/UWGeodynamics/. && \
    mv ./UWGeodynamics/tutorials /workspace/UWGeodynamics/. && \
    mv ./UWGeodynamics/benchmarks /workspace/UWGeodynamics/. && \
    mv ./UWGeodynamics/docs /workspace/UWGeodynamics/ && \
    rm -rf UWGeodynamics && \
    chown -R $NB_USER:users /workspace/UWGeodynamics

# expose notebook port
EXPOSE 8888
# expose glucifer port
EXPOSE 9999

# CHANGE USER
USER $NB_USER
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR

# setup symlink for terminal convenience
RUN ln -s $NB_WORK /home/$NB_USER/

# create a volume
VOLUME $NB_WORK/user_data
WORKDIR $NB_WORK

# launch notebook
CMD ["jupyter", "notebook", "--no-browser"]

