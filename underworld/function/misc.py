##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
Miscellaneous functions.
"""
import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function

class constant(_Function):
    """  
    This function returns a constant value.
    
    Parameters
    ----------
    value: int,float,bool. scalar or vector.
        The value the function should return.
        
    >>> import underworld as uw
    >>> import underworld.function as fn
    
    >>> fn_const = fn.misc.constant( 3 )
    >>> fn_const.evaluate(0.) # eval anywhere for constant
    array([[3]], dtype=int32)

    >>> fn_const = fn.misc.constant( (3,2,1) )
    >>> fn_const.evaluate(0.) # eval anywhere for constant
    array([[3, 2, 1]], dtype=int32)

    >>> fn_const = fn.misc.constant( 3. )
    >>> fn_const.evaluate(0.) # eval anywhere for constant
    array([[ 3.]])

    >>> fn_const = fn.misc.constant( (3.,2.,1.) )
    >>> fn_const.evaluate(0.) # eval anywhere for constant
    array([[ 3.,  2.,  1.]])
    
    >>> fn_const = fn.misc.constant( True )
    >>> fn_const.evaluate(0.) # eval anywhere for constant
    array([[ True]], dtype=bool)

    >>> fn_const = fn.misc.constant( (True,False,True) )
    >>> fn_const.evaluate(0.) # eval anywhere for constant
    array([[ True, False,  True]], dtype=bool)
    
    """
    def __init__(self, value, *args, **kwargs):
        
        # lets try and convert
        self._ioguy = self._GetIOForPyInput(value)
        self._value = value
        self._fncself = _cfn.Constant(self._ioguy)
        # build parent
        super(constant,self).__init__(argument_fns=None,**kwargs)

    @property
    def value(self):
        """    value: constant value this function returns
        """
        return self._value
    @value.setter
    def value(self,value):
        newioguy = self._GetIOForPyInput(value)
        if not isinstance(newioguy,type(self._ioguy)):
            raise TypeError("'value' object passed in must be of identical type to that used in construction of this function.")
        if newioguy.size() != self._ioguy.size():
            raise TypeError("'value' object passed in must be of identical size to that used in construction of this function.")
        self._ioguy = newioguy
        self._fncself.set_value(self._ioguy)

    def _GetIOForPyInput(self, value):
        if isinstance(value, (int,float,bool) ):
            if isinstance(value,bool):
                ioguy = _cfn.IO_bool(1,0)
            elif isinstance(value, int):
                ioguy = _cfn.IO_int(1,0)
            elif isinstance(value,float):
                ioguy = _cfn.IO_double(1,0)
            else:
                raise RuntimeError("Failure during object creation. Please contact developers.")
            # now set val
            ioguy.value(value,0)
        else:
            try:
                iterator = iter(value)
            except TypeError:
                raise ValueError("'value' object provided to Constant Function constructor does not appear to be valid.")
            else:
                # iterable
                tupleGuy = tuple(iterator)
                lenTupleGuy = len(tupleGuy)
                firstFella = tupleGuy[0]
                if isinstance(firstFella,bool):
                    ioguy = _cfn.IO_bool(lenTupleGuy,3)
                elif isinstance(firstFella, int):
                    ioguy = _cfn.IO_int(lenTupleGuy,3)
                elif isinstance(firstFella,float):
                    ioguy = _cfn.IO_double(lenTupleGuy,3)
                else:
                    raise ValueError("'value' object provided to Constant Function constructor does not appear to be valid.")
                # right, now load in ze data
                ii = 0
                for val in tupleGuy:
                    if not isinstance(val,type(firstFella)):
                        raise ValueError("Your array (or similar) must be homogeneous of type.")
                    ioguy.value(val,ii)
                    ii+=1;
        return ioguy

class max(_Function):
    """ 
    max function.  Returns the maximum of its two arguments.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = _Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = _Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        
        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.MathBinary(self._fn1._fncself, self._fn2._fncself,  _cfn.MathBinary.fmax )
        # build parent
        super(max,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)

class min(_Function):
    """ 
    min function.  Returns the minimum of its two arguments.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = _Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = _Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        
        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.MathBinary(self._fn1._fncself, self._fn2._fncself,  _cfn.MathBinary.fmin )
        # build parent
        super(min,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)

