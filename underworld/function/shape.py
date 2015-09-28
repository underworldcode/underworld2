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
from _function import input
import sys
import numpy as np

class Polygon(_Function):
    """  
    Polygon Shape function
    """
    def __init__(self, vertices, fn=None, *args, **kwargs):

        if fn:
            self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)
        else:
            self._fn = input()
        
        if not isinstance(vertices, np.ndarray):
            raise TypeError( "Provided 'vertices' must be a numpy array." )
        if len(vertices.shape) != 2:
            raise TypeError( "Provided 'vertices' array must be 2 dimensional." )
        if vertices.shape[0] < 3:
            raise TypeError( "Provided 'vertices' array must contain at least 3 vertices." )
        if vertices.shape[1] != 2:
            raise TypeError( "Provided 'vertices' array must contain 2d vectors." )
        
        # ok, need to create a 3d array from the 2d array.. create array of required size
        threedeearray = np.zeros( (vertices.shape[0],3) )
        # now copy
        threedeearray[:,0:2] = vertices[:,0:2]

        # create instance
        import random
        import string
        self._id = "".join(random.choice(string.ascii_uppercase + string.digits) for _ in range(8))
        self._fncself = _cfn.Polygon( self._fn._fncself, threedeearray, self._id )

        # build parent
        super(Polygon,self).__init__(argument_fns=[fn,], *args, **kwargs)
