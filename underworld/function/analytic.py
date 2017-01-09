##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module provides analytic solution functions to the Stokes
flow equations.
"""
import libUnderworld.libUnderworldPy.Function as _cfn
from _function import Function as _Function


class _Sol_Function(_Function):
   # This class simply wraps cpp Function class pointers
   # in our hand rolled python Function class wrapper.
   def __init__(self, cFunctionFn, **kwargs):
        self._fncself = cFunctionFn
        super(_Sol_Function,self).__init__(argument_fns=None, **kwargs)

class _SolBase(object):
    # This function returns the various fields for
    # analytic solutions. Note that this object itself is not a
    # function object, but it instead provides the required
    # Function objects via its member properties.
    def __init__(self, csol, *args, **kwargs):

        self._csol = csol
        super(_SolBase,self).__init__(**kwargs)

    @property
    def fn_velocity(self):
        return _Sol_Function( self._csol.velocityFn )
    @property
    def fn_pressure(self):
        return _Sol_Function( self._csol.pressureFn )
    @property
    def fn_stress(self):
        return _Sol_Function( self._csol.stressFn )
    @property
    def fn_strainRate(self):
        return _Sol_Function( self._csol.strainRateFn )
    @property
    def fn_viscosity(self):
        return _Sol_Function( self._csol.viscosityFn )
    @property
    def fn_bodyforce(self):
        return _Sol_Function( self._csol.bodyForceFn )


class SolCx(_SolBase):
    """
    The boundary conditions are free-slip everywhere on a unit domain.
    There is a viscosity jump in the x direction at :math:`x=xc`.
    The flow is driven by the following density perturbation:
    
    .. math::
        \\rho = \\sin (nz \\pi z) \\cos (nx \\pi x)

    Parameters
    ----------
    viscosityA : float
        Viscosty of region A.
    viscosityB : float
        Viscosty of region B.
    xc : float
        Location for viscosity jump.
    nx : unsigned
        Wavenumber in x axis.

    """
    def __init__(self, viscosityA=1., viscosityB=2., xc=0.25, nx=1, *args, **kwargs):
        if not isinstance(viscosityA, float) or viscosityA<=0:
            raise TypeError("'viscosityA' must be a positive float." )
        if not isinstance(viscosityB, float) or viscosityB<=0:
            raise TypeError("'viscosityB' must be a positive float." )
        if not isinstance(xc, float):
            raise TypeError("'xc' parameter must be of type 'float'." )
        if not isinstance(nx, int):
            raise TypeError("'nx' parameter must be of type 'int'." )

        # note the way we need to use the swig generated constructor is somewhat
        # ass about.. swig doesn't play particularly nice with CRTP, but there
        # might be a better way.
        self._ckeep = _cfn.SolCx(viscosityA,viscosityB,xc,nx)
        # the second parameter to SolCxCRTP is the dimensionality
        super(SolCx,self).__init__(_cfn.SolCxCRTP(self._ckeep,2), **kwargs)

#    @property
#    def viscosityA(self):
#        return self._csol.viscosityA
#    @viscosityA.setter
#    def viscosityA(self, value):
#        if not isinstance(value, float) or value<=0:
#            raise TypeError("'viscosityA' must be a positive float." )
#        self._csol.viscosityA = value
#    @property
#    def viscosityB(self):
#        return self._csol.viscosityB
#    @viscosityB.setter
#    def viscosityB(self, value):
#        if not isinstance(value, float) or value<=0:
#            raise TypeError("'viscosityB' must be a positive float." )
#        self._csol.viscosityB = value
#    @property
#    def xc(self):
#        return self._csol.xc
#    @xc.setter
#    def xc(self, value):
#        if not isinstance(value, float) or value<0. or value>1.:
#            raise TypeError("'xc' parameter must be of type 'float' in [0,1]." )
#        self._csol.xc = value
#    @property
#    def nx(self):
#        return self._csol.nx
#    @nx.setter
#    def nx(self, value):
#        if not isinstance(value, int) or value<=0:
#            raise TypeError("'nx' parameter must be a positive int." )
#        self._csol.nx = value


class SolKx(_SolBase):
    """
    The boundary conditions are free-slip everywhere on a unit domain. The 
    viscosity varies exponentially in the x direction and is given by
    :math:`\\eta = \\exp (2 B x)`. The flow is driven by the following density 
    perturbation:
    
    .. math::
        \\rho = -\\sigma \\sin (nz \\pi z) \\cos (nx \\pi x).

    Parameters
    ----------
    sigma : float
        Density perturbation amplitude.
    nx : float
        Wavenumber in x axis.
    nz : unsigned
        Wavenumber in  axis.
    B : float
        Viscosity parameter
    """

    def __init__(self, sigma=1., nx=1., nz=1, B=1.1512925465, *args, **kwargs):
        if not isinstance(sigma, float):
            raise TypeError("'sigma' must be a float." )
        if not isinstance(nx, float):
            raise TypeError("'nx' must be a float." )
        if not isinstance(nz, int):
            raise TypeError("'nz' parameter must be an int." )
        if not isinstance(B, float):
            raise TypeError("'B' parameter must be of type 'float'." )
        # check SolCx for some details
        self._ckeep = _cfn.SolKx(sigma,nx,nz,B)
        super(SolKx,self).__init__(_cfn.SolKxCRTP(self._ckeep,2), **kwargs)

#    @property
#    def sigma(self):
#        return self._csol.sigma
#    @sigma.setter
#    def sigma(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'sigma' must be a float." )
#        self._csol.sigma = value
#    @property
#    def nx(self):
#        return self._csol.nx
#    @nx.setter
#    def nx(self, value):
#        if not isinstance(value, int):
#            raise TypeError("'nx' must be an int." )
#        self._csol.nx = value
#    @property
#    def nz(self):
#        return self._csol.nz
#    @nz.setter
#    def nz(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'nz' parameter must be a float." )
#        self._csol.nz = value
#    @property
#    def B(self):
#        return self._csol.B
#    @B.setter
#    def B(self, value):
#        if not isinstance(value, float) or value<=0:
#            raise TypeError("'B' parameter must be a positive float." )
#        self._csol.B = value


class SolKz(_SolBase):
    """
    The boundary conditions are free-slip everywhere on a unit domain. The 
    viscosity varies exponentially in the z direction and is given by 
    :math:`\\eta = \\exp (2 B z)`. The flow is driven by the following
    density perturbation:
    
    .. math::
        \\rho = -\\sigma \\sin (nz \\pi z) \\cos (nx \\pi x).

    Parameters
    ----------
    sigma : float
        Density perturbation amplitude.
    nx : float
        Wavenumber in x axis.
    nz : unsigned
        Wavenumber in  axis.
    B : float
        Viscosity parameter

    """

    def __init__(self, sigma=1., nx=1, nz=1., B=1., *args, **kwargs):
        if not isinstance(sigma, float) or sigma!=1.:
            raise TypeError("'sigma' can be any float as long as it's 1." )
        if not isinstance(nx, int):
            raise TypeError("'nx' must be an int." )
        if not isinstance(nz, float):
            raise TypeError("'nz' parameter must be of type 'float'." )
        if not isinstance(B, float):
            raise TypeError("'B' parameter must be of type 'float'." )
        # check SolKz for some details
        self._ckeep = _cfn.SolKz(sigma,nx,nz,B)
        super(SolKz,self).__init__(_cfn.SolKzCRTP(self._ckeep,2), **kwargs)

#    @property
#    def sigma(self):
#        return self._csol.sigma
#    @sigma.setter
#    def sigma(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'sigma' must be a float." )
#        self._csol.sigma = value
#    @property
#    def nx(self):
#        return self._csol.nx
#    @nx.setter
#    def nx(self, value):
#        if not isinstance(value, int):
#            raise TypeError("'nx' must be an int." )
#        self._csol.nx = value
#    @property
#    def nz(self):
#        return self._csol.nz
#    @nz.setter
#    def nz(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'nz' parameter must be a float." )
#        self._csol.nz = value
#    @property
#    def B(self):
#        return self._csol.B
#    @B.setter
#    def B(self, value):
#        if not isinstance(value, float) or value<=0:
#            raise TypeError("'B' parameter must be a positive float." )
#        self._csol.B = value


class SolM(_SolBase):
#    """
#    SolM inanis et vacua est,
#    SolM est illusio,
#    SolM non potest suggero vos sustentationem mentis
#
#    MV (Caesar Dandenonensis)
#    """

    def __init__(self, eta0=1., m=1, n=1., r=1.5, *args, **kwargs):
        if not isinstance(eta0, float) or eta0 <= 0.:
            raise TypeError("'eta0' can be any positive float." )
        if not isinstance(m, int):
            raise TypeError("'m' must be an int." )
        if not isinstance(n, int):
            raise TypeError("'n' must be an int." )
        if not isinstance(r, float):
            raise TypeError("'r' parameter must be a 'float' and != 'm'." )
        if abs(float(m)-r) < 1e-5:
            raise TypeError("'r' must be different than 'm'." )
        # check SolM for some details
        self._ckeep = _cfn.SolM(eta0,m,n,r)
        super(SolM,self).__init__(_cfn.SolMCRTP(self._ckeep,2), **kwargs)

