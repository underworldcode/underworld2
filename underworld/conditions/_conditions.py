##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
##                                                                                   ##
##  This file forms part of the Underworld geophysics modelling application.         ##
##                                                                                   ##
##  For full license and copyright information, please refer to the LICENSE.md file  ##
##  located at the project root, or contact the authors.                             ##
##                                                                                   ##
##~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~#~##
"""
This module contains conditions used for applying constraints on model dynamics.
"""

import underworld as uw
import underworld._stgermain as _stgermain
import underworld.libUnderworld as libUnderworld
import abc

class SystemCondition(_stgermain.StgCompoundComponent, metaclass = abc.ABCMeta):
    def _add_to_stg_dict(self,componentDict):
        pass

    def __init__(self, variable, indexSetsPerDof):
        if not isinstance( variable, uw.mesh.MeshVariable ):
            raise TypeError("Provided variable must be of class 'MeshVariable'.")
        self._variable = variable

        if isinstance( indexSetsPerDof, uw.container.IndexSet ):
            indexSets = ( indexSetsPerDof, )
        elif isinstance( indexSetsPerDof, (list,tuple)):
            indexSets = indexSetsPerDof
        else:
            raise TypeError("You must provide the required 'indexSetsPerDof' item\n"+
                             "as a list or tuple of 'IndexSet' items.")
        for guy in indexSets:
            if not isinstance( guy, (uw.container.IndexSet,type(None)) ):
                raise TypeError("Provided list must only contain objects of 'NoneType' or type 'IndexSet'.")
        self._indexSets = indexSets

        if variable.nodeDofCount != len(self._indexSets):
            raise ValueError("Provided variable has a nodeDofCount of {}, however you have ".format(variable.nodeDofCount)+
                             "provided {} index set(s). You must provide an index set for each degree ".format(len(self._indexSets))+
                             "of freedom of your variable, but no more.")

        # ok, lets setup the c array
        libUnderworld.StGermain._PythonVC_SetupIndexSetArray(self._cself,len(self._indexSets))

        # now, lets add the indexSet objects
        for position,set in enumerate(self._indexSets):
            if set:
                libUnderworld.StGermain._PythonVC_SetIndexSetAtArrayPosition( self._cself, set._cself, position );

    @property
    def indexSetsPerDof(self):
        """ See class constructor for details. """
        return self._indexSets

    @property
    def variable(self):
        """ See class constructor for details. """
        return self._variable


class DirichletCondition(SystemCondition):
    """
    The DirichletCondition class provides the required functionality to imposed Dirichlet
    conditions on your differential equation system.

    The user is simply required to flag which nodes/DOFs should be considered by the system
    to be a Dirichlet condition. The values at the Dirichlet nodes/DOFs is then left
    untouched by the system.

    Parameters
    ----------
    variable : underworld.mesh.MeshVariable
        This is the variable for which the Dirichlet condition applies.
    indexSetsPerDof : list, tuple, IndexSet
        The index set(s) which flag nodes/DOFs as Dirichlet conditions.
        Note that the user must provide an index set for each degree of
        freedom of the variable.  So for a vector variable of rank 2 (say Vx & Vy),
        two index sets must be provided (say VxDofSet, VyDofSet).

    Notes
    -----
    Note that it is necessary for the user to set the required value on the variable, possibly
    via the numpy interface.

    Constructor must be called collectively all processes.

    Example
    -------
    Basic setup and usage of Dirichlet conditions:

    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> velocityField = uw.mesh.MeshVariable( linearMesh, 2 )
    >>> velocityField.data[:] = [0.,0.]  # set velocity zero everywhere, which will of course include the boundaries.
    >>> IWalls = linearMesh.specialSets["MinI_VertexSet"] + linearMesh.specialSets["MaxI_VertexSet"]  # get some wall index sets
    >>> JWalls = linearMesh.specialSets["MinJ_VertexSet"] + linearMesh.specialSets["MaxJ_VertexSet"]
    >>> freeSlipBC = uw.conditions.DirichletCondition(velocityField, (IWalls,JWalls) )  # this will give free slip sides
    >>> noSlipBC = uw.conditions.DirichletCondition(velocityField, (IWalls+JWalls,IWalls+JWalls) )  # this will give no slip sides

    """
    _objectsDict = { "_pyvc": "PythonVC" }
    _selfObjectName = "_pyvc"

    def __init__(self, variable, indexSetsPerDof):
        super(DirichletCondition,self).__init__(variable, indexSetsPerDof)


