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

class FeVariable(_stgermain.StgCompoundComponent,uw.function.Function,_stgermain.Save,_stgermain.Load):
    """
    The FeVariable class generates a variable supported by a finite element mesh.

    For example, to create a scalar feVariable:

    >>> # first create mesh
    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> scalarFeVar = uw.fevariable.FeVariable( feMesh=linearMesh, nodeDofCount=1, dataType="double" )

    or a vector fevariable can be created
    >>> vectorFeVar = uw.fevariable.FeVariable( feMesh=linearMesh, nodeDofCount=3, dataType="double" )

    To set / read nodal values, use the numpy interface via the 'data' property.

    """
    _objectsDict = { "_fevariable": "FeVariable",
                     "_meshvariable": "MeshVariable",
                     "_doflayout" : "DofLayout" }
    _selfObjectName = "_fevariable"

    _supportedDataTypes = ["char","short","int","float", "double"]

    def __init__(self, feMesh, nodeDofCount, dataType="double", **kwargs):
        """
        Parameters:
        -----------
            feMesh : FeMesh
            dataType : string (only option - 'double')
            nodeDofCount : int

        See property docstrings for further information on each argument.

        """

        if not isinstance(feMesh, uw.mesh.FeMesh):
            raise TypeError("'feMesh' object passed in must be of type 'FeMesh'")
        self._feMesh = feMesh

        if not isinstance(dataType,str):
            raise TypeError("'dataType' object passed in must be of type 'str'")
        if dataType.lower() not in self._supportedDataTypes:
            raise ValueError("'dataType' provided ({}) does not appear to be supported. \nSupported types are {}.".format(dataType.lower(),self._supportedDataTypes))
        self._dataType = dataType

        if not dataType=="double":
            raise ValueError("Only Fevariables of type 'double' are currently supported.")

        if not isinstance(nodeDofCount, int):
            raise TypeError("'nodeDofCount' object passed in must be of type 'int'")
        if nodeDofCount < 1:
            raise ValueError("'nodeDofCount' must be one or greater.")
        self._nodeDofCount = nodeDofCount

        # add this to keep second parent happy.. not ideal. actualy function setup
        # occurs in the _setup method below
        self._fncself=None
        # build parent
        super(FeVariable,self).__init__(argument_fns=None, **kwargs)

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
    def feMesh(self):
        """
        feMesh (FeMesh): Supporting FeMesh for this FeVariable.
        """
        return self._feMesh

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
        >>> scalarFeVar = uw.fevariable.FeVariable( feMesh=linearMesh, nodeDofCount=1, dataType="double" )
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
        return libUnderworld.StGermain.Variable_getAsNumpyArray(self._meshvariable)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(FeVariable,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._meshvariable.name ]["mesh"]                 = self._feMesh._cself.name
        componentDictionary[ self._meshvariable.name ]["Rank"]                 = "Vector"
        componentDictionary[ self._meshvariable.name ]["DataType"]             = self.dataType
        componentDictionary[ self._meshvariable.name ]["VectorComponentCount"] = self.nodeDofCount
        componentDictionary[ self._meshvariable.name ]["names"]                = []
        for ii in range(0,self.nodeDofCount):
            componentDictionary[ self._meshvariable.name ]["names"].append( self._meshvariable.name + "_" + str(ii) )

        componentDictionary[ self._doflayout.name    ]["MeshVariable"]         = self._meshvariable.name

        componentDictionary[ self._fevariable.name   ]["FEMesh"]               = self._feMesh._cself.name
        componentDictionary[ self._fevariable.name   ]["DofLayout"]            = self._doflayout.name
        componentDictionary[ self._fevariable.name   ]["fieldComponentCount"]  = self.nodeDofCount
        componentDictionary[ self._fevariable.name   ]["dim"]                  = self._feMesh.generator.dim

    def _setup(self):
        # now actually setup function guy
        self._fncself = _cfn.FeVariableFn(self._cself)

        self._underlyingDataItems.add(self)


    @property
    def gradientFn(self):
        """
        gradientFn (Function): Returns a Function for the gradient field of this fevariable.
        """
        # lets define a wrapper class here

        import underworld.function as function


        class _gradient(function.Function):
            def __init__(self, fevariable, **kwargs):

                # create instance
                self._fncself = _cfn.GradFeVariableFn(fevariable._cself)

                # build parent
                super(_gradient,self).__init__(argument_fns=None, **kwargs)

                self._underlyingDataItems.add(fevariable)


        return _gradient(self)

    def save( self, filename ):
        """
        Global method to save the fevariable to disk

        Parameters
        ----------
        filename : string
            the name of the output hdf5 file

        Saves the fevariable to the 'filename' in hdf5 format. An 'ExternalLink' (hdf5 
        file association) is setup to the FeVariable's mesh so the field values can 
        access the mesh geometry & topology. 
        Note that this is a global method, ie. all processes must call it.

        """
        if not isinstance(filename, str):
            raise TypeError("Expected 'filename' to be provided as a string")

        mesh = self.feMesh
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

        ## setup reference to mesh - might need to call mesh.save()

        # get path to save mesh file
        saveDir = os.path.dirname(filename)
        meshfilename = saveDir + "/Mesh.h5"

        # check if it already exists - if not CREATE a mesh file
        # ASSUMES mesh is constant in time !!!
        if not os.path.exists(meshfilename):
            # only get geometryMesh - no subMesh to save
            if hasattr( mesh.generator, "geometryMesh"):
                mesh = mesh.generator.geometryMesh

            # call save on mesh
            mesh.save( meshfilename )
            
        # set reference to mesh (all procs must call following)
        h5f["mesh"] = h5py.ExternalLink(meshfilename, "./")

        # save a hdf5 attribute to the elementType used for this field - maybe useful
        h5f.attrs["elementType"] = np.string_(mesh.elementType)

        h5f.close()

    def _oldsave(self, filename):
        """
        Save the fevariable to the provided filename. Note that this is a
        global method, ie. all processes must call it.

        File is saved using hdf5 file format.
        """
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")
        libUnderworld.StgFEM.FeVariable_SaveToFile( self._cself, filename )

    def load(self, filename ):
        """
        Load the fevariable from the provided filename (hdf5 file). Note that this is a
        global method, ie. all processes must call it.

        If the filename field is the exact same shape as the current FeVariable
        it is assumed the fields are identical and the values are read directly
        into the current FeVariable.

        If the filename and the current FeVariable have different resolutions
        then a temporary fevariable of the filename field is built on each 
        processor and interpolated to the current FeVariable's node values.
        Note: The temporary FeVariable can only be built it the filename field
        is associated with a 'mesh', i.e. it was 

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
        
        if len(dset) == self.feMesh.nodesGlobal:

            # assume dset matches field exactly
            mesh = self.feMesh
            local = mesh.nodesLocal

            self.data[0:local] = dset[mesh.data_nodegId[0:local],:]

        else:

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
            inputField = uw.fevariable.FeVariable( feMesh=inputMesh, nodeDofCount=dof )
            
            # copy hdf5 numpy array onto serial inputField
            inputField.data[:] = dset[:]

            # interpolate 'inputField' onto the self nodes
            self.data[:] = inputField.evaluate(self.feMesh.data)

        uw.libUnderworld.StgFEM._FeVariable_SyncShadowValues( self._cself )
        h5f.close()

    def copy(self, deepcopy=False):
        """
        This method returns a copy of the fevariable.

        Parameters:
        ----------

        deepcopy: bool (default False)
            If True, the underlying object data is also copied.
        """

        if not isinstance(deepcopy, bool):
            raise TypeError("'deepcopy' parameter is expected to be of type 'bool'.")

        newFe = FeVariable(self.feMesh, self.nodeDofCount, self.dataType)

        if deepcopy:
            newFe.data[:] = self.data[:]

        return newFe
