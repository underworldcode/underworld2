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

class SteadyStateHeat(_stgermain.StgCompoundComponent):
    """
    The system class that manages the Steady State Heat Equation. 
    This class holds the numerical objects that define the equation and the solver to be used to solve the equation.


    Notes
    -----

    .. math:: $\nabla(k\nabla)T = h$

    .. math:: 

         \\nabla(k\\nabla)T = h

        where, k is the conductivity, T is the temperature (solution), h is the heating r.h.s
        
        >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> tField = uw.fevariable.FeVariable( linearMesh, 1 )
        >>> topNodes = linearMesh.specialSets["MaxJ_VertexSet"]
        >>> bottomNodes = linearMesh.specialSets["MinJ_VertexSet"]
        >>> tbcs = uw.conditions.DirichletCondition(tField, topNodes + bottomNodes)
        >>> tField.data[topNodes.data] = 0.0
        >>> tField.data[bottomNodes.data] = 1.0
        >>> tSystem = uw.systems.SteadyStateHeat(temperatureField=tField, conductivityFn=1.0, conditions=[tbcs])
    """
    _objectsDict = {  "_system" : "Energy_SLE",
                      "_solver" : "Energy_SLE_Solver" }
    _selfObjectName = "_system"

    def __init__(self, temperatureField, conductivityFn, heatingFn=None, swarm=None, conditions=[], rtolerance=1.0e-5, **kwargs):
        """
        Create an instance of the Heat equation system
        
        Parameters
        ----------
        temperatureField : FeVariable
            The solution field for temperature
        conductivityFn : Function
            The conductivy function that defines the conductivity across the domain
        heatingFn : Function
            The heating r.h.s function that defines the heating across the domain

        Returns
        -------
        SteadyStateHeat :
            Constructed system for managing the Steady State Equation
        """

        if not isinstance( temperatureField, uw.fevariable.FeVariable):
            raise TypeError( "Provided 'temperatureField' must be of 'FeVariable' class." )
        self._temperatureField = temperatureField

        try:
            _conductivityFn = uw.function.Function._CheckIsFnOrConvertOrThrow(conductivityFn)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'conductivityFn' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if not heatingFn:
            heatingFn = 0.
        try:
            _heatingFn = uw.function.Function._CheckIsFnOrConvertOrThrow(heatingFn)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'heatingFn' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

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
            if cond.variable == self._temperatureField:
                libUnderworld.StgFEM.FeVariable_SetBC( self._temperatureField._cself, cond._cself )

        if not isinstance(rtolerance, float):
            raise TypeError( "Provided 'rtolerance' must be of 'rtolerance' class." )
        self._rtolerance = rtolerance
        # ok, we've set some bcs, lets recreate eqnumbering
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._temperatureField._cself )
        self._conditions = conditions

        # create solutions vector
        self._temperatureSol = sle.SolutionVector(temperatureField)

        # create force vectors
        self._fvector = sle.AssembledVector(temperatureField)

        # and matrices
        self._kmatrix = sle.AssembledMatrix( temperatureField, temperatureField, rhs=self._fvector )

        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._temperatureField.feMesh)
        self._PICSwarm = None
        if self._swarm:
            self._PICSwarm = uw.swarm.PICIntegrationSwarm(self._swarm)

        swarmguy = self._PICSwarm
        if not swarmguy:
            swarmguy = self._gaussSwarm
        self._kMatTerm = sle.MatrixAssemblyTerm_NA_i__NB_i__Fn(  integrationSwarm=swarmguy,
                                                                 assembledObject=self._kmatrix,
                                                                 fn=_conductivityFn)
        self._forceVecTerm = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=swarmguy,
                                                              assembledObject=self._fvector,
                                                              fn=heatingFn)
        super(SteadyStateHeat, self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SteadyStateHeat,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][      "StiffnessMatrix"] = self._kmatrix._cself.name
        componentDictionary[ self._cself.name ][       "SolutionVector"] = self._temperatureSol._cself.name
        componentDictionary[ self._cself.name ][          "ForceVector"] = self._fvector._cself.name
        componentDictionary[ self._cself.name ][    "killNonConvergent"] = False
        componentDictionary[ self._cself.name ][           "SLE_Solver"] = self._solver.name

        componentDictionary[ self._solver.name ][           "tolerance"] = self._rtolerance
        componentDictionary[ self._solver.name ][             "monitor"] = True
        componentDictionary[ self._solver.name ][       "minIterations"] = 1
        componentDictionary[ self._solver.name ][       "maxIterations"] = 5000


    def solve(self, nonLinearIterate=None):
        """ solve the sle using provided solver
        """
        # check for non-linearity
        nonLinear = False
        message = "Nonlinearity detected."
        if self._temperatureField in self.conductivityFn._underlyingDataItems:
            nonLinear = True
            message += "\nConductivity function depends on the temperature field provided to the system."
        if self._temperatureField in self.heatingFn._underlyingDataItems:
            nonLinear = True
            message += "\nHeating function depends on the temperature field provided to the system."

        message += "\nPlease set the 'nonLinearIterate' solve parameter to 'True' or 'False' to continue."
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

        libUnderworld.StgFEM.SystemLinearEquations_UpdateSolutionOntoNodes(self._cself, None)

    @property
    def conductivityFn(self):
        return self._kMatTerm.fn
    @conductivityFn.setter
    def conductivityFn(self, value):
        self._kMatTerm.fn = value

    @property
    def heatingFn(self):
        return self._forceVecTerm.fn
    @heatingFn.setter
    def heatingFn(self, value):
        self._forceVecTerm.fn = value

