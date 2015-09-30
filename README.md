Underworld 2 / python underworld
================================

_Underworld 2_ is a python-friendly version of the Underworld code which provides a  programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. This gives signficant advantages to the user, with access to the power of python libraries for setup of complex problems, analysis at runtime, problem steering, and coupling of multiple problems. The python toolkit was funded by the NeCTAR [eResearch tools][eresearch_tools] program.

Information 
-----------

[underworld_website](http://www.underworldproject.org)

[testing underworld](https://130.56.248.95:8080/job/underworld2/buildTimeTrend)  

[eresearch_tools](http://www.nectar.org.au/eresearch-tools)

Dependencies
------------
  * PETSc 
  * MPI
  * hdf5 (optional, but recommended)
  * numpy
  * libpng
  * build tools (gcc etc)

Note: Headers will be required for above libraries (-dev packages).  Required libraries regularly included in system distributions are not listed. 


Compilation
-----------
```bash
    $ cd libUnderworld
    $ ./configure.py
    $ ./scons.py
    $ cd libUnderworldPy ; ./swigall.py ; cd ../
    $ ./scons.py
```

Check available configuration options using `./configure.py --help`.  
Library testing options may be found by running `./scons.py --help`.

Usage
-----
You will first need to make the project directory available to import within python:
```bash
    $ export PYTHONPATH=$PYTHONPATH:/top/directory/of/project
```
(note that if you are not using the bash shell, the required command will be different.)

Various example files are provided in `InputFiles` and may be run as follows:

```bash
    $ cd InputFiles
    $ python LidDriven.py
```


Bedtime reading
---------------

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, doi: 10.1007/s00024-002-8738-3.

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497.

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, doi: 10.1016/j.pepi.2007.06.009.
