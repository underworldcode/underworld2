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
        pl.StokesModel_SetForceFn( self._cmodel, self.fn_forceTerm._fncself)

    def SetViscosity(self, fn):
        self.fn_viscosity = uw.function.Function.convert(fn)
        if not isinstance( self.fn_viscosity, uw.function.Function):
            raise TypeError( "Provided 'fn_vicosity' must be of or convertible to 'Function' class." )
        # make the c function delete the previous version of themselves
        pl.StokesModel_SetCoeffFn( self._cmodel, self.fn_viscosity._fncself)
    
    def Solve(self):
        pl.StokesModel_Solve(self._cmodel)


def validateFn(_fn, name="N/A"):
    fn = uw.function.Function.convert(_fn)
    if not isinstance( fn, uw.function.Function):
        raise TypeError( "Provided object '"+name+"' must be of or convertible to 'Function' class." )
    return fn

class pl_PoissonModel(object):
    def __init__(self, filename=None):
        self._filename = filename
        zero = (0.,)
        
        self._cmodel = pl.PoissonModel_Setup( filename )
        
        self.fn_temperature = fn.misc.constant(zero)
        pl.prob_fnSetter(self._cmodel, 0, self.fn_temperature._fncself )

        '''
        self.fn_c0 = fn.misc.constant(1.)
        pl.aux_fnSetter(self._cmodel, 0, self.fn_c0._fncself )
        '''

    def Solve(self, fn_k=1., fn_f=0., fn_dBC_outer=0., fn_dBC_inner=1.0):
        """
        Solve the poisson equation with the given fn parameters.

        Parameters
        ----------

        fn_k : underworld.function
            A underworld function describing the pointwise conductivity. It can be a function of self.fn_temperature.
        
        fn_f : underworld.function
            A underworld function describing the pointwise heating (forcing). It can be a function of self.fn_temperature.
        
        fn_dBC_outer : underworld.function
            A underworld function describing the pointwise outer dirichlet boundary condition
        
        fn_dBC_inner : underworld.function
            A underworld function describing the pointwise inner dirichlet boundary condition

        """

        self.fn_c0 = validateFn( fn_k, name="conductivity" )
        pl.PoissonModel_SetFns( self._cmodel, self.fn_c0._fncself, None, None, None)

        self.fn_forceTerm = validateFn( fn_f, name="heating term" )
        pl.PoissonModel_SetFns( self._cmodel, None,  self.fn_forceTerm._fncself, None, None)

        self.fn_dBC_outer = validateFn( fn_dBC_outer, name="dBC_outer" )
        pl.PoissonModel_SetFns( self._cmodel, None, None, self.fn_dBC_outer._fncself, None)

        self.fn_dBC_inner = validateFn( fn_dBC_inner, name="dBC_inner" )
        pl.PoissonModel_SetFns( self._cmodel, None, None, None, self.fn_dBC_inner._fncself)
        
        pl.PoissonModel_Solve(self._cmodel)
