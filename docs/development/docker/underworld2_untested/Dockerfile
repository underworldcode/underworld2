FROM underworldcode/base:dev
MAINTAINER https://github.com/underworldcode/

# set working directory to /opt, and install underworld files there.
WORKDIR /opt
ENV UW2_DIR /opt/underworld2
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR

# get underworld, compile, delete some unnecessary files, trust notebooks, copy to workspace
RUN git clone --branch "development" --single-branch https://github.com/underworldcode/underworld2 && \
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
    mkdir /workspace                                                 && \
    cd ../../docs/development/api_doc_generator/                     && \
    sphinx-build . ../../api_doc                                     && \
    rsync -av $UW2_DIR/docs/. /workspace


# expose notebook port
EXPOSE 8888
# expose glucifer port
EXPOSE 9999

# setup space for working in
VOLUME /workspace/user_data
WORKDIR /workspace

# copy this file over so that no password is required

COPY jupyter_notebook_config.json /root/.jupyter/jupyter_notebook_config.json

# launch notebook
CMD ["jupyter", "notebook", "--no-browser", "--allow-root"]
