# Utilities to convert between dimensional and non-dimensional values.
# Romain BEUCHER, December 2016

import pint
import underworld as uw
import h5py
import numpy as np
import os

from mpi4py import MPI
from ._utils import TransformedDict

u = pint.UnitRegistry()
UnitRegistry = u

scaling = TransformedDict()

scaling["[time]"]   = 1.0 * u.second
scaling["[length]"] = 1.0 * u.meter 
scaling["[mass]"] = 1.0 * u.kilogram 
scaling["[temperature]"] = 1.0 * u.degK
scaling["[substance]"] =1. * u.mole


def nonDimensionalize(dimValue):
    """
    This function uses pint object to perform a dimension analysis and
    return a value scaled according to a set of scaling coefficients:

    example:

    import unsupported.scaling as sca

    u = sca.UnitRegistry

    # Characteristic values of the system
    half_rate = 0.5 * u.centimeter / u.year
    model_height = 600e3 * u.meter
    refViscosity = 1e24 * u.pascal * u.second
    surfaceTemp = 0. * u.degK
    baseModelTemp = 1330. * u.degC
    baseCrustTemp = 550. * u.degC
    
    KL_meters = model_height
    KT_seconds = KL_meters / half_rate
    KM_kilograms = refViscosity * KL_meters * KT_seconds
    Kt_degrees = (baseModelTemp - surfaceTemp)
    K_substance = 1. * u.mole

    sca.scaling["[time]"] = KT_seconds
    sca.scaling["[length]"] = KL_meters 
    sca.scaling["[mass]"] = KM_kilograms 
    sca.scaling["[temperature]"] = Kt_degrees
    sca.scaling["[substance]"] -= K_substance

    # Get a scaled value:
    gravity = nonDimensionalize(9.81 * u.meter / u.second**2)
    """
    global scaling

    if not isinstance(dimValue, u.Quantity):
        return dimValue
    
    dimValue = dimValue.to_base_units()
    
    length = scaling["[length]"]
    time = scaling["[time]"]
    mass = scaling["[mass]"]
    temperature = scaling["[temperature]"]
    substance = scaling["[substance]"]
    
    length = length.to_base_units()
    time = time.to_base_units()
    mass = mass.to_base_units()
    temperature = temperature.to_base_units()
    substance = substance.to_base_units()
    
    @u.check('[length]','[time]', '[mass]', '[temperature]', '[substance]')
    def check(length, time, mass, temperature, substance):
        return
    
    check(length, time, mass, temperature, substance)

    # Get dimensionality
    dlength = dimValue.dimensionality['[length]']
    dtime   = dimValue.dimensionality['[time]']
    dmass   = dimValue.dimensionality['[mass]']
    dtemp   = dimValue.dimensionality['[temperature]']
    dsubstance   = dimValue.dimensionality['[substance]']
    factor = (length**(-dlength) *
              time**(-dtime) *
              mass**(-dmass) *
              temperature**(-dtemp)*
              substance**(-dsubstance))
    
    dimValue *= factor
    
    if dimValue.unitless :
        return dimValue.magnitude
    else:
        raise ValueError('Dimension Error')

def Dimensionalize(Value, units):
   
    global scaling

    unit = (1.0 * units).to_base_units()
    
    length = scaling["[length]"]
    time = scaling["[time]"]
    mass = scaling["[mass]"]
    temperature = scaling["[temperature]"]
    substance = scaling["[substance]"]
    
    length = length.to_base_units()
    time = time.to_base_units()
    mass = mass.to_base_units()
    temperature = temperature.to_base_units()
    substance = substance.to_base_units()
    
    @u.check('[length]','[time]', '[mass]', '[temperature]', '[substance]')
    def check(length, time, mass, temperature, substance):
        return
    
    # Check that the scaling parameters have the correct dimensions
    check(length, time, mass, temperature, substance)
    
    # Get dimensionality
    dlength = unit.dimensionality['[length]']
    dtime   = unit.dimensionality['[time]']
    dmass   = unit.dimensionality['[mass]']
    dtemp   = unit.dimensionality['[temperature]']
    dsubstance   = unit.dimensionality['[substance]']
    factor = (length**(dlength) *
              time**(dtime) *
              mass**(dmass) *
              temperature**(dtemp)*
              substance**(dsubstance))    
    
    if (isinstance(Value, uw.mesh._meshvariable.MeshVariable) or 
       isinstance(Value, uw.swarm._swarmvariable.SwarmVariable)) :
        tempVar = Value.copy()
        tempVar.data[...] = (Value.data[...] * factor).to(units)
        return tempVar
    else:
        return (Value * factor).to(units)



