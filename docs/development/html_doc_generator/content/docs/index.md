---
title: Underworld 2
---

![Montage.png](./images/Montage.png)

[_Underworld 2_](http://www.underworldcode.org) is a python-friendly version of the Underworld code which provides a programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. This gives signficant advantages to the user, with access to the power of python libraries for setup of complex problems, analysis at runtime, problem steering, and multi physics coupling. While Underworld2 embraces Jupyter Notebooks as the preferred modelling environment, only standard python is required.

The Underworld2 development team is based in Melbourne, Australia at the University of Melbourne and at Monash University led by Louis Moresi. We would like to acknowledge AuScope Simulation, Analysis and Modelling for providing long term funding which has made the project possible. Additional funding for specific improvements and additional functionality has come from the Australian Research Council (http://www.arc.gov.au). The python toolkit was funded by the NeCTAR eresearch_tools program. Underworld was originally developed in collaboration with the Victorian Partnership for Advanced Computing.


| Version | Status |
| :----   | :----  |
| Lastest stable release (master branch) | [![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=master)](http://130.56.252.251:32779/job/master/) |
| Development branch | [![Build Status](http://130.56.252.251:32779/buildStatus/icon?job=uw-dev)](http://130.56.252.251:32779/job/uw-dev/) |

---

## Underworld Installation (docker)

If you decide to run this tutorial on your own machine and want to build new examples, you can most easily install underworld from the docker container.

Most new users may wish to use the [Kitematic GUI](https://github.com/docker/kitematic/releases) to download and run Underworld. Simply search for 'underworldcode/underworld2' within Kitematic, and then click 'CREATE' to launch a container. You will eventually wish to modify your container settings (again through Kitematic) to enable local folder volume mapping, which will allow you to access your local drives within your container.

For Linux users, and those who prefer the command line, the following minimal command should be sufficient to access the Underworld2 Jupyter Notebook examples:

```bash
   docker run --rm -p 8888:8888 underworldcode/underworld2
```

Navigate to `localhost:8888` to see the notebooks.

## Underworld Installation (native)

For installation on HPC facilities and if you would like a local build, you will need to download, install and compile the Underworld code and the relevant dependencies. A native build can be difficult
in a non-standard environment and we are currently working on HPC deployments of containers.

For information on how to build, look on the underworld github pages under [docs/install_guides](https://github.com/underworldcode/underworld2/tree/master/docs/install_guides) and also see the [Underworld blog pages](http://www.underworldcode.org/pages/Blog/)


Privacy
-------

Note that basic usage metrics are dispatched when you use Underworld. We do this to help
assess the usage of our code which is important in justifying our funding. To opt out, set the UW_NO_USAGE_METRICS environment variable. See `PRIVACY.md` for full details. In the docker environment, we only collect information that the docker version has been run.                                    

Bedtime reading
---------------

These papers explain the theory and implementation for Underworld. The code itself can also be cited via the zenodo DOI. There is a master DOI for all releases and releases after 2.6.0 are automatically given a DOI under the master. If you are using a development branch and wish to obtain a DOI for your specific version we ask that you [contact us](mailto:anyone@underworldcode.org) to make an interim release under the master DOI.

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.1436039.svg)](https://doi.org/10.5281/zenodo.1436039)

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2002, Mantle convection modeling with viscoelastic/brittle lithosphere: Numerical methodology and plate tectonic modeling: Pure And Applied Geophysics, v. 159, no. 10, p. 2335–2356, doi: 10.1007/s00024-002-8738-3.

Moresi, L., Dufour, F., and Muhlhaus, H.B., 2003, A Lagrangian integration point finite element method for large deformation modeling of viscoelastic geomaterials: Journal of Computational Physics, v. 184, no. 2, p. 476–497.

Moresi, L., Quenette, S., Lemiale, V., Mériaux, C., Appelbe, W., Mühlhaus, 2007, Computational approaches to studying non-linear dynamics of the crust and mantle: Phys. Earth Planet. Inter, v. 163, p. 69–82, doi: 10.1016/j.pepi.2007.06.009.
