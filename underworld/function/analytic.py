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
using corresponding Function objects provided by the solution
class. Generated numerical solution for velocity and
pressure (or derived quantities) may then be compared
with exact solutions provided by solution objects.

Where appropriate, solution classes provide Latex descriptions
of the body force and viscosity functions via the `eqn_bodyforce`
and `eqn_viscosity` attributes. Note the following definitions for
rectangular and step functions:

.. math:: 

    \\operatorname{rect}(c, w; x) = \\left\\{\\begin{array}{rl}
    1 & \\text{if } |x - c| \\le \\frac{w}{2} \\\\
    0 & \\text{if } |x - c| >    \\frac{w}{2},
    \\end{array}\\right.

and

.. math:: 

    \\operatorname{step}(\\alpha,\\beta,c; x) = \\left\\{\\begin{array}{rl}
    \\alpha & \\text{if } x \\le c \\\\
    \\beta & \\text{if } x > c.
    \\end{array}\\right.


"""

import underworld.libUnderworld.libUnderworldPy.Function as _cfn
from ._function import Function as _Function

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
    eqn_bodyforce = None
    eqn_viscosity = None

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
    eqn_bodyforce = "(0,-\sigma_0 \cos(n_x \pi x) \sin(n_z \pi z))"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., n_x=3, n_z=2., eta_0=1., *args, **kwargs):
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
    eqn_bodyforce = "(0,-\sigma_0 \cos(n_x \pi x) \sinh(n_z \pi z))"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1., n_x=3, n_z=2., eta_0=1., *args, **kwargs):
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
    Fourier terms required.

    """
    eqn_bodyforce = "(0,-\sigma_0 \operatorname{step}(1,0,x_c; x)"
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
    eqn_bodyforce = "(0,-\cos(\pi x) \sin(n_z \pi z))"
    eqn_viscosity = "\operatorname{step}(\eta_A,\eta_B,x_c; x)"
    def __init__(self, n_z=3, eta_A=1., eta_B=1.e8, x_c=0.75, *args, **kwargs):
        if not isinstance(eta_A, float) or eta_A<=0:
            raise TypeError("'eta_A' must be a positive float." )
        if not isinstance(eta_B, float) or eta_B<=0:
            raise TypeError("'eta_B' must be a positive float." )
        if not isinstance(x_c, float):
            raise TypeError("'x_c' parameter must be of type 'float'." )
        if not isinstance(n_z, int):
            raise TypeError("'n_z' parameter must be of type 'int'." )

        self._ckeep = _cfn.SolCx(eta_A,eta_B,x_c,n_z)
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
    eqn_bodyforce = "(0, -\sigma_0 \operatorname{rect}(x_c,x_w; x) )"
    eqn_viscosity = "\operatorname{step}(\eta_A,\eta_B,z_c; z)"
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
    Three dimensional solution with density step profile in (x,y). Constant viscosity.

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

    Notes
    -----
    Evaluation of this Fourier based solution is extremely expensive.
    Default number of mode count is set for fast evaluation, though
    for high order or high resolution simulations significantly larger
    mode counts are required. For example, 240 modes are necessary for
    64^3 simulations on Q2/dPc1 elements.

    """
    eqn_bodyforce = "(0, 0, -\sigma_0 \operatorname{step}(1, 0, x_c; x) \operatorname{step}(1, 0, y_c; y) )"
    eqn_viscosity = "\eta_0"
    def __init__(self, sigma_0=1000., x_c=0.5, y_c=0.5, eta_0=1., nmodes=30, *args, **kwargs):
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

    Note: This solution is kept private as it does not appear to produce
    the correct pressure solution. Further investigation is required.

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
    eqn_bodyforce = "(0, 0, -\sigma_0 \operatorname{rect}(x_c,x_w; x) \operatorname{rect}(y_c,y_w; y) )"
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
    Trigonometric body forcing. Exponentially (in x)
    varying viscosity.

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
    eqn_bodyforce = "(0,-\sigma_0 \cos(n_x \pi x) \sin(n_z \pi z))"
    eqn_viscosity = "\exp(2Bx)"
    
    def __init__(self, sigma_0=1., n_x=3, n_z=2., B=2.302585092994046, *args, **kwargs):
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
    Trigonometric body forcing. Exponentially (in z)
    varying viscosity.

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
    eqn_bodyforce = "(0,-\sigma_0 \cos(n_x \pi x) \sin(n_z \pi z))"
    eqn_viscosity = "\exp(2Bz)"

    def __init__(self, sigma_0=1., n_x=3, n_z=2., B=2.302585092994046, *args, **kwargs):
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
    """
    Sinusoidal viscosity.

    Parameters
    ----------
    eta_0 : float
        Viscosity perturbation strength factor.
    n_x : int
        Velocity wavenumber parameter (in x).
    n_z : int
        Velocity wavenumber parameter (in z).
    m_x : float
        Viscosity wavenumber parameter (in x).

    """
    eqn_bodyforce = None
    eqn_viscosity = "1 + \eta_0(1+\cos(m_x \pi x))"

    def __init__(self, eta_0=1., n_x=3, n_z=2, m_x=4., *args, **kwargs):
        if not isinstance(eta_0, float) or eta_0 <= 0.:
            raise TypeError("'eta_0' can be any positive float." )
        if not isinstance(n_x, int):
            raise TypeError("'n_x' must be an int." )
        if not isinstance(n_z, int):
            raise TypeError("'n_z' must be an int." )
        if not isinstance(m_x, float):
            raise TypeError("'m_x' parameter must be a 'float' and != 'n_z'." )
        if abs(float(n_z)-m_x) < 1e-5:
            raise TypeError("'m_x' must be different than 'n_z'." )
        self._ckeep = _cfn.SolM(eta_0, n_x, n_z, m_x)
        super(SolM,self).__init__(_cfn.SolMCRTP(self._ckeep,2), **kwargs)

class SolNL(_SolBase):
    """
    Non-linear analytic solution. Note that the exactly analytic viscosity
    (function of position only) is exposed via the standard `fn_viscosity`
    attribute, while the non-linear viscosity (function of position and
    velocity) is exposed via the `get_viscosity_nl()` method.

    Parameters
    ----------
    eta_0 : float
        Viscosity perturbation strength factor.
    n_z : int
        Velocity wavenumber parameter (in z).
    r : float
        Viscosity parameter.

    """
    eqn_bodyforce = None
    eqn_viscosity = "\eta_0 ( \dot{\eta}_{ij} \dot{\eta}_{ij} )^{(1/r - 1)}"
    def __init__(self, eta_0=1., n_z=1, r=1.5, *args, **kwargs):
        if not isinstance(eta_0, float) or eta_0 <= 0.:
            raise TypeError("'eta_0' can be any positive float." )
        if not isinstance(n_z, int):
            raise TypeError("'n_z' must be an int." )
        if not isinstance(r, float):
            raise TypeError("'r' parameter must be a 'float'." )
        
        self._ckeep = _cfn.SolNL(eta_0,n_z,r)
        super(SolNL,self).__init__(_cfn.SolNLCRTP(self._ckeep,2), **kwargs)
        self.nonlinear = True

    def get_bcs( self, velVar ):
        '''
        ( Fixed, Fixed ) conditions vertical walls.
        (  Free, Fixed ) conditions hortizontal walls.

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

        #           v_x   v_y
        bcverts = [ iw,  iw+jw ]
        
        import underworld as uw
        return uw.conditions.DirichletCondition(velVar, bcverts)

    def get_viscosity_nl(self, velocity, pressure):
        """
        This method returns a `Function` object for the non-linear
        viscosity.

        Parameters
        ----------
        velocity: underworld.function.Function
            The velocity.
        pressure: underworld.function.Function
            The pressure. Pressure is not utilised for this
            viscosity.
        """
        import underworld.function as fn
        inv2 = fn.tensor.second_invariant(
                      fn.tensor.deviatoric(
                      fn.tensor.symmetric( velocity.fn_gradient ) ) )
        alpha_by_two  = (2./self._ckeep.r -2.)
        factor = 2.*self._ckeep.eta0
        return factor * fn.math.pow(inv2, alpha_by_two)


class _SolBaseFixedBc(_SolBase):
    def get_bcs(self, velVar):
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

        # normalise walls to ensure required flux
        sets = []
        sets.append( (mesh.specialSets["MinI_VertexSet"], 0) )
        sets.append( (mesh.specialSets["MaxI_VertexSet"], 0) )
        sets.append( (mesh.specialSets["MinJ_VertexSet"], 1) )
        sets.append( (mesh.specialSets["MaxJ_VertexSet"], 1) )
        if self.dim==3:
            sets.append( (mesh.specialSets["MinK_VertexSet"], 2) )
            sets.append( (mesh.specialSets["MaxK_VertexSet"], 2) )
        import underworld as uw
        for setg in sets:
            verts = setg[0]
            ind   = setg[1]
            # calc numerical wall flux
            intvn = uw.utils.Integral(           velVar[ind], mesh, integrationType="Surface", surfaceIndexSet=verts).evaluate()
            # calc analytic wall flux
            intva = uw.utils.Integral( self.fn_velocity[ind], mesh, integrationType="Surface", surfaceIndexSet=verts).evaluate()
            # normalise bcs
            velVar.data[verts,ind] -= (intvn-intva)

        # now flag required nodes
        bcverts = []
        for dim in range(self.dim):
            bcverts.append(wall_verts)

        import underworld as uw
        return uw.conditions.DirichletCondition(velVar, bcverts)


class SolDB2d(_SolBaseFixedBc):
    """
    This solution uses fixed velocity boundary conditions on all walls
    along with a constant viscosity. It is originally published in:

    Dohrmann, C.R., Bochev, P.B., A stabilized finite element method for the
    Stokes problem based on polynomial pressure projections,
    Int. J. Numer. Meth. Fluids 46, 183-201 (2004).

    """
    eqn_bodyforce = None
    eqn_viscosity = "1"
    def __init__(self, *args, **kwargs):
        self._ckeep = _cfn.SolDB2d()
        super(SolDB2d,self).__init__(_cfn.SolDB2dCRTP(self._ckeep,2), **kwargs)



class SolDB3d(_SolBaseFixedBc):
    """
    This solution uses fixed velocity boundary conditions on all walls and
    a variable viscosity. It is originally published in:

    Dohrmann, C.R., Bochev, P.B., A stabilized finite element method for the
    Stokes problem based on polynomial pressure projections,
    Int. J. Numer. Meth. Fluids 46, 183-201 (2004).

    Parameters
    ----------
    Beta: float
        Viscosity perturbation strength factor.

    """
    def __init__(self, Beta=4., *args, **kwargs):
        self._ckeep = _cfn.SolDB3d(Beta)
        super(SolDB3d,self).__init__(_cfn.SolDB3dCRTP(self._ckeep,3), **kwargs)



def _add_doc_info():
    """
    Populate docstring with info.
    Traverse classes and add info as necessary
    """
    import sys as _sys
    import inspect as _inspect
    for guy in _inspect.getmembers(_sys.modules[__name__], _inspect.isclass):
        # if private, continue
        if guy[0][0] == "_":
            continue
        guy[1].__doc__ += "\n    Notes\n    _____"

        # add equations where available
        bfeqn = guy[1].eqn_bodyforce
        visceqn = guy[1].eqn_viscosity
        if visceqn:
            guy[1].__doc__ += "\n\n    .. math:: \eta = {}".format(visceqn)
        if bfeqn:        
            guy[1].__doc__ += "\n\n    .. math:: f = {}".format(bfeqn)        

        # add images
        guy[1].__doc__ += "\n\n"
        guy[1].__doc__ += "    .. |{}_viscosity| image:: images/{}_viscosity.png\n".format(guy[0],guy[0])
        guy[1].__doc__ += "    .. |{}_bodyforce| image:: images/{}_bodyforce.png\n".format(guy[0],guy[0])
        guy[1].__doc__ += "    .. |{}_velocity|  image:: images/{}_velocity.png\n".format(guy[0],guy[0])
        guy[1].__doc__ += "    .. |{}_pressure|  image:: images/{}_pressure.png\n".format(guy[0],guy[0])
        
        guy[1].__doc__ += """
    
    .. csv-table::
        :header-rows: 1
        :align: center

        "Viscosity", "\|Body Force\|", "\|Velocity\|", "Pressure" \n"""
        guy[1].__doc__ += "        |{}_viscosity|, |{}_bodyforce|, |{}_velocity|, |{}_pressure|".format(guy[0],guy[0],guy[0],guy[0])
_add_doc_info()