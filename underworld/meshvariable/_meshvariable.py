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

    Example
    -------
    
    First create mesh
    
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

    def save( self, filename, meshFilename=None ):
        """
        Save the MeshVariable to disk. 

        Parameters
        ----------
        filename : string
            The name of the output file.
        meshFilename : string, optional
            If provided, a link to the created mesh file is created within the 
            mesh variable file.

        Notes
        -----
        This method must be called collectively by all processes.
        
        Example
        -------
        First create the mesh add a variable:

        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> var = uw.meshvariable.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double" )
        
        Write something to variable
        
        >>> import numpy as np
        >>> var.data[:,0] = np.arange(var.data.shape[0])
        
        Save to a file:
        
        >>> var.save("saved_mesh_variable.h5")
        
        Now let's try and reload.
        
        >>> clone_var = uw.meshvariable.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double" )
        >>> clone_var.load("saved_mesh_variable.h5")
        
        Now check for equality:
        
        >>> np.allclose(var.data,clone_var.data)
        True
        
        Clean up:
        >>> if uw.rank() == 0: import os; os.remove( "saved_mesh_variable.h5" )

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

        if meshFilename:
            if not os.path.exists(meshFilename):
                # call save on mesh
                mesh.save( meshfilename )

            # set reference to mesh (all procs must call following)
            h5f["mesh"] = h5py.ExternalLink(meshFilename, "./")

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
        Load the MeshVariable from disk.
        
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
                Also note that the supporting mesh mush be regular.
        
        Notes
        -----
        This method must be called collectively by all processes.

        If the file data array is the same length as the current variable
        global size, it is assumed the file contains compatible data. Note that 
        this may not be the case where for example where you have saved using a
        2*8 resolution mesh, then loaded using an 8*2 resolution mesh.

        Provided files must be in hdf5 format, and use the correct schema.
        
        Example
        -------
        Refer to example provided for 'save' method.

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
            regular = h5f['mesh'].attrs.get('regular')
            if regular and regular!=True:
                raise RuntimeError("Saved mesh file appears to correspond to a irregular mesh.\n"\
                                   "Interpolating from irregular mesh not currently supported." )
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
