##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module functions relating to rheological operations.
"""

import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function
import tensor as _tensor
import branching as _branching

class stress_limiting_viscosity(_Function):
    """
    Returns an viscosity value which effectively limits the fluid maximum stress.
    Where the stress invariant (as calculated using the provided
    stressFn) is greater than the stress limit (as provided by the
    stressLimitFn), the returned viscosity will affect a fluid stress
    at the stress limit. Otherwise, inputViscosityFn is passed through.
    """
    
    def __init__(self, stressFn, stressLimitFn, inputViscosityFn, *args, **kwargs):

        _stressFn = _Function._CheckIsFnOrConvertOrThrow(stressFn)
        if _stressFn == None:
            raise ValueError( "Provided 'stressFn' must a 'Function' or convertible type.")
        self._stressFn = _stressFn
        
        _stressLimitFn = _Function._CheckIsFnOrConvertOrThrow(stressLimitFn)
        if _stressLimitFn == None:
            raise ValueError( "Provided 'stressLimitFn' must a 'Function' or convertible type.")
        self._stressLimitFn = _stressLimitFn
        
        _inputViscosityFn = _Function._CheckIsFnOrConvertOrThrow(inputViscosityFn)
        if _inputViscosityFn == None:
            raise ValueError( "Provided 'inputViscosityFn' must a 'Function' or convertible type.")
        self._inputViscosityFn = _inputViscosityFn

        # grab second inv of stress
        secondInvFn = _tensor.second_invariant(self._stressFn)

        # create conditional
        self._conditional = _branching.conditional( [ ( secondInvFn > _stressLimitFn , inputViscosityFn*_stressLimitFn/secondInvFn ),    # if over limit, reduce viscosity
                                                      ( True                         , inputViscosityFn                            ) ] ) # else return viscosity

        # this function is not based on a c function itself, so instead point the c pointer to the conditionals.
        self._fncself = self._conditional._fncself
        
        # build parent
        super(stress_limiting_viscosity,self).__init__(argument_fns=[_stressFn,_stressLimitFn,_inputViscosityFn],**kwargs)
        
