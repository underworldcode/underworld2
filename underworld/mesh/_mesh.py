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
import h5py
from mpi4py import MPI
import numpy as np


class FeMesh(_stgermain.StgCompoundComponent, function.FunctionInput):
    """
    The FeMesh class provides the geometry and topology of a finite
    element discretised domain. The FeMesh is implicitly parallel. Some aspects
    may be local or global, but this is generally handled automatically.

    A number of element types are supported.

    Parameters
    ----------
    elementType : str
        Element type for FeMesh.  See FeMesh.elementType docstring for further info.
    generator : underworld.mesh.MeshGenerator
        Generator object which builds the FeMesh. See FeMesh.generator docstring for
        further info.

    """
    _objectsDict = { "_mesh": "FeMesh" }
    _selfObjectName = "_mesh"

    _supportedElementTypes = ["Q2","Q1","DQ1","DPC1","DQ0"]

    def __init__(self, elementType, generator=None, **kwargs):
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

        # these lists should be populated with closure functions
        # which are executed before and/or after mesh deformations
        self._pre_deform_functions = []
        self._post_deform_functions = []

        self._arr = None

        # build parent
        super(FeMesh,self).__init__(**kwargs)

    def _setup(self):
        # add the empty set
        self.specialSets["Empty"]  = lambda selfobject: uw.mesh.FeMesh_IndexSet( object           = selfobject,
                                                                                 topologicalIndex = 0,
                                                                                 size             = libUnderworld.StgDomain.Mesh_GetDomainSize( selfobject._mesh, libUnderworld.StgDomain.MT_VERTEX ))

    @property
    def elementType(self):
        """
        Returns
        -------
        str
            Element type for FeMesh. Supported types are "Q2", "Q1", "dQ1", "dPc1" and "dQ0".
        """
        return self._elementType
    @property
    def generator(self):
        """
        Getter/Setter for the mesh MeshGenerator object.

        Returns
        -------
        underworld.mesh.MeshGenerator
            Object which builds the mesh. Note that the mesh itself may be a
            generator, in which case this property will return the mesh object iself.
        """
        if isinstance(self._generator, weakref.ref):
            return self._generator()
        else:
            return self._generator
    @generator.setter
    def generator(self,generator):
        if not isinstance(generator,MeshGenerator):
            raise TypeError("'generator' object passed in must be of type 'MeshGenerator'")
        if self is generator:
            self._generator = weakref.ref(generator)  # only keep weekref here (where appropriate) to prevent circular dependency
        else:
            self._generator = generator
        libUnderworld.StgDomain.Mesh_SetGenerator(self._cself, generator._gen)

    @property
    def data_elementNodes(self):
        """
        Returns
        -------
        numpy.ndarray
            Array specifying the nodes (global node id) for a given element (local element id).
            NOTE: Length is local size.
        """
        uw.libUnderworld.StgDomain.Mesh_GenerateENMapVar(self._cself)
        arr = uw.libUnderworld.StGermain.Variable_getAsNumpyArray(self._cself.enMapVar)
        if( len(arr) % self.elementsLocal != 0 ):
            raise RuntimeError("Unsupported element to node mapping for save routine"+
                    "\nThere doesn't appear to be elements with a consistent number of nodes")

        # we ASSUME a constant number of nodes for each element
        # and we reshape the arr accordingly
        nodesPerElement = len(arr)/self.elementsLocal
        return arr.reshape(self.elementsLocal, nodesPerElement)

    @property
    def data_elgId(self):
        """
        Returns
        -------
        numpy.ndarray
            Array specifying global element ids. Length is domain size, (local+shadow).
        """
        uw.libUnderworld.StgDomain.Mesh_GenerateElGlobalIdVar(self._cself)
        arr = uw.libUnderworld.StGermain.Variable_getAsNumpyArray(self._cself.eGlobalIdsVar)
        return arr

    @property
    def data_nodegId(self):
        """
        Returns
        -------
        numpy.ndarray
            Array specifying global node ids. Length is domain size, (local+shadow).
        """
        uw.libUnderworld.StgDomain.Mesh_GenerateNodeGlobalIdVar(self._cself)
        arr = uw.libUnderworld.StGermain.Variable_getAsNumpyArray(self._cself.vGlobalIdsVar)
        return arr

    @property
    def data(self):
        """
        Numpy proxy array proxy to underlying object vertex data. Note that the
        returned array is a proxy for all the *local* vertices, and it is
        provided as 1d list.

        As these arrays are simply proxys to the underlying memory structures,
        no data copying is required.

        Note that this property returns a read-only numpy array as default. If
        you wish to modify mesh vertex locations, you are required to use the
        deform_mesh context manager.

        If you are modifying the mesh, remember to modify any submesh associated
        with it accordingly.

        Returns
        -------
        numpy.ndarray
            The data proxy array.

        Example
        -------

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
        if self._arr is None:
            self._arr = uw.libUnderworld.StGermain.Variable_getAsNumpyArray(self._cself.verticesVariable)
            self._arr.flags.writeable = False
        return self._arr

    @contextlib.contextmanager
    def deform_mesh(self, isRegular=False, remainsRegular=None):
        """
        Any mesh deformation must occur within this python context manager. Note
        that certain algorithms may be switched to their irregular mesh equivalents
        (if not already set this way). This may have performance implications.

        Any submesh will also be appropriately updated on return from the context
        manager, as will various mesh metrics.

        Parameters
        ----------
        isRegular : bool
            The general assumption is that the deformed mesh will no longer be regular
            (orthonormal), and more general but less efficient algorithms will be
            selected via this context manager. To over-ride this behaviour, set
            this parameter to True.


        Example
        -------
        >>> import underworld as uw
        >>> someMesh = uw.mesh.FeMesh_Cartesian()
        >>> with someMesh.deform_mesh():
        ...     someMesh.data[0] = [0.1,0.1]
        >>> someMesh.data[0]
        array([ 0.1,  0.1])
        """

        if not remainsRegular is None:
            raise RuntimeError("'remainsRegular' parameter has been renamed to 'isRegular'")

        # execute any pre deform functions
        for function in self._pre_deform_functions:
            function()

        self.data.flags.writeable = True
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
            self.data.flags.writeable = False
            if isRegular:
                self._cself.isRegular = True
                uw.libUnderworld.StgDomain.Mesh_SetAlgorithms( self._cself,
                                                               uw.libUnderworld.StgDomain.Mesh_RegularAlgorithms_New("",None) )
            else:
                uw.libUnderworld.StgDomain.Mesh_SetAlgorithms( self._cself, None )
                self._cself.isRegular = False
            uw.libUnderworld.StgDomain.Mesh_Sync( self._cself )
            uw.libUnderworld.StgDomain.Mesh_DeformationUpdate( self._cself )
            if hasattr(self,"subMesh") and self.subMesh:
                self.subMesh.reset()

            # execute any post deform functions
            for function in self._post_deform_functions:
                function()

    def add_variable(self, nodeDofCount, dataType='double', **kwargs):
        """
        Creates and returns a mesh variable using the discretisation of the given mesh.

        To set / read nodal values, use the numpy interface via the 'data' property.

        Parameters
        ----------
        dataType : string
            The data type for the variable.
            Note that only 'double' type variables are currently
            supported.
        nodeDofCount : int
            Number of degrees of freedom per node the variable will have

        Returns
        -------
        underworld.mesh.MeshVariable
            The newly created mesh variable.

        Example
        -------
        >>> linearMesh  = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> scalarFeVar = linearMesh.add_variable( nodeDofCount=1, dataType="double" )
        >>> q0field     = linearMesh.subMesh.add_variable( 1 )  # adds variable to secondary elementType discretisation
        """

        var = uw.mesh.MeshVariable(self, nodeDofCount, dataType, **kwargs)
        return var

    def add_pre_deform_function( self, function ):
        """
        Adds a function function to be executed before mesh deformation
        is applied.

        Parameters
        ----------
        function : FunctionType
            Python (not underworld) function to be executed. Closures should be
            used where parameters are required.

        """
        self._pre_deform_functions.append( function )

    def add_post_deform_function( self, function ):
        """
        Adds a function function to be executed after mesh deformation
        is applied.

        Parameters
        ----------
        function : FunctionType
            Python (not underworld) function to be executed. Closures should be
            used where parameters are required.

        """
        self._post_deform_functions.append( function )

    @property
    def nodesLocal(self):
        """
        Returns
        -------
        int
            Returns the number of local nodes on the mesh.
        """
        return libUnderworld.StgDomain.Mesh_GetLocalSize(self._cself, 0)

    @property
    def nodesDomain(self):
        """
        Returns
        -------
        int
            Returns the number of domain (local+shadow) nodes on the mesh.
        """
        return libUnderworld.StgDomain.Mesh_GetDomainSize(self._cself, 0)

    @property
    def nodesGlobal(self):
        """
        Returns
        -------
        int
            Returns the number of global nodes on the mesh

        Example
        -------
        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(-1.,-1.), maxCoord=(1.,1.) )
        >>> someMesh.nodesGlobal
        9
        """
        return libUnderworld.StgDomain.Mesh_GetGlobalSize(self._cself, 0)

    @property
    def elementsLocal(self):
        """
        Returns
        -------
        int
            Returns the number of local elements on the mesh

        Example
        -------
        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(-1.,-1.), maxCoord=(1.,1.) )
        >>> someMesh.elementsLocal
        4
        """
        return libUnderworld.StgDomain.Mesh_GetLocalSize(self._cself, self.dim)

    @property
    def elementsDomain(self):
        """
        Returns
        -------
        int
            Returns the number of domain (local+shadow) elements on the mesh

        Example
        -------
        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(-1.,-1.), maxCoord=(1.,1.) )
        >>> someMesh.elementsDomain
        4
        """
        return libUnderworld.StgDomain.Mesh_GetDomainSize(self._cself, self.dim)

    @property
    def elementsGlobal(self):
        """
        Returns
        -------
        int
            Returns the number of global elements on the mesh

        Example
        -------
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

        Notes
        -----
        This method must be called collectively by all processes.
        """
        self.generator._reset(self)
        # if we have a submesh, reset it as well
        if hasattr(self,"subMesh") and self.subMesh:
            self.subMesh.reset()


    @property
    def specialSets(self):
        """
        Returns
        -------
        dict
            This dictionary stores a set of special data sets relating to mesh objects.

        Example
        -------
        >>> import underworld as uw
        >>> someMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1', elementRes=(2,2), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> someMesh.specialSets.keys()
        ['MaxI_VertexSet', 'MinI_VertexSet', 'AllWalls_VertexSet', 'MinJ_VertexSet', 'MaxJ_VertexSet', 'Empty']
        >>> someMesh.specialSets["MinJ_VertexSet"]
        FeMesh_IndexSet([0, 1, 2])

        """
        if not hasattr(self, "_specialSets"):
            class _SpecialSetsDict(dict):
                """
                This special dictionary simply calls the function with the mesh object
                before returning it.
                """
                def __init__(self, mesh):
                    self._mesh = weakref.ref(mesh)

                    # call parents method
                    super(_SpecialSetsDict,self).__init__()
                def __getitem__(self,index):
                    # get item using regular dict
                    item = super(_SpecialSetsDict,self).__getitem__(index)
                    # now call using mesh and return
                    return item(self._mesh())
            self._specialSets = _SpecialSetsDict(self)

        return self._specialSets

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(FeMesh,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._mesh.name]["elementType"] = self._elementType

    def _get_iterator(self):
        # lets create the full index set
        iset = FeMesh_IndexSet(      object           = self,
                                     topologicalIndex = 0,
                                     size             = libUnderworld.StgDomain.Mesh_GetDomainSize( self._mesh, libUnderworld.StgDomain.MT_VERTEX ) )
        iset.addAll()
        return iset._get_iterator()

    def save( self, filename ):
        """
        Save the mesh to disk

        Parameters
        ----------
        filename : string
            The name of the output file.

        Returns
        -------
        underworld.utils.SavedFileData
            Data object relating to saved file. This only needs to be retained
            if you wish to create XDMF files and can be ignored otherwise.

        Notes
        -----
        This method must be called collectively by all processes.

        Example
        -------
        First create the mesh:

        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )

        Save to a file (note that the 'ignoreMe' object isn't really required):

        >>> ignoreMe = mesh.save("saved_mesh.h5")

        Now let's try and reload. First create new mesh (note the different spatial size):

        >>> clone_mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.5,1.5) )

        Confirm clone mesh is different from original mesh:

        >>> import numpy as np
        >>> np.allclose(mesh.data,clone_mesh.data)
        False

        Now reload using saved file:

        >>> clone_mesh.load("saved_mesh.h5")

        Now check for equality:

        >>> np.allclose(mesh.data,clone_mesh.data)
        True

        >>> # clean up:
        >>> if uw.rank() == 0:
        ...     import os;
        ...     os.remove( "saved_mesh.h5" )

        """

        if hasattr(self.generator, 'geometryMesh'):
            raise RuntimeError("Cannot save this mesh as it's a subMesh. "
                                + "Most likely you only need to save its geometryMesh")
        if not isinstance(filename, str):
            raise TypeError("'filename', must be of type 'str'")

        h5f = h5py.File(name=filename, mode="w", driver='mpio', comm=MPI.COMM_WORLD)

        # save attributes and simple data - MUST be parallel as driver is mpio
        h5f.attrs['dimensions'] = self.dim
        h5f.attrs['mesh resolution'] = self.elementRes
        h5f.attrs['max'] = self.maxCoord
        h5f.attrs['min'] = self.minCoord
        h5f.attrs['regular'] = self._cself.isRegular
        h5f.attrs['elementType'] = self.elementType

        # write the vertices
        globalShape = ( self.nodesGlobal, self.data.shape[1] )
        dset = h5f.create_dataset("vertices",
                                  shape=globalShape,
                                  dtype=self.data.dtype)

        local = self.nodesLocal
        # write to the dset using the local set of global node ids
        dset[self.data_nodegId[0:local],:] = self.data[0:local]

        # write the element node connectivity
        globalShape = ( self.elementsGlobal, self.data_elementNodes.shape[1] )
        dset = h5f.create_dataset("en_map",
                                  shape=globalShape,
                                  dtype=self.data_elementNodes.dtype)

        local = self.elementsLocal
        # write to the dset using the local set of global node ids
        dset[self.data_elgId[0:local],:] = self.data_elementNodes[0:local]

        h5f.close()

        # return our file handle
        return uw.utils.SavedFileData(self, filename)

    def load(self, filename ):
        """
        Load the mesh from disk.

        Parameters
        ----------
        filename: str
            The filename for the saved file. Relative or absolute paths may be
            used, but all directories must exist.

        Notes
        -----
        This method must be called collectively by all processes.

        If the file data array is the same length as the current mesh
        global size, it is assumed the file contains compatible data. Note that
        this may not be the case where for example where you have saved using a
        2*8 resolution mesh, then loaded using an 8*2 resolution mesh.

        Provided files must be in hdf5 format, and use the correct schema.

        Example
        -------
        Refer to example provided for 'save' method.

        """
        self.reset()
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")

        # get field and mesh information
        h5f = h5py.File( filename, "r", driver='mpio', comm=MPI.COMM_WORLD );

        # get resolution of old mesh
        res = h5f.attrs['mesh resolution']
        if res is None:
            raise RuntimeError("Can't read the 'mesh resolution' for the field hdf5 file,"+
                   " was it created correctly?")

        if (res == self.elementRes).all() == False:
            raise RuntimeError("Provided file mesh resolution does not appear to correspond to\n"\
                               "resolution of mesh object.")

        dset = h5f.get('vertices')
        if dset == None:
            raise RuntimeError("Can't find the 'vertices' dataset in hdf5 file '{0}'".format(filename) )

        dof = dset.shape[1]
        if dof != self.data.shape[1]:
            raise RuntimeError("Can't load hdf5 '{0}', incompatible data shape".format(filename))

        if len(dset) != self.nodesGlobal:
            raise RuntimeError("Provided data file appears to be for a different resolution mesh.")

        with self.deform_mesh(isRegular=h5f.attrs['regular']):
            self.data[0:self.nodesLocal] = dset[self.data_nodegId[0:self.nodesLocal],:]

        h5f.close()


class MeshGenerator(_stgermain.StgCompoundComponent):
    """
    Abstract base class for all mesh generators.

    Parameter
    ---------
    partitioned: bool
        If false, the mesh is not partitioned across entire processor pool. Instead
        mesh is entirely owned by processor which generated it.

    """
    _objectsDict = { "_gen": None }
    _selfObjectName = "_gen"

    def __init__(self, partitioned=True, **kwargs):
        if not isinstance(partitioned,bool):
            raise TypeError("'partitioned' parameter must be of type 'bool'.")
        self._partitioned = partitioned
        # build parent
        super(MeshGenerator,self).__init__(**kwargs)

    @property
    def dim(self):
        """
        Returns
        -------
        int
            The mesh dimensionality.
        """
        return self._dim

    @abc.abstractmethod
    def _reset(self,mesh):
        """
        Abstract class which handles mesh resetting.
        """
        pass

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MeshGenerator,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[self._gen.name]["partitioned"] = self._partitioned

class CartesianMeshGenerator(MeshGenerator):
    """
    Abstract base class for all cartesian mesh generators.
    Generators of this class provide algorithms to build meshes which are
    logically and geometrically Cartesian.

    Parameter
    ---------
    elementRes: list,tuple
        List or tuple of ints specifying mesh resolution. See CartesianMeshGenerator.elementRes
        docstring for further information.
    minCoord:  list, tuple
        List or tuple of floats specifying minimum mesh location. See CartesianMeshGenerator.minCoord
        docstring for further information.
    maxCoord: list, tuple
        List or tuple of floats specifying maximum mesh location. See CartesianMeshGenerator.maxCoord
        docstring for further information.
    periodic: list, tuple
        List or tuple of bools, specifying mesh periodicity in each direction.

    """
    def __init__(self, elementRes, minCoord, maxCoord, periodic=None, **kwargs):

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
        Returns
        -------
        list, tuple
            Element count to generate in I, J & K directions. Must be provided
            as a tuple of integers.
        """
        return self._elementRes
    @property
    def minCoord(self):
        """
        Returns
        -------
        list, tuple
            Minimum coordinate position for cartesian mesh.
            Values correspond to minimums in each direction (I,J,K) of the mesh.
            Note, this is the value used for initialisation, but mesh may be
            advecting.
        """
        return self._minCoord
    @property
    def periodic(self):
        """
        Returns
        -------
        list, tuple
            List of bools specifying mesh periodicity in each direction.
        """
        return self._periodic
    @property
    def maxCoord(self):
        """
        Returns
        -------
        list, tuple
            Maximum coordinate position for cartesian mesh.
            Values correspond to maximums in each direction (I,J,K) of the mesh.
            Note, this is the value used for initialisation, but mesh may be
            advecting.
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
        uw.libUnderworld.StgDomain.Mesh_Sync( self._cself )
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

    Parameter:
    ----------
    geometryMesh: underworld.mesh.FeMesh
        The geometry mesh.
    """
    def __init__(self, geometryMesh, **kwargs):
        if not isinstance(geometryMesh,(FeMesh)):
            raise TypeError("'geometryMesh' object passed in must be of type 'FeMesh'")
        # note we keep a weakref to avoid circular dependencies
        self._geometryMeshWeakref = weakref.ref(geometryMesh)

        self._dim = self.geometryMesh.dim

        super(TemplatedMeshGenerator,self).__init__(**kwargs)

    @property
    def geometryMesh(self):
        """
        Returns
        -------
        underworld.mesh.FeMesh
            This is the FeMesh from which the TemplatedMeshGenerator obtains
            the cells to template nodes upon. Note that this class only retains
            a weakref to the geometryMesh, and therefore this property may return
            None.
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

    Refer to parent classes for parameters beyond those below.

    Parameters
    ----------
    elementType: str
        Mesh element type. Note that this class allows the user to
        (optionally) provide a pair of elementTypes for which a dual
        mesh will be created.
        The submesh is accessible through the 'subMesh' property. The
        primary mesh itself is the object returned by this constructor.
    elementRes: list,tuple
        List or tuple of ints specifying mesh resolution. See CartesianMeshGenerator.elementRes
        docstring for further information.
    minCoord:  list, tuple
        List or tuple of floats specifying minimum mesh location. See CartesianMeshGenerator.minCoord
        docstring for further information.
    maxCoord: list, tuple
        List or tuple of floats specifying maximum mesh location. See CartesianMeshGenerator.maxCoord
        docstring for further information.
    periodic: list, tuple
        List or tuple of bools, specifying mesh periodicity in each direction.
    partitioned: bool
        If false, the mesh is not partitioned across entire processor pool. Instead
        mesh is entirely owned by processor which generated it.


    Examples
    --------
    To create a linear mesh:

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

    def __new__(cls, elementType="Q1/dQ0", elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.), periodic=None, partitioned=True, **kwargs):
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
        if elementType[0].upper() == cls._supportedElementTypes[0]:
            overruleDict["_gen"] = cls._meshGenerator[0]
        if elementType[0].upper() == cls._supportedElementTypes[1]:
            overruleDict["_gen"] = cls._meshGenerator[1]

        return super(FeMesh_Cartesian,cls).__new__(cls, objectsDictOverrule=overruleDict, **kwargs)


    def __init__(self, elementType="Q1/dQ0", elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.), periodic=None, partitioned=True, **kwargs):

        if isinstance(elementType, str):
            # convert to tuple to make things easier
            import re
            elementType = re.split(",|-|/",elementType)

        self._elementTypes = elementType
        # ok, lets go ahead and build primary mesh (ie, self)
        super(FeMesh_Cartesian,self).__init__(elementType=elementType[0], elementRes=elementRes, minCoord=minCoord, maxCoord=maxCoord, periodic=periodic, partitioned=partitioned, **kwargs)

        # lets add the special sets
        self.specialSets["MaxI_VertexSet"] = _specialSets_Cartesian.MaxI_VertexSet
        self.specialSets["MinI_VertexSet"] = _specialSets_Cartesian.MinI_VertexSet
        self.specialSets["MaxJ_VertexSet"] = _specialSets_Cartesian.MaxJ_VertexSet
        self.specialSets["MinJ_VertexSet"] = _specialSets_Cartesian.MinJ_VertexSet
        if(self.dim==3):
            self.specialSets["MaxK_VertexSet"] = _specialSets_Cartesian.MaxK_VertexSet
            self.specialSets["MinK_VertexSet"] = _specialSets_Cartesian.MinK_VertexSet
        self.specialSets["AllWalls_VertexSet"] = _specialSets_Cartesian.AllWalls

        # send some metrics
        # disable for now.  note, this seems to fire in doctests!  need to fix.
#        uw._sendData('init_femesh_cartesian', self.dim, np.prod( self.elementRes ))

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

        # add a boundary MeshVariable - 1 if nodes is on the boundary(ie 'AllWalls_VertexSet'), 0 if node is internal
        self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        self.bndMeshVariable.data[:] = 0.
        self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0


    @property
    def subMesh(self):
        """
        Returns
        -------
        underworld.mesh.FeMesh
            Returns the submesh where the object is a dual mesh, or None otherwise.
        """
        return self._secondaryMesh

    def integrate(self, fn):
        """
        Perform a domain integral of the given underworld function over this mesh

        Parameters
        ----------
        mesh : uw.mesh.FeMesh_Cartesian
            Domain to perform integral over.

        Examples
        --------

        >>> mesh = uw.mesh.FeMesh_Cartesian(minCoord=(0.0,0.0), maxCoord=(1.0,2.0))
        >>> fn_1 = uw.function.misc.constant(2.0)
        >>> np.allclose( mesh.integrate( fn_1 )[0], 4 )
        True

        >>> fn_2 = uw.function.misc.constant(2.0) * (0.5, 1.0)
        >>> np.allclose( mesh.integrate( fn_2 ), [2,4] )
        True

        """
        # check if _volume_integral exist, if not then create it
        if not hasattr(self, "_volume_integral"):
            # define volume integral class, with dummy fn=1.0
            self._volume_integral = uw.utils.Integral(mesh=self, fn=1.0)

        self._volume_integral.fn = fn
        return self._volume_integral.evaluate()

class FeMesh_IndexSet(uw.container.ObjectifiedIndexSet, function.FunctionInput):
    """
    This class ties the FeMesh instance to an index set, and stores other
    metadata relevant to the set.

    Parameters
    ----------
    object: underworld.mesh.FeMesh
        The FeMesh instance from which the IndexSet was extracted.
    topologicalIndex: int
        Mesh topological index for which the IndexSet relates. See
        docstring for further info.

    Example
    -------

    >>> amesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> iset = uw.libUnderworld.StgDomain.RegularMeshUtils_CreateGlobalMaxISet( amesh._mesh )
    >>> uw.mesh.FeMesh_IndexSet( amesh, topologicalIndex=0, size=amesh.nodesGlobal, fromObject=iset )
    FeMesh_IndexSet([ 4,  9, 14, 19, 24])

    """
    def __init__(self, object, topologicalIndex=None, *args, **kwargs):
        if topologicalIndex not in (0,1,2,3):
            raise ValueError("Topological index must be within [0,3].")
        if not isinstance(object, FeMesh):
            raise TypeError("'object' parameter must be of (sub)type 'FeMesh'.")
        self._toplogicalIndex = topologicalIndex

        super(FeMesh_IndexSet,self).__init__(object,*args,**kwargs)

    @property
    def topologicalIndex(self):
        """
        Returns
        -------
        int
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

