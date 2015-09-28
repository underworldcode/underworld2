##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module provides functions relating to tensor operations.
"""

import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function

class symmetric(_Function):
    """
    Returns the symmetric part from a full tensor.
    """
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.TensorFunc( self._fn._fncself, _cfn.TensorFunc.get_symmetric )

        # build parent
        super(symmetric,self).__init__(argument_fns=[fn,],**kwargs)
        
class antisymmetric(_Function):
    """
    Returns the antisymmetric part from a full tensor.
    """
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.TensorFunc( self._fn._fncself, _cfn.TensorFunc.get_antisymmetric )

        # build parent
        super(antisymmetric,self).__init__(argument_fns=[fn,],**kwargs)
        
class second_invariant(_Function):
    """
    Returns the second invariant from a tensor.
    """
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.TensorFunc( self._fn._fncself, _cfn.TensorFunc.second_invariant )

        # build parent
        super(second_invariant,self).__init__(argument_fns=[fn,],**kwargs)
        
class deviatoric(_Function):
    """
    Return the deviatoric tensor (symmetric) from a symmetric tensor.
    """
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.TensorFunc( self._fn._fncself, _cfn.TensorFunc.get_deviatoric )

        # build parent
        super(deviatoric,self).__init__(argument_fns=[fn,],**kwargs)
        
