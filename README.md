Underworld 2
============

![Underworld Montage](docs/development/docs_generator/images/Montage.png)

[![DOI](https://joss.theoj.org/papers/10.21105/joss.01797/status.svg)](https://doi.org/10.21105/joss.01797)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1436039.svg)](https://doi.org/10.5281/zenodo.1436039)
[![Build Status](http://115.146.85.138:32779/buildStatus/icon?job=Underworld2%2Fdevelopment)](http://115.146.85.138:32779/job/Underworld2/job/development/)
[![Docs Status](https://readthedocs.org/projects/underworld2/badge/?version=latest)](https://underworld2.readthedocs.io/en/latest/?badge=latest)
[![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/master)

[_Underworld 2_](http://www.underworldcode.org) is a Python API (Application Programming Interface) which provides functionality for the modelling of geodynamics processes, and is designed to work (almost) seamlessly across PC, cloud and HPC infrastructure. Primarily the API consists of a set of Python classes from which numerical geodynamics models may be constructed. The API also provides the tools required for inline analysis and data management. For scalability across multiprocessor platforms, MPI (Message Passing Interface) is leveraged, and for performant operation all heavy computations are executed within a statically typed layer. 

Underworld2 provides capacity for modelling 2- and 3-dimensional geodynamics processes, utilising a particle-in-cell finite element approach for solution to Stokes flow type configurations. In Underworld, the finite element mesh can be static or dynamic, but it is not constrained to move in lock-step with the evolving geometry of the fluid. This hybrid approach allows Underworld to obtain accurate velocity solutions (on the mesh) for a given material configuration, while simultaneously ensuring the accurate advection of material interfaces and history information (using particle swarms).

A primary aim of Underworld2 is to enable rapid prototyping of models, and to this end embedded visualisation ([_LavaVu_](https://github.com/OKaluza/LavaVu)) and modern development environments such as Jupyter Notebooks have been embraced, with the latter also providing a path to cloud computing amenability. The Jupyter Notebook front end provisioned across cloud facilities has also proven to be an appropriate environment for the use of Underworld as a teaching tool for solid Earth geoscience.

Underworld2 provides a minimal set of highly flexible core functionality, with user domain concerns left to the users themselves to construct. At the centre of this design is the Function class, which aims to provide a natural interface from which users can describe their problem mathematics. For example, the user may describe a viscosity which is piecewise constant, temperature dependent, or visco-elasto-plastic in behaviour. Simulation chronology is also fully exposed and the user is expected to explicitly specify when events should occur. This approach allows users to specify exactly their modelling requirements and provides transparency.

The Underworld2 development is led by Louis Moresi and is a collaboration between University of Melbourne, Monash University and the Australian National University. This project was enabled by AuScope and the Australian Government via the National Collaborative Research Infrastructure Strategy (NCRIS): ([auscope.org.au](https://auscope.org.au)). Additional funding for specific improvements and additional functionality has come from the Australian Research Council (http://www.arc.gov.au). The Python toolkit was funded by the NeCTAR eresearch_tools program. Underworld was originally developed in collaboration with the Victorian Partnership for Advanced Computing.

Documentation 
-------------
Please visit the Underworld documentation site for an overview of installation, numerical methods, usage and the API reference. 

https://underworld2.readthedocs.io

In particular, the *Getting Started* section of the User Guide might be useful place to start.

Trying out Underworld2
----------------------

You can try out the code immediately via a Jupyter Binder cloud instance. Be aware that it can take a little while for the site to fire up and that it will time-out after 30 minutes of inactivity and reset if you log back in. 

| | |
|-|-|
| [![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/v2.10.0b) | v2.10.0b (Py3) |
| [![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/v2.9.2b) | v2.9.2b (Py3) |
| [![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/v2.8.2b) | v2.8.2b (Py3) |
| [![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/v2.7.1b) | v2.7.1b (Py2) |
| [![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2/development) | dev (Py3) |


Note that the Binder environment is identical to that obtained through running an Underworld Docker image locally.


Getting Underworld2
-------------------
The Underworld2 Docker container is the recommended method of installation on Windows, Mac OSX and Linux. Native compilation is generally not recommended for personal computers, although may be necessary in HPC environments which do not support containerisation. 

Please refer to the documentation site for further information.


Where to find things
--------------------
- **underworld**                 - Underworld python modules.
- **docs/examples**              - Basic usage examples.
- **docs/user_guide**            - The user guide. 
- **docs/cheatsheet**            - One pager with quick examples.
- **docs/install_guides**        - HPC installation guides.
- **docs/tests**                 - Development tests. 

Privacy
-------
Note that basic usage metrics are dispatched when you use Underworld. To opt out, set the UW_NO_USAGE_METRICS environment variable. See `PRIVACY.md` for full details.                                   

Licensing
---------
Underworld is an open-source project licensed under LGPL Version 3. See `LICENSE.md` and `LGPLv3.txt` for details.

Note that for releases `2.9.0`, `2.9.1`, `2.9.2`, `2.9.3` and `2.9.4`, the incorrect text was erroneously included within `LGPLv3.txt`, although these releases must still to be considered as licensed under LGPL Version 3.

Citing Underworld
-----------------
If you use this software, please remember to cite it using one or more of the following references.

To cite the Underworld2 software package, please use:

Mansour, J., Giordani, J., Moresi, L., Beucher, R., Kaluza, O., Velic, M., Farrington, R., Quenette, S., Beall, A., 2020, Underworld2: Python Geodynamics Modelling for Desktop, HPC and Cloud, Journal of Open Source Software, 5(47), 1797, https://doi.org/10.21105/joss.01797

If you would also like to cite the codebase itself, you may use the Zenodo reference:

https://doi.org/10.5281/zenodo.1436039

Note that the above DOI URL will always resolve to the latest Underworld2 release. DOIs for previous versions are also available via the above DOI.

Details of the algorithms are citeable via the following references:

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, https://doi.org/10.1007/s00024-002-8738-3

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497, https://doi.org/10.1016/S0021-9991(02)00031-1

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, https://doi.org/10.1016/j.pepi.2007.06.009

