Underworld Installation
=======================

Underworld may be installed via a number of mechanisms: Docker, pip or native install. The recommended option is using Docker as it is quick to get started.

Detailed instructions for supported HPC platforms may be found at `docs/install_guides
<https://github.com/underworldcode/underworld2/tree/master/docs/install_guides>`_. You may also find useful usage information (on docker/shifter/compilation/other) on the Underworld blog.

We welcome feedback and support requests at our `github issue tracker` <https://github.com/underworldcode/underworld2/issues>`_.

Docker
------

Docker is a type of lightweight virtualisation, and is the preferred method for Underworld usage on personal computers. You will first need to install Docker on your system (see `Notes on Installing Docker`_) and then you may install Underworld via Docker. Docker can be driven from the command line, but new users may wish to use the Docker Kitematic GUI instead for ease. Simply search for 'underworldcode/underworld2' within Kitematic, and then click 'CREATE' to launch a container. You will eventually wish to modify your container settings (again through Kitematic) to enable local folder volume mapping, which will allow you to access your local drives from within your container.

For Linux users, and those who prefer the command line, the following minimal command should be sufficient to access the Underworld2 Jupyter Notebook examples:

.. code:: bash

   docker run -p 8888:8888 underworldcode/underworld2

Navigate to `localhost:8888 <localhost:8888>`_ to see the notebooks. A number of useful docker commands are provided within the Underworld cheat-sheet (in 'docs/cheatsheet'). Note that you can also use particular versions of Underworld using any of the Docker image tags. For example:

.. code:: bash

   docker run -p 8888:8888 underworldcode/underworld2:2.7.1b

By default (ie, if no tag is provided), docker will use the latest stable release. A list of available tags may be found on `our DockerHub page <https://hub.docker.com/r/underworldcode/underworld2/tags>`_. Tags can also be accessed in Kitematic via the ellipsis button on container search results. 

Notes on Installing Docker
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Linux users should be able to install *docker* using the distributions's standard package manager. 
- Windows users should note that for *Windows 10 Home* you should install *Docker Toolbox*, while for *Windows 10 Professional* you should install *Docker Desktop*.  
- All users on Apple OS X should use *Docker Desktop* (not *Docker Toolbox*). The *Docker Toolbox* edition utilised VirtualBox for virtualisation, and therefore to access any running Jupyter servers you must browse to the virtual machine address (instead of *localhost*). To find the VM address, you will generally execute

  .. code:: bash

     docker-machine ip default

  but note that this will only work correctly from the *Docker Quickstart Terminal*.

Native
------

Native installation of Underworld is now supported either directly using the Underworld build system, or via the `pip` Python package manager. In either case, you will need to satisfy the requirements listed below. For usage on HPC facilities you will generally need to generate a native build, although container usage via Shifter or Singularity is now available on a number of platforms (Pawsey-Magnus & TACC-stampede2 for example). 

Build-System
~~~~~~~~~~~~

Once you have satisfied the requirements below, you will need to obtain Underworld:

.. code:: bash

   git clone https://github.com/underworldcode/underworld2.git

You can then perform configuration & compilation as follows:

.. code:: bash

   cd underworld/libUnderworld
   ./configure.py --prefix=/underworld/install/directory
   ./compile.py 
   ./scons.py install 

Check available configuration options using `./configure.py --help`.  

Once your build is complete, you will need to update your `PYTHONPATH` so that Python knows where to find Underworld.

.. code:: bash

   export PYTHONPATH=$PYTHONPATH:/underworld/install/directory

(note that if you are not using the `bash` shell, the required command will be different.)

Pip Install
~~~~~~~~~~~

You can install Underworld using pip as follows:

.. code:: bash

   pip3 install -v git+https://github.com/underworldcode/underworld2@development

Note that installation via `pip` is still experimental and may not be robust. 

Requirements
~~~~~~~~~~~~

**PETSc**: PETSc can be installed via `pip` these days, or is usually available via platform package managers (such as `apt` on Ubuntu as `petsc-dev`). If you have PETSc installed in a non-standard location, please set the `PETSC_DIR` environment variable to specify the required location.

**MPI & mpi4py**: You will need an implementation of MPI installed on your system. Underworld is commonly used with MPICH and OpenMPI. You will also need to install the `mpi4py` package (via `pip`) which provides Python bindings to the MPI library. If non-standard, you can specify the wrapped compilers by setting the `MPICC` and `MPICXX` environment variables.

**h5py**: The standard `h5py` (installed via `pip`) is the recommended version for desktop usage. However, note that it will be the non-parallel enabled version, and for large parallel simulations saving/reading data may become a bottleneck, and collective IO via MPI-enabled `h5py` is recommended. The following command may be useful for installed MPI-enabled `h5py` where necessary:

.. code:: bash

   CC=mpicc HDF5_MPI="ON" HDF5_DIR=/path/to/your/hdf5/install/ pip install --no-binary=h5py h5py

or alternatively you might use `CC=h5pcc` (if available). Note that you will also need to have a parallel `HDF5` library installed. Please check the `h5py` site for more information. Underworld will automatically perform `save()`/`load()` operations collectively if MPI-enabled `hdf5` is available. 

**lavavu**: For rendering of visualisations, you will also need to install `lavavu` (via pip). Please check the `lavavu page <https://github.com/lavavu/LavaVu>`_ for further installation instructions. 

**Other**: The following should also be installed via a system package manager (such as apt on Ubuntu): **swig**, **git** and  **libxml2-dev** (or equivalent). The following should be installed via pip: **scons** and **numpy**.

Testing
~~~~~~~

A script to run a suite of tests may be found at the top level of the project. Simply execute it to run tests:

.. code:: bash

   ./test_basic.sh

Those using `pip` installation will of course need to download the repository first. 
