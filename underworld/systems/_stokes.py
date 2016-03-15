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
    This class provides functionality for a discrete representation
    of the incompressible Stokes equation.

    Specifically, the class uses a mixed finite element method to
    construct a system of linear equations which may then be solved
    using an object of the underworld.system.Solver class.

    The underlying element types are determined by the supporting
    mesh used for the 'velocityField' and 'pressureField' parameters.

    Parameters
    ----------
    velocityField : underworld.mesh.MeshVariable
        Variable used to record system velocity.
    pressureField : underworld.mesh.MeshVariable
        Variable used to record system pressure.
    fn_viscosity : underworld.function.Function
        Function which reports a viscosity value.
        Function must return scalar float values.
    fn_bodyforce : underworld.function.Function, default=None.
        Function which reports a body force for the system.
        Function must return float values of identical dimensionality
        to the provided velocity variable.
    swarm : uw.swarm.Swarm, default=None.
        If a swarm is provided, PIC type integration is utilised to build
        up element integrals. The provided swarm is used as the basis for
        the PIC swarm.
        If no swarm is provided, Gauss style integration is used.
    conditions : list of uw.conditions.DirichletCondition objects, default=None
        Conditions to be placed on the system. Currently only
        Dirichlet conditions are supported.

    Notes
    -----
    Constructor must be called by collectively all processes.


    """
    _objectsDict = {  "_system" : "Stokes_SLE" }
    _selfObjectName = "_system"

    def __init__(self, velocityField, pressureField, fn_viscosity=None, fn_bodyforce=None, swarm=None, conditions=[], viscosityFn=None, bodyForceFn=None, rtolerance=None, **kwargs):
        # DEPRECATE 1/16
        if viscosityFn:
            raise RuntimeError("Note that the 'viscosityFn' parameter has been renamed to 'fn_viscosity'.")
        if bodyForceFn:
            raise RuntimeError("Note that the 'bodyForceFn' parameter has been renamed to 'fn_bodyforce'.")
        if rtolerance:
            raise RuntimeError("Note that the 'rtolerance' parameter has been removed.\n" \
                               "All solver functionality has been moved to underworld.systems.Solver.")



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

        if not fn_viscosity:
            raise ValueError("You must specify a viscosity function via the 'fn_viscosity' parameter.")
        _fn_viscosity = uw.function.Function._CheckIsFnOrConvertOrThrow(fn_viscosity)
        if not isinstance( _fn_viscosity, uw.function.Function):
            raise TypeError( "Provided 'fn_viscosity' must be of or convertible to 'Function' class." )

        if not fn_bodyforce:
            if velocityField.mesh.dim == 2:
                fn_bodyforce = (0.,0.)
            else:
                fn_bodyforce = (0.,0.,0.)
        _fn_bodyforce = uw.function.Function._CheckIsFnOrConvertOrThrow(fn_bodyforce)

        if swarm and not isinstance(swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = swarm

        if not isinstance(conditions, (uw.conditions._SystemCondition, list, tuple) ):
            raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
        # error check dcs 'dirichlet conditions' and ncs 'neumann cond.' FeMesh_IndexSets
        fluxCond = None
        mesh     = velocityField.mesh
        ncs      = uw.mesh.FeMesh_IndexSet( mesh, topologicalIndex=0, size=mesh.nodesGlobal )
        dcs      = uw.mesh.FeMesh_IndexSet( mesh, topologicalIndex=0, size=mesh.nodesGlobal )

        for cond in conditions:
            if not isinstance( cond, uw.conditions._SystemCondition ):
                raise TypeError( "Provided 'conditions' must be a list '_SystemCondition' objects." )
            # set the bcs on here
            if type( cond ) == uw.conditions.DirichletCondition:
                if cond.variable == velocityField:
                    libUnderworld.StgFEM.FeVariable_SetBC( velocityField._cself, cond._cself )
                elif cond.variable == pressureField:
                    libUnderworld.StgFEM.FeVariable_SetBC( pressureField._cself, cond._cself )
                else:
                    raise ValueError("Condition object does not appear to apply to the provided velocityField or pressureField.")
                for ii in cond.indexSets:
                    if ii:
                        # add all dirichlet condition to dcs
                        dcs.add( ii )
            elif type( cond ) == uw.conditions.NeumannCondition:
                for ii in cond.indexSets:
                    if ii:
                        ncs.add( ii )
                fluxCond=cond
            else:
                raise RuntimeError("Can't decide on input condition")

        # check if condition definitions occur on the same nodes: error conditions presently
        should_be_empty = dcs & ncs
        if should_be_empty.count > 0:
            raise ValueError("It appears both Neumann and Dirichlet conditions have been specified the following nodes\n" +
                    "This is untested and we have disabled it for now.", should_be_empty.data)

        # ok, we've set some bcs, lets recreate eqnumbering
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( pressureField._cself )
        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( velocityField._cself )
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
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(velocityField.mesh)
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
                                                            fn=_fn_viscosity)
        self._forceVecTerm   = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=swarmguy,
                                                                assembledObject=self._fvector,
                                                                fn=_fn_bodyforce)
                                                                       # prepare fluxConditions
        if fluxCond:
            ##### Build everything for the VectorSurfaceAssemblyTerm_NA__Fn__ni.
            # 1) a gauss border swarm
            # 2) a mask function to only evaluate the fn_flux only on the nodes specified in fluxCond.indexSets
            #####
            pWalls = mesh.specialSets["MaxI_VertexSet"] + mesh.specialSets["MinI_VertexSet"]
            alanBorderGaussSwarm = uw.swarm.GaussBorderIntegrationSwarm( mesh=mesh, particleCount=2 )
            deltaMeshVariable = uw.mesh.MeshVariable(mesh, 1)
            # set to 1 on provided vertices and 0 elsewhere
            deltaMeshVariable.data[:] = 0.
            deltaMeshVariable.data[pWalls.data] = 1.
            #deltaMeshVariable.data[fluxCond.indexSets[0].data] = 1.
            # note we use this condition to only capture border swarm particles
            # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
            # to non-zero (but less than 1.), so we need to remove those from the integration as well.
            maskFn = uw.function.branching.conditional(
                              [  ( deltaMeshVariable > 0.999, 1. ),
                                 (                      True, 0. )   ] )
            fluxCond._gradientField = maskFn * fluxCond.gradientField

            self._surfaceFluxTerm = sle.VectorSurfaceAssemblyTerm_NA__Fn__ni(
                                        integrationSwarm = alanBorderGaussSwarm,
                                        assembledObject  = self._fvector,
                                        fluxCond         = fluxCond )
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

    def solve(self, *args, **kwargs):
        """ deprecated method
        """
        raise RuntimeError("This method is now deprecated. You now need to explicitly\n"\
                           "create a solver, and then solve it:\n\n"\
                           "    solver = uw.system.Solver(stokesSystemObject)\n"\
                           "    solver.solve() \n\n"\
                           "but note that you only need to create the solver once.")

    @property
    def fn_viscosity(self):
        """
        The viscosity function. You may change this function directly via this
        property.
        """
        return self._constitMatTerm.fn
    @fn_viscosity.setter
    def fn_viscosity(self, value):
        self._constitMatTerm.fn = value

    @property
    def fn_bodyforce(self):
        """
        The body force function. You may change this function directly via this
        property.
        """
        return self._forceVecTerm.fn
    @fn_bodyforce.setter
    def fn_bodyforce(self, value):
        self._forceVecTerm.fn = value
