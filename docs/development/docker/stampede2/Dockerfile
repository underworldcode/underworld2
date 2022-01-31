FROM ubuntu:bionic
LABEL maintainer="https://github.com/underworldcode/"

RUN  mkdir /home1 /work /scratch /gpfs /corral-repl /corral-tacc /data

########################################
# Install mpi
########################################

RUN apt-get update -qq && \
    DEBIAN_FRONTEND=noninteractive apt-get install -yq --no-install-recommends \
        bash-completion \
        build-essential \
        git \
        python3-minimal \
        python3-dev \
        python3-pip \
        libxml2-dev \
        xorg-dev \
        ssh \
        curl \
        libfreetype6-dev \
        libpng-dev \
        libxft-dev \
        xvfb \
        freeglut3 \
        freeglut3-dev \
        libgl1-mesa-dri \
        libgl1-mesa-glx \
        python3-tk \
        rsync \
        vim \
        less \
        xauth \
        swig \
        gdb \
        python3-dbg \
        cmake \
        python3-setuptools \
        wget \
        ca-certificates \
        bison \
        rdma-core \
        libnuma-dev \
        gfortran  && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install PSM2
ARG PSM=PSM2
ARG PSMV=11.2.78
ARG PSMD=opa-psm2-${PSM}_${PSMV}

RUN curl -L https://github.com/intel/opa-psm2/archive/${PSM}_${PSMV}.tar.gz | tar -xzf - \
    && cd ${PSMD} \
    && make PSM_AVX=1 -j $(nproc --all 2>/dev/null || echo 2) \
    && make LIBDIR=/usr/lib/x86_64-linux-gnu install \
    && cd ../ && rm -rf ${PSMD}

# Install mvapich2-2.3
ARG MAJV=2
ARG MINV=3
ARG DIR=mvapich${MAJV}-${MAJV}.${MINV}

RUN curl http://mvapich.cse.ohio-state.edu/download/mvapich/mv${MAJV}/${DIR}.tar.gz | tar -xzf - \
    && cd ${DIR} \
    && ./configure --with-device=ch3:psm \
    && make -j $(nproc --all 2>/dev/null || echo 2) && make install \
    && mpicc examples/hellow.c -o /usr/bin/hellow \
    && cd ../ && rm -rf ${DIR} && rm -rf /usr/local/share/doc/mvapich2

ENV LANG=C.UTF-8
# Install setuptools and wheel first, needed by plotly
RUN pip3 install -U setuptools  && \
    pip3 install -U wheel       && \
    pip3 install --no-cache-dir packaging \
        appdirs \
        numpy \
        jupyter \
        plotly \
        matplotlib \
        pillow \
        pyvirtualdisplay \
        ipython \
        ipyparallel \
        pint \
        sphinx \
        sphinx_rtd_theme \
        sphinxcontrib-napoleon \
        mock \
        scipy \ 
        tabulate \
        mpi4py

# update python to py3
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3.6 1

WORKDIR /tmp/petsc-build
ARG PETSC_VERSION="3.11.2"
RUN wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-lite-${PETSC_VERSION}.tar.gz && \
    tar zxf petsc-lite-${PETSC_VERSION}.tar.gz && cd petsc-${PETSC_VERSION}   && \
    ./configure --with-debugging=0 --prefix=/usr/local/petsc                     \
                --COPTFLAGS="-g -O3" --CXXOPTFLAGS="-g -O3" --FOPTFLAGS="-g -O3" \
                --with-zlib=1                   \
                --download-fblaslapack=1        \
                --download-ctetgen=1            \
                --download-triangle=1           \
                --download-hdf5=1               \
                --download-mumps=1              \
                --download-parmetis=1           \
                --download-metis=1              \
                --download-superlu=1            \
                --download-hypre=1              \
                --download-scalapack=1          \
                --download-superlu_dist=1       \
                --useThreads=1                  \
                --download-superlu=1            \
                --with-shared-libraries         \
                --with-cxx-dialect=C++11     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt all     && \
    make PETSC_DIR=/tmp/petsc-build/petsc-${PETSC_VERSION} PETSC_ARCH=arch-linux-c-opt install && \
    make PETSC_DIR=/usr/local/petsc PETSC_ARCH="" test && \
    cd /tmp && \
    rm -fr *

ENV PETSC_DIR=/usr/local/petsc
ENV PATH=${PETSC_DIR}/bin:$PATH

ENV PYTHONPATH $PYTHONPATH:/usr/lib
RUN CC=h5pcc HDF5_MPI="ON" HDF5_DIR=${PETSC_DIR} pip3 install --no-cache-dir --no-binary=h5py h5py && \
    pip3 install --no-cache-dir petsc4py

ENV NB_WORK /workspace
# create a volume
VOLUME $NB_WORK/user_data
WORKDIR $NB_WORK

RUN pip3 install --no-cache-dir lavavu

# install underworld under /tmp.
WORKDIR /tmp
ENV UW2_TMP /tmp/underworld2
ENV UW2_DIR /usr/local/underworld2
ENV PYTHONPATH $PYTHONPATH:$UW2_DIR/lib
RUN mkdir -p $UW2_TMP 

COPY . $UW2_TMP/

RUN cd $UW2_TMP && pip3 install -vvv .    

# clone UWGeodynamics, install 
RUN git clone -b development https://github.com/underworldcode/UWGeodynamics.git && \
    pip3 install --no-cache-dir UWGeodynamics/ && \
    mkdir $NB_WORK/UWGeodynamics && \
    mv ./UWGeodynamics/examples   $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/tutorials  $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/benchmarks $NB_WORK/UWGeodynamics/. && \
    mv ./UWGeodynamics/docs       $NB_WORK/UWGeodynamics/  && \
    rm -rf UWGeodynamics

# environment variable will internally run xvfb when uw.visualisation is imported,
# see /opt/underworld2/visualisation/__init__.py
ENV UW_USE_XVFB 1

WORKDIR $NB_WORK


