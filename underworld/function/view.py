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
    """
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.MinMax( self._fn._fncself )

        # build parent
        super(min_max,self).__init__(argument_fns=[fn,],**kwargs)

    def min(self):
        return self._fncself.getMin()
    def max(self):
        return self._fncself.getMax()
    def min_global(self):
        return self._fncself.getMinGlobal()
    def max_global(self):
        return self._fncself.getMaxGlobal()
    def reset(self):
        return self._fncself.reset()
