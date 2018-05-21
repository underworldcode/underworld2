# To execute
# python stokesSinker3D.py -elRes 9,9,9 ; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py sol.h5; $PETSC_DIR/lib/petsc/bin/petsc_gen_xdmf.py aux.h5

import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn
import numpy as np

class Poisson(object):
    def __init__(self, elementRes=None, filename=None, dim=3, forceTerm=0.):
        self._filename = filename
        self.dim = dim
        zero = (0.,)
        
        self._cmodel = pl.PoissonModel_Setup( filename )
        
        self.fn_temperature = fn.misc.constant(zero*dim)
        pl.prob_fnSetter(self._cmodel, 0, self.fn_temperature._fncself )

    def SetRHS(self, forceTerm):
        self.fn_forceTerm = uw.function.Function.convert(forceTerm)
        if not isinstance( self.fn_forceTerm, uw.function.Function):
            raise TypeError( "Provided 'forceTerm' must be of or convertible to 'Function' class." )
        # make the c function delete the previous version of themselves
        pl.StokesModel_SetForceTerm( self._cmodel, self.fn_forceTerm._fncself)
    
    def Solve(self):
        pl.PoissonModel_Solve(self._cmodel)

model = Poisson(filename=None)

model.SetRHS(fn.misc.constant(1.0))

model.Solve()

