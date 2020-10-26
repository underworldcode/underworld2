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
        python${PYVER}-minimal \
        python3-virtualenv \
        python3-pip \
        python3-numpy \
        libpng16-16 \
        libjpeg8 \
        libtiff5 \ 
        libglu1-mesa \
        libosmesa6 \
        libavcodec58 \
        libavformat58 \
        libavutil56 \
        libswscale5 \
        zlib1g \
        libopenblas0 \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*

RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        jupyter \
        pillow \
        ipython

FROM base_runtime AS build_base
# FROM base_runtime AS build_base
# install build requirements
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        python3-setuptools \
        libpython${PYVER}-dev \
sudo apt-get install        libpng-dev \
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
        libz-dev

# lavavu
# create a virtualenv to put new python modules
RUN /usr/bin/python3 -m virtualenv --system-site-packages --python=/usr/bin/python3 ${VIRTUAL_ENV}
RUN LV_OSMESA=1 pip3 install --no-cache-dir --no-binary=lavavu lavavu

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
EXPOSE 8888
ADD https://github.com/krallin/tini/releases/download/v0.18.0/tini /tini
RUN chmod +x /tini
ENV NB_USER jovyan
RUN useradd -m -s /bin/bash -N jovyan
USER $NB_USER
ENV NB_WORK /home/$NB_USER
VOLUME $NB_WORK/workspace
WORKDIR $NB_WORK
CMD ["jupyter", "notebook", "--no-browser", "--ip='0.0.0.0'"]