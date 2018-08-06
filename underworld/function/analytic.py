##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module provides a suite of models which satisfy the
Stokes system of equations.

All models are considered across a unit square (or cube)
domain, and utilise (unless otherwise stated) free-slip
conditions on all boundaries.

Each model object provides a set of Underworld Functions
for description of physical quantities such as velocity,
pressure and viscosity.

For numerical validation in Underworld, we construct a
Stokes system with appropriate domain and boundary
conditions. Viscosity and body forces are set directly
using corresponding Functions provided by the solution
object. Generated numerical solution for velocity and
pressure (or derivated quantities) may then be compared
with exact solutions provided by solution objects.
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
    @property
    def dim(self):
        return self._csol.dim


class SolA(_SolBase):
    """
    """
    def __init__(self, sigma=1., n_x=1, n_z=1., eta=1., *args, **kwargs):
        if eta<=0:
            raise TypeError("'eta' parameter must be positive." )
        if sigma<=0:
            raise TypeError("'sigma' parameter must be positive." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' parameter must be of 'int' type." )
        
        self._ckeep = _cfn.SolA(float(sigma),float(eta),n_x,float(n_z))
        super(SolA,self).__init__(_cfn.SolACRTP(self._ckeep,2), **kwargs)

class SolB(_SolBase):
    """
        """
    def __init__(self, sigma=1., n_x=1, n_z=1.5, eta=1., *args, **kwargs):
        if eta<=0:
            raise TypeError("'eta' parameter must be positive." )
        if sigma<=0:
            raise TypeError("'sigma' parameter must be positive." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' parameter must be of 'int' type." )

        self._ckeep = _cfn.SolB(float(sigma),float(eta),n_x,float(n_z))
        super(SolB,self).__init__(_cfn.SolBCRTP(self._ckeep,2), **kwargs)

class SolCx(_SolBase):
    """
    The boundary conditions are free-slip everywhere on a unit domain.
    There is a viscosity jump in the x direction at :math:`x=xc`.
    The flow is driven by the following density perturbation:
    
    .. math::
        \\rho = \\sin (n_z \\pi z) \\cos (n_x \\pi x)

    Parameters
    ----------
    n_x : unsigned
        Density wavenumber in x axis.
    eta_A : float
        Viscosity of region A.
    eta_B : float
        Viscosity of region B.
    x_c : float
        Location of viscosity jump.

    """
    def __init__(self, n_x=1, eta_A=1., eta_B=2., x_c=0.25, *args, **kwargs):
        if not isinstance(eta_A, float) or eta_A<=0:
            raise TypeError("'eta_A' must be a positive float." )
        if not isinstance(eta_B, float) or eta_B<=0:
            raise TypeError("'eta_B' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' parameter must be of type 'int'." )

        # note the way we need to use the swig generated constructor is somewhat
        # ass about.. swig doesn't play particularly nice with CRTP, but there
        # might be a better way.
        self._ckeep = _cfn.SolCx(eta_A,eta_B,x_c,n_x)
        # the second parameter to SolCxCRTP is the dimensionality
        super(SolCx,self).__init__(_cfn.SolCxCRTP(self._ckeep,2), **kwargs)


class SolKx(_SolBase):
    """
    The boundary conditions are free-slip everywhere on a unit domain. The 
    viscosity varies exponentially in the x direction and is given by
    :math:`\\eta = \\exp (2 B x)`. The flow is driven by the following density 
    perturbation:
    
    .. math::
        \\rho = -\\sigma \\sin (n_z \\pi z) \\cos (n_x \\pi x).

    Parameters
    ----------
    sigma : float
        Density perturbation amplitude.
    n_x : float
        Density wavenumber in x axis.
    n_z : float
        Density wavenumber in z axis.
    B : float
        Viscosity parameter
    """

    def __init__(self, sigma=1., n_x=1., n_z=1., B=1.1512925465, *args, **kwargs):
        if not isinstance(sigma, float):
            raise TypeError("'sigma' must be a float." )
        if not isinstance(B, float):
            raise TypeError("'B' parameter must be of type 'float'." )
        # check SolCx for some details
        self._ckeep = _cfn.SolKx(sigma,n_x,n_z,B)
        super(SolKx,self).__init__(_cfn.SolKxCRTP(self._ckeep,2), **kwargs)


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

class SolM(_SolBase):
#    """
#    SolM inanis et vacua est,
#    SolM est illusio,
#    SolM non potest suggero vos sustentationem mentis
#
#    MV (Caesar Dandenonensis)
#    """

    def __init__(self, eta0=1., m=1, n=1, r=1.5, *args, **kwargs):
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


class SolNL(_SolBase):
#    """
#    SolNL inanis et vacua est,
#    SolNL est illusio,
#    SolNL non potest suggero vos sustentationem mentis
#
#    MV (Caesar Dandenonensis)
#    """
    def __init__(self, eta0=1., n=1, r=1.5, *args, **kwargs):
        if not isinstance(eta0, float) or eta0 <= 0.:
            raise TypeError("'eta0' can be any positive float." )
        if not isinstance(n, int):
            raise TypeError("'n' must be an int." )
        if not isinstance(r, float):
            raise TypeError("'r' parameter must be a 'float' and != 'm'." )
        # check SolNL for some details
        self._ckeep = _cfn.SolNL(eta0,n,r)
        super(SolNL,self).__init__(_cfn.SolNLCRTP(self._ckeep,2), **kwargs)


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

