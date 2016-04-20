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

    """
    _objectsDict = {  "_system" : "SystemLinearEquations" }
    _selfObjectName = "_system"

    def __init__(self, temperatureField, fn_diffusivity=None, fn_heating=0., swarm=None, conditions=[], conductivityFn=None, heatingFn=None, rtolerance=None, **kwargs):
        if conductivityFn != None:
            raise RuntimeError("Note that the 'conductivityFn' parameter has been renamed to 'fn_diffusivity'.")
        if heatingFn != None:
            raise RuntimeError("Note that the 'heatingFn' parameter has been renamed to 'fn_heating'.")
        if rtolerance != None:
            raise RuntimeError("Note that the 'rtolerance' parameter has been removed.\n" \
                               "All solver functionality has been moved to underworld.systems.Solver.")

        if not isinstance( temperatureField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'temperatureField' must be of 'MeshVariable' class." )
        self._temperatureField = temperatureField

        if not fn_diffusivity:
            raise ValueError("You must specify a diffusivity function via the 'fn_diffusivity' parameter.")
        try:
            _fn_diffusivity = uw.function.Function._CheckIsFnOrConvertOrThrow(fn_diffusivity)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_diffusivity' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        try:
            _fn_heating = uw.function.Function._CheckIsFnOrConvertOrThrow(fn_heating)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn_heating' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if swarm and not isinstance(swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = swarm

        if not isinstance(conditions, (uw.conditions._SystemCondition, list, tuple) ):
            raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )

        # error check dcs 'dirichlet conditions' and ncs 'neumann cond.' FeMesh_IndexSets
        nbc = None
        mesh     = temperatureField.mesh
        ncs      = uw.mesh.FeMesh_IndexSet( mesh, topologicalIndex=0, size=mesh.nodesGlobal )
        dcs      = uw.mesh.FeMesh_IndexSet( mesh, topologicalIndex=0, size=mesh.nodesGlobal )

        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here
            if type( cond ) == uw.conditions.DirichletCondition:
                if cond.variable == temperatureField:
                    libUnderworld.StgFEM.FeVariable_SetBC( temperatureField._cself, cond._cself )
                # add all dirichlet condition to dcs
                dcs.add( cond.indexSets[0] )
            elif type( cond ) == uw.conditions.NeumannCondition:
                ncs.add( cond.indexSets[0] )
                nbc=cond
            else:
                raise RuntimeError("Can't decide on input condition")

        # check if condition definitions occur on the same nodes: error conditions presently
        should_be_empty = dcs & ncs
        if should_be_empty.count > 0:
            raise ValueError("It appears both Neumann and Dirichlet conditions have been specified the following node degrees of freedom\n" +
                    "This is currently unsupported.", should_be_empty.data)

        # ok, we've set some bcs, lets recreate eqnumbering
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( temperatureField._cself )

        # create solutions vector
        self._solutionVector = sle.SolutionVector(temperatureField)

        # create force vectors
        self._fvector = sle.AssembledVector(temperatureField)

        # and matrices
        self._kmatrix = sle.AssembledMatrix( temperatureField, temperatureField, rhs=self._fvector )

        # initialise swarms
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(mesh)
        self._PICSwarm = None
        intswarm = self._gaussSwarm

        # if user provided a swarm we create a PIC swarm that maps to it
        if self._swarm:
            self._PICSwarm = uw.swarm.PICIntegrationSwarm(self._swarm)
            intswarm = self._PICSwarm

        self._kMatTerm = sle.MatrixAssemblyTerm_NA_i__NB_i__Fn(  integrationSwarm=intswarm,
                                                                 assembledObject=self._kmatrix,
                                                                 fn=_fn_diffusivity)
        self._forceVecTerm = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=intswarm,
                                                              assembledObject=self._fvector,
                                                              fn=fn_heating)
        # prepare neumann conditions
        if nbc:
            self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                                                assembledObject = self._fvector,
                                                                surfaceGaussPoints = 2,
                                                                nbc = nbc )

        super(SteadyStateHeat, self).__init__(**kwargs)

    def _setup(self):
#        uw.libUnderworld.StgFEM.SystemLinearEquations_AddStiffnessMatrix( self._cself, self._kmatrix._cself );
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.stiffnessMatrices, self._kmatrix._cself )

#        uw.libUnderworld.StgFEM.SystemLinearEquations_AddForceVector( self._cself, self._fvector._cself );
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.forceVectors, self._fvector._cself )

#        uw.libUnderworld.StgFEM.SystemLinearEquations_AddSolutionVector( self._cself, self._solutionVector._cself );
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.solutionVectors, self._solutionVector._cself )



    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SteadyStateHeat,self)._add_to_stg_dict(componentDictionary)

    def solve(self, *args, **kwargs):
        """ deprecated method
        """
        raise RuntimeError("This method is now deprecated. You now need to explicitly\n"\
                           "create a solver, and then solve it:\n\n"\
                           "    solver = uw.system.Solver(heatSystemObject)\n"\
                           "    solver.solve() \n\n"\
                           "but note that you only need to create the solver once.")

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
