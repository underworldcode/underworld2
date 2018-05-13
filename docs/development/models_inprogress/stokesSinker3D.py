# To execute
# python stokesSinker3D.py -elRes 9,9,9 ; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py sol.h5; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py aux.h5

import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn
import numpy as np

nSpheres = 5
fn_conds = []
inside = lambda x : [ (x, 1.), (True, 0.)]
for s_i in xrange(nSpheres):
    s,lati,longi, radius =  (3, 180, 360, 2.5)*np.random.rand(4) + (6., -90, 0, 0.5)
    print s, lati, longi, radius
    lati,longi = np.radians([lati, longi])
    center = (s * np.cos(longi) * np.cos(lati),
              s * np.sin(longi) * np.cos(lati),
              s *                 np.sin(lati))
    fn_pos = fn.coord() - center
    fn_conds.append( (fn.math.dot( fn_pos, fn_pos ) < radius**2, 1.) )
    
fn_conds.append( (True, 0.0 ) ) # default condition
fn_density = fn.branching.conditional( fn_conds )

fn_r = fn.math.sqrt( fn.math.dot( fn.coord(), fn.coord() ) )
fn_gravity =  fn.misc.constant(9.8) / fn_r * (fn.coord()[0], fn.coord()[1], fn.coord()[2])

'''
sphereCentre = (.5,.5,0.7)
sphereRadius = 0.25
coord = fn.input() - sphereCentre
fn_cond = []
fn_cond.append( (fn.math.dot(coord, coord ) < sphereRadius**2, 1.) )
fn_cond.append( (True, 0.) )
fn_density = fn.branching.conditional( fn_cond )

fn_gravity =  fn.misc.constant(9.8) * (0., 0., 1.)
'''

fn_buoyancy = fn_gravity * fn_density

model = pl.StokesModel_Setup(1)

pl.StokesModel_SetForceTerm(model, fn_buoyancy._fncself)

pl.StokesModel_Solve(model)


'''
PSEUDO code for building python system
class Stokes(object)
    def __init__(self, filename, auxFields):
        #error check auxFields - tuple field and dofs
        
        sle = Stokes_Setup(filename)
        sle.fnv = sle.StokesModel_GetV(sle)
        sle.fnp = sle.StokesModel_GetP(sle)
        sle.aux = []
        naux = 0

        for field in auxFields:
            name, dof = field[0], field[1]
            sle.aux[naux] = create_fe( sle, dof, name )
'''
