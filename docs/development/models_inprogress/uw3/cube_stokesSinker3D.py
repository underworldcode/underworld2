import underworld as uw
from underworld import function as fn
import numpy as np

sphereRadius = 0.3         # define radius
spheres = [ (0.5,0.5,0.7)]
'''
spheres = [ (.5,.5,0.7),   # define list of sphere centres
            (.25,.25,0.6),
            (.25,.75, 0.5),
            (.75,.75,0.6),
            (.75,.25,0.4) ]
'''
fn_conds = []   # geometric condition for each sphere, if true, return 1.2
for s_i, centre, in enumerate(spheres):
    fn_pos = fn.coord() - centre
    fn_conds.append( (fn.math.dot( fn_pos, fn_pos ) < sphereRadius**2, 1.5) )

fn_conds.append( (True, 1.0) )
fn_density = fn.branching.conditional( fn_conds )


fn_gravity =  fn.misc.constant(9.8) * (0., 0., -1.)

fn_buoyancy = fn_gravity * fn_density

v_const = fn.misc.constant([4.0,2.,0.])
f_const = fn.misc.constant([1.0,2.,0.])
e_const = fn.misc.constant(1.0)

model = uw.systems.pl_StokesModel(filename=None)
model.SetViscosity(f_const[0])
model.SetRHS(fn_buoyancy)
model.Solve()
