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
import os
import weakref

class SwarmVariable(_stgermain.StgClass, function.Function):

    """
    The SwarmVariable class allows users to add data to swarm particles.  The data 
    can be of type "char", "short", "int", "float" or "double".
    
    Note that the swarm allocates one block of contiguous memory for all the particles.
    The per particle variable datums is then interlaced across this memory block.
    
    The recommended practise is to add all swarm variables before populating the swarm
    to avoid costly reallocations.
    
    Swarm variables should be added via the add_variable swarm method.
    
    Parameters
    ----------
    swarm : uw.swarm.Swarm
        The swarm of particles for which we wish to add the variable
    dataType: str
        The data type for the variable. Available types are  "char", 
        "short", "int", "float" or "double".
    count: unsigned
        The number of values to be stored for each particle.
    writeable: bool, default=True
        Signifies if the variable should be writeable.
    """
    _supportedDataTypes = ["char", "short", "int", "float", "double"]

    def __init__(self, swarm=None, dataType=None, count=None, writeable=True, **kwargs):
        
        if not isinstance(swarm, sab.SwarmAbstract):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = weakref.ref(swarm)

        self._arr = None
        self._writeable = writeable
        
        # clear the reference to numpy arrays, as memory layout *will* change.
        swarm._clear_variable_arrays()
        
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
            varname = self.swarm._cself.name+"_"+str(len(self.swarm.variables))
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
        # note that we only return a weakref to the swarm, hence the trailing parenthesis
        return self._swarm()

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
        >>> with swarm.deform_swarm():
        ...     swarm.particleCoordinates.data[0] = [0.2,0.2]
        >>> swarm.particleCoordinates.data[0]
        array([ 0.2,  0.2])
        """
        if self._arr is None:
            self._arr = libUnderworld.StGermain.Variable_getAsNumpyArray(self._cself.variable)
            # set to writeability
            self._arr.flags.writeable = self._writeable
            # add to swarms weakref dict
            self.swarm._livingArrays[self] = self._arr
        return self._arr

    def _clear_array(self):
        """
        This removes the potentially defunct numpy swarm variable memory 
        numpy view. It will be regenerated when required.
        """
        self._arr = None


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

        if self.swarm._checkpointMapsToState != self.swarm.stateId:
            raise RuntimeError("'Swarm' associate with this 'SwarmVariable' does not appear to be in the correct state.\n" \
                               "Please ensure that you have loaded the swarm prior to loading any swarm variables.")
        gIds = self.swarm._local2globalMap

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
            bar = uw.utils._ProgressBar( start=0, end=size-1, title="loading "+filename)

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
            
        Returns
        -------
        SavedFileData
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
                                   dtype=self.data.dtype)
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

        return uw.utils.SavedFileData( self, filename )

    def xdmf( self, filename, varSavedData, varname, swarmSavedData, swarmname, modeltime=0.  ):
        """
        Creates an xdmf file, filename, associating the varSavedData file on 
        the swarmSavedData file

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
            The xdmf name to give the swarmVariable
        swarmname : str
            The xdmf name to give the swarm
        swarmSavedData : underworld.SaveFileData
            Handler returned for saving a swarm. underworld.swarm.Swarm.save(xxx)
        varSavedData : underworld.SavedFileData
            Handler returned from saving a SwarmVariable. underworld.swarm.SwarmVariable.save(xxx)
        modeltime : float (default 0.0)
            The time recorded in the xdmf output file

        Example TODO
        -------
        
        First create the swarm and add a variable:
        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> swarm = uw.swarm.Swarm( mesh=mesh )
        >>> swarmLayout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
        >>> swarm.populate_using_layout( layout=swarmLayout )
        >>> swarmVar = swarm.add_variable( dataType="int", count=1 )
        
        Write something to variable
        
        >>> import numpy as np
        >>> swarmVar.data[:,0] = np.arange(swarmVar.data.shape[0])
        
        Save mesh and var to a file:
        
        >>> swarmDat = swarm.save("saved_swarm.h5")
        >>> swarmVarDat = swarmVar.save("saved_swarmvariable.h5")
        
        Now let's create the xdmf file
        
        >>> swarmVar.xdmf("TESTxdmf", swarmVarDat, "var1", swarmDat, "MrSwarm" )

        Does file exist?
        
        >>> import os
        >>> if uw.rank() == 0: os.path.isfile("TESTxdmf.xdmf")
        True
        
        Clean up:
        >>> if uw.rank() == 0:
        ...     import os; 
        ...     os.remove( "saved_swarm.h5" )
        ...     os.remove( "saved_swarmvariable.h5" )
        ...     os.remove( "TESTxdmf.xdmf" )

        """
        if uw.rank() == 0:
            if not isinstance(varname, str):
                raise ValueError("'varname' must be of type str")
            if not isinstance(swarmname, str):
                raise ValueError("'swarmname' must be of type str")
            if not isinstance(filename, str):
                raise ValueError("'filename' must be of type str")
            if not isinstance(swarmSavedData, uw.utils.SavedFileData ):
                raise ValueError("'swarmSavedData' must be of type SavedFileData")
            if not isinstance(varSavedData, uw.utils.SavedFileData ):
                raise ValueError("'varSavedData' must be of type SavedFileData")
            if not isinstance(modeltime, (int,float)):
                raise ValueError("'modeltime' must be of type int or float")
            modeltime = float(modeltime)    # make modeltime a float
            
            # get the elementMesh - if self is a subMeshed variable get the parent
            if self.swarm != swarmSavedData.pyobj:
                raise RuntimeError("'swarmSavedData file doesn't correspond to the object's swarm")

            if not filename.lower().endswith('.xdmf'):
                filename += '.xdmf'

            # the xmf file is stored in 'string'
            # 1st write header
            string = uw.utils._xdmfheader()
            """
            ("<?xml version=\"1.0\" ?>\n" +
                      "<Xdmf xmlns:xi=\"http://www.w3.org/2001/XInclude\" Version=\"2.0\">\n" +
                      "<Domain>\n")
            """

            string += uw.utils._swarmspacetimeschema(swarmSavedData, swarmname, modeltime )
            string += uw.utils._swarmvarschema( varSavedData, varname )
            # write the footer to the xmf    
            string += uw.utils._xdmffooter()
            """
            string += ("</Grid>\n" + 
                       "</Domain>\n" + 
                       "</Xdmf>\n" )
            """

            # write the string to file - only proc 0
            xdmfFH = open(filename, "w")
            xdmfFH.write(string)
            xdmfFH.close()
