# To execute
# python stokesSinker3D.py -elRes 9,9,9 ; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py sol.h5; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py aux.h5

import underworld as uw
from underworld import function as fn
import numpy as np

'''
nSpheres = 1
fn_conds = []
for s_i in xrange(nSpheres):
    s,lati,longi, radius =  (7, 0, 0, 1.3)
    #s,lati,longi, radius =  (3, 180, 360, 2.5)*np.random.rand(4) + (6., -90, 0, 0.5)
    print s, lati, longi, radius
    lati,longi = np.radians([lati, longi])
    center = (s * np.cos(lati) * np.cos(longi),
              s * np.cos(lati) * np.sin(longi),
              s * np.sin(lati)                 )
    fn_pos = fn.coord() - center
    fn_conds.append( (fn.math.dot( fn_pos, fn_pos ) < radius**2, 1.) )
    
fn_conds.append( (True, 0.0 ) ) # default condition
fn_density = fn.branching.conditional( fn_conds )

fn_r = fn.math.sqrt( fn.math.dot( fn.coord(), fn.coord() ) )
fn_gravity =  fn.misc.constant(9.8) / fn_r * (fn.coord()[0], fn.coord()[1], fn.coord()[2])

'''

sphereRadius = 0.2         # define radius
spheres = [ (.5,.5,0.7),   # define list of sphere centres
            (.25,.25,0.6),
            (.25,.75, 0.5),
            (.75,.75,0.6),
            (.75,.25,0.4) ]

fn_conds = []   # geometric condition for each sphere, if true, return 1.2
for s_i, centre, in enumerate(spheres):
    fn_pos = fn.coord() - centre
    fn_conds.append( (fn.math.dot( fn_pos, fn_pos ) < sphereRadius**2, 1.2) )

fn_conds.append( (True, 1.0) )
fn_density = fn.branching.conditional( fn_conds )


fn_gravity =  fn.misc.constant(9.8) * (0., 0., 1.)
fn_buoyancy = fn_gravity * fn_density

v_const = fn.misc.constant([4.0,2.,0.])
f_const = fn.misc.constant([4.0,2.,0.])
e_const = fn.misc.constant(1.0)

model = uw.systems.pl_StokesModel(filename=None)
model.SetViscosity(f_const[0])
model.SetRHS(fn_buoyancy)
model.Solve()
