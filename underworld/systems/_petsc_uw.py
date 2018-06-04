##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld as uw
from underworld import libUnderworld
from libUnderworld import petsc_layer as pl
from underworld import function as fn
import numpy as np

class pl_StokesModel(object):
    def __init__(self, elementRes=None, filename=None, dim=3, forceTerm=0.):
        self._filename = filename
        self.dim = dim
        zero = (0.,)
        
        self._cmodel = pl.StokesModel_Setup( filename )
        
        self.fn_v = fn.misc.constant(zero*dim)
        self.fn_p = fn.misc.constant(zero)
        pl.prob_fnSetter(self._cmodel, 0, self.fn_v._fncself )
        pl.prob_fnSetter(self._cmodel, 1, self.fn_p._fncself )

    def SetRHS(self, fn):
        self.fn_forceTerm = uw.function.Function.convert(fn)
        if not isinstance( self.fn_forceTerm, uw.function.Function):
            raise TypeError( "Provided 'forceTerm' must be of or convertible to 'Function' class." )
        # make the c function delete the previous version of themselves
        pl.StokesModel_SetForceTerm( self._cmodel, self.fn_forceTerm._fncself)

    def SetViscosity(self, fn):
        self.fn_viscosity = uw.function.Function.convert(fn)
        if not isinstance( self.fn_viscosity, uw.function.Function):
            raise TypeError( "Provided 'fn_vicosity' must be of or convertible to 'Function' class." )
        # make the c function delete the previous version of themselves
        pl.StokesModel_SetViscosityFn( self._cmodel, self.fn_viscosity._fncself)
    
    def Solve(self):
        pl.StokesModel_Solve(self._cmodel)


class pl_PoissonModel(object):
    def __init__(self, elementRes=None, filename=None, dim=3, forceTerm=0.):
        self._filename = filename
        self.dim = dim
        zero = (0.,)
        
        self._cmodel = pl.PoissonModel_Setup( filename )
        
        self.fn_temperature = fn.misc.constant(zero*dim)
        pl.prob_fnSetter(self._cmodel, 0, self.fn_temperature._fncself )

    def SetRHS(self, fn):
        self.fn_forceTerm = uw.function.Function.convert(fn)
        if not isinstance( self.fn_forceTerm, uw.function.Function):
            raise TypeError( "Provided 'forceTerm' must be of or convertible to 'Function' class." )
        # make the c function delete the previous version of themselves
        pl.StokesModel_SetForceTerm( self._cmodel, self.fn_forceTerm._fncself)
    
    def Solve(self):
        pl.PoissonModel_Solve(self._cmodel)
