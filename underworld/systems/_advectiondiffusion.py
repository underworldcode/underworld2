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

class AdvectionDiffusion(_stgermain.StgCompoundComponent):
    """
    The system class that manages the Advection Diffusion Equation.
    This class holds the numerical objects that define the equation and the solver to be used to solve the equation.

    Notes
    -----
    .. math::
        \frac{DT}{Dt} = \kappa \nabla^{2} T + f

        where T is the Temperature, t is time, \kappa is the thermal diffusivity and f is a general heating term 
    """
    _objectsDict = {  "_system" : "AdvectionDiffusionSLE",
                      "_solver" : "AdvDiffMulticorrector" }
    _selfObjectName = "_system"

    def __init__(self, temperatureField, velocityField, diffusivity, courantFactor=.25, conditions=[], temperatureFieldDeriv=None, **kwargs):
        """
        Constructor for the Advection Diffusion Equation system

        Parameters
        ----------
        temperatureField : FeVariable
            The temperature solution field

        velocityField : FeVariable
            The velocity field

        diffusivity : float
            The definition of diffusivity. (Currently constant, will be updated to a function next release )

        courantFactor : float (default 0.25)

        conditions[] : SystemCondition, list, tuple
            Dirichlet boundary conditions to use for solving the advection diffusion system

        temperatureFieldDeriv : FeVariable (optional)
            A FeVariable that defines the initial time derivative of the temperature field.
            Used to restart the model from a checkpoint. 
            If this FeVariable is not provided at instansiation the system will create it and 
            use it as rquired

        Returns
        -------
        AdvectionDiffusion :
            Constructed system for managing the Advection Diffusion Equation
            
        """
        self._velocityField = velocityField
        self._diffusivity   = diffusivity
        self._courantFactor = courantFactor
        
        if not isinstance( temperatureField, uw.fevariable.FeVariable):
            raise TypeError( "Provided 'temperatureField' must be of 'FeVariable' class." )
        self._temperatureField = temperatureField

        if not isinstance( temperatureFieldDeriv, (uw.fevariable.FeVariable, type(None))):
            raise TypeError( "Provided 'temperatureFieldDeriv' must be 'None' or of 'FeVariable' class." )
        self._temperatureFieldDeriv = temperatureFieldDeriv
        if temperatureFieldDeriv == None:
            self._temperatureFieldDeriv = uw.fevariable.FeVariable( temperatureField.feMesh, 1)
            self._temperatureFieldDeriv.data[:] = 0.


        if not isinstance(conditions, (uw.conditions._SystemCondition, list, tuple) ):
            raise ValueError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise ValueError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here.. will rearrange this in future. 
            if cond.variable == self._temperatureField:
                libUnderworld.StgFEM.FeVariable_SetBC( self._temperatureField._cself, cond._cself )
                libUnderworld.StgFEM.FeVariable_SetBC( self._temperatureFieldDeriv._cself, cond._cself )

        # ok, we've set some bcs, lets recreate eqnumbering
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._temperatureField._cself )
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._temperatureFieldDeriv._cself )
        self._conditions = conditions

        # create force vectors
        self._residualVector = sle.AssembledVector(temperatureField)
        self._massVector     = sle.AssembledVector(temperatureField)

        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._temperatureField.feMesh)

        super(AdvectionDiffusion, self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AdvectionDiffusion,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][   "SLE_Solver"] = self._solver.name
        componentDictionary[ self._cself.name ][     "PhiField"] = self._temperatureField._cself.name
        componentDictionary[ self._cself.name ][     "Residual"] = self._residualVector._cself.name
        componentDictionary[ self._cself.name ][   "MassMatrix"] = self._massVector._cself.name
        componentDictionary[ self._cself.name ][  "PhiDotField"] = self._temperatureFieldDeriv._cself.name
        componentDictionary[ self._cself.name ][          "dim"] = self._temperatureField.feMesh.dim
        componentDictionary[ self._cself.name ]["courantFactor"] = self._courantFactor

    def _setup(self):
        # create assembly terms here.
        # in particular, the residualTerm requires and tries to build _system, so if created in __init__
        # this causes a conflict.
        self._lumpedMassTerm = sle.LumpedMassMatrixVectorTerm( integrationSwarm = self._gaussSwarm,
                                                                assembledObject = self._massVector  )
        self._residualTerm   = sle.AdvDiffResidualVectorTerm(     velocityField = self._velocityField,
                                                                    diffusivity = self._diffusivity,
                                                               integrationSwarm = self._gaussSwarm,
                                                                assembledObject = self._residualVector,
                                                                      extraInfo = self._cself.name )
        self._cself.advDiffResidualForceTerm = self._residualTerm._cself


    def integrate(self,dt):
        """
        Integrates the advection diffusion system through time, dt

        Parameters
        ----------
        dt : float
            The timestep interval to use

        """
        self._cself.currentDt = dt
        libUnderworld.StgFEM._AdvectionDiffusionSLE_Execute( self._cself, None )

    def get_max_dt(self):
        """
        Runs all methods to calculate numerically stable timestep which are associated with this advection diffusion system. 
        The minimum timestep calculated is returned. (Minimum guarantees all time varying phenomena is appropriately captured.

        Returns
        -------
        float
            The maximum time interval appropriate to integrate the system in time
        """
        return libUnderworld.StgFEM.AdvectionDiffusionSLE_CalculateDt( self._cself, None )
