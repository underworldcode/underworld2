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

    def save(self, filename):
        """
        Save the fevariable to the provided filename. Note that this is a
        global method, ie. all processes must call it.

        File is saved using hdf5 file format.
        """
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")
        libUnderworld.StgFEM.FeVariable_SaveToFile( self._cself, filename )

    def load(self, filename):
        """
        Load the fevariable from the provided filename. Note that this is a
        global method, ie. all processes must call it.

        Provided file must be in hdf5 format, and use the correct schema.
        """
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")
        libUnderworld.StgFEM.FeVariable_ReadFromFile( self._cself, filename )

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