class _FeMesh_Regional(FeMesh_Cartesian):
    def __new__(cls, **kwargs):
        return super(_FeMesh_Regional,cls).__new__(cls, **kwargs)

    def __init__(self, elementRes=(16,16,10), radialLengths=(3.0,6.0), latExtent=90.0, longExtent=90.0, centroid=[0.0,0.0,0.0], **kwargs):
        """
        Class initialiser for Cubed-sphere sixth, centered on the 'centroid'.


        MinI_VertexSet / MaxI_VertexSet -> longitudinal walls : [min/max] = [west/east]
        MinJ_VertexSet / MaxJ_VertexSet -> latitudinal walls  : [min/max] = [south/north]
        MinK_VertexSet / MaxK_VertexSet -> radial walls       : [min/max] = [inner/outer]

        Refer to parent classes for parameters beyond those below.

        Parameter
        ---------
        elementRes : tuple
            Tuple determining number of elements (longitudinally, latitudinally, radially).
        radialLengths : tuple
            Tuple determining the (inner radialLengths, outer radialLengths).
        longExtent : float
            The angular extent of the domain between great circles of longitude.
        latExtent : float
            The angular extent of the domain between great circles of latitude.


        Example
        -------

        >>> (radMin, radMax) = (4.0,8.0)
        >>> mesh = uw.mesh._FeMesh_Regional( elementRes=(20,20,14), radialLengths=(radMin, radMax) )
        >>> integral = uw.utils.Integral( 1.0, mesh).evaluate()[0]
        >>> exact = 4/3.0*np.pi*(radMax**3 - radMin**2) / 6.0
        >>> np.fabs(integral-exact)/exact < 1e-1
        True
        """

    def __new__(cls, **kwargs):
        return super(_FeMesh_Regional,cls).__new__(cls, **kwargs)

    def __init__(self, elementRes=(16,16,10), radial=(3.0,6.0), latExtent=90.0, longExtent=90.0, **kwargs):

        if not isinstance( latExtent, (float,int) ):
            raise TypeError("Provided 'latExtent' must be a float or integer")
        self._latExtent = latExtent
        if not isinstance( longExtent, (float,int) ):
            raise TypeError("Provided 'longExtent' must be a float or integer")
        self._longExtent = longExtent
        if not isinstance( radialLengths, (tuple,list)):
            raise TypeError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        if len(radialLengths) != 2:
            raise ValueError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        for el in radialLengths:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        self._radialLengths = radialLengths

        lat_half = latExtent/2.0
        long_half = longExtent/2.0

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        super(_FeMesh_Regional,self).__init__(elementType="Q1/dQ0", elementRes=elementRes,
                    minCoord=(radialLengths[0],-long_half,-lat_half), maxCoord=(radialLengths[1],long_half,lat_half), periodic=None, **kwargs)

        self._centroid = centroid

    def _setup(self):

        with self.deform_mesh():
            # perform Cubed-sphere projection on coordinates
            # fac = np.pi/180.0
            old = self.data
            (x,y) = (np.tan(self.data[:,1]*np.pi/180.0), np.tan(self.data[:,2]*np.pi/180.0))
            d = self.data[:,0] / np.sqrt( x**2 + y**2 + 1)
            self.data[:,0] = self._centroid[0] + d*x
            self.data[:,1] = self._centroid[1] + d*y
            self.data[:,2] = self._centroid[2] + d

        # ASSUME the parent class builds the _boundaryNodeFn
        # self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        # self.bndMeshVariable.data[:] = 0.
        # # set a value 1.0 on provided vertices
        # self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0
        # # note we use this condition to only capture border swarm particles
        # # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
        # # to non-zero (but less than 1.), so we need to remove those from the integration as well.
        # self._boundaryNodeFn = uw.function.branching.conditional(
        #                                   [  ( self.bndMeshVariable > 0.999, 1. ),
        #                                      (                    True, 0. )   ] )

        self._rFn  = self._boundaryNodeFn * self.fn_unitvec_radial()
        self._nsFn = self._boundaryNodeFn * self._getNSFn()
        self._ewFn = self._boundaryNodeFn * self._getEWFn()

        nsWalls = self.specialSets["MinJ_VertexSet"] + self.specialSets['MaxJ_VertexSet']

        nsWallField = uw.mesh.MeshVariable(self, nodeDofCount=1)

        nsWallField.data[:] = 0.0
        nsWallField.data[nsWalls.data] = 1.0
        self._normal = uw.function.branching.conditional([ (nsWallField > 0.5, self._nsFn ),
                                                           (             True, self._ewFn) ] )

    def fn_unitvec_radial(self):

        pos = function.coord()
        centre = self._centroid
        r_vec = pos - centre
        mag = function.math.sqrt(function.math.dot( r_vec, r_vec ))
        r_vec = r_vec / mag
        return r_vec

    def _getEWFn(self):
        pos = function.coord() - self._centroid
        xi = function.math.atan(pos[0]/pos[2])
        # vec = [ cos(xi), 0.0, -sin(xi) ]
        vec = function.math.cos(xi) * (0., 1., 0. )
        vec = vec + function.math.sin(xi) * (0.0,0.0,-1.)
        return vec

    def _getNSFn(self):
        pos = function.coord() - self._centroid
        xi = function.math.atan(pos[1]/pos[2])
        # vec = [ 0.0, cos(xi), -sin(xi) ]
        vec = function.math.cos(xi)* (1., 0., 0. )
        vec = vec + function.math.sin(xi) * (0.0,0.0,-1.)
        return vec

