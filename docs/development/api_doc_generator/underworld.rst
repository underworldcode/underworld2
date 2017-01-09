underworld module
=================

Underworld2 is a python-friendly version of the Underworld geodynamics
code which provides a programmable and flexible front end to all the
functionality of the code running in a parallel HPC environment. This
gives signficant advantages to the user, with access to the power of
python libraries for setup of complex problems, analysis at runtime,
problem steering, and coupling of multiple problems.

Underworld2 is integrated with the literate programming environment of
the jupyter notebook system for tutorials and as a teaching tool for
solid Earth geoscience.

Underworld is an open-source, particle-in-cell finite element code
tuned for large-scale geodynamics simulations. The numerical algorithms
allow the tracking of history information through the high-strain
deformation associated with fluid flow (for example, transport of the
stress tensor in a viscoelastic, convecting medium, or the advection of
fine-scale damage parameters by the large-scale flow). The finite
element mesh can be static or dynamic, but it is not contrained to move
in lock-step with the evolving geometry of the fluid. This hybrid approach
is very well suited to complex fluids which is how the solid Earth behaves
on a geological timescale.


Module Summary
--------------
submodules:
~~~~~~~~~~~

.. toctree::
    :maxdepth: 1

    underworld.function
    underworld.container
    underworld.utils
    underworld.swarm
    underworld.mesh
    underworld.systems
    underworld.conditions

functions:
~~~~~~~~~~

.. autosummary::
    :nosignatures:

    underworld.matplotlib_inline
    underworld.nProcs
    underworld.rank
    underworld.barrier

Module Details
--------------
functions:
~~~~~~~~~~
.. autofunction:: underworld.matplotlib_inline
.. autofunction:: underworld.nProcs
.. autofunction:: underworld.rank
.. autofunction:: underworld.barrier

