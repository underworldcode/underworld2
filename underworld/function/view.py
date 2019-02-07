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
from . import _function

class min_max(_function.Function):
    """ 
    This function records the min & max result from a queried function.
    
    Note that this function simply records the min/max values encountered 
    when it is evaluated. Therefore, if it has not been evaluated at all,
    the values returned via one of its methods ('min_local',
    'min_global', etc ) will simply be initialisation values.
    
    For vector input types, this function will report on the magnitude
    of the vector.
    
    Parameters
    ----------
    fn: underworld.function.Function
        The primary function. If `fn_norm` is not provided, this is 
        used to calculate the min_max. Results from this function are
        always passed back.
    fn_norm: underworld.function.Function
        This function returns a norm like quantity by which the
        min and max are determined. For example, where the primary
        function is a vector quantity, this function might calculate
        the magnitude of that vector. This function must return a 
        scalar result, and must be provided where the primary function
        is non-scalar. See the example below for usage. 
    fn_auxiliary: underworld.function.Function
        An auxiliary function which is evaluated at the location
        of the min/max. For example, often the coordinate where the
        min/max values occur are required, and so the user may
        pass in fn.input() as the auxiliary function to achieve this
        
    Example
    -------
    Create a simple function which returns two times its input:
    
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> import numpy as np
    >>> fn_simple = fn.input()[0]*2.
    
    Let's wrap it with a min_max function:
    
    >>> fn_minmax_simple = fn.view.min_max(fn_simple)
    
    Now do an evaluation:
    
    >>> fn_minmax_simple.evaluate(5.)
    array([[ 10.]])
    
    Since there's only been one evaluation, min and max 
    values should be identical:
    
    >>> fn_minmax_simple.min_global()
    10.0
    >>> fn_minmax_simple.max_global()
    10.0
    
    Do another evaluation:
    
    >>> fn_minmax_simple.evaluate(-3.)
    array([[-6.]])
    
    Now check min and max again:
    
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
    
    Also note that for vector valued subject function, `fn_norm`
    must be provided:
    
    >>> fn_vec = fn.input()*(1.,1.)
    >>> fn_vec_mm = fn.view.min_max(fn_vec)
    >>> fn_vec_mm.evaluate( 2. )
    Traceback (most recent call last):
    ...
    RuntimeError: Issue utilising function of class 'min_max' constructed at:
       --- CONSTRUCTION TIME STACK ---
    Error message:
    Argument function does not return scalar results. You must also provide a function which calculates the required norm like quantity via the `fn_norm` parameter.

    >>> fn_vec_mm = fn.view.min_max(fn_vec, fn_norm=fn.math.dot(fn_vec,fn_vec))
    >>> fn_vec_mm.evaluate( 2. )
    array([[ 2.,  2.]])
    >>> fn_vec_mm.max_global()
    8.0
    >>> fn_vec_mm.evaluate( -1. )
    array([[-1., -1.]])
    >>> fn_vec_mm.min_global()
    2.0
    >>> fn_vec_mm.max_global()
    8.0
    
    To obtain the min/max values across a MeshVariable object,
    you will need to evaluate the function across all nodes of
    the MeshVariable:
    
    >>> mesh = uw.mesh.FeMesh_Cartesian()
    >>> meshvariable = uw.mesh.MeshVariable( mesh, 1 )
    >>> meshvariable.data[:] = np.random.randint(100,size=meshvariable.data.shape)  # init with random data
    >>> fn_mv = fn.view.min_max(meshvariable)                       # create min_max view wrapper
    >>> ignore = fn_mv.evaluate(mesh)                               # this call will evaluate at all nodes
    >>> np.allclose(fn_mv.min_local(),meshvariable.data.min())
    True
    >>> np.allclose(fn_mv.max_local(),meshvariable.data.max())
    True
    
    Note that when operating in parallel, the `min_global()` and
    `max_global()` methods are a good option for extracting
    discrete object global min/max values, as the numpy views will
    only report the local min/max values.

    Also note that since min_max views only record results as they 
    are evaluated, if the underlying subject function min/max values
    change, this will not be recorded by the min_max view until its
    evaluate encounters the new min/max values:
    
    >>> meshvariable.data[3] = 1000                               # change some random value
    >>> np.allclose(fn_mv.max_local(),meshvariable.data.max())    # check again, it should be false
    False
    >>> ignore = fn_mv.evaluate(mesh)                             # evaluate across all nodes again
    >>> np.allclose(fn_mv.max_local(),meshvariable.data.max())    # check again
    True
    
    Similarly, the view's min/max values are only updated when
    smaller/larger min/max values are encountered. So, if the underlying 
    subject function's maximum (for example) is reduced, the 
    view will not record this if its currently stored value exceeds 
    the new maximum. A call to `reset()` is required:
    
    >>> fn_mv.max_local()
    1000.0
    >>> meshvariable.data[3] = 500            # reduce max
    >>> ignore = fn_mv.evaluate(mesh)         # evaluate across all nodes again
    >>> fn_mv.max_local()                     # note that it still records old value
    1000.0
    >>> fn_mv.reset()                         # now re-init view's min/max
    >>> ignore = fn_mv.evaluate(mesh)         # evaluate across all nodes again
    >>> fn_mv.max_local()                     # it should now record new value
    500.0

    The auxiliary function allows you to obtain secondary information at the function 
    minimum. One common use case would be to obtain a location where the min/max was obtained:
    
    >>> fn_mv = fn.view.min_max(meshvariable, fn_auxiliary=fn.input())
    >>> meshvariable.data[1] = 1000.0   # set second node to have the highest value
    >>> ignore = fn_mv.evaluate(mesh)
    >>> fn_mv.max_global()
    1000.0
    >>> np.allclose( mesh.data[1], fn_mv.max_global_auxiliary() )  # ensure max is obtain at required mesh node.
    True
    

    """
    def __init__(self, fn, fn_norm=None, fn_auxiliary=None,  *args, **kwargs):

        _fn = _function.Function.convert(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        fn_norm_cself = None
        if fn_norm:
            _fn_norm = _function.Function.convert(fn_norm)
            if _fn_norm == None:
                raise ValueError( "provided 'fn_norm' must a 'Function' or convertible.")
            self._fn_norm = _fn_norm
            fn_norm_cself = _fn_norm._fncself

        fn_auxiliary_cself = None
        if fn_auxiliary:
            _fn_auxiliary = _function.Function.convert(fn_auxiliary)
            if _fn_auxiliary == None:
                raise ValueError( "provided 'fn_auxiliary' must a 'Function' or convertible.")
            self._fn_auxiliary = _fn_auxiliary
            fn_auxiliary_cself = _fn_auxiliary._fncself

        # create c instance
        self._fncself = _cfn.MinMax( self._fn._fncself, fn_norm_cself, fn_auxiliary_cself )

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
    def _functionio_for_numpy(self,function_io_guy):
        """
        This method simply takes a swig proxy to a FunctionIO, 
        and returns the data as a numpy array.
        """
        # create input function just so we can process using query
        inputfn = _function.input()
        # create function_io iterator
        func_io_it = _cfn.FunctionIOIter(function_io_guy)
        # process
        return _cfn.Query(inputfn._fncself).query(func_io_it)

    def min_local_auxiliary(self):
        """
        Returns the results of the auxiliary function evaluated at the 
        location corresponding to the primary function minimum. This
        method only considers results on the current process.
        
        Returns
        -------
        FunctionIO: value at local minimum.
        """
        return self._functionio_for_numpy(self._fncself.getMinAux())
    def max_local_auxiliary(self):
        """
        Returns the results of the auxiliary function evaluated at the
        location corresponding to the primary function maximum. This
        method only considers results on the current process.
        
        Returns
        -------
        FunctionIO: value at local maximum.
        """
        return self._functionio_for_numpy(self._fncself.getMaxAux())
    def min_global_auxiliary(self):
        """
        Returns the results of the auxiliary function evaluated at the
        location corresponding to the primary function minimum. This
        method considers results across all processes (ie, globally).
        
        Notes
        -----
        This method must be called by collectively all processes.
        
        Returns
        -------
        FunctionIO: value at global minimum.
        """
        
        # first make sure that we have determined the rank with the min
        self.min_global()
        
        import underworld as uw
        # if we are the rank with the min result, extract result
        if uw.rank() == self.min_rank():
            auxout = self.min_local_auxiliary()
        else:
            auxout = None
    
        from mpi4py import MPI
        comm = MPI.COMM_WORLD

        # broadcast
        data = comm.bcast(auxout, root=self.min_rank())

        return data
    def max_global_auxiliary(self):
        """
        Returns the results of the auxiliary function evaluated at the
        location corresponding to the primary function maximum. This
        method considers results across all processes (ie, globally).
        
        Notes
        -----
        This method must be called by collectively all processes.
        
        Returns
        -------
        FunctionIO: value at global maximum.
        """
        # first make sure that we have determined the rank with the max
        self.max_global()
        
        import underworld as uw
        # if we are the rank with the max result, extract result
        if uw.rank() == self.max_rank():
            auxout = self.max_local_auxiliary()
        else:
            auxout = None
        
        from mpi4py import MPI
        comm = MPI.COMM_WORLD
        
        # broadcast
        data = comm.bcast(auxout, root=self.max_rank())
        
        return data
    def min_rank(self):
        """
        Returns the rank where the minimum occurs. Note that this method 
        will return -1 until min_global has been called.
        
        Returns
        -------
        int: rank
        """
        rank = self._fncself.getMinRank()
        if rank < 0:
            raise RuntimeError("You must run one of the `min_global` MinMax routines first to determine \
                                which rank contains the minimum value.")
        return rank
    def max_rank(self):
        """
        Returns the rank where the maximum occurs. Note that this method
        will return -1 until max_global has been called.
        
        Returns
        -------
        int: rank
        """
        rank = self._fncself.getMaxRank()
        if rank < 0:
            raise RuntimeError("You must run one of the `max_global` MinMax routines first to determine \
                                which rank contains the maximum value.")
        return rank
    def reset(self):
        """ Resets the minimum and maximum values.
        """
        return self._fncself.reset()
