##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module provides math functions.  All functions take functions (or
convertibles) as arguments. These functions effectively wrap to the
c++ standard template library equivalent.  For example,
the 'exp' class generates a function with uses std::exp(double).

All functions operate on and return 'double' type data (or
'float' from python).
"""
import underworld as uw
import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function
from _function import input

class cos(_Function):
    """
    Computes the cosine of its argument function
    (measured in radians).

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = cos()
    >>> np.isclose( func.evaluate(0.1234), math.cos(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_cos( fncself )

        # build parent
        super(cos,self).__init__(argument_fns=[fn,],**kwargs)

class sin(_Function):
    """
    Computes the sine of its argument function
    (measured in radians).

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = sin()
    >>> np.isclose( func.evaluate(0.1234), math.sin(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_sin( fncself )

        # build parent
        super(sin,self).__init__(argument_fns=[fn,],**kwargs)

class tan(_Function):
    """
    Computes the tangent of its argument function
    (measured in radians).

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = tan()
    >>> np.isclose( func.evaluate(0.1234), math.tan(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_tan( fncself )

        # build parent
        super(tan,self).__init__(argument_fns=[fn,],**kwargs)

class acos(_Function):
    """
    Computes the principal value of the arc cosine of x, expressed in radians.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = acos()
    >>> np.isclose( func.evaluate(0.1234), math.acos(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_acos( fncself )

        # build parent
        super(acos,self).__init__(argument_fns=[fn,],**kwargs)

class asin(_Function):
    """
    Computes the principal value of the arc sine of x, expressed in radians.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = asin()
    >>> np.isclose( func.evaluate(0.1234), math.asin(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_asin( fncself )

        # build parent
        super(asin,self).__init__(argument_fns=[fn,],**kwargs)

class atan(_Function):
    """
    Computes the principal value of the arc tangent of x, expressed in radians.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = atan()
    >>> np.isclose( func.evaluate(0.1234), math.atan(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_atan( fncself )

        # build parent
        super(atan,self).__init__(argument_fns=[fn,],**kwargs)

class cosh(_Function):
    """
    Computes the hyperbolic cosine of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = cosh()
    >>> np.isclose( func.evaluate(0.1234), math.cosh(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_cosh( fncself )

        # build parent
        super(cosh,self).__init__(argument_fns=[fn,],**kwargs)

class sinh(_Function):
    """
    Computes the hyperbolic sine of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = sinh()
    >>> np.isclose( func.evaluate(0.1234), math.sinh(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_sinh( fncself )

        # build parent
        super(sinh,self).__init__(argument_fns=[fn,],**kwargs)

class tanh(_Function):
    """
    Computes the hyperbolic tangent of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = tanh()
    >>> np.isclose( func.evaluate(0.1234), math.tanh(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_tanh( fncself )

        # build parent
        super(tanh,self).__init__(argument_fns=[fn,],**kwargs)

class acosh(_Function):
    """
    Computes the inverse hyperbolic cosine of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = acosh()
    >>> np.isclose( func.evaluate(5.1234), math.acosh(5.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_acosh( fncself )

        # build parent
        super(acosh,self).__init__(argument_fns=[fn,],**kwargs)

class asinh(_Function):
    """
    Computes the inverse hyperbolic sine of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = asinh()
    >>> np.isclose( func.evaluate(5.1234), math.asinh(5.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_asinh( fncself )

        # build parent
        super(asinh,self).__init__(argument_fns=[fn,],**kwargs)

class atanh(_Function):
    """
    Computes the inverse hyperbolic tangent of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = atanh()
    >>> np.isclose( func.evaluate(0.1234), math.atanh(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_atanh( fncself )

        # build parent
        super(atanh,self).__init__(argument_fns=[fn,],**kwargs)

class exp(_Function):
    """
    Computes the exponent of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = exp()
    >>> np.isclose( func.evaluate(0.1234), math.exp(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_exp( fncself )

        # build parent
        super(exp,self).__init__(argument_fns=[fn,],**kwargs)

class log(_Function):
    """
    Computes the natural logarithm of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = log()
    >>> np.isclose( func.evaluate(0.1234), math.log(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_log( fncself )

        # build parent
        super(log,self).__init__(argument_fns=[fn,],**kwargs)

class log2(_Function):
    """
    Computes the base 2 logarithm of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = log2()
    >>> np.isclose( func.evaluate(0.1234), math.log(0.1234,2) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_log2( fncself )

        # build parent
        super(log2,self).__init__(argument_fns=[fn,],**kwargs)

class log10(_Function):
    """
    Computes the base 10 logarithm of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = log10()
    >>> np.isclose( func.evaluate(0.1234), math.log10(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_log10( fncself )

        # build parent
        super(log10,self).__init__(argument_fns=[fn,],**kwargs)

class sqrt(_Function):
    """
    Computes the square root of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = sqrt()
    >>> np.isclose( func.evaluate(0.1234), math.sqrt(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_sqrt( fncself )

        # build parent
        super(sqrt,self).__init__(argument_fns=[fn,],**kwargs)

class erf(_Function):
    """
    Computes the error function of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = erf()
    >>> np.isclose( func.evaluate(0.1234), math.erf(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_erf( fncself )

        # build parent
        super(erf,self).__init__(argument_fns=[fn,],**kwargs)

class erfc(_Function):
    """
    Computes the complementary error function of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = erfc()
    >>> np.isclose( func.evaluate(0.1234), math.erfc(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_erfc( fncself )

        # build parent
        super(erfc,self).__init__(argument_fns=[fn,],**kwargs)

class abs(_Function):
    """
    Computes the absolute value of its argument function.

    Parameters
    ----------
    fn: Function (or convertible).  Default: None
        The argument function. Default is None, in which case
        the input is processed directly.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = abs()
    >>> np.isclose( func.evaluate(-0.1234), math.fabs(0.1234) )
    array([[ True]], dtype=bool)

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function._CheckIsFnOrConvertOrThrow(fn)

        fncself = None
        if self._fn:
            fncself = self._fn._fncself

        # create instance
        self._fncself = _cfn.MathUnary_abs( fncself )

        # build parent
        super(abs,self).__init__(argument_fns=[fn,],**kwargs)

class pow(_Function):
    """
    Power function.  Raises fn1 to the power of fn2.

    Parameters
    ----------
    fn1: Function (or convertible).
         The base function.
    fn2: Function (or convertible).
         The power function.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> func = pow(input(),3.)
    >>> np.isclose( func.evaluate(2.), math.pow(2.,3.) )
    array([[ True]], dtype=bool)

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
        self._fncself = _cfn.MathBinary(self._fn1._fncself, self._fn2._fncself,  _cfn.MathBinary.pow )
        # build parent
        super(pow,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)

class dot(_Function):
    """
    Dot product function. Returns fn1.fn2. Argument functions must return values
    of identical size.

    Parameters
    ----------
    fn1,fn2: Function (or convertible).
        The argument functions.

    Examples
    --------
    >>> import math
    >>> import numpy as np
    >>> input1 = (2.,3.,4.)
    >>> input2 = (5.,6.,7.)
    >>> func = dot( input1, input2 )

    The function is constant, so evaluate anywhere:
    >>> np.isclose( func.evaluate(0.), np.dot(input1,input2) )
    array([[ True]], dtype=bool)

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
        self._fncself = _cfn.Dot(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(dot,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)


