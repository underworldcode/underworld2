FROM underworldcode/base:latest
#FROM underworldcode/base@sha256:04654de1c70622af713dd1b250689d55f20191b64f87d351ffdd6dc00930f682
MAINTAINER https://github.com/underworldcode/

# set working directory to /opt, and install underworld files there.
WORKDIR /opt
ENV UW2_DIR /opt/underworld2
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR
ENV NB_WORK /workspace

# add default user jovyan and change permissions on NB_WORK
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan

# copy this file over so that no password is required
COPY jupyter_notebook_config.json /home/$NB_USER/.jupyter/jupyter_notebook_config.json

# install lavavu
#RUN pip install --no-cache-dir lavavu

# get underworld, compile, delete some unnecessary files, trust notebooks, copy to workspace
RUN git clone --branch development --single-branch https://github.com/underworldcode/underworld2 && \
    cd underworld2/libUnderworld && \
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
    cd ../../docs/development/api_doc_generator/                     && \
    sphinx-build . ../../api_doc                                     && \
    mkdir $NB_WORK                                                   && \
    rsync -av $UW2_DIR/docs/. $NB_WORK                               && \
    cd /opt/underworld2                                              && \
    find . -name \*.os |xargs rm -f                                  && \
    git rev-parse --verify HEAD > build_commit.txt                   && \
    env > build_environment.txt                                      && \
    rm -fr .git                                                      && \
    chown -R $NB_USER:users $NB_WORK $UW2_DIR /home/$NB_USER


# kiwisolver hack, and pipe down numpy warnings
RUN sed -i "1i import kiwisolver;import warnings;warnings.filterwarnings(\"ignore\")" /opt/underworld2/underworld/__init__.py

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
CMD ["jupyter", "notebook", "--ip='*'", "--no-browser"]

