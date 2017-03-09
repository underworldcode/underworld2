underworld.function module
==========================

The function module contains the Function class, and related classes.

Function objects are constructed in python, but evaluated in C for 
efficiency. They provide a high level interface for users to compose model
behaviour (such as viscosity), as well as a natural interface by which
discrete data (such as meshvariables) may be utilised.



Module Summary
--------------
submodules:
~~~~~~~~~~~

.. toctree::
    :maxdepth: 1

    underworld.function.branching
    underworld.function.exception
    underworld.function.tensor
    underworld.function.misc
    underworld.function.analytic
    underworld.function.shape
    underworld.function.rheology
    underworld.function.math
    underworld.function.view

classes:
~~~~~~~~

.. autosummary::
    :nosignatures:

    underworld.function.Function
    underworld.function.FunctionInput
    underworld.function.coord
    underworld.function.input

Module Details
--------------
classes:
~~~~~~~~
.. autoclass:: underworld.function.Function
    :members:
    :show-inheritance:
.. autoclass:: underworld.function.FunctionInput
    :members:
    :show-inheritance:
.. autoclass:: underworld.function.coord
    :members:
    :show-inheritance:
.. autoclass:: underworld.function.input
    :members:
    :show-inheritance:

