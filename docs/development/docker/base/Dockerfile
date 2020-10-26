FROM ubuntu:20.04 as base_runtime
MAINTAINER https://github.com/underworldcode/
ENV LANG=C.UTF-8
ENV PYVER=3.8
# Setup some things in anticipation of virtualenvs
ENV VIRTUAL_ENV=/opt/venv
# The following ensures that the venv takes precedence if available
ENV PATH=${VIRTUAL_ENV}/bin:$PATH
# The following ensures venv packages are available when using system python (such as from jupyter)
ENV PYTHONPATH=${PYTHONPATH}:${VIRTUAL_ENV}/lib/python${PYVER}/site-packages

# install runtime requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        bash-completion \
        python${PYVER}-minimal \
        python3-virtualenv \
        python3-pip \
        python3-numpy \
        python3-numpy-dbg \
        vim \
        less \
        git \
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
        pint==0.9 \
        scipy \ 
        rise \
        tabulate 

# Lets grab lavavu & requirements
COPY --from=underworldcode/base:lavavu /opt       /opt
COPY --from=underworldcode/base:lavavu /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN PYTHONPATH= /usr/bin/pip3 install -r /opt/requirements.txt

# mpi, petsc, mpi4py, petsc4py, h5py
COPY --from=underworldcode/base:petsc /opt       /opt
COPY --from=underworldcode/base:petsc /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN PYTHONPATH= /usr/bin/pip3 install -r /opt/requirements.txt

# add tini, joyvan user, volume mount and expose port 8888
EXPOSE 8888
ENV NB_USER jovyan
ENV NB_WORK /home/$NB_USER
ADD https://github.com/krallin/tini/releases/download/v0.18.0/tini /tini
RUN ipcluster nbextension enable \
&&  chmod +x /tini \
&&  useradd -m -s /bin/bash -N $NB_USER -g users \
&&  mkdir -p /$NB_WORK/workspace \
&&  chown -R $NB_USER:users $NB_WORK
VOLUME $NB_WORK/workspace

# jovyan user, finalise jupyter env
USER $NB_USER
RUN ipython profile create --parallel --profile=mpi \
&&  echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> $NB_WORK/.ipython/profile_mpi/ipcluster_config.py
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]
