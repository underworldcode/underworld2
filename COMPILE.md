Underworld 2 Compilation
========================

Note that for most personal computer usage, we recommend using Underworld
through a Docker container environment. See README.md for details. 

For HPC usage, you will generally need to compile Underworld, though we 
support Shifter where available (check with the HPC admins). Basic compilation
information is below, though please submit a github issue if you have have
difficulties. 

Dependencies
-------------
  * MPI
  * PETSc 
  * numpy
  * swig
  * h5py


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
    $ ./compile.py
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

h5py-mpi
--------
Underworld now supports non-mpi versions of `h5py`. Note however that for large parallel simulations,
saving data to disk may become a bottleneck, and collective IO via MPI-enabled `h5py` is recommended.
To install `h5py` with MPI enabled, you will first required a parallel enabled version of `libhdf5`. 
The following command may be useful for installed MPI-enabled `h5py`:

```bash
    $ CC=mpicc HDF5_MPI="ON" HDF5_DIR=/path/to/your/hdf5/install/ pip install --no-binary=h5py h5py
```

or alternatively you might use `CC=h5pcc` (if available). Please check the `h5py` site for more information.
Underworld will automatically perform `save()`/`load()` operations collectively if MPI-enabled `hdf5` is 
available. 

