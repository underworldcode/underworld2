##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld as uw
import underworld._stgermain as _stgermain
import underworld.mesh as mesh
import numpy as np
import libUnderworld
import _swarmabstract as sab
import _swarm
import underworld.function as function
import libUnderworld.libUnderworldPy.Function as _cfn
from mpi4py import MPI
import h5py

class SwarmVariable(_stgermain.StgClass, function.Function):

    """
    The SwarmVariable class allows users to add data to swarm particles.  The data 
    can be of type "char", "short", "int", "float" or "double".
    
    Note that the swarm allocates one block of contiguous memory for all the particles.
    The per particle variable datums is then interlaced across this memory block.
    
    It is most best practise to add all swarm variables before populating the swarm
    to avoid costly reallocations.
    
    Swarm variables should be added via the add_variable swarm method.
    
    """
    _supportedDataTypes = ["char", "short", "int", "float", "double"]

    def __init__(self, swarm=None, dataType=None, count=None, **kwargs):
        """
        Parameters
        ----------
        swarm : uw.swarm.Swarm
            The swarm of particles for which we wish to add the variable
        dataType: str
            The data type for the variable. Available types are  "char", 
            "short", "int", "float" or "double".
        count: unsigned
            The number of values to be stored for each particle.
        
        """
        
        if not isinstance(swarm, sab.SwarmAbstract):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = swarm
        
        if len(swarm._livingArrays) != 0:
            raise RuntimeError("""
            There appears to be {} swarm variable numpy array objects still in
            existance. When a new swarm variable is added, it results in the modification
            of existing swarm variable memory layouts and locations, and therefore
            existing numpy array views of swarm variables will cease to be valid. Potential
            modification of these invalid numpy arrays is dangerous, and therefore they must
            be removed before a new variable can be added.  The python 'del' command may be useful,
            though be aware that an object cannot be destroyed while another object retains a
            reference to it. Once you have added the required swarm variables, you can easily
            regenerate the numpy views of other variables again using the 'data' property.""".format(len(swarm._livingArrays)))

        
        if not isinstance(dataType,str):
            raise TypeError("'dataType' object passed in must be of type 'str'")
        if dataType.lower() not in self._supportedDataTypes:
            raise ValueError("'dataType' provided ({}) does not appear to be supported. \nSupported types are {}.".format(dataType.lower(),self._supportedDataTypes))
        self._dataType = dataType.lower()
        
        if not isinstance(count,int) or (count<1):
            raise TypeError("Provided 'count' must be a positive integer.")
        self._count = count
        
        if self._dataType == "double" :
            dtype = libUnderworld.StGermain.Variable_DataType_Double;
        elif self._dataType == "float" :
            dtype = libUnderworld.StGermain.Variable_DataType_Float;
        elif self._dataType == "int" :
            dtype = libUnderworld.StGermain.Variable_DataType_Int;
        elif self._dataType == "char" :
            dtype = libUnderworld.StGermain.Variable_DataType_Char;
        elif self._dataType == "short" :
            dtype = libUnderworld.StGermain.Variable_DataType_Short;

        # first, check if we were passed in a cself pointer, in which case we are purely wrapping a pre-exisiting swarmvar
        if "_cself" in kwargs:
            self._cself = kwargs["_cself"]
            if self._cself.swarm.name != swarm._cself.name:
                raise ValueError("Passed in cself object's swarm must be same as that provided in arguments")
            if self._cself.dofCount != self.count:
                raise ValueError("Passed in cself object's dofcount must be same as that provided in arguments")
            # note that we aren't checking the datatype
        else:
            varname = self._swarm._cself.name+"_"+str(len(self.swarm.variables))
            self._cself = libUnderworld.StgDomain.Swarm_NewVectorVariable(self.swarm._cself, varname, -1, dtype, count )
            libUnderworld.StGermain.Stg_Component_Build( self._cself, None, False );
            libUnderworld.StGermain.Stg_Component_Initialise( self._cself, None, False );
        
        self.swarm.variables.append(self)
        
        # lets realloc swarm now
        libUnderworld.StgDomain.Swarm_Realloc(self.swarm._cself)
        
        # create function guy
        self._fncself = _cfn.SwarmVariableFn(self._cself)

        # build parent
        super(SwarmVariable,self).__init__(argument_fns=None, **kwargs)

        self._underlyingDataItems.add(self) # add weakref to self in here.. note this must occur after call to super.

    @property
    def swarm(self):
        """    
        swarm (Swarm): The swarm this variable belongs to.
        """
        return self._swarm

    @property
    def dataType(self):
        """
        dataType (str): Data type for variable.  Supported types are 'char', 
        'short', 'int', 'float' and 'double'.
        """
        return self._dataType

    @property
    def count(self):
        """
        count (int): Number of data items for this variable stored on each particle.
        """
        return self._count

    
    @property
    def data(self):
        """    
        data (np.array):  Numpy proxy array to underlying variable data.
        Note that the returned array is a proxy for all the *local* particle
        data.
        
        As numpy arrays are simply proxys to the underlying memory structures.
        no data copying is required.
        
        >>> # create mesh
        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> # create empty swarm
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> # add a variable
        >>> svar = swarm.add_variable("int",1)
        >>> # add particles
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
        >>> swarm.particleLocalCount
        1024
        >>> len(svar.data)  # should be the same as particle local count
        1024
        >>> swarm.owningCell.data  # check particle owning cells/elements.
        array([[  0],
               [  0],
               [  0],
               ..., 
               [255],
               [255],
               [255]], dtype=int32)

        >>> # particle coords
        >>> swarm.particleCoordinates.data[0]
        array([ 0.0132078,  0.0132078])
        >>> # move the particle
        >>> swarm.particleCoordinates.data[0] = [.1,.1]
        >>> swarm.particleCoordinates.data[0]
        array([ 0.1,  0.1])
        >>> # don't forget to update owners after performing any moves
        >>> swarm.update_particle_owners()
        """
        arrayguy = libUnderworld.StGermain.Variable_getAsNumpyArray(self._cself.variable)
        # add to swarms weakref dict
        self.swarm._livingArrays[self] = arrayguy
        return arrayguy

    def load( self, filename, verbose=False ):
        """
        Load the swarm variable from disk. This must be called *after* the swarm.load().
        
        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be 
            used, but all directories must exist.
        verbose : bool
            Prints a swarm variable load progress bar.
            
        Notes
        -----
        This method must be called collectively by all processes.
        

        Example
        -------
        Refer to example provided for 'save' method.
        
        """

        if not isinstance(filename, str):
            raise TypeError("'filename' parameter must be of type 'str'")
        
        gIds = self.swarm._local2globalMap
        if not isinstance(gIds, np.ndarray):
            raise RuntimeError("'Swarm' associate with this 'SwarmVariable' doesn't have a valid '_local2globalMap'.\n" \
                               "Please ensure that you have loaded the swarm prior to loading any swarm variables.")

        comm = MPI.COMM_WORLD
        rank = comm.Get_rank()

        # open hdf5 file
        h5f = h5py.File(name=filename, mode="r", driver='mpio', comm=MPI.COMM_WORLD)

        dset = h5f.get('data')
        if dset == None:
            raise RuntimeError("Can't find 'data' in file '{}'.\n".format(filename))
        particleGobalCount = self.swarm.particleGlobalCount
        if dset.shape[0] != particleGobalCount:
            raise RuntimeError("Cannot load {0}'s data on current swarm. Incompatible numbers of particles in file '{1}'.".format(filename, filename)+
                    " Particle count: file {0}, this swarm {1}\n".format(dset.shape[0], particleGobalCount))
        size = len(gIds)
        if self.data.shape[0] != size:
            raise RuntimeError("Invalid mapping from file '{0}' to swarm.\n".format(filename) +
                 "Ensure the swarm corresponds exactly to the file '{0}' by loading the swarm immediately".format(filename) +
                    "before this 'SwarmVariable' load\n")
        if dset.shape[1] != self.data.shape[1]:
            raise RuntimeError("Cannot load file data on current swarm. Data in file '{0}', " \
                               "has {1} components -the particlesCoords has {2} components".format(filename, dset.shape[1], self.particleCoordinates.data.shape[1]))

        chunk = int(1e3)
        (multiples, remainder) = divmod( size, chunk )

        if rank == 0 and verbose:
            bar = uw.utils.ProgressBar( start=0, end=size-1, title="loading "+filename)

        for ii in xrange(multiples+1):
            chunkStart = ii*chunk
            if ii == multiples:
                chunkEnd = chunkStart + remainder
                if remainder == 0:
                    break
            else:
                chunkEnd = chunkStart + chunk
            self.data[chunkStart:chunkEnd] = dset[gIds[chunkStart:chunkEnd],:] 

            if rank == 0 and verbose:
                bar.update(chunkEnd)

        h5f.close();


    def save( self, filename, swarmFilepath=None ):
        """
        Save the swarm variable to disk.
        
        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be 
            used, but all directories must exist.
        swarmFilepath : str (optional)
            Path to the save swarm file. If provided, a softlink is created within
            the swarm variable file to the swarm file.
            
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
        
        >>> swarm.save("saved_swarm.h5")
        >>> svar.save("saved_swarm_variable.h5")
        
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
        
        Clean up:
        >>> if uw.rank() == 0:
        ...     import os; 
        ...     os.remove( "saved_swarm.h5" )
        ...     os.remove( "saved_swarm_variable.h5" )

        """
        
        if swarmFilepath:
            raise RuntimeError("The 'swarmFilepath' option is currently disabled.")
        if not isinstance(filename, str):
            raise TypeError("'filename' parameter must be of type 'str'")

        # setup mpi basic vars
        comm = MPI.COMM_WORLD
        rank = comm.Get_rank()
        nProcs = comm.Get_size()

        # allgather the number of particles each proc has
        swarm = self.swarm
        procCount = comm.allgather(swarm.particleLocalCount)
        particleGlobalCount = swarm.particleGlobalCount

        # calculate the hdf5 file offset
        offset=0
        for i in xrange(rank):
            offset += procCount[i]

        # open parallel hdf5 file
        h5f = h5py.File(name=filename, mode="w", driver='mpio', comm=MPI.COMM_WORLD)
        globalShape = (particleGlobalCount, self.data.shape[1])
        dset = h5f.create_dataset("data", 
                                   shape=globalShape,
                                   dtype='f')
        dset[offset:offset+swarm.particleLocalCount] = self.data[:]

        # link to the swarm file if it's provided
        if swarmFilepath and uw.rank()==0:
            import os
            if not isinstance(swarmFilepath, str):
                raise TypeError("'swarmFilepath' parameter must be of type 'str'")
        
            if not os.path.exists(swarmFilepath):
                raise RuntimeError("Swarm file '{}' does not appear to exist.".format(swarmFilepath))
            # path trickery to create external
            (dirname, swarmfile) = os.path.split(swarmFilepath)
            if dirname == "":
                dirname = '.'
            h5f["swarm"] = h5py.ExternalLink(swarmfile, dirname)

        h5f.close()

