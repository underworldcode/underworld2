underworld.function.tensor module
=================================

This module provides functions relating to tensor operations.

All Underworld2 functions return 1d array type objects. For tensor objects, 
the following convention is used:

Full tensors:

2D:
    .. math::
        \left[ 	a_{00},	a_{01}, \\
                    a_{10},	a_{11}  \right]
3D:
    .. math::
        \left[ 	a_{00},	a_{01},	a_{02}, \\
                    a_{10},	a_{11},	a_{12}, \\
                    a_{20}, a_{21}, a_{22}  \right]
                
Symmetric tensors:

2D:
    .. math::
        \left[ 	a_{00},	a_{11}, a_{01}  \right]
3D:
    .. math::
        \left[ 	a_{00},	a_{11},	a_{22}, a_{01},	a_{02},	a_{12}  \right]



Module Summary
--------------
classes:
~~~~~~~~

.. autosummary::
    :nosignatures:

    underworld.function.tensor.symmetric
    underworld.function.tensor.deviatoric
    underworld.function.tensor.antisymmetric
    underworld.function.tensor.second_invariant

Module Details
--------------
classes:
~~~~~~~~
.. autoclass:: underworld.function.tensor.symmetric
    :members:
    :show-inheritance:
.. autoclass:: underworld.function.tensor.deviatoric
    :members:
    :show-inheritance:
.. autoclass:: underworld.function.tensor.antisymmetric
    :members:
    :show-inheritance:
.. autoclass:: underworld.function.tensor.second_invariant
    :members:
    :show-inheritance:

