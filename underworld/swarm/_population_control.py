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
import _weights

class PopulationControl(_stgermain.LeftOverParamsChecker):
    """
    This class implements swarm population control mechanism. Population control
    acts on a per element basic, with a discrete voronoi algorithm is used to 
    determine where particles should be added or removed.

    Parameters
    ----------
    swarm : uw.swarm.Swarm
        The swarm for which population control should occur.

    deleteThreshold : default 0.006
        Particle volume fraction threshold below which particle is deleted.
        i.e if (particleVolume/elementVolume)<deleteThreshold, then the
        particle is deleted.

    splitThreshold : default 0.25
        Particle volume fraction threshold above which particle is split.
        i.e if (particleVolume/elementVolume)>splitThreshold, then the
        particle is split.

    maxDeletions : int, default 0
        maximum number of particle deletions per cell

    maxSplits : int, default 3
        maximum number of particles splits per cell

    inflow : bool, default False
        When enabled, this option will invoke aggressive population control
        in elements where particle counts drop below some threshold.

    inflowThreshold : default 0.8
        lower cell particle population threshold beyond which inflow occurs.
        i.e if (cellParticleCount/particlesPerCell)<inflowThreshold, then 
        inflow will be enabled.
        Note that this option is only valid if 'inflow' is enabled.
    
    particlesPerCell:
        This is the desired number of particles each element should contain.
        Note that this option is only valid if 'inflow' is enabled.

    Example
    -------
    This simple example generates a swarm, then applies population control
    to split particles.

    >>> import underworld as uw
    >>> import numpy as np
    >>> mesh = uw.mesh.FeMesh_Cartesian()
    >>> swarm = uw.swarm.Swarm(mesh)
    >>> swarm.populate_using_layout(uw.swarm.layouts.PerCellGaussLayout(swarm,4))
    >>> population_control = uw.swarm.PopulationControl(swarm,deleteThreshold=0.,splitThreshold=0.,maxDeletions=0,maxSplits=9999)
    >>> population_control.repopulate()
    >>> swarm.particleGlobalCount
    512


    """

    def __init__(self, swarm,
                 deleteThreshold=0.006, splitThreshold=0.25, maxDeletions=0, maxSplits=3,
                 inflow=False, inflowThreshold=0.8, particlesPerCell=None,
                 **kwargs):

        self._swarm = swarm
        self._weights = _weights.PCDVC( swarm=swarm,
                                        deleteThreshold=deleteThreshold, splitThreshold=splitThreshold,
                                        maxDeletions=maxDeletions, maxSplits=maxSplits,
                                        inflow=inflow, inflowThreshold=inflowThreshold, particlesPerCell=particlesPerCell,
                                        **kwargs  )


        # build parent
        super(PopulationControl,self).__init__(**kwargs)

    def repopulate(self):
        """
        This method repopulates the swarm.
        """
        self._swarm._voronoi_swarm.repopulate(weights_calculator=self._weights)

