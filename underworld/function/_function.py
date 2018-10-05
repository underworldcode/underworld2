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
import underworld as uw

ScalarType = _cfn.FunctionIO.Scalar
VectorType = _cfn.FunctionIO.Vector
SymmetricTensorType = _cfn.FunctionIO.SymmetricTensor
TensorType = _cfn.FunctionIO.Tensor
ArrayType  = _cfn.FunctionIO.Array

types = {          'scalar' : ScalarType,
                   'vector' : VectorType,
          'symmetrictensor' : SymmetricTensorType,
                   'tensor' : TensorType,
                    'array' : ArrayType   }

class FunctionInput(underworld._stgermain.LeftOverParamsChecker, metaclass = ABCMeta):
    """
    Objects that inherit from this class are able to act as inputs
    to function evaluation from python.
    """
    @abstractmethod
    def _get_iterator(self):
        """ All children should define this method which returns the
        c iterator object """
        pass

class Function(underworld._stgermain.LeftOverParamsChecker, metaclass = ABCMeta):
    """
    Objects which inherit from this class provide user definable functions
    within Underworld.

    Functions aim to achieve a number of goals:
    * Provide a natural interface for mathematical behaviour description within python.
    * Provide a high level interface to Underworld discrete objects.
    * Allow discrete objects to be used in combination with continuous objects.
    * Handle the evaluation of discrete objects in the most efficient manner.
    * Perform all heavy calculations at the C-level for efficiency.
    * Provide an interface for users to evaluate functions directly within python, 
    utilising numpy arrays for input/output.
    """
    def __init__(self, argument_fns, **kwargs):

        self._underlyingDataItems = weakref.WeakSet()
        if argument_fns:
            for argfn in argument_fns:
                if argfn:
                    # add to current functions set.. note that we convert incase passed in
                    # function needs to be converted.
                    self._underlyingDataItems.update(self.convert(argfn)._underlyingDataItems)

        super(Function,self).__init__(**kwargs)

    @property
    def _fncself(self):
        if not hasattr(self, '_fncselfpriv'):
            return None
        else:
            return self._fncselfpriv
    @_fncself.setter
    def _fncself(self, _fncself):
        self._fncselfpriv = _fncself
        # now record the construction time stack to the c object so
        # that if it fails, it can signal to the user where the
        # constructor was called from (ie, which function failed).
        # Walk stack in reversed order. Also, only list a few frames
        # as ipython/jupyter stacks are quiet ugly.
        # Construct within try context, as extracting stack info
        # may not be robust, so better to continue quietly if things
        # go awry.
        import underworld as uw
        from inspect import stack
        rank = str(uw.rank())+'- '
        strguy = "Error in function of class '{}'".format(self.__class__.__name__)
        try:
            if uw._in_doctest():
                # doctests don't play nice with stacks
                stackstr = "   --- CONSTRUCTION TIME STACK ---"
            else:
                stackstr = ""
                for item in stack()[2:7][::-1]:
                    stackstr += rank+item[1]+':'+str(item[2]) + ',\n'
                    if item[4]:
                        stackstr += "    " + item[4][0].lstrip()
            strguy += " constructed at:\n{}\nError message:\n".format(str(stackstr))
        except:
            pass

        self._fncself.set_pyfnerrorheader(strguy)



    
    @staticmethod
    def convert(obj):
        """
        This method will attempt to convert the provided input into an equivalent
        underworld function. If the provided input is already of Function type,
        it is immediately returned. Likewise, if the input is of None type, it is
        also returned.
        
        Parameters
        ----------
        obj: fn_like
            The object to be converted. Note that if obj is of type None or
            Function, it is simply returned immediately.
            Where obj is of type int/float/double, a Constant type function 
            is returned which evaluates to the provided object's value.
            Where obj is of type list/tuple, a function will be returned
            which evaluates to a vector of the provided list/tuple's values 
            (where possible).

        Returns
        -------
        Fn.Function or None.

        Examples
        --------
        >>> import underworld as uw
        >>> import underworld.function as fn
        
        >>> fn_const = fn.Function.convert( 3 )
        >>> fn_const.evaluate(0.) # eval anywhere for constant
        array([[3]], dtype=int32)

        >>> fn_const == fn.Function.convert( fn_const )
        True

        >>> fn.Function.convert( None )

        >>> fn1 = fn.input()
        >>> fn2 = 10.*fn.input()
        >>> fn3 = 100.*fn.input()
        >>> vec = (fn1,fn2,fn3)
        >>> fn_vec = fn.Function.convert(vec)
        >>> fn_vec.evaluate([3.])
        array([[   3.,   30.,  300.]])

        """
        if isinstance(obj, (Function, type(None)) ):
            return obj
        else:
            import underworld.function.misc as misc
            try:
                # first try convert directly to const type object
                return misc.constant(obj)
            except Exception as e:
                # ok, that failed, let's now try convert to vector of function type object
                # first check that it is of type tuple or list
                if isinstance(obj, (tuple,list)):
                    # now check that it contains things that are convertible. recurse.
                    objlist = []
                    for item in obj:
                        objlist.append( Function.convert(item) )
                    # create identity matrix for basis vectors
                    import numpy as np
                    basisvecs = np.identity(len(obj))
                    # convert these to uw functions
                    basisfns = []
                    for vec in basisvecs:
                        basisfns.append( Function.convert(vec) )
                    # now return final object summed
                    vecfn = basisfns[0]*objlist[0]
                    for index in range(1,len(obj)):
                        vecfn = vecfn + basisfns[index]*objlist[index]
                    return vecfn
                
                import underworld as uw
                raise uw._prepend_message_to_exception(e, "An exception was raised while try to convert an "
                                                         +"object to an Underworld2 function. Usually this "
                                                         +"occurs because you didn't pass in the correct "
                                                         +"object. Please check your function arguments! "
                                                         +"Note that only python types 'int', 'float' or "
                                                         +"'bool' (and iterables of these types) are convertible "
                                                         +"to functions.\n\n"
                                                         +"Original encountered exception message:\n")


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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> three = misc.constant(3.)
        >>> four  = misc.constant(4.)
        >>> np.allclose( (three + four).evaluate(0.), [[ 7.]]  )  # note we can evaluate anywhere because it's a constant
        True

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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> three = misc.constant(3.)
        >>> four  = misc.constant(4.)
        >>> np.allclose( (three - four).evaluate(0.), [[ -1.]]  )  # note we can evaluate anywhere because it's a constant
        True

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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> four  = misc.constant(4.)
        >>> np.allclose( (5. - four).evaluate(0.), [[ 1.]]  )  # note we can evaluate anywhere because it's a constant
        True

        """
        return subtract( other, self )

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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> four = misc.constant(4.)
        >>> np.allclose( (-four).evaluate(0.), [[ -4.]]  )  # note we can evaluate anywhere because it's a constant
        True

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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> three = misc.constant(3.)
        >>> four  = misc.constant(4.)
        >>> np.allclose( (three*four).evaluate(0.), [[ 12.]]  )  # note we can evaluate anywhere because it's a constant
        True

        """
        return multiply( self, other )
    __rmul__ = __mul__

    def __truediv__(self,other):
        """
        Operator overloading for '/' operation:

        Fn3 = Fn1 / Fn2

        Creates a new function Fn3 which returns the quotient of Fn1 and Fn2.

        Returns
        -------
        fn.divide: Divide function

        Examples
        --------
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> np.allclose( (four/two).evaluate(0.), [[ 2.]]  )  # note we can evaluate anywhere because it's a constant
        True

        """
        return divide( self, other )

    def __rtruediv__(self,other):
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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> np.allclose( (two**four).evaluate(0.), [[ 16.]]  )  # note we can evaluate anywhere because it's a constant
        True

        """
        from . import math as fnmath
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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> (two < four).evaluate()
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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> (two <= two).evaluate()
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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> four = misc.constant(4.)
        >>> (two > four).evaluate()
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
        >>> import underworld.function.misc as misc
        >>> import numpy as np
        >>> two  = misc.constant(2.)
        >>> (two >= two).evaluate()
        array([[ True]], dtype=bool)

        """
        return greater_equal( self, other )

    def __and__(self,other):
        """
        Operator overloading for '&' operation:

        Fn3 = Fn1 & Fn2

        Creates a new function Fn3 which returns a bool result for the operation.

        Returns
        -------
        fn.logical_and: AND function

        Examples
        --------
        >>> import underworld.function.misc as misc
        >>> trueFn  = misc.constant(True)
        >>> falseFn = misc.constant(False)
        >>> (trueFn & falseFn).evaluate()
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

        Creates a new function Fn3 which returns a bool result for the operation.

        Returns
        -------
        fn.logical_or: OR function

        Examples
        --------
        >>> import underworld.function.misc as misc
        >>> trueFn  = misc.constant(True)
        >>> falseFn = misc.constant(False)
        >>> (trueFn | falseFn).evaluate()
        array([[ True]], dtype=bool)

        Notes
        -----
        The '|' operator in python is usually used for bitwise 'or' operations, 
        with the 'or' operator used for boolean type operators. It is not possible 
        to overload the 'or' operator in python, so instead the bitwise equivalent 
        has been utilised.


        """

        return logical_or( self, other )

    def __xor__(self,other):
        """
        Operator overloading for '^' operation:

        Fn3 = Fn1 ^ Fn2

        Creates a new function Fn3 which returns a bool result for the operation.

        Returns
        -------
        fn.logical_xor: XOR function

        Examples
        --------
        >>> import underworld.function.misc as misc
        >>> trueFn  = misc.constant(True)
        >>> falseFn = misc.constant(False)
        >>> (trueFn ^ falseFn).evaluate()
        array([[ True]], dtype=bool)
        >>> (trueFn ^ trueFn).evaluate()
        array([[False]], dtype=bool)
        >>> (falseFn ^ falseFn).evaluate()
        array([[False]], dtype=bool)

        Notes
        -----
        The '^' operator in python is usually used for bitwise 'xor' operations, 
        however here we always use the logical version, with the operation 
        inputs cast to their bool equivalents before the operation.  


        """

        return logical_xor( self, other )

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
        >>> import underworld.function.misc as misc
        >>> fn  = misc.constant((2.,3.,4.))
        >>> np.allclose( fn[1].evaluate(0.), [[ 3.]]  )  # note we can evaluate anywhere because it's a constant
        True

        """
        return at(self,index)

    def evaluate_global(self, inputData, inputType=None):
        """
        This method attempts to evalute inputData across all processes, and 
        then consolide the results on the root processor. This is most useful
        where you wish to evalute your functions using global coordinates 
        which may span processes in a parallel simulation.
        
        Note that this method does not currently support 'FunctionInput' class
        input data.
        
        Due to the communications required for this method, a significant 
        performance overhead may be encountered. The standard `evaluate` method 
        should be used instead wherever possible.

        Please see `evaluate` method for parameter details.

        Notes
        -----
        This method must be called collectively by all processes.
        
        Returns
        -------
        Only the root process gets the final results array. All other processes
        are returned None.

        """
        from mpi4py import MPI
        comm = MPI.COMM_WORLD
        rank = comm.Get_rank()
        nprocs = comm.Get_size()

        if isinstance(inputData, FunctionInput):
            raise TypeError("This 'inputData' type is not currently supported for global function evaluation.")
        # go through the inputData and fill elements where the data is available
        if not isinstance(inputData, np.ndarray):
            inputData = self._evaluate_data_convert_to_ndarray(inputData)
        arrayLength = len(inputData)
        local =  np.zeros(arrayLength, dtype=bool)
        local_output = None
        for i in range(arrayLength):
            try:
                # get result
                output = self.evaluate(inputData[i:i+1], inputType)
                # flag as result found
                local[i]  = True
                # if not created, create
                if not isinstance(local_output, np.ndarray):
                    local_output =  np.zeros( (arrayLength, output.shape[1]), dtype=output.dtype)
                local_output[i] = output
            except ValueError:
                # ValueError is only raised for outside domain, which suggests that the
                # evaluation probably occurred on another process.
                pass
            except:
                # if a different error was raise, we should reraise
                raise

        # distill results down to local only
        local_result_count = np.count_nonzero(local)
        if local_result_count:
            local_output_distilled = np.zeros( (local_result_count, local_output.shape[1]), dtype=local_output.dtype)
            array_positions        = np.zeros(local_result_count, dtype=int)
            j=0
            for i,val in enumerate(local):
                if val:
                    array_positions[j]=i
                    local_output_distilled[j] = local_output[i]
                    j+=1

        # data sending
        total_output = None
        if(rank!=0):
            # send count
            comm.send(local_result_count, dest=0, tag=0)
            if local_result_count:
                # next send position array
                comm.send(array_positions, dest=0, tag=1)
                # finally send actual data
                comm.send(local_output_distilled,    dest=0, tag=2)
        else:
            # have output already from rank=0 proc; and lots of empties to fill in from others
            # some data IS available two multiple processors - e.g. edges
            for iProc in range(1,nprocs):
                incoming_count = comm.recv(source=iProc, tag=0)
                if incoming_count:
                    incoming_positions = comm.recv(source=iProc, tag=1)
                    incoming_data      = comm.recv(source=iProc, tag=2)
                    # create array if not done already
                    if not isinstance(total_output, np.ndarray):
                        total_output =  np.zeros( (arrayLength, incoming_data.shape[1]), dtype=incoming_data.dtype)
                    total_output[incoming_positions] = incoming_data

        # finally copy our local results into the output
        if (rank==0) and local_result_count:
            if not isinstance(total_output,np.ndarray):
                total_output =  np.zeros( (arrayLength,local_output_distilled.shape[1]), dtype=local_output_distilled.dtype)
            total_output[array_positions] = local_output_distilled

        if (rank==0) and (isinstance(total_output,np.ndarray)==False):
            # if total_output is still non-existent, no results were found
            raise RuntimeError("No results were found anywhere in the domain for provided input.")

        if rank == 0:
            return total_output
        else:
            # all other procs return None
            return None


    def _evaluate_data_convert_to_ndarray( self, inputData ):
        # convert single values to tuples if necessary
        if isinstance( inputData, float ):
            inputData = (inputData,)
        # convert to ndarray
        if isinstance( inputData, (list,tuple) ):
            arr = np.empty( [1,len(inputData)] )
            ii = 0
            for guy in inputData:
                if not isinstance(guy, float):
                    raise TypeError("Iterable inputs must only contain python 'float' objects.")
                arr[0,ii] = guy
                ii +=1
            return arr
        else:
            raise TypeError("Input provided for function evaluation does not appear to be supported.")

    # def integrate_fn( self, mesh ):
    def integrate( self, mesh ):
        """
        Perform an integral of this underworld function over the given mesh

        Parameters
        ----------
        mesh : uw.mesh.FeMesh_Cartesian
            Domain to perform integral over.

        Examples
        --------

        >>> mesh = uw.mesh.FeMesh_Cartesian(minCoord=(0.0,0.0), maxCoord=(1.0,2.0))
        >>> fn_1 = uw.function.misc.constant(2.0)
        >>> np.allclose( fn_1.integrate( mesh )[0], 4 )
        True

        >>> fn_2 = uw.function.misc.constant(2.0) * (0.5, 1.0)
        >>> np.allclose( fn_2.integrate( mesh ), [2,4] )
        True

        """

        if not isinstance(mesh, uw.mesh.FeMesh_Cartesian):
            raise RuntimeError("Error: integrate() is only available on meshes of type 'FeMesh_Cartesian'")
        return mesh.integrate( fn=self )

    def evaluate(self,inputData=None,inputType=None):
        """
        This method performs evaluate of a function at the given input(s).

        It accepts floats, lists, tuples, numpy arrays, or any object which is of
        class `FunctionInput`. lists/tuples must contain floats only.

        `FunctionInput` class objects are shortcuts to their underlying data, often
        with performance advantages, and sometimes they are the only valid input
        type (such as using `Swarm` objects as an inputs to `SwarmVariable`
        evaluation). Objects of class `FeMesh`, `Swarm`, `FeMesh_IndexSet` and
        `VoronoiIntegrationSwarm` are also of class `FunctionInput`. See the
        Function section of the user guide for more information.

        Results are returned as numpy array.

        Parameters
        ----------
        inputData: float, list, tuple, ndarray, underworld.function.FunctionInput
            The input to the function. The form of this input must be appropriate
            for the function being evaluated, or an exception will be thrown.
            Note that if no input is provided, function will be evaluated at `0.`
        inputType: str
            Specifies the type the provided data represents. Acceptable 
            values are 'scalar', 'vector', 'symmetrictensor', 'tensor',
            'array'.

        Returns
        -------
        ndarray: array of results

        Examples
        --------
        >>> from . import _systemmath as math
        >>> import underworld.function.math as fnmath
        >>> sinfn = fnmath.sin()
        
        Single evaluation:
        
        >>> np.allclose( sinfn.evaluate(math.pi/4.), [[ 0.5*math.sqrt(2.)]]  )
        True
        
        Multiple evaluations
        
        >>> input = (0.,math.pi/4.,2.*math.pi)
        >>> np.allclose( sinfn.evaluate(input), [[ 0., 0.5*math.sqrt(2.), 0.]]  )
        True
        
        
        Single MeshVariable evaluations
        
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> var = uw.mesh.MeshVariable(mesh,1)
        >>> import numpy as np
        >>> var.data[:,0] = np.linspace(0,1,len(var.data))
        >>> result = var.evaluate( (0.2,0.5 ) )
        >>> np.allclose( result, np.array([[ 0.45]]) )
        True
        
        Numpy input MeshVariable evaluation
        
        >>> # evaluate at a set of locations.. provide these as a numpy array.
        >>> count = 10
        >>> # create an empty array
        >>> locations = np.zeros( (count,2))
        >>> # specify evaluation coodinates
        >>> locations[:,0] = 0.5
        >>> locations[:,1] = np.linspace(0.,1.,count)
        >>> # evaluate
        >>> result = var.evaluate(locations)
        >>> np.allclose( result, np.array([[ 0.08333333], \
                                          [ 0.17592593], \
                                          [ 0.26851852], \
                                          [ 0.36111111], \
                                          [ 0.4537037 ], \
                                          [ 0.5462963 ], \
                                          [ 0.63888889], \
                                          [ 0.73148148], \
                                          [ 0.82407407], \
                                          [ 0.91666667]])  )
        True
        
        Using the mesh object as a FunctionInput
        
        >>> np.allclose( var.evaluate(mesh), var.evaluate(mesh.data))
        True
        
        """
        if inputData is None:
            inputData = 0.
        if inputType != None and inputType not in types.keys():
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
            # lets check if this array owns its data.. process directly if it does, otherwise take a copy..
            # this is to avoid a bug in the way we parse non-trivial numpy arrays.  will fix in future.  #152
            # Note, we also added the check for 'F_CONTIGUOUS' as we also don't handle this correctly it seems. 
            if (not (inputData.base is None)) or inputData.flags['F_CONTIGUOUS']:
                inputData = inputData.copy()
            return _cfn.Query(self._fncself).query(_cfn.NumpyInput(inputData,inputType))
        else:
            # try convert and recurse
            return self.evaluate( self._evaluate_data_convert_to_ndarray(inputData), inputType )

class add(Function):
    """
    This class implements the addition of two functions.
    It is invoked by the overload methods __add__ and __radd__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
        fn1fn = Function.convert( x_Fn )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( y_Fn )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Subtract(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(subtract,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class multiply(Function):
    """
    This class implements the product of two functions
    It is invoked by the overload method __mul__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
    It is invoked by the overload methods __truediv__ and __rdiv__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
#        fn1fn = Function.convert( fn1 )
#        if not isinstance( fn1fn, Function ):
#            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
#        fn2fn = Function.convert( fn2 )
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
#        fn1fn = Function.convert( fn1 )
#        if not isinstance( fn1fn, Function ):
#            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
#        fn2fn = Function.convert( fn2 )
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
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
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
    This class implements the OR logical operation.
    It is invoked by the overload method __or__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_logical_or(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(logical_or,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)

class logical_xor(Function):
    """
    This class implements the XOR logical operation.
    It is invoked by the overload method __xor__.
    """
    def __init__(self, fn1, fn2, **kwargs):
        fn1fn = Function.convert( fn1 )
        if not isinstance( fn1fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")
        fn2fn = Function.convert( fn2 )
        if not isinstance( fn2fn, Function ):
            raise TypeError("Functions must be of type (or convertible to) 'Function'.")

        self._fn1 = fn1fn
        self._fn2 = fn2fn
        # ok finally lets create the fn
        self._fncself = _cfn.Relational_logical_xor(self._fn1._fncself, self._fn2._fncself )
        # build parent
        super(logical_xor,self).__init__(argument_fns=[fn1fn,fn2fn], **kwargs)



class at(Function):
    """
    This class implements extraction the nth component of a function.
    It is invoked by the overload method __getitem__.
    """
    def __init__(self, fn, n, *args, **kwargs):

        _fn = Function.convert(fn)
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
    This class generates a function which simply passes through its input. It
    is the identity function. It is often useful when construct functions where 
    the input itself needs to be accessed, such as to extract a particular 
    component.
    
    For example, you may wish to use this function when you wish to extract a 
    particular coordinate component for manipulation. For this reason, we also 
    provide an alias to this class called 'coord'.

    Returns
    -------
    fn.input: the input function

    Examples
    --------
    Here we see the input function simply passing through its input.
    
    >>> infunc = input()
    >>> np.allclose( infunc.evaluate( (1.,2.,3.) ), [ 1., 2., 3.] )
    True

    Often this behaviour is useful when we want to construct a function
    which operates on only a particular coordinate, such as a depth
    dependent density. We may wish to extract the z coordinate (in
    2d):

    >>> zcoord = input()[1]
    >>> baseDensity = 1.
    >>> density = baseDensity - 0.01*zcoord
    >>> testCoord1 = (0.1,0.4)
    >>> testCoord2 = (0.9,0.4)
    >>> np.allclose( density.evaluate( testCoord1 ), density.evaluate( testCoord2 ) )
    True

    """
    def __init__(self, *args, **kwargs):

        # create instance
        self._fncself = _cfn.Input()

        # build parent
        super(input,self).__init__(argument_fns=None, *args, **kwargs)


coord = input
