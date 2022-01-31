Underworld Installation
=======================

Detailed instructions for supported HPC platforms may be found at `docs/install_guides
<https://github.com/underworldcode/underworld2/tree/master/docs/install_guides>`_. You may also find useful usage information (on docker/hpc/compilation/other) on the Underworld blog.

We welcome feedback and support requests at our `github issue tracker <https://github.com/underworldcode/underworld2/issues>`_.

Native Installation
-------------------

Requirements
~~~~~~~~~~~~

Build environment
*****************

* cmake >= 3.16
* MPICH or OPENMPI
* PETSc (<= 3.16.1)
* HDF5  
* SWIG (>= 4.0) 
* LibXML (>= 2.0)
* Python (>3.5, <= 3.9.9) 
* Numpy

Running Requirements
********************

* h5py: The standard `h5py` (installed via `pip`) is the recommended version for desktop usage. However, note that it will be the non-parallel enabled version, and for large parallel simulations saving/reading data may become a bottleneck, and collective IO via MPI-enabled `h5py` is recommended. The following command may be useful for installed MPI-enabled `h5py` where necessary:

   .. code:: bash

      CC=mpicc HDF5_MPI="ON" HDF5_DIR=/path/to/your/hdf5/install/ pip install --no-binary=h5py h5py

   or alternatively you might use `CC=h5pcc` (if available). Note that you will also need to have a parallel `HDF5` library installed. Please check the `h5py` site for more information. Underworld will automatically perform `save()`/`load()` operations collectively if MPI-enabled `hdf5` is available. 

* mpi4py
* pint
* lavavu: For rendering of visualisations, you will also need to install `lavavu` (via pip). Please check the `lavavu page <https://github.com/lavavu/LavaVu>`_ for further installation instructions. 


Building Underworld
~~~~~~~~~~~~~~~~~~~~

You can install Underworld using pip as follows:

.. code:: bash

   pip install underworld2

or using our GitHub repository:

.. code:: bash

   pip install -v git+https://github.com/underworldcode/underworld2


Testing
~~~~~~~

Simple tests can be run to check your installation. They will require you to install *pytest* (usually via pip).
Note that some tests also require *matplotlib*

* pytest

.. code:: bash

   pytest -vvv ./docs/pytests


Docker
------

Docker is a type of lightweight virtualisation, and is the preferred method for Underworld usage on personal computers. You will first need to install Docker on your system (see `Notes on Installing Docker`_) and then you may install Underworld via Docker. Docker can be driven from the command line, but new users may wish to use the Docker Kitematic GUI instead for ease. Simply search for 'underworldcode/underworld2' within Kitematic, and then click 'CREATE' to launch a container. You will eventually wish to modify your container settings (again through Kitematic) to enable local folder volume mapping, which will allow you to access your local drives from within your container.

For Linux users, and those who prefer the command line, the following minimal command should be sufficient to access the Underworld2 Jupyter Notebook examples:

.. code:: bash

   docker run -p 8888:8888 underworldcode/underworld2

Navigate to `localhost:8888 <localhost:8888>`_ to see the notebooks. Note that you can also use particular versions of Underworld using any of the Docker image tags. For example:

.. code:: bash

   docker run -p 8888:8888 underworldcode/underworld2:2.7.1b

By default (ie, if no tag is provided), docker will use the latest stable release. A list of available tags may be found on `our DockerHub page <https://hub.docker.com/r/underworldcode/underworld2/tags>`_. Tags can also be accessed in Kitematic via the ellipsis button on container search results. 

A number of useful docker commands are provided within the `Underworld cheat-sheet <https://github.com/underworldcode/underworld2/tree/master/docs/cheatsheet/cheatsheet.pdf>`_.

Notes on Installing Docker
~~~~~~~~~~~~~~~~~~~~~~~~~~

- Linux users should be able to install *docker* using the distribution's standard package manager. 
- Windows users should note that for *Windows 10 Home* you should install *Docker Toolbox*, while for *Windows 10 Professional* you should install *Docker Desktop*.  
- All users on Apple OS X should use *Docker Desktop* (not *Docker Toolbox*). The *Docker Toolbox* edition utilised VirtualBox for virtualisation, and therefore to access any running Jupyter servers you must browse to the virtual machine address (instead of *localhost*). To find the VM address, you will generally execute

  .. code:: bash

     docker-machine ip default

  but note that this will only work correctly from the *Docker Quickstart Terminal*.