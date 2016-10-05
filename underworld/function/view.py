##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module includes functions which provide views into the results of
function queries.  These functions never modify query data. 
"""

import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function

class min_max(_Function):
    """ 
    This function records the min & max result from a queried function.
    
    Note that this function simply records maximum and minimum values
    encountered as it evaluated. Until it is evaluated at least once,
    the values returned via one of its methods ('min_local',
    'min_global', etc ) will be the initialisation values and bear no 
    relation to the min or max of the subject function.
    
    For vector input types, this function will report on the magnitude
    of the vector.
    
    Parameters
    ----------
    fn: underworld.function.Function
        The subject function. Function must return a float type.
        
    Example
    -------
    Create a simple function which returns two times its input:
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> import numpy as np
    >>> fn_simple = fn.input()[0]*2.
    
    Let's wrap it with a min_max function
    >>> fn_minmax_simple = fn.view.min_max(fn_simple)
    
    Now do an evaluation:
    >>> fn_minmax_simple.evaluate(5.)
    array([[ 10.]])
    
    Since there's only been one evaluation, min and max 
    values should be identical
    >>> fn_minmax_simple.min_global()
    10.0
    >>> fn_minmax_simple.max_global()
    10.0
    
    Do another evaluation
    >>> fn_minmax_simple.evaluate(-3.)
    array([[-6.]])
    
    Now check min and max again
    >>> fn_minmax_simple.min_global()
    -6.0
    >>> fn_minmax_simple.max_global()
    10.0
    
    Note that if we only evaluate the subject function,
    no min/max values are recorded:
    >>> fn_simple.evaluate(3000.)
    array([[ 6000.]])
    >>> fn_minmax_simple.max_global()
    10.0
    
    Also note that for vector valued subject function, only
    a magnitude is considered:
    >>> import math
    >>> fn_vec = fn.view.min_max(fn.input())
    >>> fn_vec.evaluate( ( 1., 1.) )
    array([[ 1.,  1.]])
    >>> np.allclose(fn_vec.min_global(), math.sqrt(2.))
    True
    >>> np.allclose(fn_vec.max_global(), math.sqrt(2.))
    True
    >>> fn_vec.evaluate( ( -1., 0.) )
    array([[-1.,  0.]])
    >>> np.allclose(fn_vec.min_global(), 1. )
    True
    >>> np.allclose(fn_vec.max_global(), math.sqrt(2.))
    True
    

    """
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function.convert(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.MinMax( self._fn._fncself )

        # build parent
        super(min_max,self).__init__(argument_fns=[fn,],**kwargs)

    def min_local(self):
        """
        Returns the minimum value encountered locally on the current process.
        
        Returns
        -------
        double: minimum value
        """
        return self._fncself.getMin()
    def max_local(self):
        """
        Returns the max value encountered locally on the current process.
        
        Returns
        -------
        double: maximum value
        """
        return self._fncself.getMax()
    def min_global(self):
        """
        Returns the minimum value encountered across all processes.
        
        Notes
        -----
        This method must be called by collectively all processes.
        
        Returns
        -------
        double: minimum value
        """
        return self._fncself.getMinGlobal()
    def max_global(self):
        """
        Returns the maximum value encountered across all processes.
        
        Notes
        -----
        This method must be called by collectively all processes.
        
        Returns
        -------
        double: maximum value
        """
        return self._fncself.getMaxGlobal()
    def reset(self):
        """ Resets the minimum and maximum values.
        """
        return self._fncself.reset()