class NeumannCondition(SystemCondition):
    """
    This class defines Neumann conditions for a differential equation.
    Neumann conditions specifiy a field's flux along a boundary.

    As such the user specifices the field's flux as a uw.Function and the nodes where this flux
    is to be applied - similar to uw.conditions.DirichletCondtion

    Parameters
    ----------
    fn_flux : underworld.function.Function
        Function which determines flux values.
    variable : underworld.mesh.MeshVariable
        The variable that describes the discretisation (mesh & DOFs) for 'indexSetsPerDof'
    indexSetsPerDof : list, tuple, IndexSet
        The index set(s) which flag nodes/DOFs as Neumann conditions.
        Note that the user must provide an index set for each degree of
        freedom of the variable above.  So for a vector variable of rank 2 (say Vx & Vy),
        two index sets must be provided (say VxDofSet, VyDofSet).

    Example
    -------
    Basic setup and usage of Neumann conditions:

    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> velocityField = uw.mesh.MeshVariable( linearMesh, 2 )
    >>> velocityField.data[:] = [0.,0.]  # set velocity zero everywhere, which will of course include the boundaries.
    >>> myFunc = (uw.function.coord()[1],0.0)
    >>> bottomWall = linearMesh.specialSets["MinJ_VertexSet"]
    >>> tractionBC = uw.conditions.NeumannCondition(variable=velocityField, fn_flux=myFunc, indexSetsPerDof=(None,bottomWall) )
    """
    _objectsDict = { "_pyvc": "PythonVC" }
    _selfObjectName = "_pyvc"

    def __init__(self, variable, indexSetsPerDof=None, fn_flux=None ):

        # call parent
        super(NeumannCondition,self).__init__(variable, indexSetsPerDof)

        _fn_flux  = uw.function.Function.convert(fn_flux)
        if not isinstance( _fn_flux, uw.function.Function):
            raise TypeError( "Provided 'fn_flux' must be of or convertible to 'Function' class." )
        self.fn_flux=_fn_flux

    @property
    def fn_flux(self):
        """ Get the underworld.Function that defines the flux """
        return self._fn_flux
    @fn_flux.setter
    def fn_flux(self, fn):
        """ Set the underworld.Function that defines the flux """
        _fn = uw.function.Function.convert(fn)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided '_fn' must be of or convertible to 'Function' class." )
        self._fn_flux = _fn

class CurvilinearDirichletCondition(DirichletCondition):
    """
    The CuvilinearDirichletCondition class provides the functionality to apply
    variably defined vector dirichlet boundary conditions to a system.
    
    This is required for when the domain boundaries don't align to the numerical
    coordinate system of the problem. For example, solving a problem in an annulus geometry
    using cartesian coordinate system. The normal and tangential vectors change at every point 
    along surface. 

    Users are required to provide the `basis_vectors` for the rotation to be applied to each node/DOF.
    The basis_vectors must be orthogonal and can be provided as underworld.functions.
    See examples (TODO: link to examples) for details.

    Parameters
    ----------
    variable : underworld.mesh.MeshVariable
        This is the variable for which the Dirichlet condition applies.
    indexSetsPerDof : list, tuple, IndexSet
        The index set(s) which flag nodes/DOFs as Dirichlet conditions.
        Note that the user must provide an index set for each degree of
        freedom of the variable.  So for a vector variable of rank 2 (say Vx & Vy),
        two index sets must be provided (say VxDofSet, VyDofSet).
    basis_vectors : list or tuple of underworld.function.functions
        A list (or tuple) of underworld.function.functions to define the local coodinates per
        nodal vector dirichlet condition.
        For a mesh of type uw.mesh.FeMesh_Annulus,...,  the underlying basis_vectors are
        automatically extracted from the mesh and the users doesn't need to pass in 
        the `basis_vectors`. To check if automatic basis_vectors can be extracted on a mesh
        check the attributes ._e1, ._e2, etc. on a mesh.

    Notes
    -----
    Note that it is necessary for the user to set the required value on the variable, possibly
    via the numpy interface.

    Constructor must be called collectively all processes.

    Example
    -------
    TODO


    """

    _objectsDict = { "_pyvc": "PythonVC" }
    _selfObjectName = "_pyvc"

    def __init__(self, variable, indexSetsPerDof, basis_vectors=None):
        super(CurvilinearDirichletCondition,self).__init__(variable, indexSetsPerDof)


        mesh = variable.mesh

        if basis_vectors is not None:
            self.basis_vectors=basis_vectors
        else:
            try:
                if mesh.dim == 2:
                    self.basis_vectors=( mesh._e1, mesh._e2)
                else:
                    self.basis_vectors=( mesh._e1, mesh._e2, mesh._e3)

            except:
                raise("Problem with the basis vectors in CurvilinearDirichletCondition")

        @property
        def basis_vectors(self):
            """ Get the basis_vectors """
            return self._basis_vectors

        @basis_vectors.setter
        def basis_vectors(self, basis_vectors):
            """ Set the basis_vectors, ensure they're valid """
            if not isinstance(basis_vectors, (list,tuple)):
                raise TypeError("'basis_vectors' must be of type list or tuple")
            if len(basis_vectors) != self.variable.nodeDofCount:
                raise ValueError("'variable' number of components must equal the number of 'basis_vectors'")
            for vec in basis_vectors:
                if not isinstance( vec, uw.function.Function):
                    raise TypeError("'basis_vectors', must consist of 'uw.function.Function' types")

            self._basis_vectors=basis_vectors
