FROM ubuntu:22.04 --platform=$TARGETPLATFORM as base_runtime
LABEL maintainer="https://github.com/underworldcode/"
ENV LANG=C.UTF-8
ENV PYVER=3.10
# Setup some things in anticipation of virtualenvs
ENV VIRTUAL_ENV=/opt/venv
# The following ensures that the venv takes precedence if available
ENV PATH=${VIRTUAL_ENV}/bin:$PATH
# The following ensures venv packages are available when using system python (such as from jupyter)
ENV PYTHONPATH=${PYTHONPATH}:${VIRTUAL_ENV}/lib/python${PYVER}/site-packages

# add joyvan user, volume mount and expose port 8888
EXPOSE 8888
ENV NB_USER jovyan
ENV NB_WORK /home/$NB_USER
RUN useradd -m -s /bin/bash -N $NB_USER -g users \
&&  mkdir -p /$NB_WORK/workspace \
&&  chown -R $NB_USER:users $NB_WORK
VOLUME $NB_WORK/workspace

# make virtualenv directory and set permissions 
RUN mkdir ${VIRTUAL_ENV} \
&&  chmod ugo+rwx ${VIRTUAL_ENV}

# install runtime requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        bash-completion \
        python3-minimal \
        python3-venv \
        python3-pip \
        python3-numpy \
        vim \
        less \
        git \
        cmake \
        g++ \
        gcc python3-dev \
        valgrind valgrind-dbg valgrind-mpi \
        gdb cgdb \ 
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*

RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        jupyter \
        jupytext \
        jupyterlab \
        plotly \
        matplotlib \
        pillow \
        ipython \
        ipyparallel \
        pint==0.19.2 \
        scipy \ 
        rise \
        tabulate 

# Lets grab lavavu & requirements
COPY --from=underworldcode/lavavu --platform=$TARGETPLATFORM /opt       /opt
COPY --from=underworldcode/lavavu --platform=$TARGETPLATFORM /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN PYTHONPATH= /usr/bin/pip3 install -r /opt/requirements.txt

# mpi, petsc, mpi4py, petsc4py, h5py
COPY --from=underworldcode/petsc --platform=$TARGETPLATFORM /opt       /opt
COPY --from=underworldcode/petsc --platform=$TARGETPLATFORM /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN PYTHONPATH= /usr/bin/pip3 install -r /opt/requirements.txt

# jovyan user, finalise jupyter env
USER $NB_USER
# RUN ipython profile create --parallel --profile=mpi \
# &&  echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> $NB_WORK/.ipython/profile_mpi/ipcluster_config.py
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]
