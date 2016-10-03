##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module contains functions relating to rheological operations.
"""

import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function
import tensor as _tensor
import branching as _branching

class stress_limiting_viscosity(_Function):
    """
    Returns a viscosity value which effectively limits the maximum fluid
    stress. Where the stress invariant (as calculated using the provided
    fn_stress) is greater than the stress limit (as provided by the
    fn_stresslimit), the returned viscosity will affect a fluid stress
    at the stress limit. Otherwise, fn_inputviscosity is passed through.
    
    Parameters
    ----------
    fn_stress: underworld.function.Function (or convertible).
        Function which returns the current stress in the fluid.
        Function should return a symmetric tensor of floating point values.
    fn_stresslimit: underworld.function.Function (or convertible).  
        Function which defines the stress limit.
        Function should return a scalar floating point value.
    fn_inputviscosity: underworld.function.Function (or convertible).
        Function which defines the non-yielded viscosity value.
        Function should return a scalar floating point value.

    Example
    -------
    Lets setup a simple shear type configuration but with a viscosity
    that increase vertically
    
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> mesh = uw.mesh.FeMesh_Cartesian(elementRes=(16,16), periodic=(True,False))
    >>> velVar = uw.mesh.MeshVariable(mesh,2)
    >>> pressVar = uw.mesh.MeshVariable(mesh.subMesh,1)

    Simple shear boundary conditions:
    >>> bot_nodes = mesh.specialSets["MinJ_VertexSet"]
    >>> top_nodes = mesh.specialSets["MaxJ_VertexSet"]
    >>> bc = uw.conditions.DirichletCondition(velVar, (top_nodes+bot_nodes,top_nodes+bot_nodes))
    >>> velVar.data[bot_nodes.data] = (-0.5,0.)
    >>> velVar.data[top_nodes.data] = ( 0.5,0.)

    Vertically increasing exponential viscosity:
    >>> fn_visc = 1.
    >>> stokesSys = uw.systems.Stokes(velVar,pressVar,fn_visc,conditions=[bc,])

    Solve:
    >>> solver = uw.systems.Solver(stokesSys)
    >>> solver.solve()

    Use the min_max function to determine a maximum stress
    >>> fn_stress =  2.*fn_visc*uw.function.tensor.symmetric( velVar.fn_gradient )
    >>> fn_minmax_inv = fn.view.min_max(fn.tensor.second_invariant(fn_stress))
    >>> ignore = fn_minmax_inv.evaluate(mesh)
    >>> import numpy as np
    >>> np.allclose(fn_minmax_inv.max_global(), 1.0, rtol=1e-05)
    True

    Now lets set the limited viscosity. Note that the system is now non-linear.
    >>> fn_visc_limited = fn.rheology.stress_limiting_viscosity(fn_stress,0.5,fn_visc)
    >>> stokesSys.fn_viscosity = fn_visc_limited
    >>> solver.solve(nonLinearIterate=True)

    Now check the stress.
    >>> fn_stress = 2.*fn_visc_limited*uw.function.tensor.symmetric( velVar.fn_gradient )
    >>> fn_minmax_inv = fn.view.min_max(fn.tensor.second_invariant(fn_stress))
    >>> ignore = fn_minmax_inv.evaluate(mesh)
    >>> np.allclose(fn_minmax_inv.max_global(), 0.5, rtol=1e-05)
    True


    """
    
    def __init__(self, fn_stress, fn_stresslimit, fn_inputviscosity, *args, **kwargs):

        _fn_stress = _Function.convert(fn_stress)
        if _fn_stress == None:
            raise ValueError( "Provided 'fn_stress' must a 'Function' or convertible type.")
        self._fn_stress = _fn_stress
        
        _fn_stresslimit = _Function.convert(fn_stresslimit)
        if _fn_stresslimit == None:
            raise ValueError( "Provided 'fn_stresslimit' must a 'Function' or convertible type.")
        self._fn_stresslimit = _fn_stresslimit
        
        _fn_inputviscosity = _Function.convert(fn_inputviscosity)
        if _fn_inputviscosity == None:
            raise ValueError( "Provided 'fn_inputviscosity' must a 'Function' or convertible type.")
        self._fn_inputviscosity = _fn_inputviscosity

        # grab second inv of stress
        secondInvFn = _tensor.second_invariant(self._fn_stress)

        # create conditional
        self._conditional = _branching.conditional( [ ( secondInvFn > _fn_stresslimit , fn_inputviscosity*_fn_stresslimit/secondInvFn ),    # if over limit, reduce viscosity
                                                      ( True                          , fn_inputviscosity                             ) ] ) # else return viscosity

        # this function is not based on a c function itself, so instead point the c pointer to the conditionals.
        self._fncself = self._conditional._fncself
        
        # build parent
        super(stress_limiting_viscosity,self).__init__(argument_fns=[_fn_stress,_fn_stresslimit,_fn_inputviscosity],**kwargs)
        
