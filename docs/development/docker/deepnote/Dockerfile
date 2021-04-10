FROM gcr.io/deepnote-200602/templates/deepnote

RUN sudo apt-get update -qq
RUN DEBIAN_FRONTEND=noninteractive sudo apt-get install -yq --no-install-recommends bash-completion build-essential git python3-minimal python3-dev python3-pip libxml2-dev xorg-dev ssh curl libfreetype6-dev libpng-dev libxft-dev xvfb freeglut3 freeglut3-dev libgl1-mesa-dri libgl1-mesa-glx python3-tk rsync vim less xauth swig gdb python3-dbg cmake python3-setuptools wget ca-certificates gfortran petsc-dev

RUN pip3 install --no-cache-dir packaging h5py appdirs numpy plotly matplotlib pillow pyvirtualdisplay ipython ipyparallel pint sphinx sphinx_rtd_theme sphinxcontrib-napoleon mock scipy tabulate mpi4py scons
RUN pip3 install lavavu
RUN pip3 install git+https://github.com/underworldcode/underworld2
RUN pip3 install UWGeodynamics

#  hack to force xvfb
RUN sed -i 's/in _os/not in _os/g' /opt/venv/lib/python3.7/site-packages/underworld/visualisation/__init__.py