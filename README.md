Underworld 2
============

[_Underworld 2_](http://www.underworldcode.org) is a python-friendly version of the Underworld code which provides a programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. This gives signficant advantages to the user, with access to the power of python libraries for setup of complex problems, analysis at runtime, problem steering, and multi physics coupling.

While Underworld2 embraces Jupyter Notebooks as the preferred modelling environment, only standard python is required.

Compile | [![Build Status] (http://128.250.120.238:9080/buildStatus/icon?job=Compile)](http://128.250.120.238:9080/job/Compile)

Model Tests | [![Build Status](http://128.250.120.238:9080/job/tests/badge/icon)](http://128.250.120.238:9080/job/tests/)


Getting Underworld2
-------------------
The Underworld2 docker container is the recommended method of installation for Windows, Mac OSX and Linux. It is available through the docker hub:

https://hub.docker.com/r/underworldcode/underworld2/

Please check our blog page for a writeup on using dockers:

http://www.underworldcode.org/pages/Blog/

If you need to compile Underworld2 (in particular for HPC usage), please refer to COMPILE.md


Underworld Docker Usage
------------
To access Underworld2 examples, run the following command,

```bash
   docker run -p 8888:8888 underworldcode/underworld2
```
and then (Linux users) browse to http://localhost:8888. For Windows/Mac, your docker containers will be run within a VM. To determine the address of this VM, check the output of:
```bash
   docker-machine ip default 
```
So, for example, you may need to browse to http://192.168.99.100:8888

Privacy
-------
Note that basic usage metrics are dispatched when you use Underworld. To opt out, set the UW_NO_USAGE_METRICS environment variable. See `PRIVACY.md` for full details.                                   

Bedtime reading
---------------

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, doi: 10.1007/s00024-002-8738-3.

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497.

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, doi: 10.1016/j.pepi.2007.06.009.