#    @property
#    def eta0(self):
#        return self._csol.eta0
#    @eta0.setter
#    def eta0(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'eta0' must be a float." )
#        self._csol.eta0 = value
#    @property
#    def m(self):
#        return self._csol.m
#    @m.setter
#    def m(self, value):
#        if not isinstance(value, int):
#            raise TypeError("'m' must be an int." )
#        self._csol.m = value
#
#    @property
#    def n(self):
#        return self._csol.n
#    @n.setter
#    def n(self, value):
#        if not isinstance(value, int):
#            raise TypeError("'n' must be an int." )
#        self._csol.n = value
#
#    @property
#    def r(self):
#        return self._csol.r
#    @r.setter
#    def r(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'r' parameter must be a float." )
#        self._csol.r = value


class SolNL(_SolBase):
#    """
#    SolNL inanis et vacua est,
#    SolNL est illusio,
#    SolNL non potest suggero vos sustentationem mentis
#
#    MV (Caesar Dandenonensis)
#    """
    def __init__(self, eta0=1., n=1., r=1.5, *args, **kwargs):
        if not isinstance(eta0, float) or eta0 <= 0.:
            raise TypeError("'eta0' can be any positive float." )
        if not isinstance(n, int):
            raise TypeError("'n' must be an int." )
        if not isinstance(r, float):
            raise TypeError("'r' parameter must be a 'float' and != 'm'." )
        # check SolNL for some details
        self._ckeep = _cfn.SolNL(eta0,n,r)
        super(SolNL,self).__init__(_cfn.SolNLCRTP(self._ckeep,2), **kwargs)

