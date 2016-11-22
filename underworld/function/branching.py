##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
The branching module provides functions which provide branching behaviour. 
Typically, these functions will select other user provided functions when 
certain conditions are met (with the condition also described by a function!).

"""
import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function


class map(_Function):
    """
    This function performs a map to other functions. The user provides a python 
    dictionary which maps unsigned integers keys to underworld functions. The 
    user must also provide a key function. At evaluation time, the key function
    is evaluated first, with the outcome determining which function should
    finally be evaluated to return a value.
    
    For a set of value functions :math:`\{f_{v_0},f_{v_1},\ldots,f_{v_n}\}`,
    corresponding keys :math:`\{k_0,k_1,\ldots,k_n\}', and key function
    :math:`f_{k}`, we have:
    
    .. math::
         f(\mathbf{r})=
            \begin{cases}
            f_{v_0}(\mathbf{r}),   & \text{if } f_{k}(\mathbf{r}) = k_0\\
            f_{v_1}(\mathbf{r}),   & \text{if } f_{k}(\mathbf{r}) = k_1\\
            ... \\
            f_{v_n}(\mathbf{r}),   & \text{if } f_{k}(\mathbf{r}) = k_n\\
            f_{d}  (\mathbf{r}),   & \text{otherwise}
            \end{cases}
            
    As stated, the keys must be unsigned integers. The key function need not
    return an unsigned integer, but whatever value it returns **will** be cast
    to an unsigned integer so caution is advised.
    
    The default function is optional, but if none is provided, and the key 
    function evaluates to a value which is not within the user provide set of 
    keys, an exception will be thrown.
    
    Parameters
    ----------
    fn_key:  underworld.function.Function (or convertible)
        Function which returns integer key values. This function will be evaluated 
        first to determine which function from the mapping is to be used. 
    mapping: dict(Function)
        Python dictionary providing a mapping from unsigned integer 'key' values to
        underworld 'value' functions. Note that the provided 'value' functions must
        return values of type 'double'.
    fn_default: underworld.function.Function (or convertible) (optional)
        Default function to be utilised when the key (returned by fn_key function)
        does not correspond to any key value in the mapping dictionary.
    
    
    The following example sets different function behaviour inside and outside
    of a unit sphere. The unit sphere is represented by particles which 
    record a swarm variable to determine if they are or not inside the sphere.


    Example
    -------
    Setup mesh, swarm, swarmvariable & populate
    
    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> import numpy as np
    >>> mesh = uw.mesh.FeMesh_Cartesian(elementRes=(8,8),minCoord=(-1.0, -1.0), maxCoord=(1.0, 1.0))
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> svar = swarm.add_variable("int",1)
    >>> swarm.populate_using_layout(uw.swarm.layouts.GlobalSpaceFillerLayout(swarm,20))

    For all particles in unit circle, set svar to 1

    >>> svar.data[:] = 0
    >>> for index, position in enumerate(swarm.particleCoordinates.data):
    ...     if position[0]**2 + position[1]**2 < 1.:
    ...         svar.data[index] = 1
    
    Create a function which reports the value '1.' inside the sphere, and
    '0.' otherwise. Note that while we have only used constant value functions 
    here, you can use any object of the class Function.
    
    >>> fn_map = fn.branching.map(fn_key=svar, mapping={0: 0., 1:1.})
    >>> np.allclose(np.pi, uw.utils.Integral(fn_map,mesh).evaluate(),rtol=2e-2)
    True
    
    Alternatively, we could utilise the default function to achieve the same 
    result.
    
    >>> fn_map = fn.branching.map(fn_key=svar, mapping={1: 1.}, fn_default=0.)
    >>> np.allclose(np.pi, uw.utils.Integral(fn_map,mesh).evaluate(),rtol=2e-2)
    True

    """
    

    def __init__(self, fn_key=None, mapping=None, fn_default=None, *args, **kwargs):
                    
        if not mapping:
            raise ValueError("You must specify a mapping via the 'mapping' parameter.")
        if not isinstance(mapping, dict):
            raise TypeError("'mapping' object passed in must be of python type 'dict'")

        if not fn_key:
            raise ValueError("You must specify a key function via the 'fn_key' parameter.")
        fn_key = _Function.convert(fn_key)

        self.fn_default = _Function.convert(fn_default)
        if self.fn_default == None:
            fn_defaultCself = None
        else:
            fn_defaultCself = self.fn_default._fncself
        # create instance
        self._fncself = _cfn.Map( fn_key._fncself, fn_defaultCself )

        self._fn_key  = fn_key
        self._mapping = mapping

        # build parent
        super(map,self).__init__(argument_fns=[fn_key,self.fn_default],**kwargs)
        
        self._map = {}

        for key, value in mapping.iteritems():
            if not isinstance(key, int) or key < 0:
                raise ValueError("Key '{}' not valid. Mapping keys must be unsigned integers.".format(key))
            funcVal = _Function.convert(value)
            if funcVal == None:
                raise ValueError("'None' is not valid for mapped functions.")
            
            self._underlyingDataItems.update(funcVal._underlyingDataItems) # update dictionary
            # insert mapping and keep handles in py dict
            self._map[key] = funcVal
            self._fncself.insert( key, funcVal._fncself )

class conditional(_Function):
    """
    This function provides 'if/elif' type conditional behaviour.
    
    The user provides a list of tuples, with each tuple being of the 
    form (fn_condition, fn_resultant). Effectively, each tuple provides a clause
    within the if/elif statement. 
    
    When evaluated, the function traverses the clauses, stopping at the first 
    fn_condition which returns 'true'. It then executes the corresponding 
    fn_resultant and returns the results.
    
    If none of the provided clauses return a 'True' result, an exception is 
    raised.

    For a set of condition functions :math:`\{f_{c_0},f_{c_1},\ldots,f_{c_n}\}`,
    corresponding resultant functions :math:`\{f_{r_0},f_{r_1},\ldots,f_{r_n}\}`,
    we have
    
    ```
    if   :math:`\{f_{c_0}` :
        return :math:`\{f_{r_0}`
    elif :math:`\{f_{c_1}` :
        return :math:`\{f_{r_1}`
    ...
    elif :math:`\{f_{c_n}` :
        return :math:`\{f_{r_n}`
    else :
        raise RuntimeError("Reached end of conditional statement. At least one 
                            of the clause conditions must evaluate to 'True'." );
        
    ```
    
    Parameters
    ----------
    clauses: list
        list of tuples, with each tuple being of the form (fn_condition, fn_resultant).
    
    
    Example
    -------
    The following example uses functions to represent a unit circle. Here a 
    conditional function report back the value '1.' inside the sphere (as per
    the first condition), and '0.' otherwise.

    >>> import underworld as uw
    >>> import underworld.function as fn
    >>> import numpy as np
    >>> mesh = uw.mesh.FeMesh_Cartesian(elementRes=(16,16),minCoord=(-1.0, -1.0), maxCoord=(1.0, 1.0))
    >>> circleFn = fn.coord()[0]**2 + fn.coord()[1]**2
    >>> fn_conditional = fn.branching.conditional( [ (circleFn < 1., 1. ), \
                                                     (         True, 0. ) ] )
    >>> np.allclose(np.pi, uw.utils.Integral(fn_conditional,mesh).evaluate(),rtol=1e-2)
    True
    """

    def __init__(self, clauses, *args, **kwargs):
        # error check mapping
        if not isinstance(clauses, (list,tuple)):
            raise TypeError("'clauses' object passed in must be of python type 'list' or 'tuple'")

        self._clauses = []
        funcSet = set()
        for clause in clauses:
            if not isinstance(clause, (list,tuple)):
                raise TypeError("Clauses within the clause list must be of python type 'list' or 'tuple'")
            if len(clause) != 2:
                raise ValueError("Clauses tuples must be of length 2.")
            conditionFn = _Function.convert(clause[0])
            funcSet.add(conditionFn)
            resultantFn    = _Function.convert(clause[1])
            funcSet.add(resultantFn)
            self._clauses.append( (conditionFn,resultantFn) )
        
        # build parent
        self._fncself = _cfn.Conditional()

        super(conditional,self).__init__(argument_fns=funcSet,**kwargs)

        # insert clause into c object now
        for clause in self._clauses:
            self._fncself.insert( clause[0]._fncself, clause[1]._fncself )



