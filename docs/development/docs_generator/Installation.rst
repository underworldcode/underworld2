Underworld Installation
=======================

Docker
------

Docker is a type of lightweight virtualisation, and is the preferred method for Underworld usage on personal computers. You will first need to install Docker on your system, and then you may install Underworld via Docker. Docker may be driven from the command line, but new users may wish to use the Docker Kitematic GUI instead for ease. Simply search for 'underworldcode/underworld2' within Kitematic, and then click 'CREATE' to launch a container. You will eventually wish to modify your container settings (again through Kitematic) to enable local folder volume mapping, which will allow you to access your local drives within your container.

For Linux users, and those who prefer the command line, the following minimal command should be sufficient to access the Underworld2 Jupyter Notebook examples:

.. code:: bash

   docker run -p 8888:8888 underworldcode/underworld2

Navigate to `localhost:8888 <localhost:8888>`_ to see the notebooks. A number of useful commands are provided within the Underworld cheat-sheet.

Windows users should note that for *Windows 10 Home* you should install *Docker Toolbox*, while for *Windows 10 Professional* you should install *Docker for Windows*.  The *Docker Toolbox* edition utilised VirtualBox for virtualisation, and therefore to access any running Jupyter servers you must browse to the virtual machine address (instead of *localhost*). To find the VM address, you will generally execute

.. code:: bash

   docker-machine ip default

but note that this will only work correctly from the *Docker Quickstart Terminal*.

Native
------

For installation on HPC facilities and if you would like a native local build, you will need to download, install and compile the Underworld code and relevant dependencies. A native build can be difficult in non-standard environments and we are currently investigating HPC deployment of Docker containers.

For information on how to build, please refer to the top level `COMPILE.md <https://github.com/underworldcode/underworld2/tree/master/COMPILE.md>`_ file. Instructions for HPC builds may be found at `docs/install_guides
<https://github.com/underworldcode/underworld2/tree/master/docs/install_guides>`_. You may also find useful build related information at the `Underworld blog pages <http://www.underworldcode.org/pages/Blog/>`_.
