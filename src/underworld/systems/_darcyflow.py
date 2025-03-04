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
from . import sle
import underworld.libUnderworld as libUnderworld
import numpy

class SteadyStateDarcyFlow(_stgermain.StgCompoundComponent):
    """
    This class provides functionality for a discrete representation
    of the steady state darcy flow equation.

    The class uses a standard Galerkin finite element method to
    construct a system of linear equations which may then be solved
    using an object of the underworld.system.Solver class.

    The underlying element types are determined by the supporting
    mesh used for the 'pressureField'.

    The strong form of the given boundary value problem, for :math:`f`,
    :math:`q` and :math:`h` given, is

    .. math::
        \\begin{align}
        q_i =& \\kappa \\, ( -u_{,i} + S_i )   &  \\\\
        q_{i,i} =& \\: f  & \\text{ in }  \\Omega \\\\
        u =& \\: q & \\text{ on }  \\Gamma_q \\\\
        -q_i n_i =& \\: h & \\text{ on }  \\Gamma_h \\\\
        \\end{align}

    where,

    * :math:`\\kappa` is the diffusivity, :math:`u` is the pressure,
    * :math:`S` is a flow body-source, for example due to gravity,
    * :math:`f` is a source term, :math:`q` is the Dirichlet condition, and
    * :math:`h` is a Neumann condition.

    The problem boundary, :math:`\\Gamma`, admits the decomposition
    :math:`\\Gamma=\\Gamma_q\\cup\\Gamma_h` where
    :math:`\\emptyset=\\Gamma_q\\cap\\Gamma_h`. The equivalent weak form is:

    .. math::
        -\\int_{\\Omega} w_{,i} \\, q_i \\, d \\Omega = \\int_{\\Omega} w \\, f \\, d\\Omega + \\int_{\\Gamma_h} w \\, h \\,  d \\Gamma

    where we must find :math:`u` which satisfies the above for all :math:`w`
    in some variational space.

    Parameters
    ----------
    pressureField : underworld.mesh.MeshVariable
        The solution field for pressure.
    fn_diffusivity : underworld.function.Function
        The function that defines the diffusivity across the domain.
    fn_bodyforce : underworld.function.Function
        A function that defines the flow body-force across the domain,
        for example gravity. Must be a vector. Optional.
    voronoi_swarm : underworld.swarm.Swarm
        A swarm with just one particle within each cell should be provided.
        This avoids the evaluation of the velocity on nodes and inaccuracies
        arising from diffusivity changes within cells.
        If a swarm is provided, voronoi type numerical integration is
        utilised. The provided swarm is used as the basis for the voronoi
        integration. If no voronoi_swarm is provided, Gauss integration
        is used.
    conditions : underworld.conditions.SystemCondition
        Numerical conditions to impose on the system. This should be supplied as
        the condition itself, or a list object containing the conditions.
    gauss_swarm : underworld.swarm.GaussIntegrationSwarm
        If provided this gauss_swarm will be used for (gaussian) numerical 
        integration rather than a default gauss integration swarm that is 
        automatically generated and dependent on the element order of the mesh. 
        NB: if a voronoi_swarm is defined it OVERRIDES this gauss_swarm as the
        preferred integration swarm (quadrature method).
    velocityField : underworld.mesh.MeshVariable
        Solution field for darcy flow velocity. Optional.
    swarmVarVelocity : undeworld.swarm.SwarmVariable
        If a swarm variable is provided, the velocity calculated on the swarm will be stored.
        This is the most representative velocity data object, as the velocity calculation occurs
        on the swarm, away from mesh nodes. Optional.

    Notes
    -----
    Constructor must be called collectively by all processes.

    """

    _objectsDict = {  "_system" : "SystemLinearEquations" }
    _selfObjectName = "_system"

    def __init__(self, pressureField, fn_diffusivity, fn_bodyforce=None, voronoi_swarm=None, conditions=[], velocityField=None, swarmVarVelocity=None, _removeBCs=True, gauss_swarm=None, **kwargs):

        if not isinstance( pressureField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'pressureField' must be of 'MeshVariable' class." )
        self._pressureField = pressureField

        try:
            _fn_diffusivity = uw.function.Function.convert(fn_diffusivity)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_diffusivity' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if not fn_bodyforce:
            if pressureField.mesh.dim == 2:
                fn_bodyforce = (0.,0.)
            else:
                fn_bodyforce = (0.,0.,0.)
        try:
            _fn_bodyforce = uw.function.Function.convert(fn_bodyforce)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_bodyforce' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if voronoi_swarm and not isinstance(voronoi_swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = voronoi_swarm
        if len(numpy.unique(voronoi_swarm.owningCell.data[:])) != len(voronoi_swarm.owningCell.data[:]):
            import warnings
            warnings.warn("It is not advised to fill any cell with more than one particle, as the Q1 shape function cannot capture material interfaces. Use at your own risk.")

        if velocityField and not isinstance( velocityField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        self._velocityField = velocityField

        if swarmVarVelocity and not isinstance(swarmVarVelocity, uw.swarm.SwarmVariable):
            raise TypeError("Provided 'swarmVarVelocity' must be of 'SwarmVariable' class.")
        self._swarmVarVelocity = swarmVarVelocity

        if voronoi_swarm and pressureField.mesh.elementType=='Q2':
            import warnings
            warnings.warn("Voronoi integration may yield unsatisfactory results for Q2 element types. Q2 element types can also result in spurious velocity calculations.")

        if not isinstance( _removeBCs, bool):
            raise TypeError( "Provided '_removeBCs' must be of type bool." )
        self._removeBCs = _removeBCs

        # error check dcs 'dirichlet conditions' and ncs 'neumann cond.' FeMesh_IndexSets
        nbc  = None
        mesh = pressureField.mesh

        if not isinstance(conditions,(list,tuple)):
            conditionslist = []
            conditionslist.append(conditions)
            conditions = conditionslist
        for cond in conditions:
            if not isinstance( cond, uw.conditions.SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list 'SystemCondition' objects." )
            # set the bcs on here
            if type( cond ) == uw.conditions.DirichletCondition:
                if cond.variable == pressureField:
                    libUnderworld.StgFEM.FeVariable_SetBC( pressureField._cself, cond._cself )
            elif type( cond ) == uw.conditions.NeumannCondition:
                nbc=cond
            else:
                raise RuntimeError("Can't decide on input condition")

        # setup the gauss integration swarm
        if gauss_swarm != None:
            if type(gauss_swarm) != uw.swarm.GaussIntegrationSwarm:
                raise RuntimeError( "Provided 'gauss_swarm' must be a GaussIntegrationSwarm object" )
            intswarm = gauss_swarm
        else:
            intswarm = uw.swarm.GaussIntegrationSwarm(mesh)

        # we will use voronoi if that has been requested by the user, else use
        # gauss integration.
        if self._swarm:
            intswarm = self._swarm._voronoi_swarm
            # need to ensure voronoi is populated now, as assembly terms will call
            # initial test functions which may require a valid voronoi swarm
            self._swarm._voronoi_swarm.repopulate()

        # build the equation numbering for the temperature field discretisation
        tEqNums = self._tEqNums = sle.EqNumber( pressureField, removeBCs=self._removeBCs )

        # create solutions vector
        self._solutionVector = sle.SolutionVector(pressureField, tEqNums )
        libUnderworld.StgFEM.SolutionVector_LoadCurrentFeVariableValuesOntoVector( self._solutionVector._cself )

        # create force vectors
        self._fvector = sle.AssembledVector(pressureField, tEqNums )

        # and matrices
        self._kmatrix = sle.AssembledMatrix( self._solutionVector, self._solutionVector, rhs=self._fvector )

        self._kMatTerm = sle.MatrixAssemblyTerm_NA_i__NB_i__Fn(  integrationSwarm=intswarm,
                                                                 assembledObject=self._kmatrix,
                                                                 fn=_fn_diffusivity)

        self._forceVecTerm = sle.VectorAssemblyTerm_NA_i__Fn_i(   integrationSwarm=intswarm,
                                                              assembledObject=self._fvector,
                                                              fn=fn_bodyforce*fn_diffusivity)

        # search for neumann conditions
        for cond in conditions:
            if isinstance( cond, uw.conditions.NeumannCondition ):
                #NOTE many NeumannConditions can be used but the _sufaceFluxTerm only records the last

                ### -VE flux because of Energy_SLE_Solver ###
                negativeCond = uw.conditions.NeumannCondition( fn_flux=-1.0*cond.fn_flux,
                                                               variable=cond.variable,
                                                               indexSetsPerDof=cond.indexSet )

                self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject  = self._fvector,
                                                                surfaceGaussPoints = 2,
                                                                nbc         = negativeCond )
        super(SteadyStateDarcyFlow, self).__init__(**kwargs)

    def _setup(self):
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.stiffnessMatrices, self._kmatrix._cself )
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.forceVectors, self._fvector._cself )
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.solutionVectors, self._solutionVector._cself )

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SteadyStateDarcyFlow,self)._add_to_stg_dict(componentDictionary)

    def solve_velocityField(self):
        fnVel = (-self._pressureField.fn_gradient + self.fn_bodyforce) * self._kMatTerm.fn

        if self._swarmVarVelocity:
            self._swarmVarVelocity.data[:] = fnVel.evaluate(self._swarm)

        if self._velocityField:
            velproj = uw.utils.MeshVariable_Projection(self._velocityField,fnVel,self._swarm)
            velproj.solve()


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
    def fn_bodyforce(self):
        """
        The heating function. You may change this function directly via this
        property.
        """
        return self._forceVecTerm.fn / self._kMatTerm.fn

    @fn_bodyforce.setter
    def fn_bodyforce(self, value):
        self._forceVecTerm.fn = value * self._kMatTerm.fn
