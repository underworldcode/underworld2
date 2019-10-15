Underworld 2
============

![Underworld Montage](docs/development/docs_generator/images/Montage.png)


[_Underworld 2_](http://www.underworldcode.org) is a Python API (Application Programming Interface) which provides functionality for the modelling of geodynamics processes, and is designed to work (almost) seamlessly across PC, cloud and HPC infrastructure. Primarily the API consists of a set of Python classes from which numerical geodynamics models may be constructed. The API also provides the tools required for inline analysis and data management. For scalability across multiprocessor platforms, MPI (Message Passing Interface) is leveraged, and for performant operation all heavy computations are executed within a statically typed layer. 

Development branch - [![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=uw-dev)](http://130.56.252.251:32779/job/uw-dev/) [![Join the chat at https://gitter.im/underworld2/Lobby](https://badges.gitter.im/underworld2/Lobby.svg)](https://gitter.im/underworld2/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)
=======

Underworld2 provides capacity for modelling 2- and 3-dimensional geodynamics processes, utilising a particle-in-cell finite element approach for solution to Stokes flow type configurations. In Underworld, the finite element mesh can be static or dynamic, but it is not constrained to move in lock-step with the evolving geometry of the fluid. This hybrid approach allows Underworld to obtain accurate velocity solutions (on the mesh) for a given material configuration, while simultaneously ensuring the accurate advection of material interfaces and history information (using particle swarms).

A primary aim of Underworld2 is to enable rapid prototyping of models, and to this end embedded visualisation ([_LavaVu_](https://github.com/OKaluza/LavaVu)) and modern development environments such as Jupyter Notebooks have been embraced, with the latter also providing a path to cloud computing amenability. The Jupyter Notebook front end provisioned across cloud facilities has also proven to be an appropriate environment for the use of Underworld as a teaching tool for solid Earth geoscience.

Underworld2 provides a minimal set of highly flexible core functionality, with user domain concerns left to the users themselves to construct. At the centre of this design is the Function class, which aims to provide a natural interface from which users can describe their problem mathematics. For example, the user may describe a viscosity which is piecewise constant, temperature dependent, or visco-elasto-plastic in behaviour. Simulation chronology is also fully exposed and the user is expected to explicitly specify when events should occur. This approach allows users to specify exactly their modelling requirements and provides transparency.

The Underworld2 development is led by Louis Moresi and is a collaboration between University of Melbourne, Monash University and the Australian National University. This project was enabled by AuScope and the Australian Government via the National Collaborative Research Infrastructure Strategy (NCRIS): ([auscope.org.au](https://auscope.org.au)). Additional funding for specific improvements and additional functionality has come from the Australian Research Council (http://www.arc.gov.au). The Python toolkit was funded by the NeCTAR eresearch_tools program. Underworld was originally developed in collaboration with the Victorian Partnership for Advanced Computing.

Underworld is an open-source project licensed under LGPL-3. See `LICENSE.md` for details.

[![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=Underworld2/master)](http://130.56.252.251:32779/job/Underworld2/job/master/) Latest stable release (master branch) 

[![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=Underworld2/development)](http://130.56.252.251:32779/job/Underworld2/job/development/) Development branch

The Underworld2 code can be cited via Zenodo. Details of the algorithms are citeable from the references listed at the bottom of this page.
 
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1436039.svg)](https://doi.org/10.5281/zenodo.1436039)

Documentation 
-------------

Please visit the Underworld documentation site for an overview of installation, numerical methods, usage and the API reference. 

https://underworld2.readthedocs.io

In particular, the *Getting Started* section of the User Guide might be useful place to start.

Trying out Underworld2
----------------------

You can try out the code immediately via a Jupyter Binder cloud instance. Be aware that it can take a little while for the site to fire up and that it will time-out after 30 minutes of inactivity and reset if you log back in. 

[![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/v2.8.0b) v2.8.0 (Python 3)

[![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/v2.7.1b) v2.7.1 (Python 2)

[![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/development) Development (Python 3)


Note that the Binder environment is identical to that obtained through running an Underworld Docker image locally.


Getting Underworld2
-------------------
The Underworld2 Docker container is the recommended method of installation on Windows, Mac OSX and Linux. Native compilation is generally not recommended for personal computers, although may be necessary in HPC environments which do not support containerisation. 

Please refer to the documentation site for further information.

Note that we now have experimental support for `pip` installation on the dev branch, as well as extended installation documentation. 
You can switch to the `development` branch of this repo via the GitHub interface above. Also note that the `development` branch has significant API changes, so please refer to the corresponding documentation and examples if using a `development` installation:

https://underworld2.readthedocs.io/en/development/


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
