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
from . import _swarmabstract as sab
from . import _swarm
import underworld.function as function
import libUnderworld.libUnderworldPy.Function as _cfn
from mpi4py import MPI
import h5py
import os
import weakref

class SwarmVariable(_stgermain.StgClass, function.Function):

    """
    The SwarmVariable class allows users to add data to swarm particles.  The data
	can be of type "char", "short", "int", "long, "float" or "double".

    Note that the swarm allocates one block of contiguous memory for all the particles.
    The per particle variable datums is then interlaced across this memory block.

    The recommended practise is to add all swarm variables before populating the swarm
    to avoid costly reallocations.

    Swarm variables should be added via the add_variable swarm method.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The swarm of particles for which we wish to add the variable
    dataType: str
        The data type for the variable. Available types are  "char",
        "short", "int", "long", "float" or "double".
    count: unsigned
        The number of values to be stored for each particle.
    writeable: bool
        Signifies if the variable should be writeable.
    """
    _supportedDataTypes = ["char", "short", "int", "long", "float", "double"]

    def __init__(self, swarm, dataType, count, writeable=True, **kwargs):

        if not isinstance(swarm, sab.SwarmAbstract):
            raise TypeError("'swarm' object passed in must be of type 'Swarm'")
        self._swarm = weakref.ref(swarm)

        self._arr = None
        self._arrshadow = None
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
            dtype = libUnderworld.StGermain.StgVariable_DataType_Double;
        elif self._dataType == "float" :
            dtype = libUnderworld.StGermain.StgVariable_DataType_Float;
        elif self._dataType == "int" :
            dtype = libUnderworld.StGermain.StgVariable_DataType_Int;
        elif self._dataType == "long" :
            dtype = libUnderworld.StGermain.StgVariable_DataType_Long;
        elif self._dataType == "char" :
            dtype = libUnderworld.StGermain.StgVariable_DataType_Char;
        elif self._dataType == "short" :
            dtype = libUnderworld.StGermain.StgVariable_DataType_Short;

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
        Returns
        -------
        underworld.swarm.Swarm
            The swarm this variable belongs to.
        """
        # note that we only return a weakref to the swarm, hence the trailing parenthesis
        return self._swarm()

    @property
    def dataType(self):
        """
        Returns
        -------
        str
            Data type for variable.  Supported types are 'char', 'short', 'int', 'long',
            'float' and 'double'.
        """
        return self._dataType

    @property
    def count(self):
        """
        Returns
        -------
        int
            Number of data items for this variable stored on each particle.
        """
        return self._count


    @property
    def data(self):
        """
        Returns
        -------
        numpy.ndarray
            Numpy proxy array to underlying variable data. Note that the 
            returned array is a proxy for all the *local* particle data. As 
            numpy arrays are simply proxys to the underlying memory structures,
            no data copying is required.

        Example
        -------
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
            self._arr = libUnderworld.StGermain.StgVariable_getAsNumpyArray(self._cself.variable)
            # set to writeability
            self._arr.flags.writeable = self._writeable
            # add to swarms weakref dict
            self.swarm._livingArrays[self._cself.name + "_data"] = self._arr
        return self._arr

    @property
    def data_shadow(self):
        """
        Returns
        -------
        numpy.ndarray
            Numpy proxy array to underlying variable shadow data.

        Example
        -------
        Refer to example provided for 'data' property(/method).
        
        """
        if self._arrshadow is None:
            self._arrshadow = libUnderworld.StGermain.StgVariable_getAsNumpyArray(
                                libUnderworld.StgDomain.Swarm_GetShadowVariable(self.swarm._cself, self._cself.variable) )
            # set to writeability
            self._arrshadow.flags.writeable = False
            # add to swarms weakref dict
            self.swarm._livingArrays[self._cself.name + "_data_shadow"] = self._arrshadow
        return self._arrshadow

    def _clear_array(self):
        """
        This removes the potentially defunct numpy swarm variable memory
        numpy view. It will be regenerated when required.
        """
        self._arr = None
        self._arrshadow = None

    def load( self, filename, collective=False ):
        """
        Load the swarm variable from disk. This must be called *after* the swarm.load().

        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be
            used, but all directories must exist.
        collective : bool
            If True, variable is loaded MPI collective. This is usually faster, but
            currently is problematic for passive swarms which may not have
            representation on all processes.

        Notes
        -----
        This method must be called collectively by all processes.


        Example
        -------
        Refer to example provided for 'save' method.

        """
        from ..utils._io import h5File, h5_get_dataset

        if not isinstance(filename, str):
            raise TypeError("'filename' parameter must be of type 'str'")

        if self.swarm._checkpointMapsToState != self.swarm.stateId:
            raise RuntimeError("'Swarm' associate with this 'SwarmVariable' does not appear to be in the correct state.\n" \
                               "Please ensure that you have loaded the swarm prior to loading any swarm variables.")
        gIds = self.swarm._local2globalMap

        comm = MPI.COMM_WORLD
        rank = comm.rank

        # open hdf5 file
        globalCount = self.swarm.particleGlobalCount
        with h5File(name=filename, mode="r") as h5f:

            dset = h5_get_dataset(h5f,'data')
            if dset.shape[1] != self.data.shape[1]:
                raise RuntimeError("Cannot load file data on current swarm. Data in file '{0}', " \
                                   "has {1} components -the particlesCoords has {2} components".format(filename, dset.shape[1], self.particleCoordinates.data.shape[1]))
            if dset.shape[0] != globalCount:
                raise RuntimeError("It appears that the swarm has {} particles, but provided h5 file has {} data points. Please check that " \
                                   "both the Swarm and the SwarmVariable were saved at the same time, and that you have reloaded using " \
                                   "the correct files.".format(globalCount, dset.shape[0]))

            # for efficiency, we want to load swarmvariable data in the largest stride chunks possible.
            # we need to determine where required data is contiguous.
            # first construct an array of gradients. the required data is contiguous
            # where the indices into the array are increasing by 1, ie have a gradient of 1.
            gradIds = np.zeros_like(gIds)            # creates array of zeros of same size & type
            if len(gIds) > 1:
                gradIds[:-1] = gIds[1:] - gIds[:-1]  # forward difference type gradient

            # note that we do only the first read into dset collective. this call usually
            # does the entire read, but if it doesn't we won't know how many calls will
            # be necessary, hence only collective calling the first.
            done_collective = False
            guy = 0
            while guy < len(gIds):
                # do contiguous
                start_guy = guy
                while gradIds[guy]==1:  # count run of contiguous. note bounds check not required as last element of gradIds is always zero.
                    guy += 1
                # copy contiguous chunk if found.. note that we are copying 'plus 1' items
                if guy > start_guy:
                    if collective and not done_collective:
                        with dset.collective:
                            self.data[start_guy:guy+1] = dset[gIds[start_guy]:gIds[guy]+1]
                            done_collective = True
                    else:
                        self.data[start_guy:guy+1] = dset[gIds[start_guy]:gIds[guy]+1]
                    guy += 1

                # do non-contiguous
                start_guy = guy
                while guy<len(gIds) and gradIds[guy]!=1:  # count run of non-contiguous
                    guy += 1
                # copy non-contiguous items (if found) using index array slice
                if guy > start_guy:
                    if collective and not done_collective:
                        with dset.collective:
                            self.data[start_guy:guy,:] = dset[gIds[start_guy:guy],:]
                            done_collective = True
                    else:
                        self.data[start_guy:guy,:] = dset[gIds[start_guy:guy],:]

            # if we haven't entered a collective call, do so now to
            # avoid deadlock. we just do an empty read/write.
            if collective and not done_collective:
                with dset.collective:
                    self.data[0:0,:] = dset[0:0,:]

    def save( self, filename, swarmHandle=None, collective=False ):
        """
        Save the swarm variable to disk.

        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be
            used, but all directories must exist.
        swarmHandle :uw.utils.SavedFileData , optional
            The saved swarm file handle. If provided, a reference to the swarm file
            is made.
        collective : bool
            If True, variable is saved MPI collective. This is usually faster, but
            currently is problematic for passive swarms which may not have
            representation on all processes.

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
        >>> if uw.mpi.rank == 0:
        ...     import os;
        ...     os.remove( "saved_swarm.h5" )
        ...     os.remove( "saved_swarm_variable.h5" )

        """
        from ..utils._io import h5File, h5_require_dataset

        if not isinstance(filename, str):
            raise TypeError("'filename' parameter must be of type 'str'")

        # setup mpi basic vars
        comm = MPI.COMM_WORLD
        rank = comm.rank

        # allgather the number of particles each proc has
        swarm = self.swarm
        procCount = comm.allgather(swarm.particleLocalCount)
        particleGlobalCount = np.sum(procCount)

        # calculate the hdf5 file offset
        offset=0
        for i in range(comm.rank):
            offset += procCount[i]

        # open parallel hdf5 file
        with h5File(name=filename, mode="a") as h5f:
            # write the entire local swarm to the appropriate offset position
            globalShape = (particleGlobalCount, self.data.shape[1])
            dset = h5_require_dataset(h5f, "data", shape=globalShape, dtype=self.data.dtype)

            if collective:
                with dset.collective:
                    dset[offset:offset+swarm.particleLocalCount] = self.data[:]
            else:
                dset[offset:offset+swarm.particleLocalCount] = self.data[:]

            # create an ExternalLink to the swarm - optional because intrinsic SwarmVariables
            # 'coordinates'  & 'owningCell' are SwarmVariables that don't have a corresponding
            # swarm file because they are the swarm itself.
            if swarmHandle is not None:
                if not isinstance(swarmHandle, (str, uw.utils.SavedFileData)):
                    raise TypeError("Expected 'swarmHandle' to be of type 'uw.utils.SavedFileData'")

                sFilename = swarmHandle.filename

                if not os.path.exists(sFilename):
                    raise ValueError("You are trying to link against the swarm file '{}'\n\
                                      that does not appear to exist.".format(sFilename))
                # set reference to mesh (all procs must call following)
                h5f["swarm"] = h5py.ExternalLink(sFilename, "./")

            # also write proc offsets - used for loading from checkpoint
            h5f.attrs["proc_offset"] = procCount

        return uw.utils.SavedFileData( self, filename )

    def xdmf( self, filename, varSavedData, varname, swarmSavedData, swarmname, modeltime=0.  ):
        """
        Creates an xdmf file, filename, associating the varSavedData file on
        the swarmSavedData file

        Notes
        -----
        xdmf contain 2 files: an .xml and a .h5 file. See http://www.xdmf.org/index.php/Main_Page
        This method only needs to be called by the master process, all other
        processes return quietly.

        Parameters
        ----------
        filename : str
            The output path to write the xdmf file. Relative or absolute paths may be
            used, but all directories must exist.
        varname : str
            The xdmf name to give the swarmVariable
        swarmname : str
            The xdmf name to give the swarm
        swarmSavedData : underworld.utils.SaveFileData
            Handler returned for saving a swarm. underworld.swarm.Swarm.save(xxx)
        varSavedData : underworld.utils.SavedFileData
            Handler returned from saving a SwarmVariable. underworld.swarm.SwarmVariable.save(xxx)
        modeltime : float (default 0.0)
            The time recorded in the xdmf output file

        Example
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
        >>> if uw.mpi.rank == 0: os.path.isfile("TESTxdmf.xdmf")
        True

        >>> # clean up:
        >>> if uw.mpi.rank == 0:
        ...     import os;
        ...     os.remove( "saved_swarm.h5" )
        ...     os.remove( "saved_swarmvariable.h5" )
        ...     os.remove( "TESTxdmf.xdmf" )

        """
        # use barrier as there are some file open operations below
        # and we need to ensure that all procs have finished writing
        # before we try and open any files.
        uw.mpi.barrier()
        if uw.mpi.rank == 0:
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

    def copy(self, deepcopy=False):
        """
        This method returns a copy of the swarmvariable.

        Parameters
        ----------
        deepcopy: bool
            If True, the variable's data is also copied into
            new variable.

        Returns
        -------
        underworld.swarm.SwarmVariable
            The swarm variable copy.

        Example
        -------
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm, 2))
        >>> svar = swarm.add_variable("double", 1)
        >>> svar.data[:] = 1.23456
        >>> svarCopy = svar.copy()
        >>> svarCopy.swarm == svar.swarm
        True
        >>> svarCopy.dataType == svar.dataType
        True
        >>> import numpy as np
        >>> np.allclose(svar.data,svarCopy.data)
        False
        >>> svarCopy2 = svar.copy(deepcopy=True)
        >>> np.allclose(svar.data,svarCopy2.data)
        True

        """

        if not isinstance(deepcopy, bool):
            raise TypeError("'deepcopy' parameter is expected to be of type 'bool'.")

        newSv = SwarmVariable(self.swarm, self.dataType, self.count)

        if deepcopy:
            newSv.data[:] = self.data[:]

        return newSv
