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
    swarm : underworld.swarm.Swarm
        The swarm for which population control should occur.

    deleteThreshold : float
        Particle volume fraction threshold below which particle is deleted.
        i.e if (particleVolume/elementVolume)<deleteThreshold, then the
        particle is deleted.

    splitThreshold : float
        Particle volume fraction threshold above which particle is split.
        i.e if (particleVolume/elementVolume)>splitThreshold, then the
        particle is split.

    maxDeletions : int
        maximum number of particle deletions per cell

    maxSplits : int
        maximum number of particles splits per cell

    aggressive : bool
        When enabled, this option will invoke aggressive population control
        in elements where particle counts drop below some threshold.

    aggressiveThreshold : float
        lower cell particle population threshold beyond which aggressive 
        population control occurs.
        i.e if (cellParticleCount/particlesPerCell)<aggressiveThreshold, then 
        aggressive pop control will occur.
        Note that this option is only valid if 'aggressive' is enabled.
    
    particlesPerCell: int
        This is the desired number of particles each element should contain.
        Note that this option is only valid if 'aggressive' is enabled.

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
                 aggressive=False, aggressiveThreshold=0.8, particlesPerCell=None,
                 **kwargs):

        self._swarm = swarm
        self._weights = _weights.PCDVC( swarm=swarm,
                                        deleteThreshold=deleteThreshold, splitThreshold=splitThreshold,
                                        maxDeletions=maxDeletions, maxSplits=maxSplits,
                                        aggressive=aggressive, aggressiveThreshold=aggressiveThreshold, particlesPerCell=particlesPerCell,
                                        **kwargs  )


        # build parent
        super(PopulationControl,self).__init__(**kwargs)

    def repopulate(self):
        """
        This method repopulates the swarm.
        """
        self._swarm._voronoi_swarm.repopulate(weights_calculator=self._weights)
        # repopulation potentially adds/removes particles... so we need to increment
        # the swarm id.  note that this should occur *after* the repopulate call
        # to avoid a potential unnecessary extra call to the coincident mapper
        self._swarm._toggle_state()

