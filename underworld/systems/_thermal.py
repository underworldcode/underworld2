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
    This class provides functionality for a discrete representation
    of the steady state heat equation.

    The class uses a standard Galerkin finite element method to
    construct a system of linear equations which may then be solved
    using an object of the underworld.system.Solver class.

    The underlying element types are determined by the supporting
    mesh used for the 'temperatureField'.

    .. math::
         \nabla { ( k \nabla \phi } ) + h = 0

    where, k is the diffusivity, \phi is the temperature, h is
    a source term.

    Parameters
    ----------
    temperatureField : underworld.mesh.MeshVariable
        The solution field for temperature.
    fn_diffusivity : underworld.function.Function
        The function that defines the diffusivity across the domain.
    fn_heating : underworld.function.Function, default=0.
        The heating function that defines the heating across the domain.
    voronoi_swarm : uw.swarm.Swarm, optional.
        If a voronoi_swarm is provided, voronoi type integration is utilised to 
        integrate across elements. The provided swarm is used as the basis for
        the voronoi integration. If no swarm is provided, Gauss integration 
        is used.
    conditions : list, uw.conditions._SystemCondition, default = []
        Numerical conditions to impose on the system.
        uw.conditions.DirichletCondition : define scalar values of \phi
        uw.conditions.NeumannCondition :   define the vector (k \nabla \phi)

    Notes
    -----
    Constructor must be called collectively by all processes.

    Example
    -------
    Setup a basic thermal system:

    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> tField = uw.mesh.MeshVariable( linearMesh, 1 )
    >>> topNodes = linearMesh.specialSets["MaxJ_VertexSet"]
    >>> bottomNodes = linearMesh.specialSets["MinJ_VertexSet"]
    >>> tbcs = uw.conditions.DirichletCondition(tField, topNodes + bottomNodes)
    >>> tField.data[topNodes.data] = 0.0
    >>> tField.data[bottomNodes.data] = 1.0
    >>> tSystem = uw.systems.SteadyStateHeat(temperatureField=tField, fn_diffusivity=1.0, conditions=[tbcs])

    Example - of nonlinear diffusivity
    ----------------------------------
    >>> k = tField + 1.0
    >>> tSystem = uw.systems.SteadyStateHeat(temperatureField=tField, fn_diffusivity=k, conditions=[tbcs])
    >>> solver = uw.systems.Solver(tSystem)
    >>> solver.solve()
    Traceback (most recent call last):
    ...
    RuntimeError: Nonlinearity detected.
    Diffusivity function depends on the temperature field provided to the system.
    Please set the 'nonLinearIterate' solve parameter to 'True' or 'False' to continue.
    """

    _objectsDict = {  "_system" : "SystemLinearEquations" }
    _selfObjectName = "_system"

    def __init__(self, temperatureField, fn_diffusivity=None, fn_heating=0., voronoi_swarm=None, conditions=[], _removeBCs=True, swarm=None, **kwargs):
        # DEPRECATE. JM 09/16
        if swarm:
            uw._warnings.warn("'swarm' paramater has been renamed to 'voronoi_swarm'. Please update your models. "+
                          "'swarm' parameter will be removed in the next release.")
            if voronoi_swarm:
                raise ValueError("Please provide only a 'voronoi_swarm'. 'swarm' is deprecated.")
            voronoi_swarm = swarm

        if not isinstance( temperatureField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'temperatureField' must be of 'MeshVariable' class." )
        self._temperatureField = temperatureField

        if not fn_diffusivity:
            raise ValueError("You must specify a diffusivity function via the 'fn_diffusivity' parameter.")
        try:
            _fn_diffusivity = uw.function.Function.convert(fn_diffusivity)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_diffusivity' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        try:
            _fn_heating = uw.function.Function.convert(fn_heating)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_heating' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if voronoi_swarm and not isinstance(swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = voronoi_swarm
        if voronoi_swarm and temperatureField.mesh.elementType=='Q2':
            uw._warnings.warn("Voronoi integration may yield unsatisfactory results for Q2 element types.")

        if not isinstance(conditions, (uw.conditions._SystemCondition, list, tuple) ):
            raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
        if not isinstance( _removeBCs, bool):
            raise TypeError( "Provided '_removeBCs' must be of type bool." )
        self._removeBCs = _removeBCs

        # error check dcs 'dirichlet conditions' and ncs 'neumann cond.' FeMesh_IndexSets
        nbc  = None
        mesh = temperatureField.mesh

        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here
            if type( cond ) == uw.conditions.DirichletCondition:
                if cond.variable == temperatureField:
                    libUnderworld.StgFEM.FeVariable_SetBC( temperatureField._cself, cond._cself )
            elif type( cond ) == uw.conditions.NeumannCondition:
                nbc=cond
            else:
                raise RuntimeError("Can't decide on input condition")

        # build the equation numbering for the temperature field discretisation
        tEqNums = self._tEqNums = sle.EqNumber( temperatureField, removeBCs=self._removeBCs )

        # create solutions vector
        self._solutionVector = sle.SolutionVector(temperatureField, tEqNums )
        libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._solutionVector._cself )

        # create force vectors
        self._fvector = sle.AssembledVector(temperatureField, tEqNums )

        # and matrices
        self._kmatrix = sle.AssembledMatrix( self._solutionVector, self._solutionVector, rhs=self._fvector )

        # we will use voronoi if that has been requested by the user, else use
        # gauss integration.
        if self._swarm:
            intswarm = self._swarm._voronoi_swarm
            # need to ensure voronoi is populated now, as assembly terms will call
            # initial test functions which may require a valid voronoi swarm
            self._swarm._voronoi_swarm.repopulate()
        else:
            intswarm = uw.swarm.GaussIntegrationSwarm(mesh)


        self._kMatTerm = sle.MatrixAssemblyTerm_NA_i__NB_i__Fn(  integrationSwarm=intswarm,
                                                                 assembledObject=self._kmatrix,
                                                                 fn=_fn_diffusivity)
        self._forceVecTerm = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=intswarm,
                                                              assembledObject=self._fvector,
                                                              fn=fn_heating)
        # search for neumann conditions
        for cond in conditions:
            if isinstance( cond, uw.conditions.NeumannCondition ):
                #NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last
                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject  = self._fvector,
                                                                surfaceGaussPoints = 2,
                                                                nbc         = cond )
        super(SteadyStateHeat, self).__init__(**kwargs)

    def _setup(self):
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.stiffnessMatrices, self._kmatrix._cself )
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.forceVectors, self._fvector._cself )
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.solutionVectors, self._solutionVector._cself )

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SteadyStateHeat,self)._add_to_stg_dict(componentDictionary)

    @property
    def fn_diffusivity(self):
        """
        The diffusivity function. You may change this function directly via this
        property.
        """
        return self._kMatTerm.fn
    @fn_diffusivity.setter
    def fn_diffusivity(self, value):
        self._kMatTerm.fn = value

    @property
    def fn_heating(self):
        """
        The heating function. You may change this function directly via this
        property.
        """
        return self._forceVecTerm.fn
    @fn_heating.setter
    def fn_heating(self, value):
        self._forceVecTerm.fn = value
