Underworld 2
============

[_Underworld 2_](http://www.underworldcode.org) is a python-friendly version of the Underworld code which provides a programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. This gives signficant advantages to the user, with access to the power of python libraries for setup of complex problems, analysis at runtime, problem steering, and multi physics coupling. While Underworld2 embraces Jupyter Notebooks as the preferred modelling environment, only standard python is required.

The Underworld2 development team is based in Melbourne, Australia at the University of Melbourne and at Monash University led by Louis Moresi. We would like to acknowledge AuScope Simulation, Analysis and Modelling for providing long term funding which has made the project possible. Additional funding for specific improvements and additional functionality has come from the Australian Research Council (http://www.arc.gov.au). The python toolkit was funded by the NeCTAR eresearch_tools program. Underworld was originally developed in collaboration with the Victorian Partnership for Advanced Computing.

Lastest stable release (master branch) [![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=master)](http://130.56.252.251:32779/job/master/)

Development branch - [![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=uw-dev)](http://130.56.252.251:32779/job/uw-dev/)

The Underworld 2 code can be cited via Zenodo [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1436040.svg)](https://doi.org/10.5281/zenodo.1436040)

Details of the algorithms are citeable from the references listed below. 

Trying out Underworld2
----------------------

You can try out the code at the mybinder.org site. Be aware that it can take a little while for the site to fire up - and that it will time-out after 30 minutes and reset if you log back in. 

[![Binder](https://mybinder.org/badge.svg)](https://mybinder.org/v2/gh/underworldcode/underworld2-documentation/master)

The mybinder.org installation is running our docker image which you can easily download and run on your own machine. 

### Installation

The Underworld2 docker container is the recommended method of installation for Windows, Mac OSX and Linux. You will need to install [Docker](https://www.docker.com) on your system. The Underworld docker container is available through the docker hub:

https://hub.docker.com/r/underworldcode/underworld2/

Please check our blog page for a writeup on using dockers, and also recommendations for native installation on various platforms. 

http://www.underworldcode.org/pages/Blog/

If you need to compile Underworld2 (in particular for HPC usage), please refer to COMPILE.md in this repository.

API documentation is available at http://underworld2.readthedocs.io/


### Launching from the Underworld Docker Image


Most new users may wish to use the Kitematic GUI to download and run Underworld. Simply search for 'underworldcode/underworld2' within Kitematic, and then click 'CREATE' to launch a container. You will eventually wish to modify your container settings (again through Kitematic) to enable local folder volume mapping, which will allow you to access your local drives within your container. 

For Linux users, and those who prefer the command line, the following minimal command should be sufficient to access the Underworld2 Jupyter Notebook examples: 

```bash
   docker run -p 8888:8888 underworldcode/underworld2
```


Privacy
-------
Note that basic usage metrics are dispatched when you use Underworld. To opt out, set the UW_NO_USAGE_METRICS environment variable. See `PRIVACY.md` for full details.                                   

Bedtime reading
---------------

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, doi: 10.1007/s00024-002-8738-3.

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497.

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, doi: 10.1016/j.pepi.2007.06.009.