## Following functions are temporary, far from ideal...

def _save_mesh( self, filename, units=None, scaling=False):
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

    if units == None:
        units = u.meter
    
    if scaling:
        h5f.attrs['max'] = Dimensionalize(self.maxCoord, units)
        h5f.attrs['min'] = Dimensionalize(self.minCoord, units)
    else:
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
    if scaling:
        vals = Dimensionalize(self.data[0:local], units)
    else:
        vals = self.data[0:local]

    dset[self.data_nodegId[0:local],:] = vals

    # write the element node connectivity
    self.data_elementNodes
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

def _save_meshVariable( self, filename, meshHandle=None, units=None,
        scaling=False):
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

    >>> ignoreMe = var.save("saved_mesh_variable.h5")

    Now let's try and reload.

    >>> clone_var = uw.mesh.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double" )
    >>> clone_var.load("saved_mesh_variable.h5")

    Now check for equality:

    >>> np.allclose(var.data,clone_var.data)
    True

    >>> # clean up:
    >>> if uw.rank() == 0:
    ...     import os;
    ...     os.remove( "saved_mesh_variable.h5" )

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
                              dtype=self.data.dtype)

    # write to the dset using the global node ids
    local = mesh.nodesLocal

    if scaling:
        if units == None:
            raise ValueError("units not specified")
        vals = Dimensionalize(self.data[0:local], units)
    else:
        vals = self.data[0:local]

    dset[mesh.data_nodegId[0:local],:] = vals

    # save a hdf5 attribute to the elementType used for this field - maybe useful
    h5f.attrs["elementType"] = np.string_(mesh.elementType)

    ## setup reference to mesh - THE GEOMETRY MESH
    saveDir = os.path.dirname(filename)

    if hasattr( mesh.generator, "geometryMesh"):
        mesh = mesh.generator.geometryMesh

    if meshHandle:
        if not isinstance(meshHandle, (str, uw.utils.SavedFileData)):
            raise TypeError("Expected 'meshHandle' to be of type 'uw.utils.SavedFileData'")

        if isinstance(meshHandle, str):
            # DEPRECATION check
            import warnings
            warnings.warn("'meshHandle' paramater should be of type uw.utils.SaveFileData. Please update your models. "+
                          "Accepting 'meshHandle' as a string parameter will be removed in the next release.")
            meshFilename = meshHandle
        else:
            meshFilename = meshHandle.filename

        if not os.path.exists(meshFilename):
            raise ValueError("You are trying to link against the mesh file '{}'\n\
                              that does not appear to exist. If you need to link \n\
                              against a mesh file, please make sure it is created first.".format(meshFilename))
        # set reference to mesh (all procs must call following)
        h5f["mesh"] = h5py.ExternalLink(meshFilename, "./")

    h5f.close()

    # return our file handle
    return uw.utils.SavedFileData(self, filename)

