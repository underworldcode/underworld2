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
import underworld.libUnderworld as libUnderworld
from . import _swarmabstract
from . import _swarmvariable as svar
from abc import ABCMeta
import weakref
import underworld.function as function


class IntegrationSwarm(_swarmabstract.SwarmAbstract, metaclass = ABCMeta):
    """
    Abstract class definition for IntegrationSwarms.

    All IntegrationSwarms have the following SwarmVariables from this class:
    
    1. localCoordVariable : double (number of particle, dimensions)
        For local element coordinates of the particle
    2. weightVariable : double (number of particles)
        For the integration weight of each particle

    """
    _objectsDict = { "_swarm": "IntegrationPointsSwarm" }

    def _setup(self):
        if self._cself.localCoordVariable:
            self._particleCoordinates = svar.SwarmVariable(self, "double", self.mesh.generator.dim, _cself=self._cself.localCoordVariable, writeable=False)
        if self._cself.weightVariable:
            self._weightsVariable = svar.SwarmVariable(self, "double", 1, _cself=self._cself.weightVariable, writeable=False)

    @property
    def particleWeights(self):
        """
        Returns
        -------
        underworld.swarm.SwarmVariable
            Swarm variable recording the weight of the swarm particles.
        """
        return self._weightsVariable



class VoronoiIntegrationSwarm(IntegrationSwarm,function.FunctionInput):
    """
    Class for an IntegrationSwarm that maps to another Swarm

    Note that this swarm can act as a function input. In this capacity,
    the fundamental function input type is the FEMCoordinate (ie, the particle
    local coordinate, the owning mesh, and the owning element). This input
    can be reduced to the global coordinate when returned within python. The
    FEMCoordinate particle representation is useful when deforming a mesh, as
    it is possible to deform the mesh, and then use the FEMCoordinate to reset
    the particles within the moved mesh.

    Parameters
    ----------
    swarm : underworld.swarm.Swarm
        The PIC integration swarm maps to this user provided swarm.

    Example
    -------
    This simple example checks that the true global coordiante, and that
    derived from the local coordinate, are close to equal. Note that the
    VoronoiIntegrationSwarm uses a voronoi centroid algorithm so we do not
    expect particle to exactly coincide.

    >>> import underworld as uw
    >>> import numpy as np
    >>> mesh = uw.mesh.FeMesh_Cartesian()
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,4))
    >>> vswarm = uw.swarm.VoronoiIntegrationSwarm(swarm)
    >>> vswarm.repopulate()
    >>> np.allclose(swarm.particleCoordinates.data, uw.function.input().evaluate(vswarm),atol=1e-1)
    True

    """
    _objectsDict = {  "_cellLayout" : "ElementCellLayout",
                          "_mapper" : "CoincidentMapper"
                    }

    def __init__(self, swarm, **kwargs):

        if not isinstance(swarm, uw.swarm.Swarm):
            raise ValueError("Provided swarm must be of class 'Swarm'.")
        
        self._mappedSwarm = weakref.ref(swarm)  # keep weakref to avoid circular dependency
        self._weights = uw.swarm._weights.DVC()
        
        # init this to ensure we do mapping on first pass
        self._mappedToState = -1

        # build parent
        super(VoronoiIntegrationSwarm,self).__init__(swarm.mesh, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(VoronoiIntegrationSwarm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cellLayout.name ][              "Mesh"] = self._mesh._cself.name

        componentDictionary[ self._mapper.name ][          "GeneralSwarm"] = self._mappedSwarm()._cself.name  # note _mappedSwarm is a weakref
        componentDictionary[ self._mapper.name ]["IntegrationPointsSwarm"] = self._swarm.name

        componentDictionary[ self._swarm.name ][             "CellLayout"] = self._cellLayout.name
        componentDictionary[ self._swarm.name ][ "IntegrationPointMapper"] = self._mapper.name

    def repopulate(self, weights_calculator=None):
        """
        This method repopulates the voronoi swarm using the provided
        global swarm. The weights are also recalculated.

        Parameters
        ----------
        weights_calculator: underworld.swarm.Weights
            The weights calculator for the Voronoi swarm. If none is provided,
            a default DVCWeights calculator is used.

        """
        if weights_calculator is None:
            weights_calculator = self._weights
        
        if not isinstance( weights_calculator, uw.swarm._weights.DVC ):
            raise TypeError("Provided 'weights_calculator' does not appear to be of correct class.")

        # only update if necessary
        if self._mappedToState != self._mappedSwarm().stateId:
            libUnderworld.PICellerator._CoincidentMapper_Map( self._mapper )
        # if weights calculator is PCDVC, then we need to always run as it potentially performs population control
        if (self._mappedToState != self._mappedSwarm().stateId) or isinstance(weights_calculator,uw.swarm._weights.PCDVC):
            libUnderworld.PICellerator.WeightsCalculator_CalculateAll( weights_calculator._cself, self._cself )
            libUnderworld.PICellerator.IntegrationPointsSwarm_ClearSwarmMaps( self._cself )

        self._mappedToState = self._mappedSwarm().stateId
            

    def _get_iterator(self):
        """
        This is the concrete method required by the FunctionInput class.

        It affects using the voronoi swarm as an input to functions.
        """
        return libUnderworld.Function.IntegrationSwarmInput(self._cself)


class GaussIntegrationSwarm(IntegrationSwarm):
    """
    Integration swarm which creates particles within an element at the Gauss 
    points.

    Parameters
    ----------
    mesh : underworld.mesh.FeMesh
        The FeMesh the swarm is supported by. See Swarm.mesh property docstring
        for further information.
    particleCount : unsigned. Default is 3, unless Q2 mesh which takes default 5. 
        Number of gauss particles in each direction.  Must take value in [1,5].
    """

    _objectsDict = {  "_cellLayout" : "SingleCellLayout",
                  "_particleLayout" : "GaussParticleLayout" }

    def __init__(self, mesh, particleCount=None, **kwargs):
        if particleCount == None:
            # this is fragile.....
            partCountMap = { "DQ0"  : 3,
                             "Q1"   : 3,
                             "DQ1"  : 3,
                             "DPC1" : 3,
                             "Q2"   : 5  }
            particleCount = partCountMap[ mesh.elementType.upper() ]
        if not isinstance(particleCount, int):
            raise ValueError("'particleCount' parameter must be of type 'int'.")
        if particleCount not in [1,2,3,4,5]:
            raise ValueError("'particleCount' must take a value in [1,5].")
        self._particleCount = particleCount

        # build parent
        super(GaussIntegrationSwarm,self).__init__(mesh, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(GaussIntegrationSwarm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._swarm.name          ][             "CellLayout"] = self._cellLayout.name
        componentDictionary[ self._swarm.name          ][         "ParticleLayout"] = self._particleLayout.name

        componentDictionary[ self._particleLayout.name ][                    "dim"] = self._mesh.generator.dim
        componentDictionary[ self._particleLayout.name ][         "gaussParticles"] = self._particleCount

class GaussBorderIntegrationSwarm(GaussIntegrationSwarm):
    """
    Integration swarm which creates particles within the boundary faces of an 
    element, at the Gauss points.
    
    See parent class for parameters.

    """
    _objectsDict = { "_particleLayout" : "GaussBorderParticleLayout" }

    def __init__(self, mesh, particleCount=None, **kwargs):
        # build parent
        super(GaussBorderIntegrationSwarm,self).__init__(mesh, particleCount, **kwargs)
