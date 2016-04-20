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
        \frac{\partial\phi}{\partial t}  + {\bf u } \cdot \nabla \phi= \nabla { ( k  \nabla \phi } )


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
    fn_diffusivity : uw.function.Function
        Function that defines diffusivity
    conditions : list, uw.conditions._SystemCondition, default = []
        Numerical conditions to impose on the system.
        uw.conditions.DirichletCondition : define scalar values of \phi
        uw.conditions.NeumannCondition :   define the vector (k \nabla \phi)

    Notes
    -----
    Constructor must be called by collectively all processes.

    """
    _objectsDict = {  "_system" : "AdvectionDiffusionSLE",
                      "_solver" : "AdvDiffMulticorrector" }
    _selfObjectName = "_system"

    def __init__(self, phiField, phiDotField, velocityField, fn_diffusivity, fn_sourceTerm=None, courantFactor=None, diffusivity=None, conditions=[], **kwargs):
        if courantFactor != None:
            raise RuntimeError("Note that the 'courantFactor' parameter has been deprecated.\n"\
                               "If you wish to modify your timestep, do so manually with the value\n"\
                               "returned from the get_max_dt() method.")
        if diffusivity != None:
            raise RuntimeError("Note that the 'diffusivity' parameter has been deprecated.\n"\
                               "Use the parameter 'fn_diffusivity' instead.")

        self._diffusivity   = fn_diffusivity
        self._source        = fn_sourceTerm
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

        # error check dcs 'dirichlet conditions' and ncs 'neumann cond.'
        # currently we don't support them happening on the same dof.
        nbc = None
        mesh     = phiField.mesh
        ncs      = uw.mesh.FeMesh_IndexSet( mesh, topologicalIndex=0, size=mesh.nodesGlobal )
        dcs      = uw.mesh.FeMesh_IndexSet( mesh, topologicalIndex=0, size=mesh.nodesGlobal )
        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here
            if type( cond ) == uw.conditions.DirichletCondition:
                if cond.variable == phiField:
                    libUnderworld.StgFEM.FeVariable_SetBC( self._phiField._cself, cond._cself )
                    libUnderworld.StgFEM.FeVariable_SetBC( self._phiDotField._cself, cond._cself )
                # add all dirichlet condition to dcs
                dcs.add( cond.indexSets[0] )
            elif type( cond ) == uw.conditions.NeumannCondition:
                ncs.add( cond.indexSets[0] )
                nbc=cond
            else:
                raise RuntimeError("Can't decide on input condition")

        # check if condition definitions occur on the same nodes
        should_be_empty = dcs & ncs
        if should_be_empty.count > 0:
            raise ValueError("It appears both Neumann and Dirichlet conditions have been specified the following node degrees of freedom\n" +
                    "This is currently unsupported.", should_be_empty.data)

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
                #NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject  = self._residualVector,
                                                                surfaceGaussPoints = 2,
                                                                nbc         = cond )

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
