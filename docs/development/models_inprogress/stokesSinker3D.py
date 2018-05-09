# To execute
# python stokesSinker3D.py -elRes 9,9,9 ; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py sol.h5; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py aux.h5

import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn
import numpy as np

nSpheres = 10
fn_conds = 0.
inside = lambda x : [ (x, 1.), (True, 0.)]
for s_i in xrange(nSpheres):
    s,lati,longi, radius =  (3, 180, 360, 2.5)*np.random.rand(4) + (6., -90, 0, 0.1)
    lati,longi = np.radians([lati, longi])
    center = (s * np.cos(longi) * np.sin(lati),
              s * np.sin(longi) * np.sin(lati),
              s *                 np.cos(lati))
    fn_pos = fn.coord() - center
    geometry = 1. * fn.math.dot( fn_pos, fn_pos ) < radius**2
#     fn_conds.append( fn.branching.conditional( inside( geometry ) ) ) ASK JOHNNY
    fn_conds += fn.branching.conditional( inside( geometry ) )
    
fn_shape = fn.branching.conditional([(fn_conds > 0.5, 1.), (True, 0.)])

'''
sphereCentre = (.5,.5,0.7)
sphereRadius = 0.25
sphereCentre = (.0,.0,8.)
sphereRadius = 2.5
coord = fn.input() - sphereCentre
fn_sphere = 1. * fn.math.dot( coord, coord ) < sphereRadius**2
#fn_sphere = fn.input()[0]*fn.input()[1]*fn.input()[2]
'''

model = pl.StokesModel_Setup('helloWorld')

pl.StokesModel_SetDensityFn(model, fn_shape._fncself)

#pl.StokesModel_Solve(model)
