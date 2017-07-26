##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module includes shape type functions. Shape functions generally
define some geometric object, and return boolean values to indicate
whether the queried locations are inside or outside the shape. 

"""

import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function
from _function import input as _input
import numpy as _np

class Polygon(_Function):
    """  
    This function creates a polygon shape. Note that this is 
    strictly a 2d shape, and the third dimension of any query 
    will be ignored. You may create a box type function if you 
    wish to limit the shape extent in the third dimension. 
    
    You will need to use rotations to orient the polygon in
    other directions. Rotations functions will be available 
    shortly (hopefully!).
    
    Parameters
    ----------
    vertices: np.ndarray
        This array provides the vertices for the polygon. 
        Note that the order of the vertices is important. The polygon
        is defined by a piecewise linear edge joining the vertices 
        in the order provided by the array. The final vertex and the
        initial vertex are joined to complete the polygon.
    fn: underworld.function.Function, default=None
        This is the input function. Generally it will not be 
        required, but you may need to use (for example) to 
        transform the incoming coordinates.
        
    Example
    -------
    In this example we will create a triangle shape and test some 
    points.
    
    >>> import underworld as uw
    >>> import numpy as np
    
    Create the array to define the triangle, and the function
    
    >>> vertex_array = np.array( [(0.0,0.0),(0.5,1.0),(1.0,0.0)] )
    >>> polyfn = uw.function.shape.Polygon(vertex_array)
    
    Create some test points, and do a test evaluation
    
    >>> test_array = np.array( [(0.0,0.9),(0.5,0.5),(0.9,0.2)] )
    >>> polyfn.evaluate(test_array)
    array([[False],
           [ True],
           [False]], dtype=bool)
    
    """
    def __init__(self, vertices, fn=None, *args, **kwargs):

        if fn:
            self._fn = _Function.convert(fn)
        else:
            self._fn = _input()
        
        if not isinstance(vertices, _np.ndarray):
            raise TypeError( "Provided 'vertices' must be a numpy array." )
        if len(vertices.shape) != 2:
            raise TypeError( "Provided 'vertices' array must be 2 dimensional." )
        if vertices.shape[0] < 3:
            raise TypeError( "Provided 'vertices' array must contain at least 3 vertices." )
        if vertices.shape[1] != 2:
            raise TypeError( "Provided 'vertices' array must contain 2d vectors." )
        
        # ok, need to create a 3d array from the 2d array.. create array of required size
        threedeearray = _np.zeros( (vertices.shape[0],3) )
        # now copy
        threedeearray[:,0:2] = vertices[:,0:2]

        # create instance
        self._fncself = _cfn.Polygon( self._fn._fncself, threedeearray)

        # build parent
        super(Polygon,self).__init__(argument_fns=[fn,], *args, **kwargs)