#    @property
#    def eta0(self):
#        return self._csol.eta0
#    @eta0.setter
#    def eta0(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'eta0' must be a float." )
#        self._csol.eta0 = value
#
#    @property
#    def n(self):
#        return self._csol.n
#    @n.setter
#    def n(self, value):
#        if not isinstance(value, int):
#            raise TypeError("'n' must be an int." )
#        self._csol.n = value
#
#    @property
#    def r(self):
#        return self._csol.r
#    @r.setter
#    def r(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'r' parameter must be a float." )
#        self._csol.r = value


class SolDB2d(_SolBase):
    """
    SolDB2d and solDB3d from:
    
    Dohrmann, C.R., Bochev, P.B., A stabilized finite element method for the
    Stokes problem based on polynomial pressure projections, 
    Int. J. Numer. Meth. Fluids 46, 183-201 (2004).

    """
    def __init__(self, *args, **kwargs):
        self._ckeep = _cfn.SolDB2d()
        super(SolDB2d,self).__init__(_cfn.SolDB2dCRTP(self._ckeep,2), **kwargs)

class SolDB3d(_SolBase):
    """
    SolDB2d and solDB3d from:

    Dohrmann, C.R., Bochev, P.B., A stabilized finite element method for the
    Stokes problem based on polynomial pressure projections, 
    Int. J. Numer. Meth. Fluids 46, 183-201 (2004).

    """

    def __init__(self, Beta=4., *args, **kwargs):
        self._ckeep = _cfn.SolDB3d(Beta)
        super(SolDB3d,self).__init__(_cfn.SolDB3dCRTP(self._ckeep,3), **kwargs)
#    @property
#    def Beta(self):
#        return self._csol.Beta
#    @Beta.setter
#    def Beta(self, value):
#        if not isinstance(value, float):
#            raise TypeError("'Beta' parameter must be a float." )
#        self._csol.Beta = value
