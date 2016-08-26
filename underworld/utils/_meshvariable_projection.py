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
import underworld.systems.sle as sle
import libUnderworld

class MeshVariable_Projection(_stgermain.StgCompoundComponent):
    """
    This class provides functionality for projecting data
    from any underworld function onto a provided mesh variable.

    For the variable :math:`\bf{U} =  \bf{u}_a N_a` and function :math:`F`,
    we wish to determine appropriate values for :math:`\bf{u}_a` such
    that :math:`\bf{U} \simeq F`.

    Two projection methods are supported; weighted averages and weighted
    residuals. Generally speaking, weighted averages provide robust low
    order results, while weighted residuals give higher accuracy but
    spurious results for _difficult_ functions :math:`F`.

    The weighted average method is defined as:
    .. math::
         \bf{u}_a = \frac{\int_{\Omega} \bf{F} N_a \partial\Omega }{\int_{\Omega} N_a \partial\Omega }

    The weighted residual method constructs an SLE which is then solved to
    determine the unknowns:
    .. math::
         \bf{u}_a\int_{\Omega} N_a N_b \partial\Omega = \int_{\Omega} \bf{F} N_b \partial\Omega

    Parameters
    ----------
    meshVariable : underworld.mesh.MeshVariable
        The variable you wish to project the function onto.
    fn : underworld.function.Function
        The function you wish to project.
    swarm : underworld.swarm.Swarm, default=None.
        For voronoi integration, provided the swarm to be used. Otherwise
        Gauss integration will be performed.
    type : int, default=0
        Projection type.  0:'weighted average', 1:'weighted residual'

    Notes
    -----
    Constructor must be called collectively by all processes.

    Examples
    --------
    >>> import underworld as uw
    >>> import numpy as np
    >>> mesh = uw.mesh.FeMesh_Cartesian()
    >>> U = uw.mesh.MeshVariable( mesh, 1 )

    Lets cast a constant value onto this mesh variable

    >>> const = 1.23456
    >>> projector = uw.utils.MeshVariable_Projection( U, const, type=0 )
    >>> np.allclose(U.data, const)
    False
    >>> projector.solve()
    >>> np.allclose(U.data, const)
    True

    Now cast mesh coordinates onto a vector variable

    >>> U_coord = uw.mesh.MeshVariable( mesh, 2 )
    >>> projector = uw.utils.MeshVariable_Projection( U_coord, uw.function.coord(), type=1 )
    >>> projector.solve()
    >>> np.allclose(U_coord.data, mesh.data)
    True

    Project one mesh variable onto another

    >>> U_copy = uw.mesh.MeshVariable( mesh, 2 )
    >>> projector = uw.utils.MeshVariable_Projection( U_copy, U_coord, type=1 )
    >>> projector.solve()
    >>> np.allclose(U_copy.data, U_coord.data)
    True

    Project the coords to the submesh (usually the constant mesh)

    >>> U_submesh = uw.mesh.MeshVariable( mesh.subMesh, 2 )
    >>> projector = uw.utils.MeshVariable_Projection( U_submesh, U_coord, type=1 )
    >>> projector.solve()
    >>> np.allclose(U_submesh.data, mesh.subMesh.data)
    True


    """
    _objectsDict = {  "_system" : "SystemLinearEquations" }
    _selfObjectName = "_system"

    def __init__(self, meshVariable=None, fn=None, swarm=None, type=0, **kwargs):

        if not meshVariable:
            raise ValueError("You must specify a mesh variable via the 'meshVariable' parameter.")
        if not isinstance( meshVariable, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'meshVariable' must be of 'MeshVariable' class." )
        self._meshVariable = meshVariable

        if not fn:
            raise ValueError("You must specify a function via the 'fn' parameter.")
        try:
            _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(fn)
        except Exception as e:
            raise uw._prepend_message_to_exception(e, "Exception encountered. Note that provided 'fn' must be of or convertible to 'Function' class.\nEncountered exception message:\n")

        if swarm and not isinstance(swarm, uw.swarm.Swarm):
            raise TypeError( "Provided 'swarm' must be of 'Swarm' class." )
        self._swarm = swarm

        if not type in [0,1]:
            raise ValueError( "Provided 'type' must take a value of 0 (weighted average) or 1 (weighted residual)." )
        self.type = type

        eqNum = sle.EqNumber(meshVariable)
        # create force vectors
        self._fvector = sle.AssembledVector(meshVariable, eqNum)


        # we will use voronoi if that has been requested by the user, else use
        # gauss integration.
        if self._swarm:
            intswarm = self._swarm._voronoi_swarm
            # need to ensure voronoi is populated now, as assembly terms will call
            # initial test functions which may require a valid voronoi swarm
            self._swarm._voronoi_swarm.repopulate()
        else:
            intswarm = uw.swarm.GaussIntegrationSwarm(self._meshVariable.mesh)

        # determine the required geometry mesh.  for submesh, use the parent mesh.
        geometryMesh = self._meshVariable.mesh
        if hasattr(self._meshVariable.mesh.generator, 'geometryMesh'):
            geometryMesh = self._meshVariable.mesh.generator.geometryMesh

        self._fn = _fn

        self._forceVecTerm = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=intswarm,
                                                              assembledObject=self._fvector,
                                                              fn=_fn,
                                                              mesh=geometryMesh )

        if self.type == 0:
            # create copy guy
            self._copyMeshVariable = meshVariable.copy()
            # create unity array of required dimensionality
            self._unityArray = []
            for ii in range(self._meshVariable.nodeDofCount):
                self._unityArray.append(1.)
            self.solve = self._solve_average
        else:
            # create solutions vector
            self._solutionVector = sle.SolutionVector(meshVariable, eqNum)
            # and matrices
            self._kmatrix = sle.AssembledMatrix( self._solutionVector, self._solutionVector, rhs=self._fvector )
            # matrix term
            self._kMatTerm = sle.MatrixAssemblyTerm_NA__NB__Fn(  integrationSwarm=intswarm,
                                                                 assembledObject=self._kmatrix,
                                                                 fn = 1.0,
                                                                 mesh=geometryMesh )
            self._solver = None
            self.solve = self._solve_residual

        super(MeshVariable_Projection, self).__init__(**kwargs)

    def _setup(self):
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.forceVectors, self._fvector._cself )
        if self.type == 1:
            uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.stiffnessMatrices, self._kmatrix._cself )
            uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.solutionVectors, self._solutionVector._cself )


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MeshVariable_Projection,self)._add_to_stg_dict(componentDictionary)


    def _solve_average(self):
        """
        Solve the projection for the current state of the provided function.
        """
        # first assemble \int{Fn.N}
        if self._swarm:
            self._swarm._voronoi_swarm.repopulate()
        libUnderworld.StgFEM.ForceVector_Zero( self._fvector._cself )
        libUnderworld.StgFEM.ForceVector_GlobalAssembly_General( self._fvector._cself )
        libUnderworld.StgFEM.SolutionVector_UpdateSolutionOntoNodes( self._fvector._cself );

        # now do again for \int{N}, but first create copy
        self._copyMeshVariable.data[:] = self._meshVariable.data[:]
        self._forceVecTerm.fn = self._unityArray
        libUnderworld.StgFEM.ForceVector_Zero( self._fvector._cself )
        libUnderworld.StgFEM.ForceVector_GlobalAssembly_General( self._fvector._cself )
        libUnderworld.StgFEM.SolutionVector_UpdateSolutionOntoNodes( self._fvector._cself );

        # right, now divide
        self._meshVariable.data[:] = self._copyMeshVariable.data[:] / self._meshVariable.data[:]
        # done! return to correct function
        self._forceVecTerm.fn = self._fn

    def _solve_residual(self):
        """
        Solve the projection given the current state of the provided function.
        """
        if not self._solver:
            self._solver = uw.systems.Solver(self)
        self._solver.solve()
