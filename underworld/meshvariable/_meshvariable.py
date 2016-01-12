##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld._stgermain as _stgermain
import underworld as uw
import libUnderworld.libUnderworldPy.Function as _cfn
import libUnderworld
from mpi4py import MPI
import h5py
import numpy as np
import os

class MeshVariable(_stgermain.StgCompoundComponent,uw.function.Function,_stgermain.Save,_stgermain.Load):
    """
    The MeshVariable class generates a variable supported by a finite element mesh.

    For example, to create a scalar meshVariable:

    >>> # first create mesh
    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> scalarFeVar = uw.meshvariable.MeshVariable( mesh=linearMesh, nodeDofCount=1, dataType="double" )

    or a vector meshvariable can be created
    >>> vectorFeVar = uw.meshvariable.MeshVariable( mesh=linearMesh, nodeDofCount=3, dataType="double" )

    To set / read nodal values, use the numpy interface via the 'data' property.

    """
    _objectsDict = { "_meshvariable": "FeVariable",
                     "_cmeshvariable": "MeshVariable",
                     "_doflayout" : "DofLayout" }
    _selfObjectName = "_meshvariable"

    _supportedDataTypes = ["char","short","int","float", "double"]

    def __init__(self, mesh, nodeDofCount, dataType="double", **kwargs):
        """
        Parameters:
        -----------
            mesh : FeMesh
            dataType : string (only option - 'double')
            nodeDofCount : int

        See property docstrings for further information on each argument.

        """

        if not isinstance(mesh, uw.mesh.FeMesh):
            raise TypeError("'mesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh

        if not isinstance(dataType,str):
            raise TypeError("'dataType' object passed in must be of type 'str'")
        if dataType.lower() not in self._supportedDataTypes:
            raise ValueError("'dataType' provided ({}) does not appear to be supported. \nSupported types are {}.".format(dataType.lower(),self._supportedDataTypes))
        self._dataType = dataType

        if not dataType=="double":
            raise ValueError("Only MeshVariables of type 'double' are currently supported.")

        if not isinstance(nodeDofCount, int):
            raise TypeError("'nodeDofCount' object passed in must be of type 'int'")
        if nodeDofCount < 1:
            raise ValueError("'nodeDofCount' must be one or greater.")
        self._nodeDofCount = nodeDofCount

        # add this to keep second parent happy.. not ideal. actualy function setup
        # occurs in the _setup method below
        self._fncself=None
        # build parent
        super(MeshVariable,self).__init__(argument_fns=None, **kwargs)

    @property
    def dataType(self):
        """
        dataType (str): Data type for variable.  Supported types are 'double'.
        """
        return self._dataType
    @property
    def nodeDofCount(self):
        """
        nodeDofCount (int): Degrees of freedom on each mesh node that this variable provides.
        """
        return self._nodeDofCount

    @property
    def mesh(self):
        """
        mesh (FeMesh): Supporting FeMesh for this MeshVariable.
        """
        return self._mesh

    @property
    def data(self):
        """
        data (np.array):  Numpy proxy array to underlying variable data.
        Note that the returned array is a proxy for all the *local* nodal
        data, and is provided as 1d list. It is possible to change the
        shape of this numpy array to reflect the cartesian topology (where
        appropriate), though again only the local portion of the decomposed
        domain will be available, and the shape will not necessarily be
        identical on all processors.

        As these arrays are simply proxys to the underlying memory structures,
        no data copying is required.

        >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> scalarFeVar = uw.meshvariable.MeshVariable( mesh=linearMesh, nodeDofCount=1, dataType="double" )
        >>> scalarFeVar.data.shape
        (289, 1)

        You can retrieve individual nodal values

        >>> scalarFeVar.data[100]
        array([ 0.])

        Likewise you can modify nodal values

        >>> scalarFeVar.data[100] = 15.333
        >>> scalarFeVar.data[100]
        array([ 15.333])
        """
        return libUnderworld.StGermain.Variable_getAsNumpyArray(self._cmeshvariable)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MeshVariable,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cmeshvariable.name ]["mesh"]                 = self._mesh._cself.name
        componentDictionary[ self._cmeshvariable.name ]["Rank"]                 = "Vector"
        componentDictionary[ self._cmeshvariable.name ]["DataType"]             = self.dataType
        componentDictionary[ self._cmeshvariable.name ]["VectorComponentCount"] = self.nodeDofCount
        componentDictionary[ self._cmeshvariable.name ]["names"]                = []
        for ii in range(0,self.nodeDofCount):
            componentDictionary[ self._cmeshvariable.name ]["names"].append( self._meshvariable.name + "_" + str(ii) )

        componentDictionary[ self._doflayout.name    ]["MeshVariable"]         = self._cmeshvariable.name

        componentDictionary[ self._meshvariable.name   ]["FEMesh"]               = self._mesh._cself.name
        componentDictionary[ self._meshvariable.name   ]["DofLayout"]            = self._doflayout.name
        componentDictionary[ self._meshvariable.name   ]["fieldComponentCount"]  = self.nodeDofCount
        componentDictionary[ self._meshvariable.name   ]["dim"]                  = self._mesh.generator.dim

    def _setup(self):
        # now actually setup function guy
        self._fncself = _cfn.FeVariableFn(self._cself)

        self._underlyingDataItems.add(self)


    @property
    def gradientFn(self):
        """
        gradientFn (Function): Returns a Function for the gradient field of this meshvariable.
        """
        # lets define a wrapper class here

        import underworld.function as function


        class _gradient(function.Function):
            def __init__(self, meshvariable, **kwargs):

                # create instance
                self._fncself = _cfn.GradFeVariableFn(meshvariable._cself)

                # build parent
                super(_gradient,self).__init__(argument_fns=None, **kwargs)

                self._underlyingDataItems.add(meshvariable)


        return _gradient(self)

    def save( self, filename ):
        """
        Global method to save the MeshVariable to disk

        Parameters
        ----------
        filename : string
            the name of the output hdf5 file

        Saves the MeshVariable to the 'filename' in hdf5 format. An 'ExternalLink' (hdf5
        file association) is created to the MeshVariable's mesh file so the field values can
        access the mesh's geometry & topology. Also the element type used for this field
        it saved as an attribute to output file.
        Note that this is a global method - all processes must call it.

        """
        if not isinstance(filename, str):
            raise TypeError("Expected 'filename' to be provided as a string")

        mesh = self.mesh
        h5f = h5py.File(name=filename, mode="w", driver='mpio', comm=MPI.COMM_WORLD)

        # ugly global shape def
        globalShape = ( mesh.nodesGlobal, self.data.shape[1] )
        # create dataset
        dset = h5f.create_dataset("data", 
                                  shape=globalShape,
                                  dtype='f')

        # write to the dset using the global node ids
        local = mesh.nodesLocal
        dset[mesh.data_nodegId[0:local],:] = self.data[0:local]

        # save a hdf5 attribute to the elementType used for this field - maybe useful
        h5f.attrs["elementType"] = np.string_(mesh.elementType)

        ## setup reference to mesh - THE GEOMETRY MESH
        saveDir = os.path.dirname(filename)

        if hasattr( mesh.generator, "geometryMesh"):
            mesh = mesh.generator.geometryMesh

        meshfilename = uw.utils._createMeshName( mesh, saveDir )

        # check if it already exists - if not CREATE a mesh file
        # ASSUMES mesh is constant in time !!!
        if not os.path.exists(meshfilename):
            # call save on mesh
            mesh.save( meshfilename )
            
        # set reference to mesh (all procs must call following)
        h5f["mesh"] = h5py.ExternalLink(meshfilename, "./")

        h5f.close()

    def _oldsave(self, filename):
        """
        Save the meshvariable to the provided filename. Note that this is a
        global method, ie. all processes must call it.

        File is saved using hdf5 file format.
        """
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")
        libUnderworld.StgFEM.FeVariable_SaveToFile( self._cself, filename )

    def load(self, filename, interpolate=False ):
        """
        Load the MeshVariable from the provided filename. 
        
        Parameters
        ----------
            filename: str
                The filename for the saved file. Relative or absolute paths may be
                used, but all directories must exist.
            interpolate: bool (default False)
                Set to True to interpolate a file containing different resolution data.
                Note that a temporary MeshVariable with the file data will be build 
                on **each** processor. Also note that the temporary MeshVariable 
                can only be built if its corresponding mesh file is available.
        
        Notes
        -----
        This method must be called collectively by all processes.

        If the file data is the same length as the current MeshVariable,
        it is assumed the fields are identical and the values are read 
        directly into the current MeshVariable.

        Provided files must be in hdf5 format, and use the correct schema.

        """
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")

        # get field and mesh information 
        h5f = h5py.File( filename, "r", driver='mpio', comm=MPI.COMM_WORLD );
        dset = h5f.get('data')
        if dset == None:
            raise RuntimeError("Can't find the 'data' in hdf5 file '{0}'".format(filename) )

        dof = dset.shape[1]
        if dof != self.data.shape[1]:
            raise RuntimeError("Can't load hdf5 '{0}', incompatible data shape".format(filename))
        
        if len(dset) == self.mesh.nodesGlobal:

            # assume dset matches field exactly
            mesh = self.mesh
            local = mesh.nodesLocal

            self.data[0:local] = dset[mesh.data_nodegId[0:local],:]

        else:
            if not interpolate:
                raise RuntimeError("Provided data file appears to be for a different resolution MeshVariable.\n"\
                                   "If you would like to interpolate the data to the current variable, please set\n" \
                                   "the 'interpolate' parameter. Check docstring for important caveats of interpolation method.")

            # if here then we build a local version of the entire file field and interpolate it's values

            # first get file field's mesh
            if h5f.get('mesh') == None:
                raise RuntimeError("The hdf5 field to be loaded must have an associated "+
                        "'mesh' hdf5 file, was it created correctly?")
            # get resolution of old mesh    
            res = h5f['mesh'].attrs.get('mesh resolution')
            if res is None:
                raise RuntimeError("Can't read the 'mesh resolution' for the field hdf5 file,"+
                       " was it created correctly?")

            # get max of old mesh    
            inputMax = h5f['mesh'].attrs.get('max')
            if inputMax is None:
                raise RuntimeError("Can't read the 'max' for the field hdf5 file,"+
                       " was it created correctly?")

            inputMin = h5f['mesh'].attrs.get('min')
            if inputMin is None:
                raise RuntimeError("Can't read the 'min' for the field hdf5 file,"+
                       " was it created correctly?")
            # build the NON-PARALLEL field and mesh
            inputMesh = uw.mesh.FeMesh_Cartesian( elementType = ("Q1/dQ0"), 
                                          elementRes  = tuple(res), 
                                          minCoord    = tuple(inputMin), 
                                          maxCoord    = tuple(inputMax), 
                                          partitioned=False)
            inputField = uw.meshvariable.MeshVariable( feMesh=inputMesh, nodeDofCount=dof )
            
            # copy hdf5 numpy array onto serial inputField
            inputField.data[:] = dset[:]

            # interpolate 'inputField' onto the self nodes
            self.data[:] = inputField.evaluate(self.mesh.data)

        uw.libUnderworld.StgFEM._FeVariable_SyncShadowValues( self._cself )
        h5f.close()

    def copy(self, deepcopy=False):
        """
        This method returns a copy of the meshvariable.

        Parameters:
        ----------

        deepcopy: bool (default False)
            If True, the underlying object data is also copied.
        """

        if not isinstance(deepcopy, bool):
            raise TypeError("'deepcopy' parameter is expected to be of type 'bool'.")

        newFe = MeshVariable(self.mesh, self.nodeDofCount, self.dataType)

        if deepcopy:
            newFe.data[:] = self.data[:]

        return newFe
