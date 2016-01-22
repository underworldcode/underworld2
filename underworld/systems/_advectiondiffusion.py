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
        \frac{DT}{Dt} = \kappa \nabla^{2} \phi + f

        where \phi is the Concentration (e.g. temperature), t is time, \kappa is the diffusivity and f is a general source term 
    """
    _objectsDict = {  "_system" : "AdvectionDiffusionSLE",
                      "_solver" : "AdvDiffMulticorrector" }
    _selfObjectName = "_system"

    def __init__(self, phiField, phiDotField, velocityField, diffusivity, courantFactor=.25, conditions=[], **kwargs):
        """
        Constructor for the Advection Diffusion Equation system

        Parameters
        ----------
        phiField : MeshVariable
            The concentration field, typically the temperature field

        phiDotField : MeshVariable
            A MeshVariable that defines the initial time derivative of the phiField.
            Typically 0 at the beginning of a model, e.g. phiDotField.data[:]=0
            When using a phiField loaded from disk one should also load the phiDotField to ensure 
            the solving method has the time derivative information for a smooth restart. 
            No dirichlet conditions are required for this field as the phiField degrees of freedom
            map exactly to this field's dirichlet conditions, the value of which ought to be 0 
            for constant values of phi.

        velocityField : MeshVariable
            The velocity field

        diffusivity : float
            The definition of diffusivity. (Currently constant, will be updated to a function next release )

        courantFactor : float (default 0.25)

        conditions[] : SystemCondition, list, tuple
            Dirichlet boundary conditions to use for solving the advection diffusion system

        Returns
        -------
        AdvectionDiffusion :
            Constructed system for managing the Advection Diffusion Equation
            
        """
        self._diffusivity   = diffusivity
        self._courantFactor = courantFactor
        
        if not isinstance( phiField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'phiField' must be of 'MeshVariable' class." )
        if phiField.data.shape[1] != 1:
            raise TypeError( "Provided 'phiField' must be a scalar" )
        self._phiField = phiField

        if not isinstance( phiDotField, (uw.mesh.MeshVariable, type(None))):
            raise TypeError( "Provided 'phiDotField' must be 'None' or of 'MeshVariable' class." )
        if self._phiField.data.shape != phiDotField.data.shape:
            raise TypeError( "Provided 'phiDotField' is not the same shape as the provided 'phiField'" )
        self._phiDotField = phiDotField

        # check compatibility of phiField and velocityField
        if velocityField.data.shape[1] != self._phiField.mesh.dim:
            raise TypeError( "Provided 'velocityField' must be the same dimensionality as the phiField's mesh" )
        self._velocityField = velocityField

        if not isinstance(conditions, (uw.conditions._SystemCondition, list, tuple) ):
            raise ValueError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
        if len(conditions) > 1:
            raise ValueError( "Multiple conditions are not currently supported." )
        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise ValueError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here.. will rearrange this in future. 
            if cond.variable == self._phiField:
                libUnderworld.StgFEM.FeVariable_SetBC( self._phiField._cself, cond._cself )
                libUnderworld.StgFEM.FeVariable_SetBC( self._phiDotField._cself, cond._cself )

        # ok, we've set some bcs, lets recreate eqnumbering
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._phiField._cself )
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._phiDotField._cself )
        self._conditions = conditions

        # create force vectors
        self._residualVector = sle.AssembledVector(phiField)
        self._massVector     = sle.AssembledVector(phiField)

        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._phiField.mesh)

        super(AdvectionDiffusion, self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AdvectionDiffusion,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][   "SLE_Solver"] = self._solver.name
        componentDictionary[ self._cself.name ][     "PhiField"] = self._phiField._cself.name
        componentDictionary[ self._cself.name ][     "Residual"] = self._residualVector._cself.name
        componentDictionary[ self._cself.name ][   "MassMatrix"] = self._massVector._cself.name
        componentDictionary[ self._cself.name ][  "PhiDotField"] = self._phiDotField._cself.name
        componentDictionary[ self._cself.name ][          "dim"] = self._phiField.mesh.dim
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
