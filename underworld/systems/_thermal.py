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

    The strong form of the given boundary value problem, for :math:`f`,
    :math:`q` and :math:`h` given, is
    
    .. math::
        \\begin{align}
        q_i =& - \\kappa \\, u_{,i}  & \\\\
        q_{i,i} =& \\: f  & \\text{ in }  \\Omega \\\\
        u =& \\: q & \\text{ on }  \\Gamma_q \\\\
        -q_i n_i =& \\: h & \\text{ on }  \\Gamma_h \\\\
        \\end{align}

    where, :math:`\\kappa` is the diffusivity, :math:`u` is the temperature,
    :math:`f` is a source term, :math:`q` is the Dirichlet condition, and
    :math:`h` is a Neumann condition. The problem boundary, :math:`\\Gamma`, 
    admits the decomposition :math:`\\Gamma=\\Gamma_q\\cup\\Gamma_h` where
    :math:`\\emptyset=\\Gamma_q\\cap\\Gamma_h`. The equivalent weak form is:

    .. math::
        -\\int_{\\Omega} w_{,i} \\, q_i \\, d \\Omega = \\int_{\\Omega} w \\, f \\, d\\Omega + \\int_{\\Gamma_h} w \\, h \\,  d \\Gamma
    
    where we must find :math:`u` which satisfies the above for all :math:`w` 
    in some variational space.

    Parameters
    ----------
    temperatureField : underworld.mesh.MeshVariable
        The solution field for temperature.
    fn_diffusivity : underworld.function.Function
        The function that defines the diffusivity across the domain.
    fn_heating : underworld.function.Function
        A function that defines the heating across the domain. Optional.
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

    Example with non diffusivity:
    
    >>> k = tField + 1.0
    >>> tSystem = uw.systems.SteadyStateHeat(temperatureField=tField, fn_diffusivity=k, conditions=[tbcs])
    >>> solver = uw.systems.Solver(tSystem)
    >>> solver.solve()
    Traceback (most recent call last):
    ...
    RuntimeError: Nonlinearity detected.
    Diffusivity function depends on the temperature field provided to the system.
    Please set the 'nonLinearIterate' solve parameter to 'True' or 'False' to continue.
    >>> solver.solve(nonLinearIterate=True)
    
    """

    _objectsDict = {  "_system" : "SystemLinearEquations" }
    _selfObjectName = "_system"

    def __init__(self, temperatureField, fn_diffusivity, fn_heating=0., voronoi_swarm=None, conditions=[], _removeBCs=True, **kwargs):

        if not isinstance( temperatureField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'temperatureField' must be of 'MeshVariable' class." )
        self._temperatureField = temperatureField

        try:
            _fn_diffusivity = uw.function.Function.convert(fn_diffusivity)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_diffusivity' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        try:
            _fn_heating = uw.function.Function.convert(fn_heating)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_heating' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if voronoi_swarm and not isinstance(voronoi_swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = voronoi_swarm
        if voronoi_swarm and temperatureField.mesh.elementType=='Q2':
            import warnings
            warnings.warn("Voronoi integration may yield unsatisfactory results for Q2 element types.")

        if not isinstance( _removeBCs, bool):
            raise TypeError( "Provided '_removeBCs' must be of type bool." )
        self._removeBCs = _removeBCs

        # error check dcs 'dirichlet conditions' and ncs 'neumann cond.' FeMesh_IndexSets
        nbc  = None
        mesh = temperatureField.mesh

        if not isinstance(conditions,(list,tuple)):
            conditionslist = []
            conditionslist.append(conditions)
            conditions = conditionslist
        for cond in conditions:
            if not isinstance( cond, uw.conditions.SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list 'SystemCondition' objects." )
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

                ### -VE flux because of the FEM discretisation method of the initial equation
                negativeCond = uw.conditions.NeumannCondition( fn_flux=-1.0*cond.fn_flux,
                                                               variable=cond.variable,
                                                               nodeIndexSet=cond.indexSet )

                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject  = self._fvector,
                                                                surfaceGaussPoints = 2,
                                                                nbc         = negativeCond )
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
