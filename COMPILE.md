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
  * h5py-mpi


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

