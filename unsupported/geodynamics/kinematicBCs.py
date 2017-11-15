from LecodeIsostasy import LecodeIsostasy
from scaling import nonDimensionalize as nd
import underworld as uw

class VelocityBCs(object):

    def __init__(self, Model, left=None, right=None, top=None, bottom=None,
                 indexSets=[]):


        self.Model = Model
        self.left = left
        self.right = right
        self.top = top
        self.bottom = bottom
        self.indexSets = indexSets


    def get_conditions(self):
       
        Model = self.Model
        indices = []

        for dim in range(Model.mesh.dim):
            indices.append(Model.mesh.specialSets["Empty"])

        if self.left is not None:
            for dim in range(Model.mesh.dim):
                if self.left[dim] is not None:
                    Model.velocityField.data[Model.leftWall.data, dim] = nd(self.left[dim])
                    indices[dim] += Model.leftWall
        if self.right is not None:
            for dim in range(Model.mesh.dim):
                if self.right[dim] is not None:
                    Model.velocityField.data[Model.rightWall.data, dim] = nd(self.right[dim])
                    indices[dim] += Model.rightWall
        if self.top is not None:
            for dim in range(Model.mesh.dim):
                if self.top[dim] is not None:
                    Model.velocityField.data[Model.topWall.data, dim] = nd(self.top[dim])
                    indices[dim] += Model.topWall
        if self.bottom is not None:
            if isinstance(self.bottom, LecodeIsostasy):
                Model.Isostasy = self.bottom
                Model.Isostasy.mesh = Model.mesh
                Model.Isostasy.swarm = Model.swarm
                Model.Isostasy.velocityField = Model.velocityField
                Model.Isostasy.materialIndexField = Model.materialField
                Model.Isostasy.densityFn = Model.densityFn
                indices[-1] += Model.bottomWall
            else:    
                for dim in range(Model.mesh.dim):
                    if self.bottom[dim] is not None:
                        Model.velocityField.data[Model.bottomWall.data, dim] = nd(self.bottom[dim])
                        indices[dim] += Model.bottomWall

        for indexSet, temp in self.indexSets:
            for dim in range(Model.mesh.dim):
                if indexSet[dim] is not None:
                    Model.velocityField.data[indexSet.data, dim] = nd(indexSet[dim])
                    indices[dim] += indexSet

        return uw.conditions.DirichletCondition(variable=Model.velocityField,
                                                indexSetsPerDof=indices)
