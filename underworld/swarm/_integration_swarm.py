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
import libUnderworld
import _swarmabstract
import _swarmvariable as svar
from abc import ABCMeta

class IntegrationSwarm(_swarmabstract.SwarmAbstract):
    """
    Abstract class definition for IntegrationSwarms.

    Note
    ----
    All IntegrationSwarms have the following SwarmVariables from this class.
    1) localCoordVariable : double (number of particle, dimensions)
        For local element coordinates of the particle
    2) weightVariable : double (number of particles)
        For the integration weight of each particle

    """
    _objectsDict = { "_swarm": "IntegrationPointsSwarm" }
    __metaclass__ = ABCMeta

    def _setup(self):
        if self._cself.localCoordVariable:
            self._particleCoordinates = svar.SwarmVariable(self, "double", self.mesh.dim, _cself=self._cself.localCoordVariable)
        if self._cself.weightVariable:
            self._weightsVariable = svar.SwarmVariable(self, "double", 1, _cself=self._cself.weightVariable)

    @property
    def particleWeights(self):
        """
        particleWeights (SwarmVariable): Swarm variable recording the weight of the
        swarm particles.
        """
        return self._weightsVariable



class PICIntegrationSwarm(IntegrationSwarm):
    """
    Class for an IntegrationSwarm that maps to another Swarm

    Parameters
    ----------
    swarm : uw.swarm.Swarm
        The PIC integration swarm maps to this user provided swarm.

    """
    _objectsDict = {  "_cellLayout" : "ElementCellLayout",
                          "_mapper" : "CoincidentMapper"
                    }

    def __init__(self, swarm, **kwargs):
        if not isinstance(swarm, uw.swarm.Swarm):
            raise ValueError("Provided swarm must be of class 'Swarm'.")
        self._mappedSwarm = swarm
        swarm._PICSwarm = self
        
        # note that if the mapped swarm allows particles to escape, lets by default
        # switch inflow on.
        self._weights = uw.swarm._weights.PCDVC(swarm, inFlow=self._mappedSwarm.particleEscape )

        # build parent
        super(PICIntegrationSwarm,self).__init__(swarm.mesh, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(PICIntegrationSwarm,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._swarm.name ][     "WeightsCalculator"]  = self._weights._cself.name

        componentDictionary[ self._cellLayout.name ]["Mesh"]               = self._mesh._cself.name

        componentDictionary[ self._mapper.name ][          "GeneralSwarm"] = self._mappedSwarm._cself.name
        componentDictionary[ self._mapper.name ]["IntegrationPointsSwarm"] = self._swarm.name

        componentDictionary[ self._swarm.name ][             "CellLayout"] = self._cellLayout.name
        componentDictionary[ self._swarm.name ][ "IntegrationPointMapper"] = self._mapper.name

    def repopulate(self):
        """ 
        This method repopulates the PIC swarm using the provided
        global swarm. The weights are also recalculated.
        """

        self._mappedSwarm._invalidatelocal2globalmap() # invalidate as population control will mess it
        libUnderworld.PICellerator._CoincidentMapper_Map( self._mapper )
        libUnderworld.PICellerator.WeightsCalculator_CalculateAll( self._weights._cself, self._cself )
        libUnderworld.PICellerator.IntegrationPointsSwarm_ClearSwarmMaps( self._cself )
        
class GaussIntegrationSwarm(IntegrationSwarm):
    """
    Class definition for a Gauss points swarm

    Parameters
    ----------
    mesh : uw.mesh.FeMesh
        The FeMesh the swarm is supported by. See Swarm.mesh property docstring
        for further information.
    particleCount : unsigned
        Number of gauss particles in each direction.  Must take value in [1,5].
        Default behaviour chooses an appropriate count for the provided mesh:
            Constant : 1
              Linear : 2
           Quadratic : 4

    """

    _objectsDict = {  "_cellLayout" : "SingleCellLayout",
                  "_particleLayout" : "GaussParticleLayout" }

    def __init__(self, mesh, particleCount=None, **kwargs):
        if particleCount == None:
            # this is fragile.....
            partCountMap = { "DQ0"  : 1,
                             "Q1"   : 2,
                             "DQ1"  : 2,
                             "DPC1" : 2,
                             "Q2"   : 3  }
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

        componentDictionary[ self._particleLayout.name ][                    "dim"] = self._mesh.dim
        componentDictionary[ self._particleLayout.name ][         "gaussParticles"] = self._particleCount

class GaussBorderIntegrationSwarm(GaussIntegrationSwarm):
    """
    Parameters
    ----------
    mesh : uw.mesh.FeMesh
        The FeMesh the swarm is supported by. See Swarm.mesh property docstring
        for further information.
        
    particleCount : unsigned
        Number of gauss particles in each direction.  Must take value in [1,5].
        Default behaviour chooses an appropriate count for the provided mesh:
            Constant : 1
              Linear : 2
           Quadratic : 4

    """
    _objectsDict = { "_particleLayout" : "GaussBorderParticleLayout" }

    def __init__(self, mesh, particleCount=None, **kwargs):
        # build parent
        super(GaussBorderIntegrationSwarm,self).__init__(mesh, particleCount, **kwargs)
