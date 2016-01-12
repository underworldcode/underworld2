##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld._stgermain as _stgermain
import numpy as np
#from underworld.swarm._swarmabstract import SwarmAbstract
import _swarmabstract
import _swarmvariable as svar
import underworld.function as function
import libUnderworld
import underworld as uw
from mpi4py import MPI
import h5py


class Swarm(_swarmabstract.SwarmAbstract, function.FunctionInput, _stgermain.Save):
    """
    The Swarm class supports particle like data structures. Each instance of 
    this class will store a set of unique particles. In this context, particles
    are data structures which store a location variable, along with any other
    variables the user requests.
    
    For example, to create the swarm with some variables:
    
    >>> # first we need a mesh
    >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> # create empty swarm
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> # add a variable
    >>> svar = swarm.add_variable("char",1)
    >>> # add another
    >>> svar = swarm.add_variable("double",3)
    
    Can also use a layout to fill with particles
    
    >>> swarm.particleLocalCount
    0
    >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleLocalCount
    1024
    >>> swarm.particleCoordinates.data[0]
    array([ 0.0132078,  0.0132078])
    >>> swarm.owningCell.data[0]
    array([0], dtype=int32)

    """

    _objectsDict = {            "_swarm": "GeneralSwarm",
                          "_cellLayout" : "ElementCellLayout",
                    "_pMovementHandler" : "ParticleMovementHandler",
                      "_escapedRoutine" : "EscapedRoutine"
                    }

    def __init__(self, feMesh, particleEscape=False, **kwargs):
        """
        Parameters
        ----------
        feMesh : uw.mesh.FeMesh
            The FeMesh the swarm is supported by. See Swarm.feMesh property docstring
            for further information.
        particleEscape : bool
            If set to true, particles are allowed to escape from the domain.
        
        """
        # if a PIC swarm is created for this guy, then it should record itself here
        self._PICSwarm = None
        
        self.particleEscape = particleEscape

        # numpy array to map local particle indices to global indicies, used for loading from file
        self._local2globalMap = None

        # build parent
        super(Swarm,self).__init__(feMesh, **kwargs)

    def _setup(self):
        if self._cself.particleCoordVariable:
            self._particleCoordinates = svar.SwarmVariable(self, "double", self.feMesh.dim, _cself=self._cself.particleCoordVariable)
        self._cself.isAdvecting = True


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(Swarm,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._swarm.name ][                 "dim"] = self._feMesh.dim
        componentDictionary[ self._swarm.name ][          "CellLayout"] = self._cellLayout.name
        componentDictionary[ self._swarm.name ][      "createGlobalId"] = False
        componentDictionary[ self._swarm.name ]["ParticleCommHandlers"] = [self._pMovementHandler.name,]
        if self.particleEscape:
            componentDictionary[ self._swarm.name ][  "EscapedRoutine"] = self._escapedRoutine.name
        

        componentDictionary[ self._cellLayout.name ]["Mesh"]            = self._feMesh._cself.name

    def add_particles_with_coordinates( self, coordinatesArray ):
        """
        This method adds particles to the swarm using particle coordinates provided
        using a numpy array.
        
        Note that particles with coordinates NOT local to the current processor will
        be reject/ignored.
        
        Parameters
        ----------
        coordinatesArray : np.ndarray
            The numpy array containing the coordinate of the new particles. Array is 
            expected to take shape n*dim, where n is the number of new particles, and 
            dim is the dimensionality of the swarm's supporting mesh.

        Returns
        ----------
        particleLocalIndex : np.ndarray
            Array containing the local index of the added particles. Rejected particles
            are denoted with an index of -1. 
            

        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> import numpy as np
        >>> arr = np.zeros((5,2))
        >>> arr[0] = [0.1,0.1]
        >>> arr[1] = [0.2,0.1]
        >>> arr[2] = [0.1,0.2]
        >>> arr[3] = [-0.1,-0.1]
        >>> arr[4] = [0.8,0.8]
        >>> swarm.add_particles_with_coordinates(arr)
        array([ 0,  1,  2, -1,  3], dtype=int32)
        >>> swarm.particleLocalCount
        4
        >>> swarm.particleCoordinates.data
        array([[ 0.1,  0.1],
               [ 0.2,  0.1],
               [ 0.1,  0.2],
               [ 0.8,  0.8]])
        
        """
        if len(self._livingArrays) != 0:
            raise RuntimeError("""
            There appears to be {} swarm variable numpy array objects still in
            existance. Adding particles to a swarm results in the modification
            of existing swarm variable memory layouts and locations, and therefore
            existing numpy array views of swarm variables will cease to be valid. Potential
            modification of these invalid numpy arrays is dangerous, and therefore they must
            be removed before particles can be added.  The python 'del' command may be useful,
            though be aware that an object cannot be destroyed while another object retains a
            reference to it. Once you have added the required particles, you can easily
            regenerate the numpy views of other variables again using the 'data' property.""".format(len(self._livingArrays)))

        if not isinstance( coordinatesArray, np.ndarray ):
            raise TypeError("'coordinateArray' must be provided as a numpy array")
        if not len(coordinatesArray.shape) == 2 :
            raise ValueError("The 'coordinateArray' is expected to be two dimensional.")
        if not coordinatesArray.shape[1] == self.feMesh.dim :
            raise ValueError("""The 'coordinateArray' must have shape n*dim, where 'n' is the
                              number of particles to add, and 'dim' is the dimensionality of
                              the supporting mesh ({}).""".format(self.feMesh.dim) )
        retarr = self._cself.GeneralSwarm_AddParticlesFromCoordArray( coordinatesArray )
        # lets realloc swarm now
        libUnderworld.StgDomain.Swarm_Realloc(self._cself)

        return retarr

    def _get_iterator(self):
        """
        This is the concrete method required by the FunctionInput class. 
        
        It effects using the particle coordinate swarm variable as an input 
        when the swarm is used as the input to a function.
        """
        return libUnderworld.Function.SwarmInput(self._particleCoordinates._cself)

    def save(self, filename):
        """
        Save the swarm to disk.
        
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
        First create the swarm, and populate with layout:

        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
        
        Save to a file:
        
        >>> swarm.save("saved_swarm.h5")
        
        Now let's try and reload. First create an empty swarm, and then load:
        
        >>> clone_swarm = uw.swarm.Swarm(mesh)
        >>> clone_swarm.load( "saved_swarm.h5" )
        
        Now check for equality:
        
        >>> import numpy as np
        >>> np.allclose(swarm.particleCoordinates.data,clone_swarm.particleCoordinates.data)
        True
        
        Clean up:
        >>> if uw.rank() == 0: import os; os.remove( "saved_swarm.h5" )
    
        """

        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")

        # just save the particle coordinates SwarmVariable
        self.particleCoordinates.save(filename)

    def load( self, filename, verbose=False ):
        """
        Load a swarm from disk. Note that this must be called before any SwarmVariable
        members can be loaded.
        
        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be 
            used.
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

        if rank == 0 and verbose:
            bar = uw.utils.ProgressBar( start=0, end=dset.shape[0]-1, title="loading "+filename)

        valid = np.zeros(0, dtype='i') # array for read in
        chunk=int(1e4) # read in this many points at a time

        (multiples, remainder) = divmod( dset.shape[0], chunk )
        for ii in xrange(multiples+1):
            # setup the points to begin and end reading in 
            chunkStart = ii*chunk
            if ii == multiples:
                chunkEnd = chunkStart + remainder
                if remainder == 0: # in the case remainder is 0 
                    break
            else:
                chunkEnd = chunkStart + chunk
            
            # add particles to swarm, ztmp is the corresponding local array
            # non-local particles are not added and their ztmp index is -1
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

    def _invalidatelocal2globalmap(self):
        self._local2globalMap = None # set numpy array to None

    @property
    def particleGlobalCount(self):
        """
        Returns the global number (across all processors) of particles in the swarm
        """

        # setup mpi basic vars
        comm = MPI.COMM_WORLD
        rank = comm.Get_rank()
        nProcs = comm.Get_size()

        # allgather the number of particles each proc has
        procCount = comm.allgather(self.particleLocalCount)
        return sum(procCount)
