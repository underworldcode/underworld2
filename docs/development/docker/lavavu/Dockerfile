FROM debian:latest as base_runtime
ENV LANG=C.UTF-8
# Setup some things in anticipation of virtualenvs
ENV VIRTUAL_ENV=/opt/venv
# The following ensures that the venv takes precedence if available
ENV PATH=${VIRTUAL_ENV}/bin:$PATH
# The following ensures that system packages are visible to venv Python, and vice versa
ENV PYTHONPATH=/usr/lib/python3.7/site-packages:${VIRTUAL_ENV}/lib/python3.7/site-packages
# install runtime requirements
RUN apt-get update -qq \
&&  DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        python3-minimal \
        python3-virtualenv \
        python3-pip \
        libpng16-16 \
        libtiff5 \
        libavcodec58 \
        libavformat58 \
        libswscale5 \
        zlib1g \
        libxcb-glx0 \
        libx11-xcb1 \
        libxcb-dri2-0 \
        libxdamage1 \
        libxxf86vm1 \
        libxcb-dri3-0 \
        libxcb-present0 \
        libxcb-sync1 \
        libxshmfence1 \
        libllvm7 \
&&  apt-get clean \
&&  rm -rf /var/lib/apt/lists/*
RUN pip3 install -U setuptools  \
&&  pip3 install --no-cache-dir \
        packaging \
        appdirs \
        numpy \
        jupyter \
        pillow \
        ipython

FROM base_runtime AS build_base
# install build requirements
RUN apt-get update -qq 
RUN DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        build-essential \
        libpng-dev \
        libtiff-dev \
        git \
        python3-dev \
        wget \
        libavcodec-dev \
        libavformat-dev \
        libavutil-dev \
        libswscale-dev \
        zlib1g-dev \
        libx11-dev
#Add deb-src repo
RUN echo "deb-src http://deb.debian.org/debian buster main" >> /etc/apt/sources.list
# install mesa deps
RUN apt-get update -qq
RUN DEBIAN_FRONTEND=noninteractive apt-get -y build-dep mesa
WORKDIR /tmp/osmesa-build
RUN wget ftp://ftp.freedesktop.org/pub/mesa/mesa-19.1.7.tar.xz
RUN tar xvf mesa-19.1.7.tar.xz
WORKDIR /tmp/osmesa-build/mesa-19.1.7
RUN meson build/ -Dosmesa=gallium -Dgallium-drivers=swrast -Ddri-drivers= \
                 -Dvulkan-drivers= -Degl=false -Dgbm=false -Dgles1=false  \
                 -Dgles2=false -Dbuildtype=release -Db_ndebug=true
RUN meson configure build/ -Dprefix=/usr/local
RUN ninja -C build/ -j5
RUN ninja -C build/ install
ENV LV_LIB_DIRS=/usr/local/lib/x86_64-linux-gnu/
ENV LV_INC_DIRS=/usr/local/include/
ENV LV_OSMESA=1
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/x86_64-linux-gnu/
# create a virtualenv to put new python modules
RUN /usr/bin/python3 -m virtualenv --python=/usr/bin/python3 ${VIRTUAL_ENV} 
RUN pip3 install --no-cache-dir lavavu

FROM base_runtime AS minimal
COPY --from=build_base $VIRTUAL_ENV $VIRTUAL_ENV
COPY --from=build_base /usr/local /usr/local
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/x86_64-linux-gnu/
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