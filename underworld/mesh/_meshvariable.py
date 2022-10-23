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
import underworld.libUnderworld.libUnderworldPy.Function as _cfn
import underworld.libUnderworld as libUnderworld
from mpi4py import MPI
import h5py
import numpy as np
import os
from underworld.scaling import dimensionalise, pint_degc_labels
from underworld.scaling import non_dimensionalise
from underworld.scaling import units as u
from pint.errors import UndefinedUnitError


class MeshVariable(_stgermain.StgCompoundComponent,uw.function.Function,_stgermain.Save,_stgermain.Load):
    """
    The MeshVariable class generates a variable supported by a finite element mesh.

    To set / read nodal values, use the numpy interface via the 'data' property.

    Parameters
    ----------
    mesh : underworld.mesh.FeMesh
        The supporting mesh for the variable.
    dataType : string
        The data type for the variable.
        Note that only 'double' type variables are currently
        supported.
    nodeDofCount : int
        Number of degrees of freedom per node the variable will have.


    See property docstrings for further information.


    Example
    -------
    For example, to create a scalar meshVariable:

    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> scalarFeVar = uw.mesh.MeshVariable( mesh=linearMesh, nodeDofCount=1, dataType="double" )

    or a vector meshvariable can be created:

    >>> vectorFeVar = uw.mesh.MeshVariable( mesh=linearMesh, nodeDofCount=3, dataType="double" )

    """
    _objectsDict = { "_meshvariable": "FeVariable",
                     "_cmeshvariable": "MeshVariable",
                     "_doflayout" : "DofLayout" }
    _selfObjectName = "_meshvariable"

    _supportedDataTypes = ["char","short","int","float", "double"]

    def __init__(self, mesh, nodeDofCount, dataType="double", **kwargs):
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

        # also null this guy initially
        self._fn_gradient = None
        # build parent
        super(MeshVariable,self).__init__(argument_fns=None, **kwargs)

    @property
    def dataType(self):
        """
        Returns
        -------
        str
            Data type for variable.  Supported types are 'double'.
        """
        return self._dataType
    @property
    def nodeDofCount(self):
        """
        Returns
        -------
        int
            Degrees of freedom on each mesh node that this variable provides.
        """
        return self._nodeDofCount

    @property
    def mesh(self):
        """
        Returns
        -------
        underworld.mesh.FeMesh
            Supporting FeMesh for this MeshVariable.
        """
        return self._mesh

    @property
    def data(self):
        """
        Numpy proxy array to underlying variable data.
        Note that the returned array is a proxy for all the *local* nodal
        data, and is provided as 1d list. It is possible to change the
        shape of this numpy array to reflect the cartesian topology (where
        appropriate), though again only the local portion of the decomposed
        domain will be available, and the shape will not necessarily be
        identical on all processors.


        As these arrays are simply proxies to the underlying memory structures,
        no data copying is required.

        Returns
        -------
        numpy.ndarray
            The proxy array.

        Example
        -------
        >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> scalarFeVar = uw.mesh.MeshVariable( mesh=linearMesh, nodeDofCount=1, dataType="double" )
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
        return libUnderworld.StGermain.StgVariable_getAsNumpyArray(self._cmeshvariable)

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
    def fn_gradient(self):
        """
        Returns a Function for the gradient field of this meshvariable.

        Note that for a scalar variable `T`, the gradient function returns
        an array of the form:

        .. math::

             [ \\frac{\\partial T}{\\partial x}, \\frac{\\partial T}{\\partial y}, \\frac{\\partial T}{\\partial z} ]

        and for a vector variable `v`:

        .. math::

            [ \\frac{\\partial v_x}{\\partial x}, \\frac{\\partial v_x}{\\partial y}, \\frac{\\partial v_x}{\\partial z},
              \\frac{\\partial v_y}{\\partial x}, \\frac{\\partial v_y}{\\partial y}, \\frac{\\partial v_y}{\\partial z},
              \\frac{\\partial v_z}{\\partial x}, \\frac{\\partial v_z}{\\partial y}, \\frac{\\partial v_z}{\\partial z} ]

        Returns
        -------
        underworld.function.Function
            The gradient function.

        """

        if not self._fn_gradient:
            # lets define a wrapper class here
            import underworld.function as function
            class _gradient(function.Function):
                def __init__(self, meshvariable, **kwargs):

                    # create instance
                    self._fncself = _cfn.GradFeVariableFn(meshvariable._cself)

                    # build parent
                    super(_gradient,self).__init__(argument_fns=None, **kwargs)

                    self._underlyingDataItems.add(meshvariable)

            self._fn_gradient = _gradient(self)
        return self._fn_gradient

    def xdmf( self, filename, fieldSavedData, varname, meshSavedData, meshname, modeltime=0.  ):
        """
        Creates an xdmf file, filename, associating the fieldSavedData file on
        the meshSavedData file

        Notes
        -----
        xdmf contain 2 files: an .xml and a .h5 file. See http://www.xdmf.org/index.php/Main_Page
        This method only needs to be called by the master process, all other
        processes return quiely.

        Parameters
        ----------
        filename : str
            The output path to write the xdmf file. Relative or absolute paths may be
            used, but all directories must exist.
        varname : str
            The xdmf name to give the field
        meshSavedData : underworld.utils.SaveFileData
            Handler returned for saving a mesh. underworld.mesh.save(xxx)
        meshname : str
            The xdmf name to give the mesh
        fieldSavedData : underworld.SavedFileData
            Handler returned from saving a field. underworld.mesh.save(xxx)
        modeltime : float
            The time recorded in the xdmf output file

        Example
        -------
        First create the mesh add a variable:

        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> var = uw.mesh.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double" )

        Write something to variable

        >>> import numpy as np
        >>> var.data[:,0] = np.arange(var.data.shape[0])

        Save mesh and var to a file:

        >>> meshDat = mesh.save("saved_mesh.h5")
        >>> varDat = var.save("saved_mesh_variable.h5", meshDat)

        Now let's create the xdmf file

        >>> var.xdmf("TESTxdmf", varDat, "var1", meshDat, "meshie" )

        Does file exist?

        >>> import os
        >>> if uw.mpi.rank == 0: os.path.isfile("TESTxdmf.xdmf")
        True

        Clean up:

        >>> if uw.mpi.rank == 0:
        ...     import os;
        ...     os.remove( "saved_mesh_variable.h5" )
        ...     os.remove( "saved_mesh.h5" )
        ...     os.remove( "TESTxdmf.xdmf" )

        """
        if uw.mpi.rank == 0:
            if not isinstance(varname, str):
                raise ValueError("'varname' must be of type str")
            if not isinstance(meshname, str):
                raise ValueError("'meshname' must be of type str")
            if not isinstance(filename, str):
                raise ValueError("'filename' must be of type str")
            if not isinstance(meshSavedData, uw.utils.SavedFileData ):
                raise ValueError("'meshSavedData' must be of type SavedFileData")
            if not isinstance(fieldSavedData, uw.utils.SavedFileData ):
                raise ValueError("'fieldSavedData' must be of type SavedFileData")
            if not isinstance(modeltime, (int,float)):
                raise ValueError("'modeltime' must be of type int or float")
            modeltime = float(modeltime)    # make modeltime a float

            elementMesh = self.mesh
            if hasattr(elementMesh.generator, 'geometryMesh'):
                elementMesh = elementMesh.generator.geometryMesh

            # get the elementMesh - if self is a subMeshed variable get the parent
            if elementMesh != meshSavedData.pyobj:
                raise RuntimeError("'meshSavedData file doesn't correspond to the object's mesh")


            if not filename.lower().endswith('.xdmf'):
                filename += '.xdmf'

            # the xmf file is stored in 'string'
            # 1st write header
            string = uw.utils._xdmfheader()
            string += uw.utils._spacetimeschema( meshSavedData, meshname, modeltime )
            string += uw.utils._fieldschema( fieldSavedData, varname )
            # write the footer to the xmf
            string += uw.utils._xdmffooter()

            # write the string to file - only proc 0
            xdmfFH = open(filename, "w")
            xdmfFH.write(string)
            xdmfFH.close()

    def save( self, filename, meshHandle=None, units=None, **kwargs):
        """
        Save the MeshVariable to disk.

        Parameters
        ----------
        filename : string
            The name of the output file. Relative or absolute paths may be
            used, but all directories must exist.
        meshHandle :uw.utils.SavedFileData , optional
            The saved mesh file handle. If provided, a link is created within the
            mesh variable file to this saved mesh file. Important for checkpoint when
            the mesh deforms.
        units : pint unit object (optional)
            Define the units that must be used to save the data.
            The data will be dimensionalised and saved with the defined units.
            The units are saved as a HDF attribute. 
            Note if units are in celsius (see scaling.pint_degc_labels) 
            the data is scaled and save to degrees kelvin. 

        Additional keyword arguments are saved as string attributes.


        Notes
        -----
        This method must be called collectively by all processes.

        Returns
        -------
        underworld.utils.SavedFileData
            Data object relating to saved file. This only needs to be retained
            if you wish to create XDMF files and can be ignored otherwise.

        Example
        -------
        First create the mesh add a variable:

        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> var = uw.mesh.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double" )

        Write something to variable

        >>> import numpy as np
        >>> var.data[:,0] = np.arange(var.data.shape[0])

        Save to a file (note that the 'ignoreMe' object isn't really required):

        >>> meshHandle = mesh.save("saved_mesh.h5")
        >>> ignoreMe = var.save("saved_mesh_variable.h5", meshHandle)
        >>> ignoreMe = var.save("saved_mesh_variable.h5", meshHandle) #test dup

        Now let's try and reload.

        >>> clone_var = uw.mesh.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double" )
        >>> clone_var.load("saved_mesh_variable.h5")

        Now check for equality:

        >>> np.allclose(var.data,clone_var.data)
        True

        Now check the field can be loaded on a different mesh topology (interpolation)

        >>> mesh19 = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(19,19), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> clone_var2 = mesh19.add_variable(1)
        >>> clone_var2.load("saved_mesh_variable.h5", interpolate=True)
        >>> np.allclose(mesh.integrate(var), mesh19.integrate(clone_var2)) 
        True

        >>> # clean up:
        >>> if uw.mpi.rank == 0:
        ...     import os;
        ...     os.remove( "saved_mesh_variable.h5" )
        ...     os.remove( "saved_mesh.h5" )

        """
        from ..utils._io import h5File, h5_require_dataset

        if not isinstance(filename, str):
            raise TypeError("Expected 'filename' to be provided as a string")

        mesh = self.mesh
        with h5File(name=filename, mode="w") as h5f:

            # ugly global shape def
            globalShape = ( mesh.nodesGlobal, self.data.shape[1] )
            # create dataset
            dset = h5_require_dataset(h5f, "data",
                                      shape=globalShape,
                                      dtype=self.data.dtype)

            for kwarg, val in kwargs.items():
                h5f.attrs[str(kwarg)] = str(val)

            # write to the dset using the global node ids
            local = mesh.nodesLocal

            if units:
                xxx = dimensionalise( self.data[0:local], units ).m
                # if values are celsius then convert to kelvin
                if units in pint_degc_labels:
                    units = 'degK'
                    xxx = xxx + 273.15
            else:
                xxx = self.data[0:local]

            with dset.collective:
                dset[mesh.data_nodegId[0:local],:] = xxx

            # Save unit type as attribute
            h5f.attrs['units'] = str(units)

            # save a hdf5 attribute to the elementType used for this field - maybe useful
            h5f.attrs["elementType"] = np.string_(mesh.elementType)

            # setup reference to mesh - THE GEOMETRY MESH
            saveDir = os.path.dirname(filename)

            if hasattr( mesh.generator, "geometryMesh"):
                mesh = mesh.generator.geometryMesh

            # as we're appending we remove the mesh
            if "mesh" in h5f.keys():
                del h5f["mesh"]

            if meshHandle:
                if not isinstance(meshHandle, (str, uw.utils.SavedFileData)):
                    raise TypeError("Expected 'meshHandle' to be of type 'uw.utils.SavedFileData'")

                meshFilename = meshHandle.filename

                if not os.path.exists(meshFilename):
                    raise ValueError("You are trying to link against the mesh file '{}'\n\
                                      that does not appear to exist. If you need to link \n\
                                      against a mesh file, please make sure it is created first.".format(meshFilename))
                # set reference to mesh 
                h5f["mesh"] = h5py.ExternalLink(meshFilename, ".")


        # return our file handle
        return uw.utils.SavedFileData(self, filename)

    def load(self, filename, interpolate=False ):
        """
        Load the MeshVariable from disk.

        Parameters
        ----------
        filename: str
            The filename for the saved file. Relative or absolute paths may be
            used, but all directories must exist.
        interpolate: bool
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
        from ..utils._io import h5File, h5_get_dataset

        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")

        # get field and mesh information
        with h5File(name=filename, mode="r") as h5f:
            dset = h5_get_dataset(h5f,'data')
            dof = dset.shape[1]
            if dof != self.data.shape[1]:
                raise RuntimeError("Can't load hdf5 '{0}', incompatible data shape".format(filename))

            if len(dset) == self.mesh.nodesGlobal:
                # assume dset matches field exactly
                mesh = self.mesh
                local = mesh.nodesLocal

                with dset.collective:
                    self.data[0:local] = dset[mesh.data_nodegId[0:local],:]

            else:
                if not interpolate:
                    raise RuntimeError("Provided data file appears to be for a different resolution MeshVariable.\n"\
                                       "If you would like to interpolate the data to the current variable, please set\n" \
                                       "the 'interpolate' parameter. Check docstring for important caveats of interpolation method.")

                # if here then we build a local version of the entire file field and interpolate it's values

                # first get file field's mesh
                if h5f.get('mesh') == None:
                    raise RuntimeError("The hdf5 field to be loaded with interpolation must have an associated "+
                            "'mesh' hdf5 file. Resave the field with its associated mesh."+
                            "i.e. myField.save(\"filename.h5\", meshFilename)" )
                # get resolution of old mesh
                res = h5f['mesh'].attrs.get('mesh resolution').tolist()
                if res is None:
                    raise RuntimeError("Can't read the 'mesh resolution' for the field hdf5 file,"+
                           " was it created correctly?")

                # get max of old mesh
                inputMax = h5f['mesh'].attrs.get('max').tolist()
                if inputMax is None:
                    raise RuntimeError("Can't read the 'max' for the field hdf5 file,"+
                           " was it created correctly?")

                inputMin = h5f['mesh'].attrs.get('min').tolist()
                if inputMin is None:
                    raise RuntimeError("Can't read the 'min' for the field hdf5 file,"+
                           " was it created correctly?")
                regular = h5f['mesh'].attrs.get('regular')
                if regular and regular!=True:
                    raise RuntimeError("Saved mesh file appears to correspond to a irregular mesh.\n"\
                                       "Interpolating from irregular mesh not currently supported." )

                elType = h5f['mesh'].attrs.get('elementType')
                # for backwards compatiblity, the 'elementType' attribute was added Feb2017
                if elType == None:
                    elType = 'Q1'

                # build the NON-PARALLEL field and mesh
                inputMesh = uw.mesh.FeMesh_Cartesian( elementType = (elType+"/DQ0"), # only geometryMesh can be saved
                                              elementRes  = res,
                                              minCoord    = inputMin,
                                              maxCoord    = inputMax,
                                              partitioned=False)

                # load data onto MeshVariable
                if len(dset) == inputMesh.nodesGlobal:
                    inputField = uw.mesh.MeshVariable( mesh=inputMesh, nodeDofCount=dof )
                elif  dset.shape[0] == inputMesh.subMesh.nodesGlobal:
                    # load as a subMesh
                    # assume the dset field belongs to the subMesh
                    inputField = uw.mesh.MeshVariable( mesh=inputMesh.subMesh, nodeDofCount=dof )
                else:
                    # raise error
                    raise RuntimeError("The saved mesh file can't be read onto the interpolation grid.\n" \
                                       "Note: only subMesh variable with elementType 'DQ0' can be used presently used")

                # copy hdf5 numpy array onto serial inputField
                inputField.data[:] = dset[:]

                # interpolate 'inputField' onto the self nodes
                self.data[:] = inputField.evaluate(self.mesh.data)

            # get units if they have been defined
            if "units" in h5f.attrs.keys():
                units = u.Quantity(h5f.attrs["units"])
            else:
                units = None

            if iunits:
                if iunits.units in pint_degc_labels:
                    import warnings
                    estring = \
                            f"read in file {filename} with offset unit type {iunits.units}. " \
                            f"converting values to when loading from file. "
                    warnings.warn(estring)

                    # load as kelvin
                    xxx = self.data[:] * iunits 
                    self.data[:] = non_dimensionalise(xxx.to_base_units())
                else:
                    self.data[:] = non_dimensionalise(self.data[:]*iunits)

        # add sync
        self.syncronise()

    def copy(self, deepcopy=False):
        """
        This method returns a copy of the meshvariable.

        Parameters
        ----------
        deepcopy: bool
            If True, the variable's data is also copied into
            new variable.

        Returns
        -------
        underworld.mesh.MeshVariable
            The mesh variable copy.

        Example
        -------
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> var = uw.mesh.MeshVariable(mesh,2)
        >>> import math
        >>> var.data[:] = (math.pi,math.exp(1.))
        >>> varCopy = var.copy()
        >>> varCopy.mesh == var.mesh
        True
        >>> varCopy.nodeDofCount == var.nodeDofCount
        True
        >>> import numpy as np
        >>> np.allclose(var.data,varCopy.data)
        False
        >>> varCopy2 = var.copy(deepcopy=True)
        >>> np.allclose(var.data,varCopy2.data)
        True

        """

        if not isinstance(deepcopy, bool):
            raise TypeError("'deepcopy' parameter is expected to be of type 'bool'.")

        newFe = MeshVariable(self.mesh, self.nodeDofCount, self.dataType)

        if deepcopy:
            newFe.data[:] = self.data[:]

        return newFe

    def syncronise(self):
        """
        This method is often necessary when Underworld is operating in parallel.

        It will syncronise the mesh variable so that it is consistent
        with it's parallel neighbours. Specifically, the shadow space of each
        process obtains the required data from neighbouring processes.
        """
        uw.libUnderworld.StgFEM._FeVariable_SyncShadowValues( self._cself )