def _save_swarmVariable( self, filename, units=None, scaling=False):
    """
    Save the swarm variable to disk.

    Parameters
    ----------
    filename : str
        The filename for the saved file. Relative or absolute paths may be
        used, but all directories must exist.
    swarmHandle :uw.utils.SavedFileData , optional
        The saved swarm file handle. If provided, a reference to the swarm file
        is made. Currently this doesn't provide any extra functionality.

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
    First create the swarm, populate, then add a variable:

    >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
    >>> svar = swarm.add_variable("int",1)

    Write something to variable

    >>> import numpy as np
    >>> svar.data[:,0] = np.arange(swarm.particleLocalCount)

    Save to a file:

    >>> ignoreMe = swarm.save("saved_swarm.h5")
    >>> ignoreMe = svar.save("saved_swarm_variable.h5")

    Now let's try and reload. First create a new swarm and swarm variable,
    and then load both:

    >>> clone_swarm = uw.swarm.Swarm(mesh)
    >>> clone_svar = clone_swarm.add_variable("int",1)
    >>> clone_swarm.load("saved_swarm.h5")
    >>> clone_svar.load("saved_swarm_variable.h5")

    Now check for equality:

    >>> import numpy as np
    >>> np.allclose(svar.data,clone_svar.data)
    True

    >>> # clean up:
    >>> if uw.rank() == 0:
    ...     import os;
    ...     os.remove( "saved_swarm.h5" )
    ...     os.remove( "saved_swarm_variable.h5" )

    """

    if not isinstance(filename, str):
        raise TypeError("'filename' parameter must be of type 'str'")

    # setup mpi basic vars
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    nProcs = comm.Get_size()

    # allgather the number of particles each proc has
    swarm = self.swarm
    procCount = comm.allgather(swarm.particleLocalCount)
    particleGlobalCount = np.sum(procCount) #swarm.particleGlobalCount

    # calculate the hdf5 file offset
    offset=0
    for i in xrange(rank):
        offset += procCount[i]

    # open parallel hdf5 file
    h5f = h5py.File(name=filename, mode="w", driver='mpio', comm=MPI.COMM_WORLD)
    
    # attribute of the proc offsets - used for loading from checkpoint
    h5f.attrs["proc_offset"] = procCount
    
    # write the entire local swarm to the appropriate offset position
    globalShape = (particleGlobalCount, self.data.shape[1])
    dset = h5f.create_dataset("data",
                               shape=globalShape,
                               dtype=self.data.dtype)

    if swarm.particleLocalCount > 0: # only add if there are local particles
        if scaling:
            if units == None:
                raise ValueError("units not specified")
            vals = Dimensionalize(self.data[:], units)
        else:
            vals = self.data[:]

        dset[offset:offset+swarm.particleLocalCount] = vals

    h5f.close()

    return uw.utils.SavedFileData( self, filename )

def _save_swarm(self, filename, units=None, scaling=False):
    """
    Save the swarm to disk.

    Parameters
    ----------
    filename : str
        The filename for the saved file. Relative or absolute paths may be
        used, but all directories must exist.

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
    First create the swarm, and populate with layout:

    >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))

    Save to a file:

    >>> ignoreMe = swarm.save("saved_swarm.h5")

    Now let's try and reload. First create an empty swarm, and then load:

    >>> clone_swarm = uw.swarm.Swarm(mesh)
    >>> clone_swarm.load( "saved_swarm.h5" )

    Now check for equality:

    >>> import numpy as np
    >>> np.allclose(swarm.particleCoordinates.data,clone_swarm.particleCoordinates.data)
    True

    >>> # clean up:
    >>> if uw.rank() == 0:
    ...     import os;
    ...     os.remove( "saved_swarm.h5" )

    """

    if not isinstance(filename, str):
        raise TypeError("Expected filename to be provided as a string")

    # just save the particle coordinates SwarmVariable
    if scaling:
        if units == None:
            raise ValueError("units not specified")
    self.particleCoordinates.save(filename, scaling=scaling, units=units)

    return uw.utils.SavedFileData( self, filename )


