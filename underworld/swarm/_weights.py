##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld._stgermain as _stgermain
import _swarm

class DVC(_stgermain.StgCompoundComponent):
    """
    Discrete Voronoi Cell weights calculator.

    Parameters
    ----------
    resolutionX, resolutionY, resolutionZ : int, default 15
        The resolution of the grid used for the discrete voronoi algorithm.

    """
    _objectsDict = { "_weights": "DVCWeights" }
    _selfObjectName = "_weights"

        # build parent
    def __init__(self, resx=15,resy=15,resz=15, **kwargs):
        self.resx=resx
        self.resy=resy
        self.resz=resz
        # build parent
        super(DVC,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        super(DVC,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._weights.name ]["resolutionX"] = self.resx
        componentDictionary[ self._weights.name ]["resolutionY"] = self.resy
        componentDictionary[ self._weights.name ]["resolutionZ"] = self.resz

class PCDVC(DVC):
    """
    Population Control Discrete Voronoi Cells. This class both calculates 
    particle weights, and also performs particle population control.
    
    Please check the public facing PopulationControl class for parameter
    descriptions.
    """
    _objectsDict = { "_weights": "PCDVC" }

    def __init__(   self,
                    swarm,
                    deleteThreshold, splitThreshold, maxDeletions, maxSplits,
                    inflow, inflowThreshold, particlesPerCell,
                    **kwargs):

        if not isinstance(swarm, _swarm.Swarm):
            raise ValueError("Provided 'swarm' must be of class 'Swarm'.")
        self._swarm = swarm

        if not isinstance(deleteThreshold, (int,float)):
            raise TypeError("'deleteThreshold' parameter must of type 'float'.")
        if (deleteThreshold<0.) or (deleteThreshold>1.):
            raise ValueError("'deleteThreshold' parameter must take values in [0.,1.].")
        self._deleteThreshold=deleteThreshold
        if not isinstance(splitThreshold, (int,float)):
            raise TypeError("'splitThreshold' parameter must of type 'float'.")
        if (splitThreshold<0.) or (splitThreshold>1.):
            raise ValueError("'splitThreshold' parameter must take values in [0.,1.].")
        self._splitThreshold=splitThreshold

        if deleteThreshold > splitThreshold:
            raise ValueError("'deleteThreshold' must be less than 'splitThreshold'.")
        
        if not isinstance(maxDeletions, int):
            raise TypeError("'maxDeletions' parameter must of type 'int'.")
        if maxDeletions<0:
            raise ValueError("'maxDeletions' parameter must take non-negative values.")
        self._maxDeletions=maxDeletions

        if not isinstance(maxSplits, int):
            raise TypeError("'maxSplits' parameter must of type 'int'.")
        if maxSplits<0:
            raise ValueError("'maxSplits' parameter must take non-negative values.")
        self._maxSplits=maxSplits

        if not isinstance(inflow, bool):
            raise TypeError("'inflow' parameter must of type 'bool'.")
        self._inflow=inflow

        if inflow:
            if not isinstance(inflowThreshold, (int,float)):
                raise TypeError("'inflowThreshold' parameter must of type 'float'.")
            if (inflowThreshold<0.) or (inflowThreshold>1.):
                raise ValueError("'inflowThreshold' parameter must take values in [0.,1.].")
            self._inflowThreshold=inflowThreshold

            if particlesPerCell is None:
                raise ValueError("'particlesPerCell' parameter must be provided where 'inflow' is True.")
            if not isinstance(particlesPerCell, int):
                raise TypeError("'particlesPerCell' parameter must of type 'int'.")
            if particlesPerCell<=0:
                raise ValueError("'particlesPerCell' parameter must take positive values.")
            self._particlesPerCell=particlesPerCell
        # build parent
        super(PCDVC,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        super(PCDVC,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._weights.name ][    "GeneralSwarm"] = self._swarm._cself.name
        componentDictionary[ self._weights.name ][          "lowerT"] = self._deleteThreshold*100.
        componentDictionary[ self._weights.name ][          "upperT"] = self._splitThreshold*100.
        componentDictionary[ self._weights.name ][    "maxDeletions"] = self._maxDeletions
        componentDictionary[ self._weights.name ][       "maxSplits"] = self._maxSplits
        componentDictionary[ self._weights.name ][          "Inflow"] = self._inflow
        if self._inflow:
            componentDictionary[ self._weights.name ][       "Threshold"] = self._inflowThreshold
            componentDictionary[ self._weights.name ]["particlesPerCell"] = self._particlesPerCell
