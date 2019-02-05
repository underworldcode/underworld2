Underworld 2
============

![Underworld Montage](docs/development/docs_generator/images/Montage.png)


[_Underworld 2_](http://www.underworldcode.org) is a Python friendly version of the Underworld code which provides a programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. This gives significant advantages to the user, with access to the power of Python libraries for setup of complex problems, analysis at runtime, problem steering, and multi physics coupling. While Underworld2 embraces Jupyter Notebooks as the preferred modelling environment, only standard Python is required.

The Underworld2 development team is based in Melbourne, Australia at the University of Melbourne and at Monash University led by Louis Moresi. We would like to acknowledge AuScope Simulation, Analysis and Modelling for providing long term funding which has made the project possible. Additional funding for specific improvements and additional functionality has come from the Australian Research Council (http://www.arc.gov.au). The python toolkit was funded by the NeCTAR eresearch_tools program. Underworld was originally developed in collaboration with the Victorian Partnership for Advanced Computing.

[![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=master)](http://130.56.252.251:32779/job/master/) Lastest stable release (master branch) 

[![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=Underworld2/development)](http://130.56.252.251:32779/job/Underworld2/job/development/) Development branch

The Underworld2 code can be cited via Zenodo. Details of the algorithms are citeable from the references listed at the bottom of this page.
 
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1436040.svg)](https://doi.org/10.5281/zenodo.1436040)


Documentation 
-------------

Please visit the Underworld documentation site for an overview of installation, numerical methods, usage and the API reference. 

https://underworld2.readthedocs.io

In particular, the *Getting Started* section of the User Guide might be useful place to start.

Trying out Underworld2
----------------------

You can try out the code immediately via a Jupyter Binder cloud instance. Be aware that it can take a little while for the site to fire up and that it will time-out after 30 minutes of inactivity and reset if you log back in. 

[![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/master)

Note that the Binder environment is identical to that obtained through running an Underworld Docker image locally.


Getting Underworld2
-------------------
The Underworld2 Docker container is the recommended method of installation on Windows, Mac OSX and Linux. Native compilation is generally not recommended for personal computers, although may be necessary in HPC environments which do not support containerisation. 

Please refer to the documentation site for further information.


Directories
-----------
- **underworld**                 - Underworld python modules.
- **glucifer**                   - gLucifer python modules.
- **docs**                       - Various documentation.
- **libUnderworld**              - Backend library

Privacy
-------
Note that basic usage metrics are dispatched when you use Underworld. To opt out, set the UW_NO_USAGE_METRICS environment variable. See `PRIVACY.md` for full details.                                   

Bedtime reading
---------------

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, doi: 10.1007/s00024-002-8738-3.

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497.

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, doi: 10.1016/j.pepi.2007.06.009.
