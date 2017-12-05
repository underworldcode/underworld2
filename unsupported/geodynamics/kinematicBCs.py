from LecodeIsostasy import LecodeIsostasy
from scaling import nonDimensionalize as nd
import scaling as sca
import underworld as uw

u = UnitRegistry = sca.UnitRegistry

class VelocityBCs(object):

    def __init__(self, Model, left=None, right=None, top=None, bottom=None,
                 front=None, back=None, indexSets=[]):


        self.Model = Model
        self.left = left
        self.right = right
        self.top = top
        self.bottom = bottom
        self.back = back
        self.front = front
        self.indexSets = indexSets

    @staticmethod
    def isNeumann(x):
        if not isinstance(x, u.Quantity):
            return False
        val = x.to_base_units()
        return val.units == u.kilogram / (u.meter * u.second**2)
    
    def get_conditions(self):

        # A lot of that code is redundant...
        # Could be cleaned up a bit...

        Model = self.Model
        dirichletIndices = []
        neumannIndices = []

        for dim in range(Model.mesh.dim):
            dirichletIndices.append(Model.mesh.specialSets["Empty"])
            neumannIndices.append(Model.mesh.specialSets["Empty"])

        if self.left is not None:
            for dim in range(Model.mesh.dim):
                if self.left[dim] is not None:
                    if VelocityBCs.isNeumann(self.left[dim]):
                        Model.tractionField.data[Model.leftWall.data, dim] = nd(self.left[dim])
                        neumannIndices[dim] += Model.leftWall
                    else:
                        Model.velocityField.data[Model.leftWall.data, dim] = nd(self.left[dim])
                        dirichletIndices[dim] += Model.leftWall
        if self.right is not None:
            for dim in range(Model.mesh.dim):
                if self.right[dim] is not None:
                    if VelocityBCs.isNeumann(self.right[dim]):
                        Model.tractionField.data[Model.rightWall.data, dim] = nd(self.right[dim])
                        neumannIndices[dim] += Model.rightWall
                    else:
                        Model.velocityField.data[Model.rightWall.data, dim] = nd(self.right[dim])
                        dirichletIndices[dim] += Model.rightWall
   
        if self.top is not None:
            for dim in range(Model.mesh.dim):
                if self.top[dim] is not None:
                    if VelocityBCs.isNeumann(self.top[dim]):
                        Model.tractionField.data[Model.topWall.data, dim] = nd(self.top[dim])
                        neumannIndices[dim] += Model.topWall
                    else:
                        Model.velocityField.data[Model.topWall.data, dim] = nd(self.top[dim])
                        dirichletIndices[dim] += Model.topWall

        if self.bottom is not None:
            if isinstance(self.bottom, LecodeIsostasy):
                Model.Isostasy = self.bottom
                Model.Isostasy.mesh = Model.mesh
                Model.Isostasy.swarm = Model.swarm
                Model.Isostasy.velocityField = Model.velocityField
                Model.Isostasy.materialIndexField = Model.materialField
                Model.Isostasy.densityFn = Model.densityFn
                dirichletIndices[-1] += Model.bottomWall
            else:
                Model.Isostasy = None
                for dim in range(Model.mesh.dim):
                    if self.bottom[dim] is not None:
                        if VelocityBCs.isNeumann(self.bottom[dim]):
                            Model.tractionField.data[Model.bottomWall.data, dim] = nd(self.bottom[dim])
                            neumannIndices[dim] += Model.bottomWall
                        else:
                            Model.velocityField.data[Model.bottomWall.data, dim] = nd(self.bottom[dim])
                            dirichletIndices[dim] += Model.bottomWall
        
        if self.front is not None and Model.mesh.dim > 2:
            for dim in range(Model.mesh.dim):
                if self.front[dim] is not None:
                    if VelocityBCs.isNeumann(self.front[dim]):
                        Model.tractionField.data[Model.frontWall.data, dim] = nd(self.front[dim])
                        neumannIndices[dim] += Model.frontWall
                    else:
                        Model.velocityField.data[Model.frontWall.data, dim] = nd(self.front[dim])
                        dirichletIndices[dim] += Model.frontWall
        
        if self.back is not None and Model.mesh.dim > 2:
            for dim in range(Model.mesh.dim):
                if self.back[dim] is not None:
                    if VelocityBCs.isNeumann(self.back[dim]):
                        Model.tractionField.data[Model.backWall.data, dim] = nd(self.back[dim])
                        neumannIndices[dim] += Model.backWall
                    else:
                        Model.velocityField.data[Model.backWall.data, dim] = nd(self.back[dim])
                        dirichletIndices[dim] += Model.backWall

        for indexSet, temp in self.indexSets:
            for dim in range(Model.mesh.dim):
                if indexSet[dim] is not None:
                    Model.velocityField.data[indexSet.data, dim] = nd(indexSet[dim])
                    dirichletIndices[dim] += indexSet

        conditions = []            

        conditions.append(uw.conditions.DirichletCondition(variable=Model.velocityField,
                                                           indexSetsPerDof=dirichletIndices))

        neumannIndices = tuple([val if val.data.size > 0 else None for val in neumannIndices])
       
        if neumannIndices != (None, None):
            conditions.append(uw.conditions.NeumannCondition(fn_flux=Model.tractionField,
                                                             variable=Model.velocityField,
                                                             indexSetsPerDof=neumannIndices))

        if not conditions:
            raise ValueError("Undefined conditions, please check your condition")

        return conditions
