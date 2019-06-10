.. underworld documentation master file, created by
   sphinx-quickstart on Mon Nov 21 16:44:00 2016.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Underworld
==========

.. image:: ./images/Montage.png

Underworld2 is a Python API (Application Programming Interface) which provides functionality for the modelling of geodynamics processes, and is designed to work (almost) seamlessly across PC, cloud and HPC infrastructure. Primarily the API consists of a set of Python classes from which numerical geodynamics models may be constructed. The API also provides the tools required for inline analysis and data management. For scalability across multiprocessor platforms, MPI (Message Passing Interface) is leveraged, and for performant operation all heavy computations are executed within a statically typed layer.

Underworld2 provides capacity for modelling 2- and 3-dimensional geodynamics processes, utilising a particle-in-cell finite element approach for solution to Stokes flow type configurations. In Underworld, the finite element mesh can be static or dynamic, but it is not constrained to move in lock-step with the evolving geometry of the fluid. This hybrid approach allows Underworld to obtain accurate velocity solutions (on the mesh) for a given material configuration, while simultaneously ensuring the accurate time advection of material interfaces and history information (using particle swarms).

A primary aim of Underworld2 is to enable rapid prototyping of models, and to this end embedded visualisation (LavaVu) and modern development environments such as Jupyter Notebooks have been embraced, with the latter also providing a path to cloud computing amenability. The Jupyter Notebook front end provisioned across cloud facilities has also proven to be an appropriate environment for the use of Underworld as a teaching tool for solid Earth geoscience.

Underworld2 provides a minimal set of highly flexible core functionality, with user domain concerns left to the users themselves to construct. At the centre of this design is the Function class, which aims to provide a natural interface from which users can describe their problem mathematics. For example, the user may describe a viscosity which is piecewise constant, temperature dependent, or visco-elasto-plastic in behaviour. Simulation chronology is also fully exposed and the user is expected to explicitly specify when events should occur. This approach allows users to specify exactly their modelling requirements and provides transparency.

Underworld is an open-source project.


.. toctree::
    :hidden:
    :maxdepth: 1

    Geodynamics <Geodynamics>
    Numerical Methods <Numerics>
    Installation <Installation>
    User Guide <UserGuide>

    Underworld API Reference <build/underworld>

    UW2 Github  <https://github.com/underworldcode/underworld2>
    UW2 Website <http://www.underworldcode.org>
    UW2 Zenodo <https://doi.org/10.5281/zenodo.1436039>


