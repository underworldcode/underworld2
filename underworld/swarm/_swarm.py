##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld._stgermain as _stgermain
import underworld.libUnderworld.libUnderworldPy.Function as _cfn
import numpy as np
from . import _swarmabstract
from . import _swarmvariable as svar
import underworld.function as function
import underworld.libUnderworld as libUnderworld
import underworld as uw
from mpi4py import MPI
import h5py
import contextlib
from underworld.scaling import non_dimensionalise
from underworld.scaling import units as u 
from pint.errors import UndefinedUnitError


class Swarm(_swarmabstract.SwarmAbstract, function.FunctionInput, _stgermain.Save):
    """
    The Swarm class supports particle like data structures. Each instance of
    this class will store a set of unique particles. In this context, particles
    are data structures which store a location variable, along with any other
    variables the user requests.

    Parameters
    ----------
    mesh : underworld.mesh.FeMesh
        The FeMesh the swarm is supported by. See Swarm.mesh property docstring
        for further information.
    particleEscape : bool
        If set to true, particles are deleted when they leave the domain. This
        may occur during particle advection, or when the mesh is deformed.


    Example
    -------
    Create a swarm with some variables:
    
    >>> # First we need a mesh:
    >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> # Create empty swarm:
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> # Add a variable:
    >>> svar = swarm.add_variable("char",1)
    >>> # Add another:
    >>> svar = swarm.add_variable("double",3)
    >>> # Now use a layout to fill with particles
    >>> swarm.particleLocalCount
    0
    >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleLocalCount
    1024
    >>> swarm.data[0]
    array([ 0.0132078,  0.0132078])
    >>> swarm.owningCell.data[0]
    array([0], dtype=int32)

    With particleEscape enabled, particles which are no longer within the mesh
    domain are deleted.

    >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> swarm = uw.swarm.Swarm(mesh, particleEscape=True)
    >>> swarm.particleLocalCount
    0
    >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleGlobalCount
    1024
    >>> with mesh.deform_mesh():
    ...     mesh.data[:] += (0.5,0.)
    >>> swarm.particleGlobalCount
    512

    Alternatively, moving the particles:

    >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> swarm = uw.swarm.Swarm(mesh, particleEscape=True)
    >>> swarm.particleLocalCount
    0
    >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
    >>> swarm.populate_using_layout(layout)
    >>> swarm.particleGlobalCount
    1024
    >>> with swarm.deform_swarm():
    ...     swarm.data[:] -= (0.5,0.)
    >>> swarm.particleGlobalCount
    512

    """

    _objectsDict = {            "_swarm": "GeneralSwarm",
                          "_cellLayout" : "ElementCellLayout",
                    "_pMovementHandler" : "ParticleMovementHandler",
                      "_escapedRoutine" : "EscapedRoutine",
                  "_particleShadowSync" : "ParticleShadowSync"
                    }

    def __init__(self, mesh, particleEscape=False, **kwargs):

        self.particleEscape = particleEscape
        # escape routine will be used during swarm advection, but lets also add
        # it to the mesh post deform hook so that when the mesh is deformed,
        # any particles that are found wanting are culled accordingly.
        import weakref
        wkref = weakref.ref(self)  # use weakref to avoid circular dependency here
        def _update_owners():
            selfguy = wkref()
            if selfguy:  # if for some reason the swarm is gone, this will be none, in which case we're done here.
                selfguy.update_particle_owners()
        mesh.add_post_deform_function( _update_owners )

        # init this to -1 to signify no mapping has occurred
        self._checkpointMapsToState = -1
        
        # build parent
        super(Swarm,self).__init__(mesh, **kwargs)

    def _setup(self):
        if self._cself.particleCoordVariable:
            self._particleCoordinates = svar.SwarmVariable(self, "double", self.mesh.dim, _cself=self._cself.particleCoordVariable, writeable=False)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(Swarm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._swarm.name ][                 "dim"] = self._mesh.dim
        componentDictionary[ self._swarm.name ][          "CellLayout"] = self._cellLayout.name
        componentDictionary[ self._swarm.name ][      "createGlobalId"] = False
        componentDictionary[ self._swarm.name ]["ParticleCommHandlers"] = [self._pMovementHandler.name,]
        componentDictionary[ self._swarm.name ][  "EscapedRoutine"]     = self._escapedRoutine.name
        componentDictionary[ self._escapedRoutine.name][ "particlesToRemoveDelta" ] = 1000

        componentDictionary[ self._cellLayout.name ]["Mesh"]            = self._mesh._cself.name

    def add_particles_with_coordinates( self, coordinatesArray ):
        """
        This method adds particles to the swarm using particle coordinates provided
        using a numpy array.

        Note that particles with coordinates NOT local to the current processor will
        be reject/ignored.

        Parameters
        ----------
        coordinatesArray : numpy.ndarray
            The numpy array containing the coordinate of the new particles. Array is
            expected to take shape n*dim, where n is the number of new particles, and
            dim is the dimensionality of the swarm's supporting mesh.

        Returns
        -------
        numpy.ndarray
            Array containing the local index of the added particles. Rejected particles
            are denoted with an index of -1.


        Example
        -------
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
        >>> swarm.data
        array([[ 0.1,  0.1],
               [ 0.2,  0.1],
               [ 0.1,  0.2],
               [ 0.8,  0.8]])

        """
        # need to increment this as particles have been added!
        self._toggle_state()
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
        if not coordinatesArray.shape[1] == self.mesh.dim :
            raise ValueError("""The 'coordinateArray' must have shape n*dim, where 'n' is the
                              number of particles to add, and 'dim' is the dimensionality of
                              the supporting mesh ({}).""".format(self.mesh.dim) )
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

    def save(self, filename, collective=False, units=None, **kwargs):
        """
        Save the swarm to disk.

        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be
            used, but all directories must exist.
        collective : bool
            If True, swarm is saved MPI collective. This is usually faster, but
            currently is problematic for passive swarms which may not have
            representation on all processes.
        units : pint unit object (optional)
            Define the units that must be used to save the data.
            The data will be dimensionalised and saved with the defined units.
            The units are saved as a HDF attribute.

        Additional keyword arguments are saved as string attributes.

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
        >>> np.allclose(swarm.data,clone_swarm.data)
        True

        >>> # clean up:
        >>> if uw.mpi.rank == 0:
        ...     import os;
        ...     os.remove( "saved_swarm.h5" )

        """

        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")

        # just save the particle coordinates SwarmVariable
        self.particleCoordinates.save(filename, collective, units=units, **kwargs)

        return uw.utils.SavedFileData( self, filename )

    def load( self, filename, collective=False, try_optimise=True ):
        """
        Load a swarm from disk. Note that this must be called before any SwarmVariable
        members are loaded. Note also that this method will not replace the currently
        existing particles, but will instead *add* all the particles from the provided
        h5 file to the current swarm. Usually you will want to call this method when
        the swarm is still empty.

        Parameters
        ----------
        filename : str
            The filename for the saved file. Relative or absolute paths may be
            used.
        collective : bool
            If True, swarm is loaded MPI collective. This is usually faster, but
            currently is problematic for passive swarms which may not have
            representation on all processes.
        try_optimise : bool, Default=True
            Will speed up the swarm load time but warning - this algorithm assumes the 
            previously saved swarm data was made on an identical mesh and mesh partitioning 
            (number of processors) with respect to the current mesh. If this isn't the case then
            the reloaded particle ordering will be broken, leading to an invalid swarms.
            One can disable this optimisation and revert to a brute force algorithm, much slower,
            by setting this option to False.

        Notes
        -----
        This method must be called collectively by all processes.

        Example
        -------
        Refer to example provided for 'save' method.

        """
        from ..utils._io import h5File, h5_get_dataset

        if not isinstance(filename, str):
            raise TypeError("Expected 'filename' to be provided as a string")

        rank = uw.mpi.rank
        nProcs = uw.mpi.size

        # open hdf5 file
        with h5File(name=filename, mode="r") as h5f:

            try:
                units = u.Quantity(h5f.attrs["units"])
            except (KeyError, UndefinedUnitError) as e:
                units = None

            dset = h5_get_dataset(h5f, 'data')
            if dset.shape[1] != self.data.shape[1]:
                raise RuntimeError("Cannot load file data on current swarm. Data in file '{0}', " \
                                   "has {1} components -the particlesCoords has {2} components".format(filename, dset.shape[1], self.data.shape[1]))

            # try and read the procCount attribute & assume that if nProcs in .h5 file
            # is equal to the current no. procs then the particles will be distributed the
            # same across the processors. (Danger if different discretisations are used... i think)
            # else try and load the whole .h5 file.
            # we set the 'offset' & 'size' variables to achieve the above

            offset = 0
            size = dset.shape[0] # number of particles in h5 file

            if try_optimise:
                procCount = h5f.attrs.get('proc_offset')
                if procCount is not None and nProcs == len(procCount):
                    for p_i in range(rank):
                        offset += procCount[p_i]
                    size = procCount[rank]

            valid = np.zeros(0, dtype='i') # array for read in
            chunk=int(2e7) # read in max this many points at a time

            firstChunk = True
            (multiples, remainder) = divmod( size, chunk )
            for ii in range(multiples+1):
                # setup the points to begin and end reading in
                chunkStart = offset + ii*chunk
                if ii == multiples:
                    chunkEnd = chunkStart + remainder
                    if remainder == 0: # in the case remainder is 0
                        break
                else:
                    chunkEnd = chunkStart + chunk

                # Add particles to swarm, ztmp is the corresponding local array
                # non-local particles are not added and their ztmp index is -1.
                # Note that for the first chunk, we do collective read, as this
                # is the only time that we can guaranteed that all procs will
                # take part, and usually most (if not all) particles are loaded
                # in this step.
                if firstChunk and collective:
                    with dset.collective:
                        if units:
                            vals = non_dimensionalise(dset[ chunkStart : chunkEnd ] * units)
                            ztmp = self.add_particles_with_coordinates(vals)
                        else:
                            ztmp = self.add_particles_with_coordinates(dset[ chunkStart : chunkEnd ])
                        firstChunk = False
                else:
                    if units:
                        vals = non_dimensionalise(dset[ chunkStart : chunkEnd ] * units)
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

        self._local2globalMap = valid
        # record which swarm state this corresponds to
        self._checkpointMapsToState = self.stateId

    def fn_particle_found(self):
        """
        This function returns True where a particle is able to be found using 
        the provided input to function evaluation.
        
        Returns
        -------
        underworld.function.Function
            The function object.

        Example
        -------
        Setup some things:
        
        >>> import numpy as np
        >>> mesh = uw.mesh.FeMesh_Cartesian(elementRes=(32,32))
        >>> swarm = uw.swarm.Swarm(mesh, particleEscape=True)
        >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
        >>> swarm.populate_using_layout(layout)
        
        Now, evaluate the fn_particle_found function on the swarm.. all 
        should be true
        
        >>> fn_pf = swarm.fn_particle_found()
        >>> fn_pf.evaluate(swarm).all()
        True
        
        Evalute at arbitrary coord... should return False
        
        >>> fn_pf.evaluate( (0.3,0.9) )
        array([[False]], dtype=bool)
        
        Now, lets get rid of all particles outside of a circle, and look
        to obtain pi/4.  First eject particles:
        
        >>> with swarm.deform_swarm():
        ...    for ind,coord in enumerate(swarm.data):
        ...        if np.dot(coord,coord)>1.:
        ...            swarm.data[ind] = (99999.,99999.)
        
        Now integrate and test
        
        >>> incirc = uw.function.branching.conditional( ( (fn_pf,1.),(True,0.)  ) )
        >>> np.isclose(uw.utils.Integral(incirc, mesh).evaluate(),np.pi/4.,rtol=2e-2)
        array([ True], dtype=bool)
        
        """

        if not hasattr(self, '_fn_particle_found'):
            import underworld.function as function
            # create inline class here.. as it's only required here
            class _fn_particle_found(function.Function):
                def __init__(self, swarm, **kwargs):

                    # create instance
                    self._fncself = _cfn.ParticleFound(swarm._cself)

                    # build parent
                    super(_fn_particle_found,self).__init__(argument_fns=None, **kwargs)
                    self._underlyingDataItems.add(swarm)

            self._fn_particle_found = _fn_particle_found(self)
        return self._fn_particle_found


    @property
    def particleGlobalCount(self):
        """
        Returns
        -------
        int
            The global number (across all processes) of particles in the swarm.
        """

        # setup mpi basic vars
        comm = MPI.COMM_WORLD
        return comm.allreduce(self.particleLocalCount, op=MPI.SUM)

    @property
    def _voronoi_swarm(self):
        """
        This property points to an integration type swarm which mirrors the 
        current swarm. The mirror swarms particles are coincident with the 
        current swarms, but use local coordinates to record positions, and also
        records particle weights.
        """
        if not hasattr(self, '_voronoi_swarm_private'):
            self._voronoi_swarm_private = uw.swarm.VoronoiIntegrationSwarm(self)
        return self._voronoi_swarm_private

    @property
    def allow_parallel_nn(self):
        """
        By default, parallel nearest neighbour search is disabled as consistent
        results are not currently guaranteed. Set this attribute to `True` to 
        allow parallel NN.
        """
        return self._cself.allow_parallel_nn
    @allow_parallel_nn.setter
    def allow_parallel_nn(self,val):
        self._cself.allow_parallel_nn = val


    @contextlib.contextmanager
    def deform_swarm(self, update_owners=True):
        """
        Any particle location modifications must occur within this python 
        context manager. This is necessary as it is critical that certain
        internal objects are updated when particle locations are modified.
        Note that this method must be called collectively by all processes, 
        irrespective of whether any given process does or does not need to 
        locate any particles. 

        Parameters
        ----------
        update_owners : bool, default=True
            If this is set to False, particle ownership (which element owns a 
            particular particle) is not updated at the conclusion of the context
            manager. This is often necessary when both the mesh and particles 
            are advecting simutaneously.

        Notes
        -----
        This method must be called collectively by all processes.


        Example
        -------
        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> layout = uw.swarm.layouts.PerCellGaussLayout(swarm,2)
        >>> swarm.populate_using_layout(layout)
        >>> swarm.data[0]
        array([ 0.0132078,  0.0132078])
        
        Attempted modification without using deform_swarm() should fail:
        
        >>> swarm.data[0] = [0.2,0.2]
        Traceback (most recent call last):
        ...
        ValueError: assignment destination is read-only
        
        Within the deform_swarm() context manager, modification is allowed:
        
        >>> with swarm.deform_swarm():
        ...     swarm.data[0] = [0.2,0.2]
        >>> swarm.data[0]
        array([ 0.2,  0.2])

        """
        # lock swarm to prevent particle addition etc
        self._locked = True
        # enable writeable array
        self._particleCoordinates.data.flags.writeable = True
        try:
            yield
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "An exception was encountered during particle deformation."
                                                     +"Particle locations may not be correctly modified." )
        finally:
            # disable writeable array
            self._particleCoordinates.data.flags.writeable = False
            # unlock swarm
            self._locked = False
            # update owners
            if update_owners:
                self.update_particle_owners()

    def shadow_particles_fetch(self):
        """
        When called, neighbouring processor particles which have coordinates 
        within the current processor's shadow zone will be communicated to the 
        current processor. Ie, the processor shadow zone is populated using 
        particles that are owned by neighbouring processors. After this 
        method has been called, particle shadow data is available via the 
        `data_shadow` handles of SwarmVariable objects. This data is read only.
        
        Note that you will need to call this whenever neighbouring information
        has potentially changed, for example after swarm advection, or after
        you have modified a SwarmVariable object. 
        
        Any existing shadow information will be discarded when this is called.

        Notes
        -----
        This method must be called collectively by all processes.

        """
        self._clear_variable_arrays()
        uw.libUnderworld.StgDomain._ParticleShadowSync_Execute(self._particleShadowSync,self._cself)
    

    def update_particle_owners(self):
        """
        This routine will update particles owners after particles have been
        moved. This is both in terms of the cell/element that the
        particle resides within, and also in terms of the parallel processor
        decomposition (particles belonging on other processors will be sent across).
        
        Users should not generally need to call this as it will be called automatically at the
        conclusion of a deform_swarm() block.

        Notes
        -----
        This method must be called collectively by all processes.
        
        Example
        -------
        >>> mesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(16,16), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
        >>> swarm.data[0]
        array([ 0.0132078,  0.0132078])
        >>> swarm.owningCell.data[0]
        array([0], dtype=int32)
        >>> with swarm.deform_swarm():
        ...     swarm.data[0] = [0.1,0.1]
        >>> swarm.owningCell.data[0]
        array([17], dtype=int32)

        """
        orig_total_particles = self.particleGlobalCount
        libUnderworld.StgDomain.Swarm_UpdateAllParticleOwners( self._cself )
        libUnderworld.PICellerator.EscapedRoutine_RemoveFromSwarm(self._escapedRoutine, self._cself)
        new_total_particles = self.particleGlobalCount
        if (uw.mpi.rank==0) and (not self.particleEscape) and (orig_total_particles != new_total_particles):
            raise RuntimeError("Particles appear to have left the domain, but swarm flag `particleEscape` is False. "
                               "Check your velocity field or your particle relocation routines, or set the "
                               "`particleEscape` swarm constructor parameter to True to allow escape.")


        libUnderworld.PICellerator.GeneralSwarm_ClearSwarmMaps( self._cself )
        libUnderworld.PICellerator.GeneralSwarm_DeleteIndex( self._cself )
        self._toggle_state()
