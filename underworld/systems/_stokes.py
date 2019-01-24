##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
import underworld as uw
import underworld._stgermain as _stgermain
import sle
import libUnderworld

class Stokes(_stgermain.StgCompoundComponent):
    """
    This class provides functionality for a discrete representation
    of the Stokes flow equations.

    Specifically, the class uses a mixed finite element method to
    construct a system of linear equations which may then be solved
    using an object of the underworld.system.Solver class.

    The underlying element types are determined by the supporting
    mesh used for the 'velocityField' and 'pressureField' parameters.

    The strong form of the given boundary value problem, for :math:`f`,
    :math:`g` and :math:`h` given, is

    .. math::
        \\begin{align}
        \\sigma_{ij,j} + f_i =& \\: 0  & \\text{ in }  \\Omega \\\\
        u_{k,k} + \\frac{p}{\\lambda} =& \\: H  & \\text{ in }  \\Omega \\\\
        u_i =& \\: g_i & \\text{ on }  \\Gamma_{g_i} \\\\
        \\sigma_{ij}n_j =& \\: h_i & \\text{ on }  \\Gamma_{h_i} \\\\
        \\end{align}

    where,

    * :math:`\\sigma_{i,j}` is the stress tensor
    * :math:`u_i` is the velocity,
    * :math:`p`   is the pressure,
    * :math:`f_i` is a body force,
    * :math:`\\lambda` is pseudo compressibility factor,
    * :math:`H` is the compressible equation source term,
    * :math:`g_i` are the velocity boundary conditions (DirichletCondition)
    * :math:`h_i` are the traction boundary conditions (NeumannCondition).

    The problem boundary, :math:`\\Gamma`,
    admits the decompositions :math:`\\Gamma=\\Gamma_{g_i}\\cup\\Gamma_{h_i}` where
    :math:`\\emptyset=\\Gamma_{g_i}\\cap\\Gamma_{h_i}`. The equivalent weak form is:

    .. math::
        \\int_{\Omega} w_{(i,j)} \\sigma_{ij} \\, d \\Omega = \\int_{\\Omega} w_i \\, f_i \\, d\\Omega + \sum_{j=1}^{n_{sd}} \\int_{\\Gamma_{h_j}} w_i \\, h_i \\,  d \\Gamma

    where we must find :math:`u` which satisfies the above for all :math:`w`
    in some variational space.

    Parameters
    ----------
    velocityField : underworld.mesh.MeshVariable
        Variable used to record system velocity.
    pressureField : underworld.mesh.MeshVariable
        Variable used to record system pressure.
    fn_viscosity : underworld.function.Function
        Function which reports a viscosity value.
        Function must return scalar float values.
    fn_bodyforce : underworld.function.Function, Default = None
        Function which reports a body force for the system.
        Function must return float values of identical dimensionality
        to the provided velocity variable.
    fn_one_on_lambda : underworld.function.Function, Default = None
        Pseudo-compressibility factor. Note that non-zero values are
        incompatible with the 'penalty' stokes solver. Ensure a
        'penalty' equal to 0 is used if this function is non-zero.
        By default this is the case.
    fn_source : underworld.function.Function, Default = None
        Mass source term. Check fn_one_on_lambda for usage caveats.
    fn_stresshistory : underworld.function.Function, Default = None
        Function which defines the stress history term used for viscoelasticity.
        Function is a vector of size 3 (dim=2) or 6 (dim=3) representing a symetric tensor.
    voronoi_swarm : underworld.swarm.Swarm
        If a voronoi_swarm is provided, voronoi type numerical integration is
        utilised. The provided swarm is used as the basis for the voronoi
        integration. If no voronoi_swarm is provided, Gauss integration
        is used.
    conditions : underworld.conditions.SystemCondition
        Numerical conditions to impose on the system. This should be supplied as
        the condition itself, or a list object containing the conditions.

    Notes
    -----
    Constructor must be called by collectively all processes.


    """
    _objectsDict = {  "_system" : "Stokes_SLE" }
    _selfObjectName = "_system"


    def __init__(self, velocityField, pressureField, fn_viscosity, fn_bodyforce=None, fn_one_on_lambda=None,
                fn_source=None, voronoi_swarm=None, conditions=[],
                _removeBCs=True, _fn_viscosity2=None, _fn_director=None, fn_stresshistory=None, _fn_stresshistory=None,
                _fn_v0=None, _fn_p0=None, _callback_post_solve=None, **kwargs):

        if _fn_stresshistory:
            raise TypeError( "The parameter '_fn_stresshistory' has been updated to 'fn_stresshistory'." )

        if not isinstance( velocityField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        if velocityField.nodeDofCount != velocityField.mesh.dim:
            raise ValueError( "Provided 'velocityField' must be a vector field of same dimensionality as its mesh." )
        self._velocityField = velocityField
        if not isinstance( pressureField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'pressureField' must be of 'MeshVariable' class." )
        if pressureField.nodeDofCount != 1:
            raise ValueError( "Provided 'pressureField' must be a scalar field (ie pressureField.nodeDofCount==1)." )
        self._pressureField = pressureField

        _fn_viscosity  = uw.function.Function.convert(fn_viscosity)
        if not isinstance( _fn_viscosity, uw.function.Function):
            raise TypeError( "Provided 'fn_viscosity' must be of or convertible to 'Function' class." )
        if _fn_viscosity2:
            _fn_viscosity2 = uw.function.Function.convert(_fn_viscosity2)
            if not isinstance( _fn_viscosity2, uw.function.Function):
                raise TypeError( "Provided 'fn_viscosity2' must be of or convertible to 'Function' class." )

        if not isinstance( _removeBCs, bool):
            raise TypeError( "Provided '_removeBCs' must be of type bool." )
        self._removeBCs = _removeBCs

        if _fn_director:
            _fn_director = uw.function.Function.convert(_fn_director)
            if not isinstance( _fn_director, uw.function.Function):
                raise TypeError( "Provided 'fn_director' must be of or convertible to 'Function' class." )

        if fn_stresshistory:
            fn_stresshistory = uw.function.Function.convert(fn_stresshistory)
            if not isinstance( fn_stresshistory, uw.function.Function):
                raise TypeError( "Provided 'fn_stresshistory' must be of or convertible to 'Function' class." )

        self._fn_minus_one_on_lambda = None
        if fn_one_on_lambda != None:
            self._fn_minus_one_on_lambda = uw.function.Function.convert(-1.0 * fn_one_on_lambda)
            if not isinstance(self._fn_minus_one_on_lambda, uw.function.Function):
                raise ValueError("Provided 'fn_one_on_lambda' must be of, or convertible to, the 'Function' class.")

        if fn_source != None:
            self._fn_source = uw.function.Function.convert(fn_source)
            if not isinstance(self._fn_source, uw.function.Function):
                raise ValueError("Provided 'fn_source' must be of, or convertible to, the 'Function' class.")

        if not fn_bodyforce:
            if velocityField.mesh.dim == 2:
                fn_bodyforce = (0.,0.)
            else:
                fn_bodyforce = (0.,0.,0.)
        _fn_bodyforce = uw.function.Function.convert(fn_bodyforce)
        
        if _fn_p0 is not None:
            if not isinstance(_fn_p0, uw.function.misc.constant):
                raise ValueError("Provided '_fn_p0' must be of type 'uw.function.misc.constant'")
        self._fn_p0 = _fn_p0       
        if _fn_v0 is not None:
            if not isinstance(_fn_v0, uw.function.misc.constant):
                raise ValueError("Provided '_fn_v0' must be of type 'uw.function.misc.constant'")
        self._fn_v0 = _fn_v0


        if voronoi_swarm and not isinstance(voronoi_swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'voronoi_swarm' must be of 'Swarm' class." )
        self._swarm = voronoi_swarm
        if voronoi_swarm and velocityField.mesh.elementType=='Q2':
            import warnings
            warnings.warn("Voronoi integration may yield unsatisfactory results for Q2 mesh.")

        mesh = velocityField.mesh

        if not isinstance(conditions,(list,tuple)):
            conditionslist = []
            conditionslist.append(conditions)
            conditions = conditionslist
        for cond in conditions:
            # set the bcs on here
            if not isinstance( cond, uw.conditions.SystemCondition ):
                raise TypeError( "Provided 'conditions' must be 'SystemCondition' objects." )
            elif type(cond) == uw.conditions.DirichletCondition:
                if cond.variable == self._velocityField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._velocityField._cself, cond._cself )
                elif cond.variable == self._pressureField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._pressureField._cself, cond._cself )
                else:
                    raise ValueError("Provided condition does not appear to correspond to the system unknowns.")

        self._conditions = conditions

        self._eqNums = dict()
        self._eqNums[velocityField] = sle.EqNumber( self._velocityField, self._removeBCs )
        self._eqNums[pressureField] = sle.EqNumber( self._pressureField, self._removeBCs )

        # create solutions vectors and load fevariable values onto them for best first guess
        self._velocitySol = sle.SolutionVector(velocityField, self._eqNums[velocityField])
        self._pressureSol = sle.SolutionVector(pressureField, self._eqNums[pressureField])
        libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._velocitySol._cself );
        libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._pressureSol._cself );

        # set callback to default or user defined
        self.callback_post_solve = None
        if _callback_post_solve is None:
            if _fn_p0:
                mesh = self._velocityField.mesh
                top = mesh.specialSets["MaxJ_VertexSet"]
                surfInt = uw.utils.Integral(fn=(1.0,self._pressureField), mesh=mesh, integrationType='surface', surfaceIndexSet=top)

                def avtop_pressure_nullspace_removal():
                    result = surfInt.evaluate()
                    _fn_p0.value = result[1]/result[0]
                self.callback_post_solve = avtop_pressure_nullspace_removal
        else:
            self.callback_post_solve = _callback_post_solve

        # create force vectors
        self._fvector = sle.AssembledVector(velocityField, self._eqNums[velocityField] )
        self._hvector = sle.AssembledVector(pressureField, self._eqNums[pressureField] )

        # and matrices
        self._kmatrix = sle.AssembledMatrix( self._velocitySol, self._velocitySol, rhs=self._fvector )
        self._gmatrix = sle.AssembledMatrix( self._velocitySol, self._pressureSol, rhs=self._fvector, rhs_T=self._hvector )
        self._preconditioner = sle.AssembledMatrix( self._pressureSol, self._pressureSol, rhs=self._hvector )

        # create assembly terms which always use gauss integration
        gaussSwarm = uw.swarm.GaussIntegrationSwarm(mesh)
        self._gradStiffMatTerm = sle.GradientStiffnessMatrixTerm(   integrationSwarm=gaussSwarm,
          assembledObject=self._gmatrix)
        self._preCondMatTerm   = sle.PreconditionerMatrixTerm(  integrationSwarm=gaussSwarm,
                                                                assembledObject=self._preconditioner)

        # for the following terms, we will use voronoi if that has been requested
        # by the user, else use gauss again.
        intswarm = gaussSwarm

        if self._swarm:
            intswarm = self._swarm._voronoi_swarm
            # need to ensure voronoi is populated now, as assembly terms will call
            # initial test functions which may require a valid voronoi swarm
            self._swarm._voronoi_swarm.repopulate()

        self._constitMatTerm = sle.ConstitutiveMatrixTerm(  integrationSwarm = intswarm,
                                                            assembledObject  = self._kmatrix,
                                                            fn_visc1         = _fn_viscosity,
                                                            fn_visc2         = _fn_viscosity2,
                                                            fn_director      = _fn_director)

        self._forceVecTerm   = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=intswarm,
                                                                assembledObject=self._fvector,
                                                                fn=_fn_bodyforce)

        if fn_source != None:
            self._cforceVecTerm   = sle.VectorAssemblyTerm_NA__Fn(  integrationSwarm=intswarm,
                                                                    assembledObject=self._hvector,
                                                                    fn=self.fn_source,
                                                                    mesh=mesh)


        for cond in self._conditions:
            if isinstance( cond, uw.conditions.NeumannCondition ):
                #NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject    = self._fvector,
                                                                surfaceGaussPoints = 3, # increase to resolve stress bc fluctuations
                                                                nbc                = cond )
        if self._fn_minus_one_on_lambda != None:
            # add matrix and associated assembly term for compressible stokes formulation
            # a mass matrix goes into the lower right block of the stokes system coeff matrix
            self._mmatrix = sle.AssembledMatrix( self._pressureSol, self._pressureSol, rhs=self._hvector )
            # -1. as per Hughes, The Finite Element Method, 1987, Table 4.3.1, [M]

            self._compressibleTerm = sle.MatrixAssemblyTerm_NA__NB__Fn(  integrationSwarm=intswarm,
                                                                         assembledObject=self._mmatrix,
                                                                         mesh=mesh,
                                                                         fn=self._fn_minus_one_on_lambda )

        if fn_stresshistory != None:
            self._NA_j__Fn_ijTerm    = sle.VectorAssemblyTerm_NA_j__Fn_ij(  integrationSwarm=intswarm,
        		                                                assembledObject=self._fvector,
                		                                        fn=fn_stresshistory )
        # objects used for analysis, dictionary for organisation
        self._aObjects = dict()
        self._aObjects['vdotv_fn'] = uw.function.math.dot( self._velocityField, self._velocityField )
        
        super(Stokes, self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(Stokes,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][   "StressTensorMatrix"] = self._kmatrix._cself.name
        componentDictionary[ self._cself.name ][       "GradientMatrix"] = self._gmatrix._cself.name
        componentDictionary[ self._cself.name ][     "DivergenceMatrix"] = None
        if hasattr(self, "_mmatrix"):
            componentDictionary[ self._cself.name ]["CompressibilityMatrix"] = self._mmatrix._cself.name
        componentDictionary[ self._cself.name ][       "VelocityVector"] = self._velocitySol._cself.name
        componentDictionary[ self._cself.name ][       "PressureVector"] = self._pressureSol._cself.name
        componentDictionary[ self._cself.name ][          "ForceVector"] = self._fvector._cself.name
        componentDictionary[ self._cself.name ]["ContinuityForceVector"] = self._hvector._cself.name

    @property
    def fn_viscosity(self):
        """
        The viscosity function. You may change this function directly via this
        property.
        """
        return self._constitMatTerm.fn_visc1
    @fn_viscosity.setter
    def fn_viscosity(self, value):
        self._constitMatTerm.fn_visc1 = value

    @property
    def fn_bodyforce(self):
        """
        The body force function. You may change this function directly via this
        property.
        """
        return self._forceVecTerm.fn

    @fn_bodyforce.setter
    def fn_bodyforce(self, value):
        self._forceVecTerm.fn = value

    # define getter and setter decorators for fn_minus_one_on_lambda - will be conditionally available to users
    @property
    def fn_one_on_lambda(self):
        """
        A bulk viscosity parameter
        """
        return -self._fn_minus_one_on_lambda

    @fn_one_on_lambda.setter
    def fn_one_on_lambda(self, newFn):
        if hasattr(self, '_compressibleTerm'):
            self._fn_minus_one_on_lambda = uw.function.Function.convert(-1.0*newFn)
            self._compressibleTerm._fn = self._fn_minus_one_on_lambda
            self._compressibleTerm._set_fn_function(self._compressibleTerm._cself, self._fn_minus_one_on_lambda._fncself)
        else:
            import warnings
            warnings.warn("Cannot add fn_minus_one_on_lambda to existing stokes object. Instead you should build a new object with fn_minus_one_on_lambda defined", RuntimeWarning)

    # define decorators for fn_source
    @property
    def fn_source(self):
        """
        The volumetric source term function. You may change this function directly via this
        property.
        """
        return self._fn_source

    @fn_source.setter
    def fn_source(self, value):
        if hasattr(self, '_cforceVecTerm'):
            self._fn_source = uw.function.Function.convert(value)
            self._cforceVecTerm._fn = self._fn_source
            self._cforceVecTerm._set_fn_function(self._cforceVecTerm._cself, self._fn_source._fncself)

        else:
            import warnings
            warnings.warn("Cannot add fn_source to existing stokes object. Instead you should build a new object with fn_source defined", RuntimeWarning)

    @property
    def eqResiduals(self):
        """
        Returns the stokes flow equations' residuals from the latest solve. Residual calculations
        use the matrices and vectors of the discretised problem.
        The residuals correspond to the momentum equation and the continuity equation.

        Return
        ------
        (r1, r2) - 2 tuple of doubles
            r1 is the momentum equation residual
            r2 is the continuity equation residual

        Notes
        -----
        This method must be called collectively by all processes.
        """

        res_mEq = uw.libUnderworld.StgFEM.Stokes_MomentumResidual(self._cself)
        res_cEq = uw.libUnderworld.StgFEM.Stokes_ContinuityResidual(self._cself)

        return res_mEq, res_cEq

    @property
    def stokes_callback(self):
        """
        Return the callback function used by this system
        """
        return self._stokes_callback

    @stokes_callback.setter
    def stokes_callback(self, value):
        """
        Setter for stokes_callback, must be a callable python function
        """
        if value is not None:
            if not callable(value):
                raise RuntimeError("The 'callback_post_solve' parameter is not 'None' and isn't callable")
        self._stokes_callback = value
        
    def velocity_rms(self):
        """
        Calculates RMS velocity as follows
        
        .. math:: v_{rms}  =  \\sqrt{ \\frac{ \\int_V (\\mathbf{v}.\\mathbf{v}) \\, \\mathrm{d}V } {\\int_V \\, \\mathrm{d}V} }

        """
        # get the mesh and perform integrals over it
        mesh = self._velocityField.mesh
        
        fn_2_integrate = ( self._aObjects['vdotv_fn'], 1. )
        (v2,vol)       = mesh.integrate( fn=fn_2_integrate )
        import math
        return math.sqrt(v2/vol)
        
