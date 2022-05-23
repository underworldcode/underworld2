Welcome to UWGeodynamics's documentation!
=========================================

.. image:: ./images/UWGeo/Tutorial1.gif

.. image:: ./images/UWGeo/collision_wedge.gif

.. toctree::
   :hidden:
   :maxdepth: 2

   User Guide <UWGeoUserGuide>

   Examples <UWGeoExamples>
   Benchmarks <UWGeoBenchmarks>
   Tutorials <UWGeoTutorials>

The UWGeodynamics module intents to facilitate rapid prototyping of geodynamics
models using Underworld. It can be seen as a set of high-level functions within
the Underworld ecosystem. It is a means to quickly get the user into Underworld
modelling and assumes very little knowledge in coding. The module make some
assumptions based on how the user defines the boundary conditions and the
properties of the materials (rocks, phases). Its simplicity comes with a
relatively more rigid workflow (compared to the classic Underworld functions).
However, the user can easily break the high level objects and get back to core
Underworld function at any step of model design.

The UWGeodynamics is inspired by the Lithospheric Modelling Recipe (LMR)
originally developed by Luke Mondy, Guillaume Duclaux and Patrice Rey
for Underworld 1. Some of the naming conventions have been reused to facilitate
the transition from LMR. The Rheological libraries are also taken from LMR.

As we think the low-level interface is more flexible, and in so allows for more
complex models, we strongly encourage users to explore and break
the High Level functions.

We hope that the user will naturally move to the low-level functionalities as
he or her gets more confident, and by doing so will access the wide range
of possibilities offered by Underworld.

UWGeodynamics and Underworld
----------------------------

*UWGeodynamics* uses the *Underworld_*  Application Programming Interface (API).
Both projects are supported by The Underworld development team led by Louis Moresi and based in Melbourne, Australia
at the University of Melbourne and at Monash University.

*Underworld* and *UWGeodynamics* both provide powerful tools to develop numerical geodynamic models.
But their approaches are different: *UWGeodynamics* largely guides users into a way of doing things.
The Underworld API provides a series of tools and components (Mesh, Mesh variables, system of equations, functions)
and leaves the responsibility to arrange those components to the user. The main advantage of the Underworld API is its flexibility.
The main inconvenient resides in a somewhat steeper learning curve. *UWGeodynamics* components are
designed to be more natural to non-experimented numerical modellers or people with little knowledge in programming.
It is a way to quickly get started and design numerical models. Developing complex models can also be facilitated
by the *UWGeodynamics* high-level interface as it requires less time and less involvement
with the details of the Underworld API.

The two approaches are complementary and mixing the two approaches is possible and highly encouraged.

Versioning
------------------

Since version 1.0 The Underworld development team has decided to match the *UWGeodynamics* version number with
the latest supported version of Underworld. 
UWGeodynamics v2.7 is then supporing Underworld up to version 2.7.

The third number is used for *UWGeodynamics* only (v2.7.1, v2.7.2 etc.)

The development branch is based on the current *Underworld* development branch.

Quick Start / Testing
----------------------

We provide a docker container via binder_.
This is a quick solution to get you started and run the examples and tutorials
without installing anything on your machine. That is a good way to see if the
software can actually be useful to you. 
The ressource are however limited and you should not try to run model with high resolution.
3D models can not be run in the binder.

.. image:: ./images/UWGeo/SandboxCompression.gif

