##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module contains the IndexSet class and related functions.

"""
import underworld as uw
import underworld._stgermain as _stgermain
import numpy as np
import libUnderworld
import copy
import numbers

class IndexSet(object):
    """
    The IndexSet class provides a set type container for integer values.
    The underlying implementation is designed for memory efficiency. 
    Index insertion and removal is a constant time operation.
    
    .. The following is just for rendering the docstrings with Sphinx.
    .. role:: python(code)
       :language: python

    Parameters
    ----------
    
    size : int
        The size of the IndexSet. Note that the size corresponds to the maximum index
        value (plus 1) the set is required to hold, *NOT* the number of elements in
        the set. See IndexSet.size docstring for more information.
    fromObject : iterable, array_like, IndexSet. Optional.
        If provided, an IndexSet will be constructed using provided object's data. 
        See 'add' method for more details on acceptable objects.
        If not provided, empty set is generated.

    Examples
    --------
    You can add items via the constructor:
    
    >>> someSet = uw.container.IndexSet( 15, [3,14,2] )
    >>> someSet
    IndexSet([ 2,  3, 14])
    
    Alternatively, create an empty set and add items as necessary:
    
    >>> someSet = uw.container.IndexSet( 15 )
    >>> someSet
    IndexSet([])
    >>> someSet.add(3)
    >>> someSet
    IndexSet([3])
    >>> someSet.add( [2,11] )
    >>> someSet
    IndexSet([ 2,  3, 11])
    
    Python operators are overloaded for convenience. Check class method details
    for full details.
    
    """

    def __init__(self, size, fromObject=None):
        if not isinstance(size,int):
            raise ValueError("The 'size' parameter must be of type int.")
        if size <= 0:
            raise ValueError("The 'size' parameter must be positive.")
        self._size = size
        
        # ok, let's do a cheeky, and if the object passed in is a native stg guy,
        # we'll take ownership        
        if isinstance(fromObject,libUnderworld.StGermain.IndexSet):
            self._cself = fromObject
            self._size = fromObject.size
        else:
            # lets create our IndexSet object
            self._cself = libUnderworld.StGermain.IndexSet_New( self.size )

            if fromObject is not None:
                self.add(fromObject)

    def __del__(self):
        # delete stg class
        libUnderworld.StGermain.Stg_Class_Delete(self._cself)
    
    @property
    def size(self):
        """
        The size of the IndexSet. Note that the size corresponds to the maximum index
        value (plus 1) the set is required to hold, *NOT* the number of elements in 
        the set. So for example, a size of 16, would result in an IndexSet which can 
        retain values between 0 and 15 (inclusive). Note also that the the IndexSet
        will require ( size/8 + 1 ) bytes of memory storage.
        """
        return self._size

    def add(self, indices):
        """
        Add item(s) to IndexSet.
        
        Parameters
        ----------
        indices: unsigned int, ndarray, IndexSet, iterable object.
            Index or indices to be added to the IndexSet. Ensure value(s) are integer 
            and non-negative. An iterable object may also be provided, with numpy arrays
            and IndexSets being significantly more efficient.
            
        
        Example
        -------
        Create an empty set and add items as necessary:
        >>> someSet = uw.container.IndexSet( 15 )
        >>> someSet.add(3)
        >>> someSet
        IndexSet([3])
        >>> 3 in someSet
        True
        >>> someSet.add([5,3,7,8])
        >>> someSet
        IndexSet([3, 5, 7, 8])
        >>> someSet.add(np.array([10,11,3]))
        >>> someSet
        IndexSet([ 3,  5,  7,  8, 10, 11])
        
        """
        
        self._addremove(indices,True);
        
    def remove(self, indices):
        """
        Remove item(s) from IndexSet.
        
        Parameters
        ----------
        indices: unsigned int, ndarray, iterable object
            Index or indices to be removed from the IndexSet. Ensure value(s) are integer
            and non-negative. An iterable object may also be provided, with numpy arrays
            being significantly more efficient. Note that the 'remove' method can not 
            be provided with an IndexSet object, as the 'add' object can.

        Example
        -------
        >>> someSet = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet
        IndexSet([ 3,  9, 10])
        >>> someSet.remove(3)
        >>> 3 in someSet
        False
        >>> someSet
        IndexSet([ 9, 10])
        >>> someSet.remove([9,10])
        >>> someSet
        IndexSet([])

        """
        self._addremove(indices,False);
    
        
    def _addremove(self, indices, isadding):
        # note we use numbers.Integral here which also catches numpy int types
        if isinstance(indices, numbers.Integral):
            indices = int(indices)
            if indices >= self.size or indices < 0:
                raise ValueError("Provided index must be non-negative and "+
                                  "less than IndexSet maximum ({}).".format(self.size-1))
            if isadding:
                libUnderworld.StGermain.IndexSet_Add(self._cself,indices)
            else:
                libUnderworld.StGermain.IndexSet_Remove(self._cself,indices)
            
        elif isinstance(indices, np.ndarray):  # if numpy, add the quick way
            self._AddOrRemoveWithNumpyArray(indices, isadding)
        elif isinstance(indices, IndexSet):
            self._checkCompatWith(indices)
            if isadding:
                libUnderworld.StGermain.IndexSet_Merge_OR(self._cself,indices._cself)
            else:
                # ok, there's no explict stg IndexSet remove routine.. lets convert to numpy and recurse
                self.remove( indices.data )
        else:    # other iterable, first convert to numpy, then add
            try:
                iterator = iter(indices)
            except TypeError:
                raise TypeError("The object passed in does not appear to be iterable or other compatible type.")
            except:
                raise RuntimeError("An unknown error occurred relating to the object passed in.")

            self._AddOrRemoveWithNumpyArray( np.fromiter(indices, np.int), isadding )

    def AND(self, indices):
        """
        Logical AND operation performed with provided IndexSet.
        
        Parameters
        ----------
        indices: IndexSet
            IndexSet for which AND operation is performed. Note that provided set must be of type IndexSet.
            
        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1.AND(someSet2)
        >>> someSet1
        IndexSet([9])

        """
        self._checkCompatWith(indices)
        libUnderworld.StGermain.IndexSet_Merge_AND(self._cself,indices._cself)
    
    
    def __contains__(self, index):
        """
        Check if item is in IndexSet.
        
        Parameters
        ----------
        index: unsigned int
            Check if index is in IndexSet.
            
        Returns
        -------
        inSet: bool
            True if item is in set, False otherwise.

        Example
        -------
        >>> someSet = uw.container.IndexSet( 15, [3,9,10] )
        >>> 3 in someSet
        True

        """
        return libUnderworld.StGermain.IndexSet_IsMember(self._cself,index)

    def _AddOrRemoveWithNumpyArray(self,ndarray,adding):
        """
        Add values from a numpy array to a set
        
        Parameters
        ----------
        ndarray: numpyp.ndarray (uint32, int32, uint64, int64)
            Array from which to add values. Must contain only non-negative integers.
        adding: bool
            If True, we are adding to the set. Else we are removing.
        """
        if not isinstance(ndarray,np.ndarray):
            raise TypeError("Object must be of 'ndarray' type")
        
        if len(ndarray) == 0:  # nothing to do, so return
            return

        if ndarray.max() > self.size-1:
            raise ValueError("Provided object contains a value ({}) greater than ".format(ndarray.max())+"the IndexSet maximum ({}).".format(self.size-1))
        if ndarray.min() < 0:
            raise ValueError("Provided object contains a value ({}) less than zero.".format(ndarray.min()))

        # lets simply try using the lowest type and working our way up
        try:
            self._cself.AddOrRemoveWithNumpyUInt(ndarray,adding)
        except TypeError:
            try:
                self._cself.AddOrRemoveWithNumpyInt(ndarray,adding)
            except TypeError:
                try:
                    self._cself.AddOrRemoveWithNumpyULong(ndarray,adding)
                except TypeError:
                    try:
                        self._cself.AddOrRemoveWithNumpyLong(ndarray,adding)
                    except TypeError:
                        raise TypeError("Incompatible array type ({}) provided. Must be of integer type.".format(ndarray.dtype))

    @property
    def data(self):
        """
        Returns the set members as a numpy array.
        
        Note that only a numpy copy of the set is returned, and modifying this
        array is disabled (and would have no effect).
        
        Returns
        -------
        numpy.ndarray (uint32)
            Array containing IndexSet members.
            
        Example
        -------
        >>> someSet = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet.data
        array([ 3,  9, 10], dtype=uint32)

        """
        # get member count
        memberCount = libUnderworld.StGermain.IndexSet_UpdateMembersCount(self._cself)
        # create un-initialised array of required size
        arr = np.ndarray(memberCount, dtype='uint32')
        # pass into routine to set values
        self._cself.GetAsNumpyArray(arr)
        arr.flags.writeable = False
        return arr

    @property
    def count(self):
        """
        Returns
        -------
        int: member count
            Returns the total number of members this set contains.
        
        Example
        -------
        >>> someSet = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet.count
        3

        """
        # get member count
        return libUnderworld.StGermain.IndexSet_UpdateMembersCount(self._cself)
    
    def invert(self):
        """
        Inverts the index set in place.
        
        Example
        -------
        >>> someSet = uw.container.IndexSet( 15, [1,3,5,7,9,11,13] )
        >>> someSet.invert()
        >>> someSet
        IndexSet([ 0,  2,  4,  6,  8, 10, 12, 14])

        """
        libUnderworld.StGermain.IndexSet_Invert(self._cself)
    
    def addAll(self):
        """
        Set all indices of set to added.

        Example
        -------
        >>> someSet = uw.container.IndexSet( 5 )
        >>> someSet
        IndexSet([])
        >>> someSet.addAll()
        >>> someSet
        IndexSet([0, 1, 2, 3, 4])
        """
        libUnderworld.StGermain.IndexSet_AddAll(self._cself)

    def clear(self):
        """
        Clear set.  ie, set all indices to not included.

        Example
        -------
        >>> someSet = uw.container.IndexSet( 5, [1,2,3] )
        >>> someSet
        IndexSet([1, 2, 3])
        >>> someSet.clear()
        >>> someSet
        IndexSet([])

        """
        libUnderworld.StGermain.IndexSet_RemoveAll(self._cself)

    def __repr__(self):
        return repr(self.data).replace("array","IndexSet").replace(", dtype=uint32","")

    def __iter__(self):
        return iter(self.data)

    def __add__(self,other):
        """
        Operator overloading for :python:`C = A + B`
        
        Creates a new set C, then adds indices from A and B.
        
        Returns
        -------
        indexSet: IndexSet
            The new set (C).

        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 + someSet2
        IndexSet([ 1,  3,  9, 10, 12])

        
        """
        if not isinstance( other, IndexSet ):
            raise TypeError("Indices provided must be of type 'IndexSet'.")

        set = copy.deepcopy(self)
        set.add(other)
        return set

    def __iadd__(self,other):
        """
        Operator overloading for :python:`A += B`
        
        Adds indices from A and B.

        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 += someSet2
        >>> someSet1
        IndexSet([ 1,  3,  9, 10, 12])
        
        """
        self.add(other)
        return self

    def __sub__(self,other):
        """
        Operator overloading for :python:`C = A - B`
        
        Creates a new set C, then adds indices from A, and removes those
        from B.

        
        Returns
        -------
        indexSet: IndexSet
            The new set (C).

        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 - someSet2
        IndexSet([ 3, 10])

        """
        if not isinstance( other, IndexSet ):
            raise TypeError("Indices provided must be of type 'IndexSet'.")

        set = copy.deepcopy(self)
        set.remove(other)
        return set

    def __isub__(self,other):
        """
        Operator overloading for :python:`A -= B`
        
        Removes from A indices in B.
        
        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 -= someSet2
        >>> someSet1
        IndexSet([ 3, 10])

        """
        self.remove(other)
        return self


    def __and__(self,other):
        """
        Operator overloading for :python:`C = A & B`
        
        Creates a new set C, then adds indices from A, and performs
        AND logic with B.

        
        Returns
        -------
        indexSet: IndexSet
            The new set (C).

        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 & someSet2
        IndexSet([9])
        """
        if not isinstance( other, IndexSet ):
            raise TypeError("Indices provided must be of type 'IndexSet'.")

        set = copy.deepcopy(self)
        set.AND(other)
        return set

    def __iand__(self,other):
        """
        Operator overloading for :python:`A &= B`
        
        Performs logical AND operation with A and B. Results are stored
        in A.
        
        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 &= someSet2
        >>> someSet1
        IndexSet([9])

        """
        self.AND(other)
        return self

    def __or__(self,other):
        """
        Operator overloading for :python:`C = A | B`
        
        Creates a new set C, then adds indices from A, and performs OR 
        logic with B.

        
        Returns
        -------
        indexSet: IndexSet
            The new set (C).

        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 | someSet2
        IndexSet([ 1,  3,  9, 10, 12])
        """
        if not isinstance( other, IndexSet ):
            raise TypeError("Indices provided must be of type 'IndexSet'.")

        set = copy.deepcopy(self)
        set.add(other)
        return set

    def __ior__(self,other):
        """
        Operator overloading for :python:`A |= B`
        
        Performs logical OR operation with A and B. Results are stored in A.
        
        Example
        -------
        >>> someSet1 = uw.container.IndexSet( 15, [3,9,10] )
        >>> someSet2 = uw.container.IndexSet( 15, [1,9,12] )
        >>> someSet1 |= someSet2
        >>> someSet1
        IndexSet([ 1,  3,  9, 10, 12])

        """
        self.add(other)
        return self

    def __deepcopy__(self, memo):
        """ 
        Custom deepcopy routine required because python won't know how to copy
        memory owned by stgermain. 
        """
        newguy = copy.copy(self)  # this should copy all python attributes
        newguy._cself = libUnderworld.StGermain.IndexSet_Duplicate( newguy._cself )  # creates new stg guy, and overwrites py _cself attribute
        return newguy

    def _checkCompatWith(self,other):
        """ 
        Checks that these IndexSets are compatible. This should be overwritten by child classes.
        """
        if type(self) != type(other):
            raise TypeError("This operation is illegal. These objects do not appear to be of identical type.")


class ObjectifiedIndexSet(IndexSet):
    """
    This class simply adds an object to IndexSet data. Usually this object will be
    the object for which the IndexSet data relates to.. For example, we can attach a 
    Mesh object to an IndexSet containing mesh vertices.
    """
    def __init__(self, object=None, *args, **kwargs):
        """
        Class initialiser
        
        Parameters
        ----------
        object : any, default=None
            Object to tether to data

        See parent classes for further parameters.
        
        Returns
        -------
        objectifiedIndexSet : ObjectifiedIndexSet
        
        """
        self._object = object

        super(ObjectifiedIndexSet,self).__init__(*args,**kwargs)

    @property
    def object(self):
        """
        Object for which IndexSet data relates.
        """
        return self._object

    def __repr__(self):
        return repr(self.data).replace("array","ObjectifiedIndexSet").replace(", dtype=uint32","")

    def _checkCompatWith(self,other):
        """ 
        Checks that these IndexSets are compatible.
        """
        # check parent first
        super(ObjectifiedIndexSet,self)._checkCompatWith(other)

        if type(self.object) != type(other.object):
            raise TypeError("This operation is illegal. The associated objects for these IndexSets do not appear to be of identical type.")

        