def _load_mesh(self, filename, units=None):
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
        if units:
            vals = dset[self.data_nodegId[0:self.nodesLocal],:]
            vals = nonDimensionalize(vals*units) 
        else:
            vals = dset[self.data_nodegId[0:self.nodesLocal],:]   

        self.data[0:self.nodesLocal] = vals

    h5f.close()

def _load_meshVariable(self, filename, interpolate=False, units=None):
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
            raise RuntimeError("The hdf5 field to be loaded with interpolation must have an associated "+
                    "'mesh' hdf5 file. Resave the field with its associated mesh."+
                    "i.e. myField.save(\"filename.h5\", meshFilename)" )
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

        elType = h5f['mesh'].attrs.get('elementType')
        # for backwards compatiblity, the 'elementType' attribute was added Feb2017
        if elType == None:
            elType = 'Q1'

        # build the NON-PARALLEL field and mesh
        inputMesh = uw.mesh.FeMesh_Cartesian( elementType = (elType+"/DQ0"), # only geometryMesh can be saved
                                      elementRes  = tuple(res),
                                      minCoord    = tuple(inputMin),
                                      maxCoord    = tuple(inputMax),
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
    
    if units:
        self.data[:] = nonDimensionalize(self.data[:]*units) 

    uw.libUnderworld.StgFEM._FeVariable_SyncShadowValues( self._cself )
    h5f.close()

def _load_swarm( self, filename, try_optimise=True, verbose=False, units=None ):
    """
    Load a swarm from disk. Note that this must be called before any SwarmVariable
    members are loaded.

    Parameters
    ----------
    filename : str
        The filename for the saved file. Relative or absolute paths may be
        used.
    try_optimise : bool, Default=True
        Will speed up the swarm load time but warning - this algorithm assumes the 
        previously saved swarm data was made on an identical mesh and mesh partitioning 
        (number of processors) with respect to the current mesh. If this isn't the case then
        the reloaded particle ordering will be broken, leading to an invalid swarms.
        One can disable this optimisation and revert to a brute force algorithm, much slower,
        by setting this option to False.
    verbose : bool
        Prints a swarm load progress bar.

    Notes
    -----
    This method must be called collectively by all processes.

    Example
    -------
    Refer to example provided for 'save' method.

    """

    if not isinstance(filename, str):
        raise TypeError("Expected 'filename' to be provided as a string")
    
    # open hdf5 file
    h5f = h5py.File(name=filename, mode="r", driver='mpio', comm=MPI.COMM_WORLD)

    dset = h5f.get('data')
    if dset == None:
        raise RuntimeError("Can't find 'data' in file '{0}'.\n".format(filename))
    if dset.shape[1] != self.particleCoordinates.data.shape[1]:
        raise RuntimeError("Cannot load file data on current swarm. Data in file '{0}', " \
                           "has {1} components -the particlesCoords has {2} components".format(filename, dset.shape[1], self.particleCoordinates.data.shape[1]))
    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    nProcs = comm.Get_size()
    
    if rank == 0 and verbose:
        bar = uw.utils._ProgressBar( start=0, end=dset.shape[0]-1, title="loading "+filename)
    
    # try and read the procCount attribute & assume that if nProcs in .h5 file
    # is equal to the current no. procs then the particles will be distributed the 
    # same across the processors. (Danger if different discretisations are used... i think)
    # else try and load the whole .h5 file.
    # we set the 'offset' & 'size' variables to achieve the above 
    
    offset = 0
    totalsize = size = dset.shape[0] # number of particles in h5 file
    
    if try_optimise:
        procCount = h5f.attrs.get('proc_offset')
        if procCount is not None and nProcs == len(procCount):
            for p_i in xrange(rank):
                offset += procCount[p_i]
            size = procCount[rank]
        
    valid = np.zeros(0, dtype='i') # array for read in
    chunk=int(1e4) # read in this many points at a time

    (multiples, remainder) = divmod( size, chunk )
    for ii in xrange(multiples+1):
        # setup the points to begin and end reading in
        chunkStart = offset + ii*chunk
        if ii == multiples:
            chunkEnd = chunkStart + remainder
            if remainder == 0: # in the case remainder is 0
                break
        else:
            chunkEnd = chunkStart + chunk

        # add particles to swarm, ztmp is the corresponding local array
        # non-local particles are not added and their ztmp index is -1
        if units:
            vals = nonDimensionalize(dset[ chunkStart : chunkEnd] * units)
            ztmp = self.add_particles_with_coordinates(vals)
        else:
            ztmp = self.add_particles_with_coordinates(dset[ chunkStart : chunkEnd ])

        tmp = np.copy(ztmp) # copy because ztmp is 'readonly'

        # slice out -neg bits and make the local indices global
        it = np.nditer(tmp, op_flags=['readwrite'], flags=['f_index'])
        while not it.finished:
            if it[0] >= 0:
                it[0] = chunkStart+it.index # local to global
            it.iternext()

        # slice out -neg bits
        tmp = tmp[tmp[:]>=0]
        # append to valid
        valid = np.append(valid, tmp)

        if rank == 0 and verbose:
            bar.update(chunkEnd)

    h5f.close()
    self._local2globalMap = valid
    # record which swarm state this corresponds to
    self._checkpointMapsToState = self.stateId

    
def _load_swarmVariable( self, filename, units=None):
    """
    Load the swarm variable from disk. This must be called *after* the swarm.load().

    Parameters
    ----------
    filename : str
        The filename for the saved file. Relative or absolute paths may be
        used, but all directories must exist.

    Notes
    -----
    This method must be called collectively by all processes.


    Example
    -------
    Refer to example provided for 'save' method.

    """

    if not isinstance(filename, str):
        raise TypeError("'filename' parameter must be of type 'str'")

    if self.swarm._checkpointMapsToState != self.swarm.stateId:
        raise RuntimeError("'Swarm' associate with this 'SwarmVariable' does not appear to be in the correct state.\n" \
                           "Please ensure that you have loaded the swarm prior to loading any swarm variables.")
    gIds = self.swarm._local2globalMap

    comm = MPI.COMM_WORLD
    rank = comm.Get_rank()
    nProcs = comm.Get_size()

    # open hdf5 file
    h5f = h5py.File(name=filename, mode="r", driver='mpio', comm=MPI.COMM_WORLD)

    dset = h5f.get('data')
    if dset == None:
        raise RuntimeError("Can't find 'data' in file '{}'.\n".format(filename))

    if dset.shape[1] != self.data.shape[1]:
        raise RuntimeError("Cannot load file data on current swarm. Data in file '{0}', " \
                           "has {1} components -the particlesCoords has {2} components".format(filename, dset.shape[1], self.particleCoordinates.data.shape[1]))

    particleGobalCount = self.swarm.particleGlobalCount
    
    if dset.shape[0] != particleGobalCount:
        if rank == 0:
            import warnings
            warnings.warn("Warning, it appears {} particles were loaded, but this h5 variable has {} data points". format(particleGobalCount, dset.shape[0]), RuntimeWarning)

    size = len(gIds) # number of local2global mapped indices
    if size > 0:     # only if there is a non-zero local2global do we load
        if units:
            self.data[:] = nonDimensionalize(dset[gIds,:] * units)
        else:
            self.data[:] = dset[gIds,:]

    h5f.close();


uw.mesh.FeMesh.save = _save_mesh
uw.mesh.MeshVariable.save = _save_meshVariable
uw.swarm.Swarm.save = _save_swarm
uw.swarm.SwarmVariable.save = _save_swarmVariable

uw.mesh.FeMesh.load = _load_mesh
uw.mesh.MeshVariable.load = _load_meshVariable
uw.swarm.Swarm.load = _load_swarm
uw.swarm.SwarmVariable.load = _load_swarmVariable

