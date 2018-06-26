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
    the evaluation of its argument function:
    
    * Divide by zero
    * Invalid domain was used for evaluation
    * Value overflow errors
    * Value underflow errors

    If any of the above are encountered, an exception is thrown at the conclusion
    of the argument function evaluation.
    
    Parameters
    ----------
    fn: underworld.function.Function
        The function that is subject to the testing.
        
    Example
    -------
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> one = fn.misc.constant(1.)
    >>> zero  = fn.misc.constant(0.)
    >>> fn_dividebyzero = one/zero
    >>> safedividebyzero = fn.exception.SafeMaths(fn_dividebyzero)
    >>> safedividebyzero.evaluate()
    Traceback (most recent call last):
    RuntimeError: SafeMaths function constructed at
<BLANKLINE>
    0- doctefsts.py:11,
        res = doctest.testmod(mod)
    0- doctest.py:1911,
        runner.run(test)
    0- doctest.py:1454,
        return self.__run(test, compileflags, out)
    0- doctest.py:1315,
        compileflags, 1) in test.globs
    0- <doctest underworld.function.exception.SafeMaths[5]>:1,
        safedividebyzero = fn.exception.SafeMaths(fn_dividebyzero)
<BLANKLINE>
    detected the following floating point exception(s), generated while evaluating its argument function:
       Divide by zero
    """
    
    def __init__(self, fn, *args, **kwargs):

        _fn = _Function.convert(fn)
        if _fn == None:
            raise ValueError( "provided 'fn' must a 'Function' or convertible.")
        self._fn = _fn

        # create instance
        self._fncself = _cfn.SafeMaths( self._fn._fncself )

        # build parent
        super(SafeMaths,self).__init__(argument_fns=[_fn,],**kwargs)
        

class CustomException(_Function):
    """  
    This function allows you to set custom exceptions within your model. You
    must pass it two functions: the first function is the pass through function,
    the second function is the required condition. You may also pass in a optional
    third function whose output will be printed if the condition evaluates to False.

    A CustomException function will perform the following logic:
    
        1. Evaluate the condition function.
        2. If it evaluates to False, an exception is thrown and the simulation
           is halted. If a print function is provided, it will be evaluated 
           and its results will be included in the exception message.
        3. If it evaluates to True, the pass through function is evaluated
           with the result then being return.
    
    
    Parameters
    ----------
    fn_passthrough: underworld.function.Function
        The pass through function
    fn_condition: underworld.function.Function
        The condition function
    fn_print: underworld.function.Function
        The print function (optional).
        
    Example
    -------
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> one = fn.misc.constant(1.)
    >>> passing_one = fn.exception.CustomException( one, (one < 2.) )
    >>> passing_one.evaluate()
    array([[ 1.]])
    >>> failing_one = fn.exception.CustomException( one, (one > 2.) )
    >>> failing_one.evaluate()
    Traceback (most recent call last):
    ...
    RuntimeError: CustomException condition function has evaluated to False for current input!
    
    Now with printing

    >>> failing_one_by_five = fn.exception.CustomException( one, (one*5. > 20.), one*5. )
    >>> failing_one_by_five.evaluate()
    Traceback (most recent call last):
    ...
    RuntimeError: CustomException condition function has evaluated to False for current input!
    Print function returns the following values (cast to double precision):
        ( 5 )
    
    """
    
    def __init__(self, fn_input, fn_condition, fn_print=None, *args, **kwargs):

        _fn_input = _Function.convert(fn_input)
        if _fn_input == None:
            raise ValueError( "provided 'fn_input' must a 'Function' or convertible.")
        self._fn_input = _fn_input

        _fn_condition = _Function.convert(fn_condition)
        if _fn_condition == None:
            raise ValueError( "provided 'fn_condition' must a 'Function' or convertible.")
        self._fn_condition = _fn_condition

        if fn_print != None:
            _fn_print = _Function.convert(fn_print)
            if _fn_print == None:
                raise ValueError( "provided 'fn_print' must a 'Function' or convertible.")
            self._fn_print = _fn_print

        # create instance
        if not fn_print:
            self._fncself = _cfn.CustomException( self._fn_input._fncself, self._fn_condition._fncself )
        else:
            self._fncself = _cfn.CustomException( self._fn_input._fncself, self._fn_condition._fncself, self._fn_print._fncself )

        # build parent
        # note that we only pass in _fn_input as the argument_fns, as _fn_condition & _fn_print are
        # not dynamically relevant... it is only used for performing the exception check.
        super(CustomException,self).__init__(argument_fns=[_fn_input,],**kwargs)
        


