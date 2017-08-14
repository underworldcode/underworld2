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
    This class provides functionality for a discrete representation
    of an advection-diffusion equation.

    The class uses the Streamline Upwind Petrov Galerkin SUPG method
    to integrate through time.

    .. math::
        \\frac{\\partial\\phi}{\\partial t}  + {\\bf u } \\cdot \\nabla \\phi= \\nabla { ( k  \\nabla \\phi ) } + H


    Parameters
    ----------
    phiField : underworld.mesh.MeshVariable
        The concentration field, typically the temperature field
    phiDotField : underworld.mesh.MeshVariable
        A MeshVariable that defines the initial time derivative of the phiField.
        Typically 0 at the beginning of a model, e.g. phiDotField.data[:]=0
        When using a phiField loaded from disk one should also load the phiDotField to ensure
        the solving method has the time derivative information for a smooth restart.
        No dirichlet conditions are required for this field as the phiField degrees of freedom
        map exactly to this field's dirichlet conditions, the value of which ought to be 0
        for constant values of phi.
    velocityField : underworld.mesh.MeshVariable
        The velocity field.
    fn_diffusivity : underworld.function.Function
        A function that defines the diffusivity within the domain.
    fn_sourceTerm : underworld.function.Function
        A function that defines the heating within the domain. Optional.
    conditions : underworld.conditions.SystemCondition
        Numerical conditions to impose on the system. This should be supplied as
        the condition itself, or a list object containing the conditions.

    Notes
    -----
    Constructor must be called by collectively all processes.

    """
    _objectsDict = {  "_system" : "AdvectionDiffusionSLE",
                      "_solver" : "AdvDiffMulticorrector" }
    _selfObjectName = "_system"

    def __init__(self, phiField, phiDotField, velocityField, fn_diffusivity, fn_sourceTerm=None, conditions=[], _allow_non_q1=False, **kwargs):

        self._diffusivity   = fn_diffusivity
        self._source        = fn_sourceTerm

        if not isinstance( phiField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'phiField' must be of 'MeshVariable' class." )
        if phiField.data.shape[1] != 1:
            raise TypeError( "Provided 'phiField' must be a scalar" )
        self._phiField = phiField
        if self._phiField.mesh.elementType != 'Q1':
            import warnings
            warnings.warn("The 'phiField' is discretised on a {} mesh. This 'uw.system.AdvectionDiffusion' implementation is ".format(self._phiField.mesh.elementType)+
                          "only stable for a phiField discretised with a Q1 mesh. Either create a Q1 mesh for the 'phiField' or, if you know "+
                          "what you're doing, override this error with the argument '_allow_non_q1=True' in the constructor", category=RuntimeWarning)
            if _allow_non_q1 == False:
                raise ValueError( "Error as provided 'phiField' discretisation is unstable")

        if not isinstance( phiDotField, (uw.mesh.MeshVariable, type(None))):
            raise TypeError( "Provided 'phiDotField' must be 'None' or of 'MeshVariable' class." )
        if self._phiField.data.shape != phiDotField.data.shape:
            raise TypeError( "Provided 'phiDotField' is not the same shape as the provided 'phiField'" )
        self._phiDotField = phiDotField

        # check compatibility of phiField and velocityField
        if velocityField.data.shape[1] != self._phiField.mesh.dim:
            raise TypeError( "Provided 'velocityField' must be the same dimensionality as the phiField's mesh" )
        self._velocityField = velocityField

        if not isinstance(conditions,(list,tuple)):
            conditionslist = []
            conditionslist.append(conditions)
            conditions = conditionslist

        # check input 'conditions' list is valid
        nbc = None
        for cond in conditions:
            if not isinstance( cond, uw.conditions.SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list 'SystemCondition' objects." )
            # set the bcs on here
            if type(cond) == uw.conditions.NeumannCondition:
                if nbc != None:
                    # check only one nbc condition is given in 'conditions' list
                    RuntimeError( "Provided 'conditions' can only accept one NeumannConditions condition object.")
            elif type(cond) == uw.conditions.DirichletCondition:
                if cond.variable == self._phiField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._phiField._cself, cond._cself )
                    libUnderworld.StgFEM.FeVariable_SetBC( self._phiDotField._cself, cond._cself )
            else:
                raise RuntimeError("Input condition type not recognised.")
        self._conditions = conditions

        # force removal of BCs as SUPG cannot handle leaving them in
        self._eqNumPhi    = sle.EqNumber( phiField, removeBCs=True )
        self._eqNumPhiDot = sle.EqNumber( phiDotField, removeBCs=True )

        self._phiSolution    = sle.SolutionVector( phiField, self._eqNumPhi )
        self._phiDotSolution = sle.SolutionVector( phiDotField, self._eqNumPhiDot )

        # create force vectors
        self._residualVector = sle.AssembledVector(phiField, self._eqNumPhi )
        self._massVector     = sle.AssembledVector(phiField, self._eqNumPhi )

        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._phiField.mesh)

        super(AdvectionDiffusion, self).__init__(**kwargs)

        self._cself.phiVector = self._phiSolution._cself
        self._cself.phiDotVector = self._phiDotSolution._cself


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AdvectionDiffusion,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][   "SLE_Solver"] = self._solver.name
        componentDictionary[ self._cself.name ][     "PhiField"] = self._phiField._cself.name
        componentDictionary[ self._cself.name ][     "Residual"] = self._residualVector._cself.name
        componentDictionary[ self._cself.name ][   "MassMatrix"] = self._massVector._cself.name
        componentDictionary[ self._cself.name ][  "PhiDotField"] = self._phiDotField._cself.name
        componentDictionary[ self._cself.name ][          "dim"] = self._phiField.mesh.dim
        componentDictionary[ self._cself.name ]["courantFactor"] = 0.50

    def _setup(self):
        # create assembly terms here.
        # in particular, the residualTerm requires and tries to build _system, so if created in __init__
        # this causes a conflict.
        self._lumpedMassTerm = sle.LumpedMassMatrixVectorTerm( integrationSwarm = self._gaussSwarm,
                                                                assembledObject = self._massVector  )
        self._residualTerm   = sle.AdvDiffResidualVectorTerm(     velocityField = self._velocityField,
                                                                    diffusivity = self._diffusivity,
                                                                     sourceTerm = self._source,
                                                               integrationSwarm = self._gaussSwarm,
                                                                assembledObject = self._residualVector,
                                                                      extraInfo = self._cself.name )
        for cond in self._conditions:
            if isinstance( cond, uw.conditions.NeumannCondition ):

                ### -VE flux because of the FEM discretisation method of the initial equation
                negativeCond = uw.conditions.NeumannCondition( fn_flux=-1.0*cond.fn_flux,
                                                               variable=cond.variable,
                                                               indexSetsPerDof=cond.indexSetsPerDof )

                #NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject  = self._residualVector,
                                                                surfaceGaussPoints = 2,
                                                                nbc         = negativeCond )

        self._cself.advDiffResidualForceTerm = self._residualTerm._cself


    def integrate(self,dt):
        """
        Integrates the advection diffusion system through time, dt
        Must be called collectively by all processes.

        Parameters
        ----------
        dt : float
            The timestep interval to use

        """
        self._cself.currentDt = dt
        libUnderworld.Underworld._AdvectionDiffusionSLE_Execute( self._cself, None )

    def get_max_dt(self):
        """
        Returns a numerically stable timestep size for the current system.
        Note that as a default, this method returns a value one quarter the
        size of the Courant timestep.

        Returns
        -------
        float
            The timestep size.
        """
        return libUnderworld.Underworld.AdvectionDiffusionSLE_CalculateDt( self._cself, None )
