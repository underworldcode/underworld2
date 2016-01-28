##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module provides functions which raise an exception when given conditions
are encountered during function evaluations. Exception functions never modify
query data.
"""

import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function

class SafeMaths(_Function):
    """  
    This function checks if any of the following have been encountered during
    the evaluation of its subject function:
    
    * Divide by zero
    * Invalid domain was used for evaluation
    * Value overflow errors
    * Value underflow errors
    
    If any of the above are encountered, and exception is thrown immediately. 
    
    Parameters
    ----------
    fn: underworld.Function
        The function that is subject to the testing.
        
    Example
    -------
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> one = fn.misc.constant(1.)
    >>> zero  = fn.misc.constant(0.)
    >>> safeDivideByZero = fn.exception.SafeMaths(one/zero)
    >>> safeDivideByZero.evaluate(0.)  # constant function, so eval anywhere
    Traceback (most recent call last):
    ...
    RuntimeError: Divide by zero encountered while evaluating function.
    
    
    """
    
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.SafeMaths( self._fn._fncself )

        # build parent
        super(SafeMaths,self).__init__(argument_fns=[_fn,],**kwargs)
        

