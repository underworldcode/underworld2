import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn
import numpy as np

model = uw.systems.pl_PoissonModel()

# these following 2 fn are made available by the model
temperature  = model.fn_temperature

'''
PI = fn.misc.constant(3.14159265359)
my_k = fn.math.sin( PI * ( fn_r / 5. - 1. ) )
'''

someFn = fn.misc.constant(-1)
fn_r = fn.math.sqrt( fn.math.dot( fn.coord(), fn.coord() ) )
fn_r2 = (10.*temperature+1.)*fn_r

benFn = fn.branching.conditional( [ (fn.coord()[2] > 0.0, fn_r),
                                    (True               , fn_r2) ] )

#model.Solve(fn_k=fn_r, fn_f=1., fn_dBC_inner=2., fn_dBC_outer=0.)

model.Solve(fn_k=benFn, fn_f=1., fn_dBC_inner=2., fn_dBC_outer=0.)
