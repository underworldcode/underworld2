FROM badlandsmodel/pybadlands-demo:latest
MAINTAINER mansourjohn@gmail.com

# change badlands branch
RUN cd /build/pyBadlands            && \
    git checkout underlands         && \
    cd pyBadlands/libUtils          && \
    make                            && \
    pip install -e /build/pyBadlands


# install things
RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        build-essential \
        git \
        python \
        python-dev \
        petsc-dev \
        libhdf5-openmpi-dev \
        python-pip \
        libxml2-dev \
        xorg-dev \
        ssh \
        curl \
        libfreetype6-dev \
        libpng12-dev \
        libxft-dev \
        xvfb \
        freeglut3 \
        freeglut3-dev \
        libgl1-mesa-dri \
        libgl1-mesa-glx \
        rsync \
        vim \
        less \
        xauth \
        python-tk \
        swig && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*  && \
    pip install \
        numpy \
        jupyter \
        plotly \
        mpi4py \
        matplotlib \
        runipy \
        pillow \
        pyvirtualdisplay \
        ipyparallel \
        scipy && \
    pip install 'ipython==4.2.0' --force-reinstall && \
    pip install six --upgrade

# script for xvfb-run.  all docker commands will effectively run under this via the entrypoint
RUN printf "#\041/bin/sh \n rm -f /tmp/.X99-lock && xvfb-run -s '-screen 0 1600x1200x16' \$@" >> /usr/local/bin/xvfbrun.sh && \
    chmod +x /usr/local/bin/xvfbrun.sh


# set working directory to /root
WORKDIR /root

# setup environment
ENV PYTHONPATH $PYTHONPATH:/root/underworld2

# get underworld, compile, delete some unnecessary files, trust notebooks, copy to workspace
RUN git clone --branch "development" --single-branch https://github.com/underworldcode/underworld2 && \
    cd underworld2/libUnderworld && \
    ./configure.py --with-debugging=1  && \
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
    find /root/underworld2/docs -name \*.ipynb  -print0 | xargs -0 jupyter trust && \
    rsync -av /root/underworld2/docs/examples /workspace             && \
    rsync -av /root/underworld2/docs/user_guide /workspace           && \
    rsync -av /root/underworld2/docs/publications /workspace 

# expose notebook port
EXPOSE 8888
# expose glucifer port
EXPOSE 9999

# run tests
RUN cd /root/underworld2 && \
    xvfbrun.sh ./test_basic.sh

# note we also use xvfb which is required for viz
ENTRYPOINT ["/usr/local/bin/tini", "--", "xvfbrun.sh"]

WORKDIR /workspace

# launch notebook.. use the badlands version
# CMD ["jupyter", "notebook", " --no-browser"]
CMD /build/run.sh
