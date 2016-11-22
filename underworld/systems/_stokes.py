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
    of the incompressible Stokes equation.

    Specifically, the class uses a mixed finite element method to
    construct a system of linear equations which may then be solved
    using an object of the underworld.system.Solver class.

    The underlying element types are determined by the supporting
    mesh used for the 'velocityField' and 'pressureField' parameters.

    Parameters
    ----------
    velocityField : underworld.mesh.MeshVariable
        Variable used to record system velocity.
    pressureField : underworld.mesh.MeshVariable
        Variable used to record system pressure.
    fn_viscosity : underworld.function.Function
        Function which reports a viscosity value.
        Function must return scalar float values.
    fn_bodyforce : underworld.function.Function, default=None.
        Function which reports a body force for the system.
        Function must return float values of identical dimensionality
        to the provided velocity variable.
    fn_lambda : underworld.function.Function, default=None.
        Function which defines a non solenoidal velocity field via the relationship
        div(velocityField) = fn_lambda * pressurefield
        If fn_lambda is <= 1e-8 it's effect is considered negligable and
        div(velocityField) = 0 is enofrced as the constaint equation.
        This method is incompatible with the 'penalty' stokes solver, ensure
        the 'penalty' of 0, is used when fn_lambda is used. By default this is the case.
    voronoi_swarm : uw.swarm.Swarm, optional
        If a voronoi_swarm is provided, voronoi type integration is utilised to
        integrate across elements. The provided swarm is used as the basis for
        the voronoi integration. If no swarm is provided, Gauss integration
        is used.
    conditions : uw.conditions.DirichletCondition object (or list of), default=None
        Conditions to be placed on the system.

    Notes
    -----
    Constructor must be called by collectively all processes.


    """
    _objectsDict = {  "_system" : "Stokes_SLE" }
    _selfObjectName = "_system"

    def __init__(self, velocityField, pressureField, fn_viscosity=None, fn_bodyforce=None, fn_lambda=None, voronoi_swarm=None, conditions=[],
                _removeBCs=True, _fn_viscosity2=None, _fn_director=None, _fn_stresshistory=None, swarm=None, **kwargs):

        # DEPRECATE. JM 09/16
        if swarm:
            import warnings
            warnings.warn("'swarm' paramater has been renamed to 'voronoi_swarm'. Please update your models. "+
                          "'swarm' parameter will be removed in the next release.")
            if voronoi_swarm:
                raise ValueError("Please provide only a 'voronoi_swarm'. 'swarm' is deprecated.")
            voronoi_swarm = swarm
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

        if not fn_viscosity:
            raise ValueError("You must specify a viscosity function via the 'fn_viscosity' parameter.")
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

        if _fn_stresshistory:
            _fn_stresshistory = uw.function.Function.convert(_fn_stresshistory)
            if not isinstance( _fn_stresshistory, uw.function.Function):
                raise TypeError( "Provided '_fn_stresshistory' must be of or convertible to 'Function' class." )

        if fn_lambda != None:
            fn_lambda = uw.function.Function.convert(fn_lambda)
            if not isinstance(fn_lambda, uw.function.Function):
                raise ValueError("Provided 'fn_lambda' must be of, or convertible to, the 'Function' class.")

        if not fn_bodyforce:
            if velocityField.mesh.dim == 2:
                fn_bodyforce = (0.,0.)
            else:
                fn_bodyforce = (0.,0.,0.)
        _fn_bodyforce = uw.function.Function.convert(fn_bodyforce)

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
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise TypeError( "Provided 'conditions' must be 'SystemCondition' objects." )
            elif type(cond) == uw.conditions.DirichletCondition:
                if cond.variable == self._velocityField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._velocityField._cself, cond._cself )
                if cond.variable == self._pressureField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._pressureField._cself, cond._cself )

        self._conditions = conditions

        self._eqNums = dict()
        self._eqNums[velocityField] = sle.EqNumber( self._velocityField, self._removeBCs )
        self._eqNums[pressureField] = sle.EqNumber( self._pressureField, self._removeBCs )

        # create solutions vectors and load fevariable values onto them for best first guess
        self._velocitySol = sle.SolutionVector(velocityField, self._eqNums[velocityField])
        self._pressureSol = sle.SolutionVector(pressureField, self._eqNums[pressureField])
        libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._velocitySol._cself );
        libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._pressureSol._cself );

        # create force vectors
        self._fvector = sle.AssembledVector(velocityField, self._eqNums[velocityField] )
        self._hvector = sle.AssembledVector(pressureField, self._eqNums[pressureField] )

        # and matrices
        self._kmatrix = sle.AssembledMatrix( self._velocitySol, self._velocitySol, rhs=self._fvector )
        self._gmatrix = sle.AssembledMatrix( self._velocitySol, self._pressureSol, rhs=self._fvector, rhs_T=self._hvector )
        self._preconditioner = sle.AssembledMatrix( self._pressureSol, self._pressureSol, rhs=self._hvector )
        if fn_lambda != None:
            self._mmatrix = sle.AssembledMatrix( self._pressureSol, self._pressureSol, rhs=self._hvector )

        # create assembly terms which always use gauss integration
        gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._velocityField.mesh)
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
        for cond in self._conditions:
            if isinstance( cond, uw.conditions.NeumannCondition ):
                #NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject    = self._fvector,
                                                                surfaceGaussPoints = 2, # increase to resolve stress bc fluctuations
                                                                nbc                = cond )
        if fn_lambda != None:
            # some logic for constructing the lower-right [2,2] matrix in the stokes system
            # [M] = [Na * 1.0/fn_lambda * Nb], where in our formulation Na and Nb are the pressure shape functions.
            # see 4.3.21 of Hughes, Linear static and dynamic finite element analysis

            # If fn_lambda is negligable, ie <1.0e-8, then we set the entry to 0.0, ie, incompressible
            # otherwise we provide 1.0/lambda to [M]

            logicFn = uw.function.branching.conditional(
                                                      [  ( fn_lambda > 1.0e-8, 1.0/fn_lambda ),
                                                      (                  True,     0.        )   ] )

            self._compressibleTerm = sle.MatrixAssemblyTerm_NA__NB__Fn(  integrationSwarm=intswarm,
                                                                         assembledObject=self._mmatrix,
                                                                         mesh=self._velocityField.mesh,
                                                                         fn=logicFn )

        if _fn_stresshistory != None:
            self._vepTerm    = sle.VectorAssemblyTerm_VEP__Fn(  integrationSwarm=intswarm,
        		                                                assembledObject=self._fvector,
                		                                        fn=_fn_stresshistory )


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
