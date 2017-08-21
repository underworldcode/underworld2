import numpy as np
import underworld as uw

def layer(top, bottom, minX, maxX):
    if bottom == None or top == None:
        return None
    vertex_array = np.array( [(minX, bottom),(minX, top),(maxX, top),(maxX, bottom)] )
    return uw.function.shape.Polygon(vertex_array)
