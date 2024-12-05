ARG UBUNTU_VERSION="24.04"
ARG PYTHON_VERSION="3.12"

FROM ubuntu:${UBUNTU_VERSION} as base_runtime
LABEL maintainer="https://github.com/underworldcode/"

# need to repeat ARGS after every FROM 
ARG UBUNTU_VERSION
ARG PYTHON_VERSION

ENV LANG=C.UTF-8
ENV PYVER=${PYTHON_VERSION}

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
        python3-minimal \
        python3-venv \
        python3-pip \
        python3-numpy \
        libpng16-16t64 \
        libjpeg8 \
        libtiff-dev \ 
        libglu1-mesa-dev \
        libosmesa6 \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libswscale-dev \
        zlib1g-dev \
        libopenblas0 \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*


FROM base_runtime AS build_base

ARG UBUNTU_VERSION
ARG PYTHON_VERSION

# install build requirements
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        python3-setuptools \
        libpython${PYTHON_VERSION}-dev \ 
        libpng-dev \
        libjpeg-dev \
        libtiff-dev \
        mesa-utils \
        libglu1-mesa-dev \
        libosmesa6-dev \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libswscale-dev \
        zlib1g-dev \
        cmake \
        libopenblas-dev \
        libz-dev \
        gcc \
        python3-full


# lavavu
# create a virtualenv to put new python modules
USER $NB_USER
RUN /usr/bin/python3 -m venv --system-site-packages ${VIRTUAL_ENV}
RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        jupyter \
        pillow \
        ipython

#RUN LV_OSMESA=1 pip3 install --no-cache-dir lavavu #--no-binary=lavavu lavavu 

RUN pip3 install --no-cache-dir lavavu-osmesa

FROM base_runtime AS minimal
COPY --from=build_base $VIRTUAL_ENV $VIRTUAL_ENV
COPY --from=build_base /usr/local /usr/local
# ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/x86_64-linux-gnu/
# Record Python packages, but only record system packages! 
# Not venv packages, which will be copied directly in.
RUN PYTHONPATH= /usr/bin/pip3 freeze >/opt/requirements.txt
# Record manually install apt packages.
RUN apt-mark showmanual >/opt/installed.txt

# Add user environment
FROM minimal
USER $NB_USER
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]
