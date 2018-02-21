FROM underworldcode/base:dev
MAINTAINER https://github.com/underworldcode/

# set working directory to /opt, and install underworld files there.
WORKDIR /opt
ENV UW2_DIR /opt/underworld2
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR
ENV NB_WORK /workspace

# add default user jovyan and change permissions on NB_WORK
ENV NB_USER jovyan
RUN mkdir $NB_WORK && \
    useradd -m -s /bin/bash -N jovyan && \
    chown -R $NB_USER:users /opt $NB_WORK

# CHANGE USER
USER $NB_USER
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR

# get underworld, compile, delete some unnecessary files, trust notebooks, copy to workspace
RUN git clone --branch "v2.5" --depth 1 https://github.com/underworldcode/underworld2 && \
    cd underworld2/libUnderworld && \
    ./configure.py --with-debugging=0  && \
    ./compile.py                 && \
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
    rsync -av $UW2_DIR/docs/. $NB_WORK


# expose notebook port
EXPOSE 8888
# expose glucifer port
EXPOSE 9999

# copy this file over so that no password is required
COPY jupyter_notebook_config.json /home/$NB_USER/.jupyter/jupyter_notebook_config.json

# setup symlink for terminal convenience 
RUN ln -s $NB_WORK /home/$NB_USER/

# create a volume
VOLUME $NB_WORK/user_data
WORKDIR $NB_WORK

# launch notebook
CMD ["jupyter", "notebook", "--ip='*'", "--no-browser"]
