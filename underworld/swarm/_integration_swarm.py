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
import underworld.function as function


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
            self._particleCoordinates = svar.SwarmVariable(self, "double", self.mesh.generator.dim, _cself=self._cself.localCoordVariable)
        if self._cself.weightVariable:
            self._weightsVariable = svar.SwarmVariable(self, "double", 1, _cself=self._cself.weightVariable)

    @property
    def particleWeights(self):
        """
        particleWeights (SwarmVariable): Swarm variable recording the weight of the
        swarm particles.
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
    swarm : uw.swarm.Swarm
        The PIC integration swarm maps to this user provided swarm.

    lowerVolumeThreshold : 0-100%, default 0.6
        lower % threshold volume for deletion of particles.
        i.e if a particle volume < 0.25% of total then delete it

    upperVolumeThreshold : 0-100%, default 25
        upper % threshold volume for deletion of particles.
        i.e if a particle volume > 15% of total then split it

    maxDeletions : int, default 3
        maximum number of particle deletions per cell

    maxSplits : int, default 3
        maximum number of particles splits per cell

    resolutionX, resolutionY, resolutionZ : int, default 15
        The resolution of the mesh used for the discrete voronoi algorithm

    shotgun : bool, default False
        Enable shotgun-style population control, for agressive repopulation.
        Was formerly known as the 'inflow' option.

    threshold : 0.0-1.0, default 0.8
        Threshold for cell population in an inflow problem:
        If a cell has less than 80% of its assigned particles then we re-populate

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
    >>> picswarm = uw.swarm.VoronoiIntegrationSwarm(swarm)
    >>> picswarm.repopulate()
    >>> np.allclose(swarm.particleCoordinates.data, uw.function.input().evaluate(picswarm),atol=1e-1)
    True

    """
    _objectsDict = {  "_cellLayout" : "ElementCellLayout",
                          "_mapper" : "CoincidentMapper"
                    }

    def __init__(self, swarm, resx=15,resy=15,resz=15,
                 lowerVolumeThreshold=0.6, upperVolumeThreshold=25,
                 maxDeletions=3, maxSplits=10,
                 centroidPositionRatio=0.01,
                 threshold = 0.8,
                 particlesPerCell = 25,
                 shotgun=False, **kwargs):

        if not isinstance(swarm, uw.swarm.Swarm):
            raise ValueError("Provided swarm must be of class 'Swarm'.")
        self._mappedSwarm = swarm
        swarm._PICSwarm = self

        # note that if the mapped swarm allows particles to escape, lets by default
        # switch inflow on.
        self._weights = uw.swarm._weights.PCDVC(swarm, inFlow=shotgun,
            particlesPerCell=particlesPerCell, maxDeletions=maxDeletions, maxSplits=maxSplits, resx=resx,resy=resy,resz=resz,
            lowerVolumeThreshold=lowerVolumeThreshold, upperVolumeThreshold=upperVolumeThreshold,
            centroidPositionRatio=centroidPositionRatio,
            threshold=threshold)

        # build parent
        super(VoronoiIntegrationSwarm,self).__init__(swarm.mesh, **kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method

        super(VoronoiIntegrationSwarm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._swarm.name ][      "WeightsCalculator"] = self._weights._cself.name

        componentDictionary[ self._cellLayout.name ][              "Mesh"] = self._mesh._cself.name

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

    def _get_iterator(self):
        """
        This is the concrete method required by the FunctionInput class.

        It effects using the PICSwarm as an input to functions.
        """
        return libUnderworld.Function.IntegrationSwarmInput(self._cself)


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

        componentDictionary[ self._particleLayout.name ][                    "dim"] = self._mesh.generator.dim
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
