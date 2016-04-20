##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
Base module for the Function class. The Function class provides generic
function construction capabilities.

"""

import underworld
import libUnderworld.libUnderworldPy.Function as _cfn
from abc import ABCMeta,abstractmethod
import numpy as np
import weakref

ScalarType = _cfn.FunctionIO.Scalar
VectorType = _cfn.FunctionIO.Vector
SymmetricTensorType = _cfn.FunctionIO.SymmetricTensor
TensorType = _cfn.FunctionIO.Tensor
ArrayType  = _cfn.FunctionIO.Array

class FunctionInput(underworld._stgermain.LeftOverParamsChecker):
    """
    Objects that inherit from this class are able to act as inputs
    to function evaluation from python.
    """
    __metaclass__ = ABCMeta
    @abstractmethod
    def _get_iterator(self):
        """ All children should define this method which returns the
        c iterator object """
        pass

class Function(underworld._stgermain.LeftOverParamsChecker):
    """
    Objects which inherit from this class provide user definable functions
    within Underworld.

    Functions aim to achieve a number of goals:
    * Provide a natural interface for mathematical behaviour description within python.
    * Provide a high level interface to Underworld discrete objects.
    * Allow discrete objects to be used in combination to continuous objects.
    * Handle the evaluation of discrete objects in the most efficient manner.
    * Perform all heavy calculations at the C-level for efficiency.
    * Provide an interface for users to evaluate functions directly within python,
      utilising numpy arrays for input/output.

    """
    __metaclass__ = ABCMeta
    def __init__(self, argument_fns, **kwargs):

        # all of these guys must define a _fncself, as this will be expected by objects that receive functions.
        if not hasattr(self, '_fncself'):
            raise RuntimeError("Failure during object creation. Object with class \n'{}'\n".format(type(self)) \
                             + "does not appear to have set a value for '_fncself'. Please contact developers.")

        self._underlyingDataItems = weakref.WeakSet()
        if argument_fns:
            for argfn in argument_fns:
                if argfn:
                    # add to current functions set
                    self._underlyingDataItems.update(argfn._underlyingDataItems)

        super(Function,self).__init__(**kwargs)

    @staticmethod
    def _CheckIsFnOrConvertOrThrow(guy):
        """
        This method will simply check if the provided fn is of Function type
        or None type. If it is, it is simply returned. Otherwise, conversion
        to a constant function is attempted.
        """
        if isinstance(guy, (Function, type(None)) ):
            return guy
        else:
            import misc
            return misc.constant(guy)

    def __add__(self,other):
        """
        Operator overloading for '+' operation:

        Fn3 = Fn1 + Fn2

        Creates a new function Fn3 which performs additions of Fn1 and Fn2.

        Returns
        -------
        fn.add: Add function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> three = misc.constant(3.)
        >>> four  = misc.constant(4.)
        >>> np.isclose( (three + four).evaluate(0.), [[ 7.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return add( self, other )

    __radd__=__add__

    def __sub__(self,other):
        """
        Operator overloading for '-' operation:

        Fn3 = Fn1 - Fn2

        Creates a new function Fn3 which performs subtraction of Fn2 from Fn1.

        Returns
        -------
        fn.subtract: Subtract function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> three = misc.constant(3.)
        >>> four  = misc.constant(4.)
        >>> np.isclose( (three - four).evaluate(0.), [[ -1.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return subtract( self, other )

    def __rsub__(self,other):
        """
        Operator overloading for '-' operation.  Right hand version.

        Fn3 = Fn1 - Fn2

        Creates a new function Fn3 which performs subtraction of Fn2 from Fn1.

        Returns
        -------
        fn.subtract: RHS subtract function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> four  = misc.constant(4.)
        >>> np.isclose( (5. - four).evaluate(0.), [[ 1.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return -subtract( self, other )

    def  __neg__(self):
        """
        Operator overloading for unary '-'.

        FnNeg = -Fn

        Creates a new function FnNeg which is the negative of Fn.

        Returns
        -------
        fn.multiply: Negative function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> four = misc.constant(4.)
        >>> np.isclose( (-four).evaluate(0.), [[ -4.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return multiply( self, -1.0)

    def __mul__(self,other):
        """
        Operator overloading for '*' operation:

        Fn3 = Fn1 * Fn2

        Creates a new function Fn3 which returns the product of Fn1 and Fn2.

        Returns
        -------
        fn.multiply: Multiply function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> three = misc.constant(3.)
        >>> four  = misc.constant(4.)
        >>> np.isclose( (three*four).evaluate(0.), [[ 12.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return multiply( self, other )
    __rmul__ = __mul__

    def __div__(self,other):
        """
        Operator overloading for '/' operation:

        Fn3 = Fn1 / Fn2

        Creates a new function Fn3 which returns the quotient of Fn1 and Fn2.

        Returns
        -------
        fn.divide: Divide function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> np.isclose( (four/two).evaluate(0.), [[ 2.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return divide( self, other )

    def __rdiv__(self,other):
        return divide( other, self )

    def __pow__(self,other):
        """
        Operator overloading for '**' operation:

        Fn3 = Fn1 ** Fn2

        Creates a new function Fn3 which returns Fn1 to the power of Fn2.

        Returns
        -------
        fn.math.pow: Power function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> np.isclose( (two**four).evaluate(0.), [[ 16.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        import math
        import _math as fnmath
        return fnmath.pow( self, other )

    def __lt__(self,other):
        """
        Operator overloading for '<' operation:

        Fn3 = Fn1 < Fn2

        Creates a new function Fn3 which returns a bool result for the relation.

        Returns
        -------
        fn.less: Less than function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> (two < four).evaluate(0.) # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return less( self, other )

    def __le__(self,other):
        """
        Operator overloading for '<=' operation:

        Fn3 = Fn1 <= Fn2

        Creates a new function Fn3 which returns a bool result for the relation.

        Returns
        -------
        fn.less_equal: Less than or equal to function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> (two <= two).evaluate(0.) # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return less_equal( self, other )

    def __gt__(self,other):
        """
        Operator overloading for '>' operation:

        Fn3 = Fn1 > Fn2

        Creates a new function Fn3 which returns a bool result for the relation.

        Returns
        -------
        fn.greater: Greater than function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> (two > four).evaluate(0.) # note we can evaluate anywhere because it's a constant
        array([[False]], dtype=bool)

        """
        return greater( self, other )

    def __ge__(self,other):
        """
        Operator overloading for '>=' operation:

        Fn3 = Fn1 >= Fn2

        Creates a new function Fn3 which returns a bool result for the relation.

        Returns
        -------
        fn.greater_equal: Greater than or equal to function

        Examples
        --------
        >>> import misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> (two >= two).evaluate(0.) # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return greater_equal( self, other )

    def __and__(self,other):
        """
        Operator overloading for '&' operation:

        Fn3 = Fn1 & Fn2

        Creates a new function Fn3 which returns a bool result for the relation.

        Returns
        -------
        fn.logical_and: AND function

        Examples
        --------
        >>> import misc
        >>> trueFn  = misc.constant(True)
        >>> falseFn = misc.constant(False)
        >>> (trueFn & falseFn).evaluate(0.) # note we can evaluate anywhere because it's a constant
        array([[False]], dtype=bool)
        
        Notes
        -----
        The '&' operator in python is usually used for bitwise 'and' operations, with the 
        'and' operator used for boolean type operators. It is not possible to overload the
        'and' operator in python, so instead the bitwise equivalent has been utilised.

        """
        return logical_and( self, other )

    def __or__(self,other):
        """
        Operator overloading for '|' operation:

        Fn3 = Fn1 | Fn2

        Creates a new function Fn3 which returns a bool result for the relation.

        Returns
        -------
        fn.logical_or: OR function

        Examples
        --------
        >>> import misc
        >>> trueFn  = misc.constant(True)
        >>> falseFn = misc.constant(False)
        >>> (trueFn | falseFn).evaluate(0.) # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        Notes
        -----
        The '|' operator in python is usually used for bitwise 'or' operations, with the 'or'
        operator used for boolean type operators. It is not possible to overload the
        'or' operator in python, so instead the bitwise equivalent has been utilised.


        """

        return logical_or( self, other )

    def __getitem__(self,index):
        """
        Operator overloading for '[]' operation:

        FnComponent = Fn[0]

        Creates a new function FnComponent which returns the required component of Fn.

        Returns
        -------
        fn.at: component function

        Examples
        --------
        >>> import misc
        >>> fn  = misc.constant((2.,3.,4.))
        >>> np.isclose( fn[1].evaluate(0.), [[ 3.]]  )  # note we can evaluate anywhere because it's a constant
        array([[ True]], dtype=bool)

        """
        return at(self,index)

    def evaluate(self,inputData,inputType=None):
        """
        This method performs evaluate of a function at the given input(s).

        It accepts floats, lists, tuples, numpy arrays, or any object which is of
        class 'FunctionInput'. lists/tuples must contain floats only.

        Results are returned as numpy array.

        Parameters
        ----------
        inputData: float, list, tuple, ndarray, FunctionInput
            The input to the function. The form of this input must be appropriate
            for the function being evaluated, or an exception will be thrown.

        Returns
        -------
        ndarray: array of results

        Examples
        --------
        >>> import math
        >>> import _math as fnmath
        >>> sinfn = fnmath.sin()
        >>> np.isclose( sinfn.evaluate(math.pi/4.), [[ 0.5*math.sqrt(2.)]]  )
        array([[ True]], dtype=bool)
        >>> intup = (0.,math.pi/4.,2.*math.pi)
        >>> np.isclose( sinfn.evaluate(intup), [[ 0., 0.5*math.sqrt(2.), 0.]]  )
        array([[ True,  True,  True]], dtype=bool)

        """
        if inputType != None and inputType not in (ScalarType,VectorType,SymmetricTensorType,TensorType,ArrayType):
            raise ValueError("Provided input type does not appear to be valid.")
        if isinstance(inputData, FunctionInput):
            if inputType != None:
                raise ValueError("'inputType' specification not supported for this input class.")
            return _cfn.Query(self._fncself).query(inputData._get_iterator())
        elif isinstance(inputData, np.ndarray):
            if inputType != None:
                if inputType == ScalarType:
                    if inputData.shape[1] != 1:
                        raise ValueError("You have specified ScalarType input, but your input size is {}.\n".format(inputData.shape[1]) \
                                        +"ScalarType inputs must be of size 1.")
                if inputType == VectorType:
                    if inputData.shape[1] not in (2,3):
                        raise ValueError("You have specified VectorType input, but your input size is {}.\n".format(inputData.shape[1]) \
                                        +"VectorType inputs must be of size 2 or 3 (for 2d or 3d).")
                if inputType == SymmetricTensorType:
                    if inputData.shape[1] not in (3,6):
                        raise ValueError("You have specified SymmetricTensorType input, but your input size is {}.\n".format(inputData.shape[1]) \
                                        +"SymmetricTensorType inputs must be of size 3 or 6 (for 2d or 3d).")
                if inputType == TensorType:
                    if inputData.shape[1] not in (4,9):
                        raise ValueError("You have specified TensorType input, but your input size is {}.\n".format(inputData.shape[1]) \
                                        +"TensorType inputs must be of size 4 or 9 (for 2d or 3d).")
            else:
                inputType = ArrayType
            return _cfn.Query(self._fncself).query(_cfn.NumpyInput(inputData,inputType))
        elif isinstance(inputData, float):
            tupleInput = (inputData,)
            # recurse
            return self.evaluate( tupleInput, inputType )
        elif isinstance(inputData, (list,tuple)):
            arr = np.empty( [1,len(inputData)] )
            ii = 0
            for guy in inputData:
                if not isinstance(guy, float):
                    raise TypeError("Iterable inputs must only contain python 'float' objects.")
                arr[0,ii] = guy
                ii +=1
            # recurse
            return self.evaluate( arr, inputType )
        else:
            raise TypeError("Input provided for function evaluation does not appear to be supported.")

class add(Function):
    """
    This class implements the addition of two functions.
    It is invoked by the overload methods __add__ and __radd__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Add(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(add,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class subtract(Function):
    """
    This class implements the difference of two functions.
    It is invoked by the overload methods __sub__ and __rsub__.
    """
    def __init__(self, x_Fn, y_Fn, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( x_Fn )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( y_Fn )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.MathBinary(self._fn1._fncself, self._fn2._fncself,  _cfn.MathBinary.subtract )
        # build parent
        super(subtract,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class multiply(Function):
    """
    This class implements the product of two functions
    It is invoked by the overload method __mul__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Multiply(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(multiply,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class divide(Function):
    """
    This class implements the quotient of two functions
    It is invoked by the overload methods __div__ and __rdiv__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Divide(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(divide,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class less(Function):
    """
    This class implements the 'less than' relational operation.
    It is invoked by the overload method __lt__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_less(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(less,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class less_equal(Function):
    """
    This class implements the 'less than or equal to' relational operation.
    It is invoked by the overload method __le__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_less_equal(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(less_equal,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class greater(Function):
    """
    This class implements the 'greater than' relational operation.
    It is invoked by the overload method __gt__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_greater(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(greater,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class greater_equal(Function):
    """
    This class implements the 'greater than or equal to' relational operation.
    It is invoked by the overload method __ge__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_greater_equal(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(greater_equal,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

#class equal_to(Function):
#    """
#    Disabled for now.  Probably not useful for float comparison.
#    This class implements an equality of the two operands.
#
#    Fn3 = equal_to(Fn1,Fn2)
#
#    Creates a new function Fn3 which returns a bool result for the relation.
#
#    Returns
#    -------
#    fn.equal_to: equal_to function
#
#    """
#
#    def __init__(self, fn1, fn2, **kwargs):
#        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
#        if not isinstance( fn1fn, Function ):
#            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
#        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
#        if not isinstance( fn2fn, Function ):
#            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
#
#        self._fn1 = fn1fn
#        self._fn2 = fn2fn
#        # ok finally lets create the fn
#        self._fncself = _cfn.Relational_equal_to(self._fn1._fncself, self._fn2._fncself )
#        # build parent
#        super(equal_to,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

#class not_equal_to(Function):
#    """
#    Disabled for now.  Probably not useful for float comparison.
#    This class implements an inequality test of the two operands.
#
#    Fn3 = not_equal_to(Fn1,Fn2)
#
#    Creates a new function Fn3 which returns a bool result for the relation.
#
#    Returns
#    -------
#    fn.not_equal_to: not_equal_to function
#
#    """
#    def __init__(self, fn1, fn2, **kwargs):
#        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
#        if not isinstance( fn1fn, Function ):
#            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
#        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
#        if not isinstance( fn2fn, Function ):
#            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
#
#        self._fn1 = fn1fn
#        self._fn2 = fn2fn
#        # ok finally lets create the fn
#        self._fncself = _cfn.Relational_not_equal_to(self._fn1._fncself, self._fn2._fncself )
#        # build parent
#        super(not_equal_to,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class logical_and(Function):
    """
    This class implements the AND relational operation.
    It is invoked by the overload method __and__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_logical_and(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(logical_and,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class logical_or(Function):
    """
    This class implements the OR relational operation.
    It is invoked by the overload method __or__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function._CheckIsFnOrConvertOrThrow( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function._CheckIsFnOrConvertOrThrow( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_logical_or(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(logical_or,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class at(Function):
    """
    This class implements extraction the nth component of a function.
    It is invoked by the overload method __getitem__.
    """
    def __init__(self, fn, n, *args, **kwargs):

        _fn = Function._CheckIsFnOrConvertOrThrow(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        if not isinstance( n, int ):
            raise TypeError("'n' argument is expected to be of type 'int'.")
        if n < 0:
            raise TypeError("'n' argument must be a non-negative integer.")

        # create instance
        self._fncself = _cfn.At( self._fn._fncself, n )

        # build parent
        super(at,self).__init__(argument_fns=[fn,], *args,  **kwargs)

class input(Function):
    """
    This class generates a function which simply passes through its input.
    It is often useful when construct functions where the input itself needs
    to be manipulated directly, such as to extract a particular component.
    
    This function class is most often used when you wish to extract a particular
    coodinate component for manipulation. For this reason, we also provide a 
    proxy to this class called 'coord'.

    Returns
    -------
    fn.input: the input function

    Examples
    --------
    Here we see the input function simply passing through its input.
    >>> infunc = input()
    >>> np.isclose( infunc.evaluate( (1.,2.,3.) ), [ 1., 2., 3.] )
    array([[ True,  True,  True]], dtype=bool)

    Often this behaviour is useful when we want to construct a function
    which operates on only a particular coordinate, such as a depth
    dependent density. We may wish to extract the z coordinate (in
    2d):

    >>> zcoord = input()[1]
    >>> baseDensity = 1.
    >>> density = baseDensity - 0.01*zcoord
    >>> testCoord1 = (0.1,0.4)
    >>> testCoord2 = (0.9,0.4)
    >>> np.isclose( density.evaluate( testCoord1 ), density.evaluate( testCoord2 ) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, *args, **kwargs):

        # create instance
        self._fncself = _cfn.Input()

        # build parent
        super(input,self).__init__(argument_fns=None, *args, **kwargs)


coord = input
