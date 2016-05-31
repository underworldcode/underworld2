Underworld 2 Compilation
========================

Dependencies
-------------
  * MPI
  * PETSc 
  * numpy
  * libpng
  * swig
  * libhdf5-mpi
  * h5py-mpi (optional)

Note that h5py (as installed via pip) is usually not parallel enabled. If you have an installation of libhdf5 (parallel), Underworld will attempted to download and build its own h5py during the configuration stage. If you appear to be having difficulties with h5py, it is often useful to try and import it directly from the top underworld level. 

Getting the code
----------------
```bash
git clone https://github.com/underworldcode/underworld2.git
```

Compilation
-----------

We periodically post build recipes for particular platforms on the [underworld blog](http://www.underworldcode.org/pages/Blog) but the build strategy generally looks like this


```bash
    $ cd libUnderworld
    $ ./configure.py
    $ ./scons.py
    $ cd libUnderworldPy ; ./swigall.py ; cd ../
    $ ./scons.py
```

Check available configuration options using `./configure.py --help`.  
Library testing options may be found by running `./scons.py --help`.

Self compiled Underworld2 usage
-------------------------------

You will first need to make the project directory available to import within python:
```bash
    $ export PYTHONPATH=$PYTHONPATH:/top/directory/of/project
```
(note that if you are not using the bash shell, the required command will be different.)

Various example files are provided in the `docs` directory in either ipython notebook format or in python format.