class _FeMesh_Annulus(FeMesh_Cartesian):

    def __new__(cls, **kwargs):
        return super(_FeMesh_Annulus,cls).__new__(cls, **kwargs)

    def __init__(self, elementRes=(10,16), radialLengths=(3.0,6.0), angularExtent=[0.0,360.0], centroid=[0.0,0.0], periodic=[False, True], **kwargs):
        """
        Class initialiser for Annulus mesh, centered on the 'centroid'.

        TODO - Fix documentation
        MinI_VertexSet / MaxI_VertexSet -> radial walls       : [min/max] = [inner/outer]

        Parameter
        ---------
            elementRes : 3-tuple
                1st element - Number of elements across the radial length of the domain
                2nd element - Number of elements along the circumfrance

            radialLengths : 2-tuple, default (3.0,6.0)
                The radial position of the inner and outer surfaces respectively.
                (inner radialLengths, outer radialLengths)

            angularExtent : 2-tuple, default (0.0,360.0)
                The angular extent of the domain

            radialData : Return the mesh node locations in polar form.
                (radial length, angle in degrees)

            See parent classes for further required/optional parameters.

        >>> (radMin, radMax) = (4.0,8.0)
        >>> mesh = uw.mesh._FeMesh_Annulus( elementRes=(14, 36), radialLengths=(radMin, radMax), angularExtent=[0.0,180.0] )
        >>> integral = uw.utils.Integral( 1.0, mesh).evaluate()[0]
        >>> exact = np.pi*(radMax**2 - radMin**2)/2.
        >>> np.fabs(integral-exact)/exact < 1e-1
        True

        """

        if not isinstance( angularExtent, (tuple,list)):
            raise TypeError("Provided 'angularExtent' must be a tuple/list of 2 floats")
        if len(angularExtent) != 2:
            raise ValueError("Provided 'angularExtent' must be a tuple/list of 2 floats")
        for el in angularExtent:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'angularExtent' must be a tuple/list of 2 floats")
        self._angularExtent = angularExtent

        if not isinstance( radialLengths, (tuple,list)):
            raise TypeError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        if len(radialLengths) != 2:
            raise ValueError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        for el in radialLengths:
            if not isinstance( el, (float,int)) :
                raise TypeError("Provided 'radialLengths' must be a tuple/list of 2 floats")
        self._radialLengths = radialLengths

        # build 3D mesh cartesian mesh centred on (0.0,0.0,0.0) - in _setup() we deform the mesh
        super(_FeMesh_Annulus,self).__init__(elementType="Q1/dQ0", elementRes=elementRes,
                    minCoord=(radialLengths[0],angularExtent[0]), maxCoord=(radialLengths[1],angularExtent[1]), periodic=periodic, **kwargs)

        # TODO: make annulus specific specialSets
        self.specialSets["inner"] = _specialSets_Cartesian.MinI_VertexSet
        # uw.mesh.FeMesh_IndexSet(object=iset.object,
        #                                 topologicalIndex=iset.topologicalIndex,
        #                                 size=iset.object.nodesGlobal,
        #                                 fromObject=iset)
        self.specialSets["outer"] = _specialSets_Cartesian.MaxI_VertexSet
        # uw.mesh.FeMesh_IndexSet(object=iset.object,
        #                                 topologicalIndex=iset.topologicalIndex,
        #                                 size=iset.object.nodesGlobal,
        #                                 fromObject=iset)

        # self.specialSets["MinI_VertexSet"] = None
        # self.specialSets["MaxI_VertexSet"] = None
        # self.specialSets["MinJ_VertexSet"] = None
        # self.specialSets["MaxJ_VertexSet"] = None

        self._centroid = centroid

    @property
    def radialLengths(self):
        """
        Returns:
        Annulus min/max radius
        """
        return self._radialLengths

    @property
    def angularExtent(self):
        """
        Returns:
        Annulus min/max angular extents
        """
        return self._angularExtent

    def fn_unitvec_radial(self):
        # returns the radial position
        pos = function.coord()
        centre = self._centroid
        r_vec = pos - centre
        mag = function.math.sqrt(function.math.dot( r_vec, r_vec ))
        r_vec = r_vec / mag
        return r_vec

    def fn_unitvec_tangent(self):
        # returns the radial position
        pos = function.coord()
        centre = self._centroid
        r_vec = pos - centre
        theta = [-1.0*r_vec[1], r_vec[0]]
        mag = function.math.sqrt(function.math.dot( theta, theta ))
        theta = theta / mag
        return theta

    @property
    def radialData(self):
        # returns data in polar form
        r = np.sqrt((self.data ** 2).sum(1))
        theta = (180/np.pi)*np.arctan2(self.data[:,1],self.data[:,0])
        return np.array([r,theta]).T

    def _setup(self):
        with self.deform_mesh():
            # basic polar coordinate -> cartesian map, i.e. r,t -> x,y
            r = self.data[:,0]
            t = self.data[:,1] * np.pi/180.0

            offset_x = self._centroid[0]
            offset_y = self._centroid[1]

            (self.data[:,0], self.data[:,1]) = offset_x + r*np.cos(t), offset_y + r*np.sin(t)

        self.bndMeshVariable = uw.mesh.MeshVariable(self, 1)
        self.bndMeshVariable.data[:] = 0.
        # set a value 1.0 on provided vertices
        self.bndMeshVariable.data[self.specialSets["AllWalls_VertexSet"].data] = 1.0
        # note we use this condition to only capture border swarm particles
        # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
        # to non-zero (but less than 1.), so we need to remove those from the integration as well.

        self.rot_vec_normal = uw.function.branching.conditional(
                            [ ( self.bndMeshVariable > 0.9, self.fn_unitvec_radial() ),
                              (               True, uw.function.misc.constant(1.0)*(1.0,0.0) ) ] )

        self.rot_vec_tangent = uw.function.branching.conditional(
                            [ ( self.bndMeshVariable > 0.9, self.fn_unitvec_tangent() ),
                              (               True, uw.function.misc.constant(1.0)*(0.0,1.0) ) ] )
