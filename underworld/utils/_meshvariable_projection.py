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
    
    To determine the projection onto the nodes, a weighted residual 
    method is used to construct an SLE which is then solved to 
    determine the unknowns.
    
    For the variable 
    .. math::
         \bf{U} =  \bf{u}_a N_a
    
    To project function F onto the variable, we solve:
    .. math::
         \bf{u}_a\int_{\Omega} N_a N_b \partial\Omega = \int_{\Omega} \bf{F}\partial\Omega

    Parameters
    ----------
    meshVariable : underworld.mesh.MeshVariable
        The variable you wish to project the function onto.
    fn : underworld.function.Function
        The function you wish to project.
    swarm : underworld.swarm.Swarm, default=None.
        For voronoi integration, provided the swarm to be used. Otherwise
        Gauss integration will be performed.

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
    >>> projector = uw.utils.MeshVariable_Projection( U, const )
    >>> np.allclose(U.data, const)
    False
    >>> projector.solve()
    >>> np.allclose(U.data, const)
    True
    
    Now cast mesh coordinates onto a vector variable

    >>> U_coord = uw.mesh.MeshVariable( mesh, 2 )
    >>> projector = uw.utils.MeshVariable_Projection( U_coord, uw.function.coord() )
    >>> projector.solve()
    >>> np.allclose(U_coord.data, mesh.data)
    True

    Project one mesh variable onto another
    
    >>> U_copy = uw.mesh.MeshVariable( mesh, 2 )
    >>> projector = uw.utils.MeshVariable_Projection( U_copy, U_coord )
    >>> projector.solve()
    >>> np.allclose(U_copy.data, U_coord.data)
    True
    
    Project the coords to the submesh (usually the constant mesh)

    >>> U_submesh = uw.mesh.MeshVariable( mesh.subMesh, 2 )
    >>> projector = uw.utils.MeshVariable_Projection( U_submesh, U_coord )
    >>> projector.solve()
    >>> np.allclose(U_submesh.data, mesh.subMesh.data)
    True
    

    """
    _objectsDict = {  "_system" : "SystemLinearEquations" }
    _selfObjectName = "_system"

    def __init__(self, meshVariable=None, fn=None, swarm=None, **kwargs):

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

        libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( self._meshVariable._cself )

        # create solutions vector
        self._solutionVector = sle.SolutionVector(meshVariable)

        # create force vectors
        self._fvector = sle.AssembledVector(meshVariable)

        # and matrices
        self._kmatrix = sle.AssembledMatrix( meshVariable, meshVariable, rhs=self._fvector )

        # create swarm
        self._gaussSwarm = uw.swarm.GaussIntegrationSwarm(self._meshVariable.mesh)
        self._PICSwarm = None
        if self._swarm:
            self._PICSwarm = uw.swarm.PICIntegrationSwarm(self._swarm)

        swarmguy = self._PICSwarm
        if not swarmguy:
            swarmguy = self._gaussSwarm

        # determine the required geometry mesh.  for submesh, use the parent mesh.
        geometryMesh = self._meshVariable.mesh
        if hasattr(self._meshVariable.mesh.generator, 'geometryMesh'):
            geometryMesh = self._meshVariable.mesh.generator.geometryMesh

        self._kMatTerm = sle.MassMatrixTerm(  integrationSwarm=swarmguy,
                                               assembledObject=self._kmatrix,
                                                          mesh=geometryMesh )
        self._forceVecTerm = sle.VectorAssemblyTerm_NA__Fn(   integrationSwarm=swarmguy,
                                                              assembledObject=self._fvector,
                                                              fn=_fn,
                                                              mesh=geometryMesh )
                                                              
        self._solver = None

        super(MeshVariable_Projection, self).__init__(**kwargs)

    def _setup(self):
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.stiffnessMatrices, self._kmatrix._cself )
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.forceVectors, self._fvector._cself )
        uw.libUnderworld.StGermain.Stg_ObjectList_Append( self._cself.solutionVectors, self._solutionVector._cself )


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MeshVariable_Projection,self)._add_to_stg_dict(componentDictionary)

    @property
    def fn(self):
        """
        The diffusivity function. You may change this function directly via this
        property.
        """
        return self._kMatTerm.fn
    @fn.setter
    def fn(self, value):
        self._kMatTerm.fn = value

    def solve(self):
        """
        Solve the projection given the current state of the provided function.
        """
        if not self._solver:
            self._solver = uw.systems.Solver(self)
        self._solver.solve()
