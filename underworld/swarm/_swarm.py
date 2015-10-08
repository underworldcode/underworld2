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
                         "_pShadowSync" : "ParticleShadowSync",
                      "_escapedRoutine" : "EscapedRoutine"
                    }
#                      "_particleLayout" : "SpaceFillerParticleLayout",

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
        componentDictionary[ self._swarm.name ]["ParticleCommHandlers"] = [self._pMovementHandler.name,
                                                                           self._pShadowSync.name]
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
        Save the swarm to the provided filename. Note that this is a
        global method, ie. all processes must call it.
        
        File is saved using hdf5 file format.
        """
        if not isinstance(filename, str):
            raise TypeError("Expected filename to be provided as a string")
        libUnderworld.StgDomain.Swarm_DumpToHDF5( self._cself, filename )










