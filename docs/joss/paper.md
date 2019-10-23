---
title: 'Underworld2: Python Geodynamics Modelling for Desktop, HPC and Cloud'
tags:
  - Python
  - geodynamics
  - HPC
  - FEM
  - subduction
authors:
  - name: John Mansour
    orcid: 0000-0001-5865-1664
    affiliation: 1
  - name: Julian Giordani 
    orcid: 0000-0003-4515-9296
    affiliation: 2
  - name: Louis Moresi
    orcid: 0000-0003-3685-174X
    affiliation: "3,2"
  - name: Owen Kaluza  
    orcid: 0000-0001-6303-5671
    affiliation: 1
  - name: Romain Beucher  
    orcid: 0000-0003-3891-5444
    affiliation: 2
  - name: Rebecca Farrington  
    orcid: 0000-0002-2594-6965
    affiliation: 2
  - name: Steve Quenette
    orcid: 0000-0002-0368-7341
    affiliation: 1
  - name: Adam Beall
    orcid: 0000-0002-7182-1864
    affiliation: 4

affiliations:
 - name: Monash eResearchCentre, Monash University, Clayton, Australia.
   index: 1
 - name: School of Earth Science, The University of Melbourne, Melbourne, Australia.
   index: 2
 - name: Research School of Earth Sciences, The Australian National University, Canberra, Australia.
   index: 3
 - name: School of Earth and Ocean Sciences, Cardiff University, Cardiff, UK.
   index: 4
date: 03 September 2019
bibliography: paper.bib
---

# Summary

--> This TODO needs to be resolved
<!--
TODO:
* add Mirko to authors
-->

The modelling of long time-scale solid-Earth processes presents challenges on many fronts. Typically a large range of length scales must be resolved, and computational requirements can easily go beyond a desktop machine's capability, even in 2-dimensions. 
Rheological behaviours are diverse and modellers often wish to investigate a spectrum of dynamics. With this also comes the requirement of robust solution strategies capable of capturing a system of large dynamic range. Time dependent aspects of a model also require careful consideration such that numerical artefacts do not subsume the required signal. ``Underworld2`` is a Python API which seeks to meet these challenges, while also providing a friendly and intuitive interface to model development. 

Designed to work (almost) seamlessly across PC, cloud and HPC infrastructure, ``Underworld2`` consists of a set of Python classes, instances of which will be used to construct numerical geodynamics models. The API also provides the tools required for inline analysis and data management. `Underworld2` continues the legacy of Underworld1 [@MoresiEtAl2002] [@MoresiEtAl2003] [@MoresiEtAl2007], sharing much of the underlying mathematical approach. But `Underworld2` is not a Python wrapper to Underworld1, and modelling paradigms have been completely redesigned to best fit the Python language. 

`Underworld2` provides capacity for modelling 2- and 3-dimensional geodynamics processes, utilising a particle-in-cell finite element approach for solving Stokes flow type configurations. For the multi-processor scalability required of high-resolution models, we use MPI (Message Passing Interface) based domain-decomposition, and for performant operation all heavy computations are executed within a statically typed layer.
In `Underworld2`, the finite element mesh can be static or dynamic, but it is not constrained to move in lock-step with the evolving geometry of the fluid. This hybrid approach allows for the accurate solution to the velocity problem (on the mesh) for a given material configuration, while simultaneously ensuring the accurate tracking of material interfaces and history information (using particle swarms). Templated systems are provided for solutions to Stokes flow, steady state heat (or Darcy) flow, and time-dependent advection-diffusion models. 

A primary aim of `Underworld2` is to enable rapid and frictionless model development. To this end, the rich syntactic possibilities of Python have been leveraged empowering users with intuitive access to complex operations, while paternalistic patterns help to guarantee data integrity and ensure consistent parallel operation. By design, `Underworld2` provides only a minimal set of flexible core functionality, and leaves _user domain_ concerns to the users themselves to construct. At the centre of this design is the _Function_ class, which aims to provide a natural interface for the description of problem dynamics. For example, the user may describe a viscosity which is piecewise constant, temperature dependent, or visco-elasto-plastic in behaviour. Simulation chronology is also fully exposed and the user is expected to explicitly specify when events should occur. This free-form approach allows users to define exactly the physics they require, and more importantly provides transparency. 

<!-- It also prevents API bloat, helping to ensure the long term viability of the project. -->

the second reference in the next paragraph does not work in the pdf
Although not a requirement, `Underworld2` is very much at home within the Jupyter [@Kluyver:2016aa] environment. When coupled with the `lavavu` [owen_kaluza_2019_3348451] visualisation engine, users are able to interactively develop models piecemeal. Jupyter also provides a path to cloud amenability, with users accessing their model Notebooks via any standard web browser. Cloud based operation has been used successfully within teaching environments, and also enables our _live_ user documentation and examples. 

`Underworld2` is utilised by the `UWGeodynamics` Python module [@Beucher2019UWGeodynamics], which provides a more structured interface to geodynamics model construction. It has enabled numerous recent publications, including [@BeallEtAl2018], [@MeriauxEtAl2018] and [@SandifordMoresi2019].  

# Acknowledgements

Underworld development was financially supported by AuScope and the Australian Government via the National Collaborative Research Infrastructure Strategy (NCRIS): (auscope.org.au). Additional funding for specific improvements and additional functionality has come from the Australian Research Council (http://www.arc.gov.au). The Python tool-kit was funded by the NeCTAR e-research_tools program.

