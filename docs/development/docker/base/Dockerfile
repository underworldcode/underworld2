FROM debian:latest
MAINTAINER https://github.com/underworldcode/
ENV LANG=C.UTF-8
# Put this here in anticipation of virtualenvs
ENV VIRTUAL_ENV=/opt/venv
ENV PATH=${VIRTUAL_ENV}/bin:$PATH
ENV PYTHONPATH=$PYTHONPATH:/usr/lib/python3.7/site-packages
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/:/usr/local/lib/x86_64-linux-gnu/
# install runtime requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        python3-minimal \
        python3-virtualenv \
        python3-pip \
        python3-tk \
        vim \
        less \
        git
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*

RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        numpy \
        jupyter \
        plotly \
        matplotlib \
        pillow \
        ipython \
        ipyparallel \
        pint \
        scipy \ 
        tabulate 

# Lets grab lavavu & requirements
COPY --from=underworldcode/base:lavavu /opt       /opt
COPY --from=underworldcode/base:lavavu /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN pip3 install -r /opt/requirements.txt
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/x86_64-linux-gnu/

# mpi, petsc, mpi4py, petsc4py, h5py
COPY --from=underworldcode/base:petsc /opt       /opt
COPY --from=underworldcode/base:petsc /usr/local /usr/local
RUN apt-get update \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends $(awk '{print $1'} /opt/installed.txt) \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/* 
RUN pip3 install -r /opt/requirements.txt

EXPOSE 8888
RUN ipcluster nbextension enable
ADD https://github.com/krallin/tini/releases/download/v0.18.0/tini /tini
RUN chmod +x /tini
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan
USER $NB_USER
ENV NB_WORK /home/$NB_USER
RUN ipython profile create --parallel --profile=mpi && \
    echo "c.IPClusterEngines.engine_launcher_class = 'MPIEngineSetLauncher'" >> $NB_WORK/.ipython/profile_mpi/ipcluster_config.py
VOLUME $NB_WORK/workspace
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]
