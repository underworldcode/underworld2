from unsupported.linkage import SPM
import underworld.function as fn
from unsupported.scaling import nonDimensionalize as nd


class Badlands(object):
    """ A wrapper class for Badlands Linkage"""

    def __init__(self, airIndex,
                 sedimentIndex, XML, resolution, checkpoint_interval,
                 surfElevation=0., verbose=True, Model=None):

        self.airIndex = airIndex
        self.sedimentIndex = sedimentIndex
        self.XML = XML
        self.resolution = resolution
        self.checkpoint_interval = checkpoint_interval
        self.surfElevation = surfElevation
        self.verbose = verbose
        self.Model = Model
        return

    @property
    def Model(self):
        return self._Model

    @Model.setter
    def Model(self, value):
        self._Model = value
        if value:
            self._init_Badlands()

    def _init_Badlands(self):
        self.mesh = self._Model.mesh
        self.velocityField = self._Model.velocityField
        self.swarm = self._Model.swarm
        self.materialField = self._Model.materialField
        
        self._BadlandsModel = SPM(self.mesh, self.velocityField, self.swarm,
                                 self.materialField, self.airIndex, self.sedimentIndex,
                                 self.XML, nd(self.resolution), 
                                 nd(self.checkpoint_interval),
                                 nd(self.surfElevation), self.verbose)
        return


    def solve(self, dt):
        self._BadlandsModel.solve(dt)
        return


class FullErosionAboveSeaLevel(object):

    def __init__(self, swarm=None, materialIndexField=None,
                 airIndices=None, sealevel=None):

        self.materialIndexField = materialIndexField
        self.swarm = swarm
        self.sealevel = sealevel

        aboveSeaLevel = [(((self.materialIndexField not in airIndices) &(fn.input()[1] >  nd(sealevel))), airIndices[0]),
                         (True, materialIndexField)]

        self._fn = fn.branching.conditional(aboveSeaLevel) 

    def solve(self, dt):
        self._fn.evaluate(self.swarm)
        return


class FullSedimentationAboveSeaLevel(object):

    def __init__(self, swarm=None, materialIndexField=None,
                 sedimentIndex=None, sealevel=None):

        self.materialIndexField = materialIndexField
        self.swarm = swarm
        self.sealevel = sealevel

        belowSeaLevel = [(((self.materialIndexField in airIndices) & (fn.input()[1] < nd(sealevel))), sedimentIndex[0]),
                         (True, materialIndexField)]

        self._fn = fn.branching.conditional(belowSeaLevel) 

    def solve(self, dt):
        self._fn.evaluate(self.swarm)
        return


class FullErosionAndSedimentationAboveSeaLevel(object):

    def __init__(self, swarm=None, materialIndexField=None, airIndices=None,
                 sedimentIndex=None, sealevel=None):

        self.materialIndexField = materialIndexField
        self.swarm = swarm
        self.sealevel = sealevel

        erosion = FullErosionAboveSeaLevel(swarm, materialIndexField,
                                           airIndices, sealevel)
        self._fn1 = erosion._fn

        sedimentation = FullSedimentationAboveSeaLevel(swarm, materialIndexField,
                                                       sedimentIndex, sealevel)
        self._fn2 = sedimentation._fn

    def solve(self, dt):
        self._fn1.evaluate(self.swarm)
        self._fn2.evaluate(self.swarm)
        return

