# Utilities to convert between dimensional and non-dimensional values.
# Romain BEUCHER, December 2016

import pint
import underworld as uw
import h5py
import numpy as np
import os

from mpi4py import MPI

u = pint.UnitRegistry()
UnitRegistry = u


scaling = {"[time]": 1.0 * u.second,
           "[length]": 1.0 * u.meter, 
           "[mass]": 1.0 * u.kilogram, 
           "[temperature]": 1.0 * u.degK,
           "[substance]": 1. * u.mole}


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

    # import pdb; pdb.set_trace()
    # open parallel hdf5 file
    h5f = h5py.File(name=filename, mode="w", driver='mpio', comm=MPI.COMM_WORLD)
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

uw.mesh.FeMesh.save = _save_mesh
uw.mesh.MeshVariable.save = _save_meshVariable
uw.swarm.Swarm.save = _save_swarm
uw.swarm.SwarmVariable.save = _save_swarmVariable
