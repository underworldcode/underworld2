# To execute
# python stokesSinker3D.py -elRes 9,9,9 ; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py sol.h5; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py aux.h5

import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn
import numpy as np

nSpheres = 1
fn_conds = []
inside = lambda x : [ (x, 1.), (True, 0.)]
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

v_const = fn.misc.constant([4.0,2.,0.])
f_const = fn.misc.constant([4.0,2.,0.])
p_const = fn.misc.constant(0.0)

model = pl.StokesModel_Setup("helloworld")

pl.prob_fnSetter(model, 0, v_const._fncself )
pl.prob_fnSetter(model, 1, p_const._fncself )
fn_x = f_const[0]

pl.StokesModel_SetViscosityFn( model, fn_x._fncself )

pl.StokesModel_SetForceTerm(model, fn_buoyancy._fncself)

pl.StokesModel_Solve(model)


'''
PSEUDO code for building python system
class Stokes(object):
    def __init__(self, elementRes=None, filename=None, dim=3, forceTerm=1.):
        self._filename = filename
        self.dim = dim
        
        self._cmodel = pl.StokesModel_Setup( filename )
        zero = (0.,)
        self.fn_velcoity = fn.misc.constant(zero*dim)
        self.fn_pressure = fn.misc.constant(zero)
        pl.someSetter(self._cmodel, 0, v_const._fncself )
        pl.someSetter(self._cmodel, 1, p_const._fncself )
        
        self.fn_viscosity = fn.misc.constant(1.)

        # need some auxField dictionary
        self.auxFields {}

        # make the c function delete the previous version of themselves
        pl.StokesModel_SetViscosityFn( model, self.fn_viscosity._fncself )
        pl.StokesModel_SetForceTerm(model, forceTerm._fncself)
        
        return
    
    def AddField(self):
        # create auxField and append the name to the self.auxFields dictionary

    def Solve(self):
        pl.StokesModel_Solve(self._cmodel)
'''
