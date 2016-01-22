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
    The system class that manages the Incompressible Stokes Equation
    This class holds the numerical objects that define the equation and the solver to be used to solve the equation.

    Notes
    -----
        .. math::


    """
    _objectsDict = {  "_system" : "Stokes_SLE",
                      "_solver" : "Stokes_SLE_UzawaSolver" }
    _selfObjectName = "_system"

    def __init__(self, velocityField, pressureField, viscosityFn, bodyForceFn=None, swarm=None, conditions=[], rtolerance=1.0e-5, **kwargs):

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

        _viscosityFn = uw.function.Function._CheckIsFnOrConvertOrThrow(viscosityFn)
        if not isinstance( _viscosityFn, uw.function.Function):
            raise TypeError( "Provided 'viscosityFn' must be of or convertible to 'Function' class." )

        _bodyForceFn = uw.function.Function._CheckIsFnOrConvertOrThrow(bodyForceFn)
        if _bodyForceFn and not isinstance( _bodyForceFn, uw.function.Function):
            raise TypeError( "Provided 'bodyForceFn' must be of or convertible to 'Function' class." )
        self._bodyForceFn = _bodyForceFn

        if swarm and not isinstance(swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = swarm

        if not isinstance(conditions, (uw.conditions._SystemCondition, list, tuple) ):
            raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
        if len(conditions) > 1:
            raise ValueError( "Multiple conditions are not currently supported." )
        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here.. will rearrange this in future. 
            if cond.variable == self._velocityField:
                libUnderworld.StgFEM.FeVariable_SetBC( self._velocityField._cself, cond._cself )
            elif cond.variable == self._pressureField:
                libUnderworld.StgFEM.FeVariable_SetBC( self._pressureField._cself, cond._cself )
            else:
                raise ValueError("Condition object does not appear to apply to the provided velocityField or pressureField.")

        if not isinstance(rtolerance, float):
            raise TypeError( "Provided 'rtolerance' must be of 'rtolerance' class." )
        self._rtolerance = rtolerance
        # ok, we've set some bcs, lets recreate eqnumbering
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._pressureField._cself )
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._velocityField._cself )
        self._conditions = conditions

        # create solutions vectors
        self._velocitySol = sle.SolutionVector(velocityField)
        self._pressureSol = sle.SolutionVector(pressureField)

        # create force vectors
        self._fvector = sle.AssembledVector(velocityField)
        self._hvector = sle.AssembledVector(pressureField)

        # and matrices
        self._kmatrix = sle.AssembledMatrix( velocityField, velocityField, rhs=self._fvector )
        self._gmatrix = sle.AssembledMatrix( velocityField, pressureField, rhs=self._fvector, rhs_T=self._hvector )
        self._preconditioner = sle.AssembledMatrix( pressureField, pressureField, rhs=self._hvector, allowZeroContrib=True )
        
        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._velocityField.mesh)
        self._PICSwarm = None
        if self._swarm:
            self._PICSwarm = uw.swarm.PICIntegrationSwarm(self._swarm)
            self._PICSwarm.repopulate()
        # create assembly terms
        self._gradStiffMatTerm = sle.GradientStiffnessMatrixTerm(   integrationSwarm=self._gaussSwarm,
                                                                    assembledObject=self._gmatrix)
        self._preCondMatTerm   = sle.PreconditionerMatrixTerm(  integrationSwarm=self._gaussSwarm,
                                                                assembledObject=self._preconditioner)
        
        swarmguy = self._PICSwarm
        if not swarmguy:
            swarmguy = self._gaussSwarm
        self._constitMatTerm = sle.ConstitutiveMatrixTerm(  integrationSwarm=swarmguy,
                                                            assembledObject=self._kmatrix,
                                                            fn=viscosityFn)
        self._forceVecTerm   = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=swarmguy,
                                                                assembledObject=self._fvector,
                                                                fn=bodyForceFn)
        super(Stokes, self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(Stokes,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][   "StressTensorMatrix"] = self._kmatrix._cself.name
        componentDictionary[ self._cself.name ][       "GradientMatrix"] = self._gmatrix._cself.name
        componentDictionary[ self._cself.name ][     "DivergenceMatrix"] = None
        componentDictionary[ self._cself.name ]["CompressibilityMatrix"] = None
        componentDictionary[ self._cself.name ][       "VelocityVector"] = self._velocitySol._cself.name
        componentDictionary[ self._cself.name ][       "PressureVector"] = self._pressureSol._cself.name
        componentDictionary[ self._cself.name ][          "ForceVector"] = self._fvector._cself.name
        componentDictionary[ self._cself.name ]["ContinuityForceVector"] = self._hvector._cself.name
        componentDictionary[ self._cself.name ][    "killNonConvergent"] = False
        componentDictionary[ self._cself.name ][           "SLE_Solver"] = self._solver.name

        componentDictionary[ self._solver.name ][       "Preconditioner"] = self._preconditioner._cself.name
        componentDictionary[ self._solver.name ][            "tolerance"] = self._rtolerance
        componentDictionary[ self._solver.name ][              "monitor"] = True
        componentDictionary[ self._solver.name ][        "minIterations"] = 1
        componentDictionary[ self._solver.name ][        "maxIterations"] = 5000


    def solve(self, nonLinearIterate=None):
        """ solve the sle using provided solver
        """
        # check for non-linearity
        nonLinear = False
        message = "Nonlinearity detected."
        if self._velocityField in self.viscosityFn._underlyingDataItems:
            nonLinear = True
            message += "\nViscosity function depends on the velocity field provided to the Stokes system."
        if self._pressureField in self.viscosityFn._underlyingDataItems:
            nonLinear = True
            message += "\nViscosity function depends on the pressure field provided to the Stokes system."
        if self._velocityField in self.bodyForceFn._underlyingDataItems:
            nonLinear = True
            message += "\nBody force function depends on the velocity field provided to the Stokes system."
        if self._pressureField in self.bodyForceFn._underlyingDataItems:
            nonLinear = True
            message += "\nBody force function depends on the pressure field provided to the Stokes system."

        message += "\nPlease pass the 'nonLinearIterate' solve parameter to 'True' or 'False' to continue."
        if nonLinear and (nonLinearIterate==None):
            raise RuntimeError(message)
        
        if nonLinear and nonLinearIterate:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._cself, True )
        else:
            libUnderworld.StgFEM.SystemLinearEquations_SetToNonLinear(self._cself, False )


        if self._PICSwarm:
            self._PICSwarm.repopulate()
        libUnderworld.StgFEM.SystemLinearEquations_BC_Setup(self._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_LM_Setup(self._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_ZeroAllVectors(self._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_MatrixSetup(self._cself, None)
        libUnderworld.StgFEM.SystemLinearEquations_VectorSetup(self._cself, None)
        if nonLinear and nonLinearIterate:
            libUnderworld.StgFEM.SystemLinearEquations_NonLinearExecute(self._cself, None)
        else:
            libUnderworld.StgFEM.SystemLinearEquations_ExecuteSolver(self._cself, None)

        ## execute solver
        ### _SLE_Solver_Execute
        #### SLE_Solver_SolverSetup
        #### SLE_Solver_Solve

        libUnderworld.StgFEM.SystemLinearEquations_UpdateSolutionOntoNodes(self._cself, None)

    @property
    def viscosityFn(self):
        return self._constitMatTerm.fn
    @viscosityFn.setter
    def viscosityFn(self, value):
        self._constitMatTerm.fn = value

    @property
    def bodyForceFn(self):
        return self._forceVecTerm.fn
    @bodyForceFn.setter
    def bodyForceFn(self, value):
        self._forceVecTerm.fn = value

