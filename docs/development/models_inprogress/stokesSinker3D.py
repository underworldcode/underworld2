# To execute
# python stokesSinker3D.py -elRes 9,9,9 ; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py sol.h5; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py aux.h5

import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn

sphereCentre = (.5,.5,0.7)
sphereRadius = 0.25
coord = fn.input() - sphereCentre
fn_sphere = 1. * fn.math.dot( coord, coord ) < sphereRadius**2
#fn_sphere = fn.input()[0]*fn.input()[1]*fn.input()[2]

model = pl.StokesModel_Setup(0)

pl.StokesModel_SetDensityFn(model, fn_sphere._fncself)

pl.StokesModel_Solve(model)
