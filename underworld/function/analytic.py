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
    eqn_bodyforce = ""
    eqn_viscosity = ""

    def __init__(self, csol, *args, **kwargs):
        self._csol = csol
        super(_SolBase,self).__init__(**kwargs)
    
        self.fn_velocity   = _Sol_Function( self._csol.velocityFn )
        self.fn_pressure   = _Sol_Function( self._csol.pressureFn )
        self.fn_stress     = _Sol_Function( self._csol.stressFn )
        self.fn_strainRate = _Sol_Function( self._csol.strainRateFn )
        self.fn_viscosity  = _Sol_Function( self._csol.viscosityFn )
        self.fn_bodyforce  = _Sol_Function( self._csol.bodyForceFn )
        self.dim           = self._csol.dim
        self.nonlinear     = False

class _SolBaseFreeSlipBc(_SolBase):
    def get_bcs( self, velVar ):
        '''
        All wall free-slip is the default condition. Concrete classes
        should overwrite to set alternate conditions.

        Parameters
        ----------
        velVar : underworld.mesh.MeshVariable
            The velocity variable is required to construct the BC
            object.

        Returns
        -------
        underworld.conditions.SystemCondition
            The BC object. It should be passed in to the system being constructed.

        '''
        mesh = velVar.mesh

        bcverts = []
        bcverts.append( mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"] )
        bcverts.append( mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"] )
        if self.dim == 3:
            bcverts.append( mesh.specialSets["MinK_VertexSet"] + mesh.specialSets["MaxK_VertexSet"] )
        import underworld as uw
        return uw.conditions.DirichletCondition(velVar, bcverts)


class SolA(_SolBaseFreeSlipBc):
    """
    Trigonometric body forcing. Isoviscous.

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    n_x : int
        Wavenumber parameter (in x).
    n_z : float
        Wavenumber parameter (in z).
    eta_0 : float
        Viscosity.

    """
    eqn_bodyforce = "(0,-\sigma_0 \cos(k_x x) \sin(k_z z))"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., n_x=1, n_z=1., eta_0=1., *args, **kwargs):
        if eta_0<=0:
            raise TypeError("'eta_0' parameter must be positive." )
        if sigma_0<=0:
            raise TypeError("'sigma_0' parameter must be positive." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' parameter must be of 'int' type." )

        # note the way we need to use the swig generated constructor is somewhat
        # ass about.. swig doesn't play particularly nice with CRTP, but there
        # might be a better way.
        self._ckeep = _cfn.SolA(float(sigma_0),float(eta_0),n_x,float(n_z))
        # the second parameter to SolACRTP is the dimensionality
        super(SolA,self).__init__(_cfn.SolACRTP(self._ckeep,2), **kwargs)

class SolB(_SolBaseFreeSlipBc):
    """
    Trigonometric/hyperbolic body forcing. Isoviscous.
    
    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    n_x : int
        Wavenumber parameter (in x).
    n_z : float
        Wavenumber parameter (in z).
    eta_0 : float
        Viscosity.

    """
    eqn_bodyforce = "(0,-\sigma_0 \cos(k_x x) \sinh(k_z z))"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., n_x=1, n_z=1.5, eta_0=1., *args, **kwargs):
        if eta_0<=0:
            raise TypeError("'eta_0' parameter must be positive." )
        if sigma_0<=0:
            raise TypeError("'sigma_0' parameter must be positive." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' parameter must be of 'int' type." )
        if abs(float(n_x)-float(n_z)) < 1e-5:
            raise TypeError("'n_z' must be different than 'n_z'." )

        self._ckeep = _cfn.SolB(float(sigma_0),float(eta_0),n_x,float(n_z))
        super(SolB,self).__init__(_cfn.SolBCRTP(self._ckeep,2), **kwargs)

class SolC(_SolBaseFreeSlipBc):
    """
    Discontinuous body forcing. Isoviscous.

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    x_c : float
        Perturbation step location.
    eta_0 : float
        Viscosity.
    nmodes: int
        Number of Fourier modes used when evaluating
        analytic solution.

    Notes
    -----
    This solution is quiet slow to evaluate due to large number of
    Fourier terms required. Number of terms is hard code in `solC.c`.

    """
    eqn_bodyforce = "(0,-\sigma_0\mathrm{step}(1,0,x_c; x)"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., x_c=0.5, eta_0=1., nmodes=200, *args, **kwargs):
        if not isinstance(eta_0, float) or eta_0<=0:
            raise TypeError("'eta_0' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )

        self._ckeep = _cfn.SolC(sigma_0,eta_0,x_c, nmodes)
        super(SolC,self).__init__(_cfn.SolCCRTP(self._ckeep,2), **kwargs)


class SolCx(_SolBaseFreeSlipBc):
    """
    Viscosity step profile in x, trigonometric density profile.

    Parameters
    ----------
    n_x : unsigned
        Wavenumber parameter (in x).
    eta_A : float
        Viscosity of region A.
    eta_B : float
        Viscosity of region B.
    eta_c : float
        Viscosity step location.

    """
    eqn_bodyforce = "(0,-\cos(k_x x) \sin(k_z z))"
    eqn_viscosity = "\mathrm{step}(\eta_A,\eta_B,x_c; x)"
    def __init__(self, n_x=1, eta_A=1., eta_B=100000., x_c=0.75, *args, **kwargs):
        if not isinstance(eta_A, float) or eta_A<=0:
            raise TypeError("'eta_A' must be a positive float." )
        if not isinstance(eta_B, float) or eta_B<=0:
            raise TypeError("'eta_B' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' parameter must be of type 'int'." )

        self._ckeep = _cfn.SolCx(eta_A,eta_B,x_c,n_x)
        super(SolCx,self).__init__(_cfn.SolCxCRTP(self._ckeep,2), **kwargs)

class SolDA(_SolBaseFreeSlipBc):
    """
    Columnar density profile in x, and viscosity step in z.

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    x_c : float
        Centre of column.
    x_w : float
        Width of column.
    eta_A : float
        Viscosity of region A.
    eta_B : float
        Viscosity of region B.
    z_c : float
        Viscosity step location.
    nmodes: int
        Number of Fourier modes used when evaluating
        analytic solution.

    """
    eqn_bodyforce = "(0, -\sigma_0 \mathrm{rect}(x_c,x_w; x) )"
    eqn_viscosity = "\mathrm{step}(\eta_A,\eta_B,z_c; z)"
    def __init__(self, sigma_0=1., x_c=0.375, x_w=0.25, eta_A=1., eta_B=10., z_c=0.75, nmodes=200, *args, **kwargs):
        if not isinstance(sigma_0, float):
            raise TypeError("'sigma_0' must be a float." )
        if not isinstance(eta_A, float) or eta_A<=0:
            raise TypeError("'eta_A' must be a positive float." )
        if not isinstance(eta_B, float) or eta_B<=0:
            raise TypeError("'eta_B' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )
        if not isinstance(x_w, float):
            raise TypeError("'x_w' parameter must be of type 'float'." )
        if not isinstance(z_c, float):
            raise TypeError("'z_c' parameter must be of type 'float'." )
        if not isinstance(nmodes, int) or nmodes<=0:
            raise TypeError("'nmodes' parameter must be a positive int." )

        self._ckeep = _cfn.SolDA(sigma_0,x_c,x_w,eta_A,eta_B,z_c,nmodes)
        super(SolDA,self).__init__(_cfn.SolDACRTP(self._ckeep,2), **kwargs)

class SolH(_SolBaseFreeSlipBc):
    """
    Density step profile in (x,y). Constant viscosity.

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    x_c : float
        Step position (in x).
    y_c : float
        Step position (in y).
    eta_0 : float
        Viscosity.
    nmodes: int
        Number of Fourier modes used when evaluating
        analytic solution.

    """
    eqn_bodyforce = "(0, 0, -\sigma_0 \mathrm{step}(1, 0, x_c; x) \mathrm{step}(1, 0, y_c; y) )"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., x_c=0.5, y_c=0.5, eta_0=1., nmodes=30, *args, **kwargs):
        if not isinstance(sigma_0, float):
            raise TypeError("'sigma_0' must be a float." )
        if not isinstance(eta_0, float) or eta_0<=0:
            raise TypeError("'eta_0' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )
        if not isinstance(y_c, float):
            raise TypeError("'y_c' parameter must be of type 'float'." )
        if not isinstance(nmodes, int) or nmodes<=0:
            raise TypeError("'nmodes' parameter must be a positive int." )

        self._ckeep = _cfn.SolH(sigma_0,x_c,y_c,eta_0,nmodes)
        super(SolH,self).__init__(_cfn.SolHCRTP(self._ckeep,3), **kwargs)


class _SolHA(_SolBaseFreeSlipBc):
    """
    Columnar density profile in (x,y). Constant viscosity.

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    x_c : float
        Centre of column (in x).
    x_w : float
        Width of column (in x).
    y_c : float
        Centre of column (in y).
    y_w : float
        Width of column (in y).
    eta_0 : float
        Viscosity.
    nmodes: int
        Number of Fourier modes used when evaluating
        analytic solution.

    """
    eqn_bodyforce = "(0, 0, \sigma_0 -\mathrm{rect}(x_c,x_w; x) \mathrm{rect}(y_c,y_w; y) )"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., x_c=0.5, x_w=0.5, y_c=0.5, y_w=0.5, eta_0=1., nmodes=10, *args, **kwargs):
        import warning
        warning.warm("SolHA does not appear to generate correct pressure (and possibly other) results.")

        if not isinstance(sigma_0, float):
            raise TypeError("'sigma_0' must be a float." )
        if not isinstance(eta_0, float) or eta_0<=0:
            raise TypeError("'eta_0' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )
        if not isinstance(x_w, float):
            raise TypeError("'x_w' parameter must be of type 'float'." )
        if not isinstance(y_c, float):
            raise TypeError("'y_c' parameter must be of type 'float'." )
        if not isinstance(y_w, float):
            raise TypeError("'y_w' parameter must be of type 'float'." )
        if not isinstance(nmodes, int) or nmodes<=0:
            raise TypeError("'nmodes' parameter must be a positive int." )

        self._ckeep = _cfn.SolHA(sigma_0,x_c,x_w,y_c,y_w,eta_0,nmodes)
        super(SolHA,self).__init__(_cfn.SolHACRTP(self._ckeep,3), **kwargs)


class SolKx(_SolBaseFreeSlipBc):
    """
    The boundary conditions are free-slip everywhere on a unit domain. The
    viscosity varies exponentially in the x direction and is given by
    :math:`\\eta = \\exp (2 B x)`. The flow is driven by the following density
    perturbation:

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    n_x : int
        Wavenumber parameter (in x).
    n_z : float
        Wavenumber parameter (in z).
    B : float
        Viscosity parameter.
    """
    eqn_bodyforce = "(0,-\sigma_0 \cos(k_x x) \sin(k_z z))"
    eqn_viscosity = "\exp(2Bx)"
    
    def __init__(self, sigma_0=1., n_x=1, n_z=1., B=1.1512925465, *args, **kwargs):
        if not isinstance(sigma_0, float):
            raise TypeError("'sigma_0' must be a float." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' must be an int." )
        if not isinstance(n_z, (int,float)):
            raise TypeError("'n_z' parameter must be of type 'float'." )
        if not isinstance(B, float):
            raise TypeError("'B' parameter must be of type 'float'." )

        self._ckeep = _cfn.SolKx(sigma_0,n_x,n_z,B)
        super(SolKx,self).__init__(_cfn.SolKxCRTP(self._ckeep,2), **kwargs)


class SolKz(_SolBaseFreeSlipBc):
    """
    The boundary conditions are free-slip everywhere on a unit domain. The
    viscosity varies exponentially in the z direction and is given by
    :math:`\\eta = \\exp (2 B z)`. The flow is driven by the following
    density perturbation:

    Parameters
    ----------
    sigma_0 : float
        Perturbation strength factor.
    n_x : int
        Wavenumber parameter (in x).
    n_z : float
        Wavenumber parameter (in z).
    B : float
        Viscosity parameter.

    """
    eqn_bodyforce = "(0,-\sigma_0 \cos(k_x x) \sin(k_z z))"
    eqn_viscosity = "\exp(2Bz)"

    def __init__(self, sigma_0=1., n_x=1, n_z=1., B=1.1512925465, *args, **kwargs):
        if not isinstance(sigma_0, float) or sigma_0!=1.:
            raise TypeError("'sigma_0' can be any float as long as it's 1." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' must be an int." )
        if not isinstance(n_z, (int,float)):
            raise TypeError("'n_z' parameter must be of type 'float'." )
        if not isinstance(B, float):
            raise TypeError("'B' parameter must be of type 'float'." )

        self._ckeep = _cfn.SolKz(sigma_0,n_x,n_z,B)
        super(SolKz,self).__init__(_cfn.SolKzCRTP(self._ckeep,2), **kwargs)

class SolM(_SolBaseFreeSlipBc):
#    """
#    SolM inanis et vacua est,
#    SolM est illusio,
#    SolM non potest suggero vos sustentationem mentis
#
#    MV ()
#    """

    def __init__(self, eta_0=1., n_x=1, n_z=1, r=1.5, *args, **kwargs):
        if not isinstance(eta_0, float) or eta_0 <= 0.:
            raise TypeError("'eta_0' can be any positive float." )
        if not isinstance(n_z, int):
            raise TypeError("'n_z' must be an int." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' must be an int." )
        if not isinstance(r, float):
            raise TypeError("'r' parameter must be a 'float' and != 'n_z'." )
        if abs(float(n_z)-r) < 1e-5:
            raise TypeError("'r' must be different than 'n_z'." )

        self._ckeep = _cfn.SolM(eta_0,n_z,n_x,r)
        super(SolM,self).__init__(_cfn.SolMCRTP(self._ckeep,2), **kwargs)


class _SolBaseFixedBc(_SolBase):
    def get_bcs( self, velVar ):
        '''
        ( Fixed,Fixed ) conditions on all walls.
        
        Fixed DOFs set to analytic soln values.

        Parameters
        ----------
        velVar : underworld.mesh.MeshVariable
            The velocity variable is required to construct the BC
            object.

        Returns
        -------
        underworld.conditions.SystemCondition
            The BC object. It should be passed in to the system being constructed.

        '''
        mesh = velVar.mesh
        
        # set vel as necessary
        wall_verts = mesh.specialSets["AllWalls_VertexSet"]
        velVar.data[wall_verts.data] = self.fn_velocity.evaluate(wall_verts)

        # now flag required nodes
        bcverts = []
        for dim in range(self.dim):
            bcverts.append(wall_verts)

        import underworld as uw
        return uw.conditions.DirichletCondition(velVar, bcverts)

class SolNL(_SolBase):
    """
    SolNL requires tighter solver tolerances and/or a direct solve for best results.
    Need to check in with Caesar Dandenonensis as to the origins of this solution.
    
    Check `get_bcs()` for BC setup.
    """
    def __init__(self, eta_0=1., n_z=1, r=1.5, *args, **kwargs):
        if not isinstance(eta_0, float) or eta_0 <= 0.:
            raise TypeError("'eta_0' can be any positive float." )
        if not isinstance(n_z, int):
            raise TypeError("'n_z' must be an int." )
        if not isinstance(r, float):
            raise TypeError("'r' parameter must be a 'float'." )
            
        self.nonlinear = True

        self._ckeep = _cfn.SolNL(eta_0,n_z,r)
        super(SolNL,self).__init__(_cfn.SolNLCRTP(self._ckeep,2), **kwargs)

    def get_bcs( self, velVar ):
        '''
        ( Fixed,Fixed ) conditions left/right.
        ( Free, Fixed ) conditions top/bottom.

        All fixed DOFs set to analytic soln values.
        
        Parameters
        ----------
        velVar : underworld.mesh.MeshVariable
            The velocity variable is required to construct the BC
            object.
        
        Returns
        -------
        underworld.conditions.SystemCondition
            The BC object. It should be passed in to the system being constructed.
        
        '''
        mesh = velVar.mesh
        # set vel on all boundaries
        wall_verts = mesh.specialSets["AllWalls_VertexSet"]
        velVar.data[wall_verts.data] = self.fn_velocity.evaluate(wall_verts)
        
        # now flag required fixed nodes
        iw = mesh.specialSets["MinI_VertexSet"] + mesh.specialSets["MaxI_VertexSet"]
        jw = mesh.specialSets["MinJ_VertexSet"] + mesh.specialSets["MaxJ_VertexSet"]

        bcverts = [ iw,  iw+jw ]
        
        import underworld as uw
        return uw.conditions.DirichletCondition(velVar, bcverts)

#    @property
#    def fn_viscosity(self):
#        return _Sol_Function( self._csol.viscosityFn )

class SolDB2d(_SolBaseFixedBc):
    """
    SolDB2d and solDB3d from:

    Dohrmann, C.R., Bochev, P.B., A stabilized finite element method for the
    Stokes problem based on polynomial pressure projections,
    Int. J. Numer. Meth. Fluids 46, 183-201 (2004).
        
    Check `get_bcs()` for BC setup.


    """
    def __init__(self, *args, **kwargs):
        self._ckeep = _cfn.SolDB2d()
        super(SolDB2d,self).__init__(_cfn.SolDB2dCRTP(self._ckeep,2), **kwargs)



class SolDB3d(_SolBaseFixedBc):
    """
    SolDB2d and solDB3d from:

    Dohrmann, C.R., Bochev, P.B., A stabilized finite element method for the
    Stokes problem based on polynomial pressure projections,
    Int. J. Numer. Meth. Fluids 46, 183-201 (2004).

    """

    def __init__(self, Beta=4., *args, **kwargs):
        self._ckeep = _cfn.SolDB3d(Beta)
        super(SolDB3d,self).__init__(_cfn.SolDB3dCRTP(self._ckeep,3), **kwargs)
