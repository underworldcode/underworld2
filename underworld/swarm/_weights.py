##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld._stgermain as _stgermain

class _Weights(_stgermain.StgCompoundComponent):
    """
    """
    _objectsDict = { "_weights": None }
    _selfObjectName = "_weights"

    def __init__(self, **kwargs):
        """
        """

        # build parent
        super(_Weights,self).__init__(**kwargs)

    #def _add_to_stg_dict(self,componentDictionary):
     #   super(_Weights,self)._add_to_stg_dict(componentDictionary)


class PCDVC(_Weights):
    """

    """
    _objectsDict = { "_weights": "PCDVC" }

    def __init__(self, swarm,
                 resx=15,resy=15,resz=15,
                 lowerVolumeThreshold=0.6, upperVolumeThreshold=25,
                 maxDeletions=3, maxSplits=3,
                 centroidPositionRatio=0.01,
                 threshold = 0.8,
                 particlesPerCell = 25,
                 inFlow=True, **kwargs
             ):

        self._swarm = swarm
        self.resx=resx
        self.resy=resy
        self.resz=resz
        self.lowerT=lowerVolumeThreshold
        self.upperT=upperVolumeThreshold
        self.thresh=threshold
        self.particlesPerCell=particlesPerCell
        self.inFlow=inFlow
        self.maxSplits=maxSplits
        self.maxDeletions=maxDeletions
        self.centroidPositionRatio=centroidPositionRatio
        # build parent
        super(PCDVC,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        super(PCDVC,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._weights.name ]["resolutionX"] = self.resx
        componentDictionary[ self._weights.name ]["resolutionY"] = self.resy
        componentDictionary[ self._weights.name ]["resolutionZ"] = self.resz
        componentDictionary[ self._weights.name ]["lowerT"] = self.lowerT
        componentDictionary[ self._weights.name ]["upperT"] = self.upperT
        componentDictionary[ self._weights.name ]["maxDeletions"] = self.maxDeletions
        componentDictionary[ self._weights.name ]["maxSplits"] = self.maxSplits
        componentDictionary[ self._weights.name ]["centPosRatio"] = self.centroidPositionRatio
        componentDictionary[ self._weights.name ]["Inflow"] = self.inFlow
        componentDictionary[ self._weights.name ]["particlesPerCell"] = self.particlesPerCell
        componentDictionary[ self._weights.name ]["GeneralSwarm"] = self._swarm._cself.name
