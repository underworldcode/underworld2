from scaling import nonDimensionalize as nd
import underworld as uw

class TemperatureBCs(object):

    def __init__(self, Model, left=None, right=None, top=None, bottom=None,
                 front=None, back=None,
                 indexSets=[], materials=[(None,None)]):


        self.Model = Model
        self.left = left
        self.right = right
        self.top = top
        self.bottom = bottom
        self.front = front
        self.back = back
        self.indexSets = indexSets
        self.materials = materials
    
    def get_conditions(self):
       
        Model = self.Model
        
        indices = [Model.mesh.specialSets["Empty"]]
        if self.left is not None:
            Model.temperature.data[Model.leftWall.data] = nd(self.left)
            indices[0] += Model.leftWall
        if self.right is not None:
            Model.temperature.data[Model.rightWall.data] = nd(self.right)
            indices[0] += Model.rightWall
        if self.top is not None:
            Model.temperature.data[Model.topWall.data] = nd(self.top)
            indices[0] += Model.topWall
        if self.bottom is not None:
            Model.temperature.data[Model.bottomWall.data] = nd(self.bottom)
            indices[0] += Model.bottomWall
        if self.back is not None and Model.mesh.dim > 2:
            Model.temperature.data[Model.backWall.data] = nd(self.back)
            indices[0] += Model.backWall
        if self.front is not None and Model.mesh.dim > 2:
            Model.temperature.data[Model.frontWall.data] = nd(self.front)
            indices[0] += Model.frontWall

        for indexSet, temp in self.indexSets:
            Model.temperature.data[indexSet.data] = nd(temp)
            indices[0] += indexSet

        for (material, temp) in self.materials:
            if material and nd(temp):
                indexSet = Model._get_material_indices(material)
                Model.temperature.data[indexSet.data] = nd(temp)
                indices[0] += indexSet

        return uw.conditions.DirichletCondition(variable=Model.temperature,
                                                 indexSetsPerDof=indices)
