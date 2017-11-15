import numpy as np
import underworld as uw
import underworld.function as fn
from scaling import nonDimensionalize as nd


class Shape(object):

    def __init__(self):
        return
    
    @property
    def fn(self):
        self._init_shape()
        return self._fn

    def evaluate(self, obj):
        self._init_shape()
        return self._fn.evaluate(obj)


class Polygon(Shape):

    def __init__(self, vertices):
        self.vertices = vertices

    def _init_shape(self):
        vertices = [(nd(x), nd(y)) for x, y in self.vertices]
        self._fn = uw.function.shape.Polygon(np.array(vertices))


class Layer(Shape):

    def __init__(self, top, bottom, minX=0., maxX=0., minY=None, maxY=None):
        self.top = top
        self.bottom = bottom
        self.minX = minX
        self.maxX = maxX
        self.minY = minY
        self.maxY = maxY

    def _init_shape(self):
        coord = fn.input()
        if (self.minY is not None) and (self.maxY is not None):
            self._fn = ((coord[2] <= nd(self.top)) & (coord[2] >= nd(self.bottom)))
        else:
            self._fn = ((coord[1] <= nd(self.top)) & (coord[1] >= nd(self.bottom)))

    @property
    def minX(self):
        return self._minX
    
    @minX.setter
    def minX(self, value):
        self._minX = value

    @property
    def maxX(self):
        return self._maxX
    
    @maxX.setter
    def maxX(self, value):
        self._maxX = value
   
    @property
    def top(self):
        return self._top
    
    @top.setter
    def top(self, value):
        self._top = value
    
    @property
    def bottom(self):
        return self._bottom
    
    @bottom.setter
    def bottom(self, value):
        self._bottom = value


class Disk(Shape):

    def __init__(self, center, radius):
        self.center = center
        self.radius = radius

    def _init_shape(self):
        center = tuple(nd(x) for x in list(self.center))
        radius = nd(self.radius)
        coord = fn.input() - center
        self._fn = fn.math.dot(coord, coord) < radius**2


class Annulus(Shape):
    
    def __init__(self, center, r1, r2):
        self.center = center
        self.r1 = r1
        self.r2 = r2

    def _init_shape(self):
        center = tuple(nd(x) for x in list(self.center))
        r1 = nd(self.r1)
        r2 = nd(self.r2)
        coord = fn.input() - center
        self._fn = (fn.math.dot(coord, coord) < r2**2) & (fn.math.dot(coord, coord) > r1**2)

