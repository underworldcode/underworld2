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
import weakref
import libUnderworld
import _swarmvariable as svar

class SwarmAbstract(_stgermain.StgCompoundComponent):
    """
    The SwarmAbstract class supports particle like data structures. Each instance of 
    this class will store a set of unique particles. In this context, particles
    are data structures which store a location variable, along with any other
    variables the user requests.
    """
    _objectsDict = {            "_swarm" : None,
                           "_cellLayout" : None
                    }
#                       "_particleLayout" : None

    _selfObjectName = "_swarm"

    _supportedDataTypes = ["char","short","int","float", "double"]

    def __init__(self, feMesh, **kwargs):
        """
        Parameters
        ----------
        feMesh : uw.mesh.FeMesh
            The FeMesh the swarm is supported by. See Swarm.feMesh property docstring
            for further information.
        """
        if not isinstance(feMesh, mesh.FeMesh):
            raise TypeError("'feMesh' object passed in must be of type 'FeMesh'")
        self._feMesh = feMesh

        # lets init these guy
        self._variables = []
        self._livingArrays = weakref.WeakValueDictionary()

        # build parent
        super(SwarmAbstract,self).__init__(**kwargs)

    @property
    def feMesh(self):
        """    
        feMesh (FeMesh): Supporting FeMesh for this Swarm. All swarms are required to be
            supported by mesh (or similar) objects, which provide the data structures
            necessary for efficient particle locating/tracking, as well as the necessary
            mechanism for the swarm parallel decomposition.
        """
        return self._feMesh

    def _setup(self):
        # add coord swarm variable now (if available)
        if self._cself.owningCellVariable:
            self._owningCell = svar.SwarmVariable(self, "int", 1, _cself=self._cself.owningCellVariable)
        if self._cself.globalIdVariable:
            self._globalId = svar.SwarmVariable(self, "int", 1, _cself=self._cself.globalIdVariable)
        # set this flag
        self._cself.isAdvecting = False
    
    @property
    def variables(self):
        """    
        variables (list): List of swarm variables associated with this swarm.
        """
        return self._variables

    @property
    def particleLocalCount(self):
        """    
        particleLocalCount (int): Number of swarm particles stored on this processor.
        """
        return self._cself.particleLocalCount

    @property
    def owningCell(self):
        """
        owningCell (SwarmVariable): Swarm variable recording the owning cell of the
        swarm particles. This will usually correspond to the owning element local id.
        """
        return self._owningCell

    @property
    def globalId(self):
        """
        globalId (SwarmVariable): Swarm variable recording a global integer identifier
        for the swarm particle.
        """
        return self._globalId

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(SwarmAbstract,self)._add_to_stg_dict(componentDictionary)
        componentDictionary[ self._swarm.name ][                 "dim"] = self._feMesh.dim
        componentDictionary[ self._swarm.name ][          "CellLayout"] = self._cellLayout.name
        componentDictionary[ self._swarm.name ][      "createGlobalId"] = False
        componentDictionary[ self._swarm.name ][              "FeMesh"] = self._feMesh._cself.name

        componentDictionary[ self._cellLayout.name ]["Mesh"]            = self._feMesh._cself.name

    def add_variable(self, dataType, count):
        """ 
        Add a variable to each particle in this swarm. Variables can be added
        at any point. Removal of variables is however not currently supported.
        See help(SwarmVariable) for further information.
        
        Parameters
        ----------
        dataType: str
            The data type for the variable. Available types are  "char", 
            "short", "int", "float" or "double".
        count: unsigned
            The number of values to be stored for each particle.
        
        Returns
        -------
        variable: SwarmVariable
            The newly created swarm variable.
            
        >>> # first we need a mesh
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> # create swarm
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> # add a variable
        >>> svar = swarm.add_variable("char",1)
        >>> # add another
        >>> svar = swarm.add_variable("double",3)
        >>> # add some particles
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
        >>> # add another variable
        >>> svar = swarm.add_variable("double",5)

        """
        return svar.SwarmVariable( self, dataType, count )

    def populate_using_layout( self, layout ):
        """ 
        This method uses the provided layout to populate the swarm with particles.
        Usually layouts add particles across the entire domain. Available
        layouts may be found in the swarm.layouts module.
        Note that Layouts can only currently be used on empty swarms.
        Also note that all numpy arrays associated with swarm variables must 
        be deleted before a layout can be applied.
        
        Parameters
        ----------
        layout: ParticleLayout
            The layout which determines where particles are created and added.

        >>> # first we need a mesh
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> # create swarm
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> # add populate
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
        
        """
        if self.particleLocalCount > 0:
            raise RuntimeError("Swarm appears to already have particles. \nLayouts can only be used with empty swarms.")
        if not isinstance( layout, uw.swarm.layouts._ParticleLayout ):
            raise TypeError("Provided layout does not appear to be a subclass of ParticleLayout")

        if not (self == layout.swarm):
            raise ValueError("The swarm associated with the layout appears to be a different swarm from self.")

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


        libUnderworld.StgDomain.ParticleLayout_SetInitialCounts( layout._cself, self._cself )
        libUnderworld.StgDomain._Swarm_BuildParticles( self._cself, None )
        libUnderworld.StgDomain.ParticleLayout_InitialiseParticles( layout._cself, self._cself )
        libUnderworld.StgDomain._Swarm_InitialiseParticles( self._cself, None )
        
    def update_particle_owners(self):
        """
        This routine will update particles owners after particles have been
        moved. This is both in terms of the cell/element the the
        particle resides within, and also in terms of the parallel processor
        decomposition (particles belonging on other processors will be sent across).
        
        >>> mesh = uw.mesh.FeMesh_Cartesian()
        >>> swarm = uw.swarm.Swarm(mesh)
        >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,2))
        >>> swarm.particleCoordinates.data[0]
        array([ 0.0132078,  0.0132078])
        >>> swarm.owningCell.data[0]
        array([0], dtype=int32)
        >>> swarm.particleCoordinates.data[0] = [0.1,0.1]
        >>> swarm.owningCell.data[0]
        array([0], dtype=int32)
        >>> swarm.update_particle_owners()
        >>> swarm.owningCell.data[0]
        array([17], dtype=int32)

        """
        libUnderworld.StgDomain.Swarm_UpdateAllParticleOwners( self._cself );

    @property
    def particleCoordinates(self):
        """
        particleCoordinates (SwarmVariable): Swarm variable recording the coordinates of the 
        swarm particles.
        """
        return self._particleCoordinates
