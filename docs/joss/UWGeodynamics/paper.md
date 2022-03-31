---
title: 'UWGeodynamics: A teaching an research tool for numerical geodynamic modelling'
tags:
  - Python
  - geodynamics
  - geophysics
  - geology
  - earth science
  - tectonics
  - rift
  - subduction
  - basin evolution
  - orogenic collision
  - subduction
  - sandbox experiment
  - stokes
authors:
 - name: Romain Beucher
   orcid: 0000-0003-3891-5444
   affiliation: "1"
 - name: Louis Moresi
   orcid: 0000-0003-3685-174X
   affiliation: "1"
 - name: Julian Giordani
   orcid: 0000-0003-4515-9296
   affiliation: "1"
 - name: John Mansour
   orcid: 0000-0001-5865-1664
   affiliation: "2"
 - name: Dan Sandiford
   orcid: 0000-0002-2207-6837
   affiliation: "1"
 - name: Rebecca Farrington
   orcid: 0000-0002-2594-6965
   affiliatioin: "1"
 - name: Luke Mondy
   orcid: 0000-0001-7779-509X
   affiliation:  "3"
 - name: Claire Mallard
   orcid: 0000-0003-2595-2414
   affiliation: "3"
 - name: Patrice Rey
   orcid: 0000-0002-1767-8593
   affiliation: "3"
 - name: Guillaume Duclaux
   orcid: 0000-0002-9512-7252
   affiliation: "4"
 - name: Owen Kaluza
   orcid: 0000-0001-6303-5671
   affiliation: "2"
 - name: Arijit Laik
   orcid: 0000-0002-3484-7985
   affiliation: "5"
 - name: Sara Morón
   orcid: 0000-0002-1270-4377
   affiliation: "1"

affiliations:
 - name: School of Earth Science, The University of Melbourne, Melbourne, Australia
   index: 1
 - name: Monash eResearch Centre, Monash University, Clayton, Australia
   index: 2
 - name: School of Geosciences, Earthbyte Research Group, The University of Sydney, Australia
   index: 3
 - name: Laboratoire Géoazur, Université Nice Sophia Antipolis, Nice, France
   index: 4
 - name: Department of Earth Science, Faculty of Science, Vrije Universiteit, Amsterdam
   index: 5

date: 05 October 2018
bibliography: paper.bib
---

# Summary

The UWGeodynamics module facilitates development of 2D and 3D thermo-mechanical
geodynamic models (Subduction, Rift, Passive Margins, Orogenic systems etc.).
It is designed to be used for research and teaching, and combined the flexibility
of the Underworld Application Programming Interface, 
[@Moresi2002, @Moresi2003, @Moresi2007] with a structured workflow.

Designing geodynamic numerical models can be a daunting task which
often requires good understanding of the numerical code.
UWGeodynamics provides a simple interface with examples to get you started
with development of numerical models. Users can start designing their models without any
pre-existing knowledge of programming. Expert users can easily modify the
framework and adapt it to more specific needs. The code can be run in parallel
on multiple CPUs on personal computers and/or High Performance Computing systems.

Although UWGeodynamics has been primarily designed to address geodynamic
problems, it can also be used to teach fluid dynamics and material mechanics.

UWGeodynamics uses the flexibility of the python language and the Jupyter
notebook environment which allows leveraging the wide range of scientific
libraries available from the python community.
It also facilitates the coupling with existing scientific python modules such
as Badlands [@Salles2018].

The functionalities include:

- Dimensional input values, using user's choice of physical units.
- Automated and transparent scaling of dimensional values.
- Sets of predefined geometries that can be combined to define the
  initial geometry of a model.
- Handles Newtonian and non-Newtonian rheologies (Viscous, Visco-plastic and
  Visco-elasto-plastic).
- Database of common rheologies used in geodynamics, which can be
  personalised / extended by users.
- Simple definition of kinematic, stress, and thermal boundary conditions.
- Lithostatic pressure calculation.
- Thermal equilibrium (steady-state) calculation.
- Pseudo Isostasy using a range of kinematic or stress boundary conditions.
- Partial melt calculation and associated change in viscosity / heat production.
- Simple definition of passive tracers and grid of tracers.
- Simple Phase changes
- 2 way coupling with the surface processes model pyBadlands [@Salles2018].

UWGeo comes with a series of examples, benchmarks and tutorials setups that can
be used as cookbook recipes. They also provide a wide range of teaching materials
useful to introduce numerical geodynamic modeling to students.

New functionalities are constantly added to the code and contributions are more than
welcomed. You can access the full documentation online at https://uwgeodynamics.readthedocs.io

# Audience

The module is directed towards a large audience, including earth-science
students, structural geologists, expert numerical geodynamicists and
industry research and development teams. It is used as a research and teaching
tool at the University of Melbourne and the University of Sydney.

# Acknowledgments

Development of Underworld / UWGeodynamics is financially supported by AuScope as
part of the Simulation Analysis Modelling platform (SAM).
UWGeodynamics has been developed to assist researchers and students of the
Basin Genesis Hub and their industry partners.


# References
