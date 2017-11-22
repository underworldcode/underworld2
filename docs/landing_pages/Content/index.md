---
layout: home
title: Introduction
description: Getting started with Underworld  
# permalink: index.html   # make this the default home page which will be served at [link](/)
---


## Introduction

Welcome to the Underworld online user environment. Behind this webpage is a full installation of the Underworld code that can be accessed via jupyter notebooks. 

The links in the Navigation bar will take you to  

   * [Home](/files/landing_pages/_site/Content/index.html) - this page
   * [Index](/files/landing_pages/_site/Content/Map.html) - a list of pages / facilities
   * [User Guide](/tree/user_guide/ ) - Step by step introduction to Underworld
   * [Examples](/tree/examples/) - Sample notebooks to solve geodynamics problems
   * [Publications](/tree/publications/) - Reproducing Published results
   * [Terminal](/terminal/1) - a unix terminal running on the backend machine

 You can also use the `Jupyter` icon in the notebooks to come back to this page (the notebook will keep running in the background and you can return to it later)

## What is Underworld ?

Underworld2 is an open-source, python-friendly geodynamics code which provides a programmable and flexible front end to all the functionality of the code running in a parallel HPC environment. Underworld2 shares the same C backend as the original Underworld but the user-interface is dynamic `python` rather than static xml. This gives significant advantages to the user, with access to the power of python libraries for setup of complex problems, analysis at runtime, problem steering, and coupling of multiple problems. Underworld2 is integrated with the literate programming environment of the jupyter notebook system for tutorials and as a teaching tool for solid Earth geoscience.

Underworld is a particle-in-cell finite element code tuned for large-scale geodynamics simulations. The numerical algorithms allow the tracking of history information through the high-strain deformation associated with fluid flow (for example, transport of the stress tensor in a viscoelastic, convecting medium, or the advection of fine-scale damage parameters by the large-scale flow). The finite element mesh can be static or dynamic, but it is not constrained to move in lock-step with the evolving geometry of the fluid. This hybrid approach is very well suited to complex fluids which is how the solid Earth behaves on a geological timescale.

## A scalable code for research and teaching

You are reading a live document that includes both running software examples and tutorial materials. You can navigate freely between the web content and the live notebooks. Underworld can be deployed in massively parallel environments but demonstration models and prototypes can be developed interactively in notebooks. Underworld python scripts do not need any modification to run in parallel.

This tutorial environment can be deployed to a laptop or desktop via docker or installed in the cloud (Google, Amazon ... ).
We blend interactive coding with descriptive comments including mathematics in the notebooks.
We can include static content such as pdf class notes or markdown / html files with mathematics and diagrams.


