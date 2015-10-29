##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
#
#

"""
This module contains FeMesh classes, and associated implementation.
"""

import underworld as uw
import underworld._stgermain as _stgermain
import weakref
import libUnderworld
import _specialSets_Cartesian
import underworld.function as function
import contextlib
import time
import abc

class FeMesh(_stgermain.StgCompoundComponent, function.FunctionInput):
    """
    The FeMesh class provides the geometry and topology of a finite
    element discretised domain. The FeMesh is implicitly parallel. Some aspects
    may be local or global, but this is generally handled automatically.

    A number of element types are supported.
    """
    _objectsDict = { "_femesh": "FeMesh" }
    _selfObjectName = "_femesh"
    
    _supportedElementTypes = ["Q2","Q1","DQ1","DPC1","DQ0"]

    def __init__(self, elementType, generator=None, **kwargs):
        """
        Class initialiser.
        
        Parameter
        ---------
        elementType : str
            Element type for FeMesh.  See FeMesh.elementType docstring for further info.
        generator : MeshGenerator, default=None
            Generator object which builds the FeMesh.  See FeMesh.generator docstring for
            further info.
        
        Returns
        -------
        feMesh : FeMesh
            Constructed FeMesh object.

        """
        
        if not isinstance(elementType,str):
            raise TypeError("'elementType' object passed in must be of type 'str'")
        if elementType.upper() not in self._supportedElementTypes:
            raise ValueError("'elementType' provided ({}) does not appear to be supported.\n \
                               Supported types are {}.".format(elementType.upper(),self._supportedElementTypes))
        self._elementType = elementType.upper()

        if generator == None:
            if isinstance(self,MeshGenerator):
                generator = self
            else:
                raise ValueError("No generator provided for mesh.\n \
                                  You must provide a generator, or the mesh itself \
                                  must be of the MeshGenerator class.")
        self.generator = generator
        
        self._dataWriteable = False

        # build parent
        super(FeMesh,self).__init__(**kwargs)

    def _setup(self):
        # add the empty set
        self.specialSets["Empty"]  = lambda selfobject: uw.mesh.FeMesh_IndexSet( object           = selfobject,
                                                                                 topologicalIndex = 0,
                                                                                 size             = libUnderworld.StgDomain.Mesh_GetDomainSize( selfobject._femesh, libUnderworld.StgDomain.MT_VERTEX ))
    @property
    def elementType(self):
        """ 
        elementType (str): Element type for FeMesh.
        Supported types are "Q2", "Q1", "dPc1" and "dQ0".
        """
        return self._elementType
    @property
    def generator(self):
        """ 
        generator (MeshGenertor): Object which builds the mesh. Note that the mesh
        itself may be a generator, in which case this property will return the mesh
        object iself.
        """
        if isinstance(self._generator, weakref.ref):
            return self._generator()
        else:
            return self._generator
    @generator.setter
    def generator(self,generator):
        """
        Associates the passed in generator with this mesh object

        Parameters:
        -----------
        generator : MeshGenerator
            generator to use
        """
        if not isinstance(generator,MeshGenerator):
            raise TypeError("'generator' object passed in must be of type 'MeshGenerator'")
        if self is generator:
            self._generator = weakref.ref(generator)  # only keep weekref here (where appropriate) to prevent circular dependency
        else:
            self._generator = generator
        libUnderworld.StgDomain.Mesh_SetGenerator(self._cself, generator._gen)

    @property
    def data(self):
        """ 
        data (np.array):  Numpy proxy array proxy to underlying object 
        vertex data. Note that the returned array is a proxy for all the *local*
        vertices, and it is provided as 1d list.
        
        As these arrays are simply proxys to the underlying memory structures, 
        no data copying is required.
        
        Note that this property returns a read-only numpy array as default. If 
        you wish to modify mesh vertex locations, you are required to use the 
        deform_mesh context manager.
        
        If you are modifying the mesh, remember to modify any submesh associated with
        it accordingly.

        >>> import underworld as uw
        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(-1.,-1.), maxCoord=(1.,1.) )
        >>> someMesh.data.shape
        (9, 2)
        
        You can retrieve individual vertex locations
        
        >>> someMesh.data[1]
        array([ 0., -1.])
        
        You can modify these locations directly, but take care not to tangle the mesh!
        Mesh modifications must occur within the deform_mesh context manager.

        >>> with someMesh.deform_mesh():
        ...    someMesh.data[1] = [0.1,-1.1]
        >>> someMesh.data[1]
        array([ 0.1, -1.1])

        """
        arr = self._cself.getAsNumpyArray()
        arr.flags.writeable = self._dataWriteable
        return arr

    @contextlib.contextmanager
    def deform_mesh(self):
        """
        Any mesh deformation should occur within this context manager. Note that
        certainly algorithms may be switched to their irregular mesh equivalents
        (if not already set this way).
        
        Any submesh will also be appropriately updated on return from the context
        manager.

        >>> import underworld as uw
        >>> someMesh = uw.mesh.FeMesh_Cartesian()
        >>> with someMesh.deform_mesh():
        ...     someMesh.data[0] = [0.1,0.1]
        >>> someMesh.data[0]
        array([ 0.1,  0.1])


        """
        # set the general mesh algorithm now
        uw.libUnderworld.StgDomain.Mesh_SetAlgorithms( self._cself, None )
        self._cself.isRegular = False
        self._dataWriteable = True
        try:
            yield
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "An exception was raised during mesh deformation. "
                                                     +"Your mesh may only be partially deformed. "
                                                     +"You can reset your mesh using the 'reset' method. "
                                                     +"Note that any submesh should not be modified directly, "
                                                     +"but will instead be updated automatically on return from "
                                                     +"the 'deform_mesh' context manager. \nEncountered exception message:\n")
        finally:
            self._dataWriteable = False
            # call deformupdate, which updates various mesh metrics
            uw.libUnderworld.StgDomain.Mesh_DeformationUpdate( self._cself )
            if hasattr(self,"subMesh") and self.subMesh:
                # remesh the submesh based on the new primary
                self.subMesh.reset()

    @property
    def nodesLocal(self):
        """
        Returns the number of local nodes on the mesh
        """
        return libUnderworld.StgDomain.Mesh_GetLocalSize(self._cself, 0)

    @property 
    def nodesGlobal(self):
        """
        Returns the number of global nodes on the mesh

        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(-1.,-1.), maxCoord=(1.,1.) )
        >>> someMesh.nodesGlobal
        9
        """
        return libUnderworld.StgDomain.Mesh_GetGlobalSize(self._cself, 0)

    @property 
    def elementsGlobal(self):
        """
        Returns the number of global elements on the mesh

        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(-1.,-1.), maxCoord=(1.,1.) )
        >>> someMesh.elementsGlobal
        4
        """
        return libUnderworld.StgDomain.Mesh_GetGlobalSize(self._cself, self.dim)

    def reset(self):
        """
        Reset the mesh.  
        
        Templated mesh (such as the DQ0 mesh) will be reset according
        to the current state of their geometryMesh.
        
        Other mesh (such as the Q1 & Q2) will be reset to their 
        post-construction state.

        """
        self.generator._reset(self)
        # if we have a submesh, reset it as well
        if hasattr(self,"subMesh") and self.subMesh:
            self.subMesh.reset()


    @property
    def specialSets(self):
        """
        This dictionary stores a set of special data sets relating to mesh objects. 
        
        >>> import underworld as uw
        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> someMesh.specialSets.keys()
        ['MaxI_VertexSet', 'MinI_VertexSet', 'AllWalls', 'AllWalls_VertexSet', 'MinJ_VertexSet', 'MaxJ_VertexSet', 'Empty']
        >>> someMesh.specialSets["MinJ_VertexSet"]
        FeMesh_IndexSet([0, 1, 2])
        
        """
        if not hasattr(self, "_specialSets"):
            class _SpecialSetsDict(dict):
                """
                This special dictionary simply calls the function with the mesh object
                before returning it.
                """
                def __init__(self, femesh):
                    self._femesh = weakref.ref(femesh)

                    # call parents method
                    super(_SpecialSetsDict,self).__init__()
                def __getitem__(self,index):
                    # get item using regular dict
                    item = super(_SpecialSetsDict,self).__getitem__(index)
                    # now call using femesh and return
                    return item(self._femesh())
            self._specialSets = _SpecialSetsDict(self)
            
        return self._specialSets

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(FeMesh,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[self._femesh.name]["elementType"] = self._elementType

    def _get_iterator(self):
        # lets create the full index set
        iset = FeMesh_IndexSet(      object           = self,
                                     topologicalIndex = 0,
                                     size             = libUnderworld.StgDomain.Mesh_GetDomainSize( self._femesh, libUnderworld.StgDomain.MT_VERTEX ) )
        iset.addAll()
        return iset._get_iterator()

    def save( self, filename ):
        """
        Saves the mesh in hdf5 format to 'filename'. Note, this is a
        global method, ie. all processes must call it.
        """
        if not isinstance(filename, str):
            raise TypeError("'filename', must be of type 'str'")
        
        # test if the filename is writable - is this safe in parallel?
        try:
            handle=open(filename, "w")
        except IOError:
            sys.exit("Could not open file {}".format(filename))
        handle.close()
            
        uw.libUnderworld.StgFEM._FeMesh_DumpMeshHDF5( self._cself, filename )




class MeshGenerator(_stgermain.StgCompoundComponent):
    """
    Abstract base class for all mesh generators.
    """
    _objectsDict = { "_gen": None }
    _selfObjectName = "_gen"

    @property
    def dim(self):
        """ dim (int): FeMesh dimensionality. """
        return self._dim

    @abc.abstractmethod
    def _reset(self,mesh):
        """
        Abstract class which handles mesh resetting.

        """
        pass

class CartesianMeshGenerator(MeshGenerator):
    """
    Abstract base class for all cartesian mesh generators.
    Generators of this class provide algorithms to build meshes which are 
    logically and geometrically Cartesian.
    """
    def __init__(self, elementRes, minCoord, maxCoord, periodic=None, **kwargs):
        """
        Class initialiser.
        
        Parameter
        ---------
        elementRes: list,tuple
            List or tuple of ints specifying mesh resolution. See CartesianMeshGenerator.elementRes
            docstring for further information.
        minCoord:  list, tuple
            List or tuple of floats specifying minimum mesh location. See CartesianMeshGenerator.minCoord
            docstring for further information.
        maxCoord: list, tuple
            List or tuple of floats specifying maximum mesh location. See CartesianMeshGenerator.minCoord
            docstring for further information.
        periodic: list, tuple
            List or tuple of bools, specifying mesh periodicity in each direction.
            
        Returns
        ------
        feMesh: CartesianMeshGenerator
                
        
        """

        if not isinstance(elementRes,(list,tuple)):
            raise TypeError("'elementRes' object passed in must be of type 'list' or 'tuple'")
        for item in elementRes:
            if not isinstance(item,(int)) or (item < 1):
                raise TypeError("'elementRes' list must only contain positive integers.")
        if not len(elementRes) in [2,3]:
            raise ValueError("For 'elementRes', you must provide a tuple of length 2 or 3 (for respectively a 2d or 3d mesh).")
        self._elementRes = elementRes
        
        if not isinstance(minCoord,(list,tuple)):
            raise TypeError("'minCoord' object passed in must be of type 'list' or 'tuple'")
        for item in minCoord:
            if not isinstance(item,(int,float)):
                raise TypeError("'minCoord' object passed in must only contain objects of type 'int' or 'float'")
        if len(minCoord) != len(elementRes):
            raise ValueError("'minCoord' tuple length ({}) must be the same as that of 'elementRes' ({}).".format(len(minCoord),len(elementRes)))
        self._minCoord = minCoord

        if not isinstance(maxCoord,(list,tuple)):
            raise TypeError("'maxCoord' object passed in must be of type 'list' or 'tuple'")
        for item in maxCoord:
            if not isinstance(item,(int,float)):
                raise TypeError("'maxCoord' object passed in must only contain objects of type 'int' or 'float'")
        if len(maxCoord) != len(elementRes):
            raise ValueError("'maxCoord' tuple length ({}) must be the same as that of 'elementRes' ({}).".format(len(maxCoord),len(elementRes)))
        self._maxCoord = maxCoord

        self._dim = len(elementRes)

        if periodic:
            if not isinstance(periodic,(list,tuple)):
                raise TypeError("'periodic' object passed in must be of type 'list' or 'tuple' in CartesianMeshGenerator")
            for item in periodic:
                if not isinstance(item,(bool)):
                    raise TypeError("'periodic' list must only contain booleans.")
            if len(periodic) != len(elementRes):
                raise ValueError("'periodic' tuple length ({}) must be the same as that of 'elementRes' ({}).".format(len(periodic),len(elementRes)))
        self._periodic = periodic
        
        for ii in range(0,self.dim):
            if minCoord[ii] >= maxCoord[ii]:
                raise ValueError("'minCoord[{}]' must be less than 'maxCoord[{}]'".format(ii,ii))

        # build parent
        super(CartesianMeshGenerator,self).__init__(**kwargs)
        
    @property
    def elementRes(self):
        """ 
        elementRes (list, tuple): Element count to generate in I, J & K
        directions. Must be provided as a tuple of integers.
        """
        return self._elementRes
    @property
    def minCoord(self):
        """ 
        minCoord (list, tuple): Minimum coordinate position for cartesian mesh.
        Values correspond to minimums in each direction (I,J,K) of the mesh.
        Note, this is the value used for initialisation, but mesh may be advecting.
        """
        return self._minCoord
    @property
    def periodic(self):
        """ 
        periodic (list,tuple): List of bools specifying mesh periodicity in each direction.
        """
        return self._periodic
    @property
    def maxCoord(self):
        """ 
        maxCoord (list, tuple): Maximum coordinate position for cartesian mesh.
        Values correspond to maximums in each direction (I,J,K) of the mesh.
        Note, this is the value used for initialisation, but mesh may be advecting.
        """
        return self._maxCoord

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(CartesianMeshGenerator,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[self._gen.name][      "size"] = self._elementRes
        componentDictionary[self._gen.name][  "minCoord"] = self._minCoord
        componentDictionary[self._gen.name][  "maxCoord"] = self._maxCoord
        componentDictionary[self._gen.name][       "dim"] = self._dim
        if self.periodic:
            componentDictionary[self._gen.name]["periodic_x"] = self._periodic[0]
            componentDictionary[self._gen.name]["periodic_y"] = self._periodic[1]
            if self._dim == 3:
                componentDictionary[self._gen.name]["periodic_z"] = self._periodic[2]

    def _reset(self,mesh):
        """
        Reset method for mesh generated using the cartesian generator. This method
        will reset the mesh to regular cartesian geometry.
        
        >>> import underworld as uw
        >>> import numpy as np
        >>> mesh = uw.mesh.FeMesh_Cartesian(elementType='Q1')

        Grab copy of original state:
        >>> vertexCopy = mesh.data.copy()

        Deform mesh:
        >>> with mesh.deform_mesh():
        ...     mesh.data[:] += 0.1*np.sin(mesh.data[:])

        Confirm mesh is different:
        >>> np.allclose(mesh.data,vertexCopy)
        False

        Now reset mesh and test again:
        >>> mesh.reset()
        >>> np.allclose(mesh.data,vertexCopy)
        True
        
        Lets do the same for the Q2 mesh:
        >>> mesh = uw.mesh.FeMesh_Cartesian(elementType='Q2')
        >>> vertexCopy = mesh.data.copy()
        >>> with mesh.deform_mesh():
        ...     mesh.data[:] += 0.1*np.sin(mesh.data[:])
        >>> np.allclose(mesh.data,vertexCopy)
        False
        >>> mesh.reset()
        >>> np.allclose(mesh.data,vertexCopy)
        True

        """
        uw.libUnderworld.StgDomain.CartesianGenerator_GenGeom( self._gen, mesh._cself, None)
        mesh._cself.isRegular = True
        # set algos back to regular
        uw.libUnderworld.StgDomain.Mesh_SetAlgorithms( mesh._cself,
                                                       uw.libUnderworld.StgDomain.Mesh_RegularAlgorithms_New("",None) )
        uw.libUnderworld.StgDomain.Mesh_DeformationUpdate( mesh._cself )


class QuadraticCartesianGenerator(CartesianMeshGenerator):
    """
    This class provides the algorithms to generate a 'Q2' (ie quadratic) mesh.
    """
    _objectsDict = { "_gen": "C2Generator" }

class LinearCartesianGenerator(CartesianMeshGenerator):
    """
    This class provides the algorithms to generate a 'Q1' (ie linear) mesh.
    """
    _objectsDict = { "_gen": "CartesianGenerator" }


class TemplatedMeshGenerator(MeshGenerator):
    """
    Abstract Class. Children of this class provide algorithms to generate a 
    mesh by stenciling nodes on the cells of the provided geometry mesh.
    """
    def __init__(self, geometryMesh, **kwargs):
        """
        Parameter:
        ----------
            geometryMesh (FeMesh)
        """
        if not isinstance(geometryMesh,(FeMesh)):
            raise TypeError("'geometryMesh' object passed in must be of type 'FeMesh'")
        # note we keep a weakref to avoid circular dependencies
        self._geometryMeshWeakref = weakref.ref(geometryMesh)
        
        self._dim = self.geometryMesh.dim
        
        super(TemplatedMeshGenerator,self).__init__(**kwargs)

    @property
    def geometryMesh(self):
        """    
        geometryMesh (FeMesh): This is the FeMesh from which the 
        TemplatedMeshGenerator obtains the cells to template nodes upon.
        Note that this class only retains a weakref to the geometryMesh, and 
        therefore this property may return None.
        """
        return self._geometryMeshWeakref()

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(TemplatedMeshGenerator,self)._add_to_stg_dict(componentDictionary)
        componentDictionary[self._gen.name]["elementMesh"] = self.geometryMesh._cself.name

class LinearInnerGenerator(TemplatedMeshGenerator):
    """
    This class provides the algorithms to generate a 'dPc1' mesh.
    """
    _objectsDict = { "_gen": "Inner2DGenerator" }

    def _reset(self,mesh):
        """
        Reset method for mesh generated using the dPc1 generator. This method
        will reset the mesh according to its geometryMesh's current state.
        
        >>> import underworld as uw
        >>> import numpy as np
        >>> mesh = uw.mesh.FeMesh_Cartesian(elementType='Q2/dPc1')

        Grab copy of original state:
        >>> vertexCopy = mesh.subMesh.data.copy()

        Deform mesh.  Remember, we always modify the parent mesh:
        >>> with mesh.deform_mesh():
        ...     mesh.data[:] += 0.1*np.sin(mesh.data[:])

        Confirm subMesh has been deformed:
        >>> np.allclose(mesh.subMesh.data,vertexCopy)
        False

        Now reset mesh and test again:
        >>> mesh.reset()
        >>> np.allclose(mesh.subMesh.data,vertexCopy)
        True
        """

        uw.libUnderworld.StgFEM.Inner2DGenerator_BuildGeometry( self._gen, mesh._cself)

class dQ1Generator(TemplatedMeshGenerator):
    """
    This class provides the algorithms to generate a 'dQ1' mesh.
    """
    _objectsDict = { "_gen": "dQ1Generator" }

    def _reset(self,mesh):
        """
        Reset method for mesh generated using the dQ1 generator. This method
        will reset the mesh according to its geometryMesh's current state.
        
        >>> import underworld as uw
        >>> import numpy as np
        >>> mesh = uw.mesh.FeMesh_Cartesian(elementType='Q2/dQ1')

        Grab copy of original state:
        >>> vertexCopy = mesh.subMesh.data.copy()

        Deform mesh.  Remember, we always modify the parent mesh:
        >>> with mesh.deform_mesh():
        ...     mesh.data[:] += 0.1*np.sin(mesh.data[:])

        Confirm subMesh has been deformed:
        >>> np.allclose(mesh.subMesh.data,vertexCopy)
        False

        Now reset mesh and test again:
        >>> mesh.reset()
        >>> np.allclose(mesh.subMesh.data,vertexCopy)
        True
        """
        uw.libUnderworld.StgFEM.dQ1Generator_BuildGeometry( self._gen, mesh._cself)

class ConstantGenerator(TemplatedMeshGenerator):
    """
    This class provides the algorithms to generate a 'dQ0' mesh.
    """
    _objectsDict = { "_gen": "C0Generator" }

    def _reset(self,mesh):
        """
        Reset method for mesh generated using the dQ1 generator. This method
        will reset the mesh according to its geometryMesh's current state.
        
        >>> import underworld as uw
        >>> import numpy as np
        >>> mesh = uw.mesh.FeMesh_Cartesian(elementType='Q1/dQ0')

        Grab copy of original state:
        >>> vertexCopy = mesh.subMesh.data.copy()

        Deform mesh.  Remember, we always modify the parent mesh:
        >>> with mesh.deform_mesh():
        ...     mesh.data[:] += 0.1*np.sin(mesh.data[:])

        Confirm subMesh has been deformed:
        >>> np.allclose(mesh.subMesh.data,vertexCopy)
        False

        Now reset mesh and test again:
        >>> mesh.reset()
        >>> np.allclose(mesh.subMesh.data,vertexCopy)
        True
        """
        uw.libUnderworld.StgFEM.C0Generator_BuildGeometry( self._gen, mesh._cself)


class FeMesh_Cartesian(FeMesh, CartesianMeshGenerator):
    """
    This class generates a finite element mesh which is topologically cartesian
    and geometrically regular. It is possible to directly build a dual mesh by 
    passing a pair of element types to the constructor.
    
    For example, to create a linear mesh:
        
    >>> import underworld as uw
    >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> someMesh.dim
    2
    >>> someMesh.elementRes
    (16, 16)
    
    Alternatively, you can create a linear/constant dual mesh
    
    >>> someDualMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> someDualMesh.elementType
    'Q1'
    >>> subMesh = someDualMesh.subMesh
    >>> subMesh.elementType
    'DQ0'

    To set / read vertex coords, use the numpy interface via the 'data' property.

    """
    _meshGenerator = [ "C2Generator", "CartesianGenerator" ]
    _objectsDict = { "_gen": None }  # this is set programmatically in __new__
    
    def __new__(cls, elementType="Q1/dQ0", elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.), periodic=None, **kwargs):
        # This class requires a custom __new__ so that we can decide which
        # type of generator is required dynamically

        if not isinstance(elementType,(str,tuple,list)):
            raise TypeError("'elementType' object passed in must be of type 'str', 'list' or 'tuple'")

        if isinstance(elementType, str):
            # convert to tuple to make things easier
            import re
            elementType = re.split(",|-|/",elementType)

        if len(elementType) > 2:
            raise ValueError("A maximum of two mesh types are currently supported.")
        for elType in elementType:
            if not isinstance(elType,str):
                raise TypeError("Items in provided 'elementType' object must be of type 'str'")
            if elType.upper() not in cls._supportedElementTypes:
                raise ValueError("'elementType' provided ({}) does not appear to be supported. \n \
                                  Supported types are {}.".format(elType.upper(),cls._supportedElementTypes))

        # Only supporting 2 'main' element types here
        if elementType[0].upper() not in cls._supportedElementTypes[0:2]:
            raise ValueError("Primary elementType must be of type '{}' or '{}'.".format(cls._supportedElementTypes[0],cls._supportedElementTypes[1]))
        if len(elementType) == 2:
            # all element types after first one can be in a sub-mesh.
            if elementType[1].upper() not in cls._supportedElementTypes[1:]:
                st = ' '.join('{}'.format(t) for t in cls._supportedElementTypes[1:])
                raise ValueError("Secondary elementType must be one of type '{}'.".format(st) )

        overruleDict = {}
        # Only supporting 2 'main' elements types here
        if elementType[0] == cls._supportedElementTypes[0]:
            overruleDict["_gen"] = cls._meshGenerator[0]
        if elementType[0] == cls._supportedElementTypes[1]:
            overruleDict["_gen"] = cls._meshGenerator[1]

        return super(FeMesh_Cartesian,cls).__new__(cls, objectsDictOverrule=overruleDict, **kwargs)
    
    
    def __init__(self, elementType="Q1/dQ0", elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.), periodic=None, **kwargs):
        """
        Class initialiser.
        
        Parameters
        ----------
            elementType: str
                Mesh element type. Note that this class allows the user to 
                (optionally) provide a pair of elementTypes for which a dual 
                mesh will be created.
                The submesh is accessible through the 'subMesh' property. The
                primary mesh itself is the object returned by this constructor.
                
            Refer to parent classes for further information on parameters.
            
        Returns
        ------
        feMesh: FeMesh_Cartesian
        
        """

        if isinstance(elementType, str):
            # convert to tuple to make things easier
            import re
            elementType = re.split(",|-|/",elementType)
        
        self._elementTypes = elementType
        # ok, lets go ahead and build primary mesh (ie, self)
        super(FeMesh_Cartesian,self).__init__(elementType=elementType[0], elementRes=elementRes, minCoord=minCoord, maxCoord=maxCoord, periodic=periodic, **kwargs)

        # lets add the special sets
        self.specialSets["MaxI_VertexSet"] = _specialSets_Cartesian.MaxI_VertexSet
        self.specialSets["MinI_VertexSet"] = _specialSets_Cartesian.MinI_VertexSet
        self.specialSets["MaxJ_VertexSet"] = _specialSets_Cartesian.MaxJ_VertexSet
        self.specialSets["MinJ_VertexSet"] = _specialSets_Cartesian.MinJ_VertexSet
        if(self.dim==3):
            self.specialSets["MaxK_VertexSet"] = _specialSets_Cartesian.MaxK_VertexSet
            self.specialSets["MinK_VertexSet"] = _specialSets_Cartesian.MinK_VertexSet
        self.specialSets["AllWalls_VertexSet"] = _specialSets_Cartesian.AllWalls
        def exceptionfunc(thing):
            raise RuntimeError("This set has been renamed to AllWalls_VertexSet.")
        self.specialSets["AllWalls"          ] = exceptionfunc

    def _setup(self):
        # build the sub-mesh now
        self._secondaryMesh = None
        if len(self._elementTypes) == 2:
            if  self._elementTypes[1].upper() == self._supportedElementTypes[1]:
                genSecondary = LinearCartesianGenerator(elementRes=elementRes, minCoord=minCoord, maxCoord=maxCoord, periodic=periodic, **kwargs)
            elif self._elementTypes[1].upper() == self._supportedElementTypes[2]:
                genSecondary = dQ1Generator( geometryMesh=self )
            elif self._elementTypes[1].upper() == self._supportedElementTypes[3]:
                genSecondary = LinearInnerGenerator( geometryMesh=self )
            elif self._elementTypes[1].upper() == self._supportedElementTypes[4]:
                genSecondary = ConstantGenerator( geometryMesh=self )
            else:
                st = ' '.join('{}'.format(t) for t in self._supportedElementTypes[1:])
                raise ValueError("The secondary mesh must be of type '{}': '{}' was passed in. Tested against '{}'".format(st,self._elementTypes[1].upper(), self._supportedElementTypes[1]) )

            self._secondaryMesh = FeMesh( elementType=self._elementTypes[1].upper(), generator=genSecondary )
    
    @property
    def subMesh(self):
        """
        Returns the submesh where the object is a dual mesh, or None otherwise.
        """
        return self._secondaryMesh
        

