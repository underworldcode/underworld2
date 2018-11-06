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
import libUnderworld.libUnderworldPy.Function as _cfn
from ._function import Function as _Function
import underworld as _uw

class cos(_Function):
    """
    Computes the cosine of its argument function
    (measured in radians).

    Parameters
    ----------
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = cos()
    >>> np.allclose( func.evaluate(0.1234), sysmath.cos(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = sin()
    >>> np.allclose( func.evaluate(0.1234), sysmath.sin(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = tan()
    >>> np.allclose( func.evaluate(0.1234), sysmath.tan(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = acos()
    >>> np.allclose( func.evaluate(0.1234), sysmath.acos(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = asin()
    >>> np.allclose( func.evaluate(0.1234), sysmath.asin(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = atan()
    >>> np.allclose( func.evaluate(0.1234), sysmath.atan(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = cosh()
    >>> np.allclose( func.evaluate(0.1234), sysmath.cosh(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = sinh()
    >>> np.allclose( func.evaluate(0.1234), sysmath.sinh(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = tanh()
    >>> np.allclose( func.evaluate(0.1234), sysmath.tanh(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = acosh()
    >>> np.allclose( func.evaluate(5.1234), sysmath.acosh(5.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = asinh()
    >>> np.allclose( func.evaluate(5.1234), sysmath.asinh(5.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = atanh()
    >>> np.allclose( func.evaluate(0.1234), sysmath.atanh(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = exp()
    >>> np.allclose( func.evaluate(0.1234), sysmath.exp(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.


    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = log()
    >>> np.allclose( func.evaluate(0.1234), sysmath.log(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = log2()
    >>> np.allclose( func.evaluate(0.1234), sysmath.log(0.1234,2) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = log10()
    >>> np.allclose( func.evaluate(0.1234), sysmath.log10(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = sqrt()
    >>> np.allclose( func.evaluate(0.1234), sysmath.sqrt(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = erf()
    >>> np.allclose( func.evaluate(0.1234), sysmath.erf(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = erfc()
    >>> np.allclose( func.evaluate(0.1234), sysmath.erfc(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn: underworld.function.Function (or convertible).
        Optionally provided for function composition.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = abs()
    >>> np.allclose( func.evaluate(-0.1234), sysmath.fabs(0.1234) )
    True

    """
    def __init__(self, fn=None, *args, **kwargs):

        self._fn = _Function.convert(fn)

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
    fn1: underworld.function.Function (or convertible).
         The base function.
    fn2: underworld.function.Function (or convertible).
         The power function.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> func = pow(_uw.function.input(),3.)
    >>> np.allclose( func.evaluate(2.), sysmath.pow(2.,3.) )
    True

    """
    def __init__(self, fn1, fn2, **kwargs):
        # lets convert integer powers to floats
        if isinstance(fn2, int):
            fn2 = float(fn2)
        fn1fn = _Function.convert( fn1 )
        if not isinstance( fn1fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = _Function.convert( fn2 )
        if not isinstance( fn2fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Pow(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(pow,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)

class atan2(_Function):
    """
    arctan2 function. Returns the arc tangent of y/x, expressed in radians.

    Parameters
    ----------
    fn1: underworld.function.Function (or convertible).
         The function to compute y values.
    fn2: underworld.function.Function (or convertible).
         The function to compute x values.

    Example
    -------
    >>> import numpy as np
    >>> func = atan2(_uw.function.input(),3.)
    >>> np.allclose( func.evaluate(3.), np.pi/4 ) # TODO:think it should fail.
    True

    """
    def __init__(self, fn1, fn2, **kwargs):
        # lets convert integer powers to floats
        fn1fn = _Function.convert( fn1 )
        if not isinstance( fn1fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = _Function.convert( fn2 )
        if not isinstance( fn2fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Atan2(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(atan2,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)


class dot(_Function):
    """
    Dot product function. Returns fn1.fn2. Argument functions must return values
    of identical size.

    Parameters
    ----------
    fn1: underworld.function.Function (or convertible).
         Argument function 1.
    fn2: underworld.function.Function (or convertible).
         Argument function 2.

    Example
    -------
    >>> import math as sysmath
    >>> import numpy as np
    >>> input1 = (2.,3.,4.)
    >>> input2 = (5.,6.,7.)
    >>> func = dot( input1, input2 )

    The function is constant, so evaluate anywhere:
    
    >>> np.allclose( func.evaluate(0.), np.dot(input1,input2) )
    True

    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = _Function.convert( fn1 )
        if not isinstance( fn1fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = _Function.convert( fn2 )
        if not isinstance( fn2fn, _Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Dot(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(dot,self).__init__(argument_fns=[fn1fn,fn2fn],**kwargs)