class FeMesh_IndexSet(uw.container.ObjectifiedIndexSet, function.FunctionInput):
    """
    This class ties the FeMesh instance to an index set, and stores other metadata relevant to the set.
    """
    def __init__(self, object, topologicalIndex=None, *args, **kwargs):
        """
        Class initialiser.
        
        Parameter
        ---------
            topologicalIndex: int
                Mesh topological index for which the IndexSet relates. See FeMesh_IndexSet.topologicalIndex
                docstring for further info.
            object: FeMesh
                The FeMesh instance from which the IndexSet was extracted.
                
            See parent classes for further required/optional parameters.
            
        Returns
        ------
        feMeshIndices: FeMesh_IndexSet

        >>> amesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> iset = uw.libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMaxISet( amesh._femesh )
        >>> uw.mesh.FeMesh_IndexSet( amesh, topologicalIndex=0, size=amesh.nodesGlobal, fromObject=iset )
        FeMesh_IndexSet([ 4,  9, 14, 19, 24])
                
        """
        if topologicalIndex not in (0,1,2,3):
            raise ValueError("Topological index must be within [0,3].")
        if not isinstance(object, FeMesh):
            raise TypeError("'object' parameter must be of (sub)type 'FeMesh'.")
        self._toplogicalIndex = topologicalIndex

        super(FeMesh_IndexSet,self).__init__(object,*args,**kwargs)
        
    @property
    def topologicalIndex(self):
        """
        The topological index for the indices. The mapping is:
            0 - vertex
            1 - edge
            2 - face
            3 - volume
        """
        return self._toplogicalIndex


    def __repr__(self):
        return repr(self.data).replace("array","FeMesh_IndexSet").replace(", dtype=uint32","")

    def _checkCompatWith(self,other):
        """ 
        Checks that these have identical topo
        """
        # check parent first
        super(FeMesh_IndexSet,self)._checkCompatWith(other)

        if self.topologicalIndex != other.topologicalIndex:
            raise TypeError("This operation is illegal. The topologicalIndex for these sets do not correspond.")

        if self.object._cself != other.object._cself:
            raise TypeError("This operation is illegal. The meshes associated with these IndexSets appear to be different.")
    def __call__(self, *args, **kwards):
        raise RuntimeError("Note that if you accessed this IndexSet via a specialSet dictionary,\n"+
                           "the interface has changed, and you should no longer call the object.\n"+
                           "This is now handled internally. Simpy use the objects directly.\n"+
                           "Ie, remove the '()'.")
    def _get_iterator(self):
        return libUnderworld.Function.MeshIndexSet(self._cself, self.object._cself)

