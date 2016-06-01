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
import libUnderworld

class _SystemCondition(_stgermain.StgCompoundComponent):
    def _add_to_stg_dict(self,componentDict):
        pass

class DirichletCondition(_SystemCondition):
    """
    The DirichletCondition class provides the required functionality to imposed Dirichlet
    conditions on your differential equation system.

    The user is simply required to flag which nodes/DOFs should be considered by the system
    to be a Dirichlet condition. The values at the Dirichlet nodes/DOFs is then left
    untouched by the system.

    Parameters
    ----------
    variable : uw.mesh.MeshVariable
        This is the variable for which the Direchlet condition applies.
    indexSetsPerDof : list, tuple, IndexSet
        The index set(s) which flag nodes/DOFs as Dirichlet conditions.
        Note that the user must provide an index set for each degree of
        freedom of the variable.  So for a vector variable of rank 2 (say Vx & Vy),
        two index sets must be provided (say VxDofSet, VyDofSet).

    Notes
    -----
    Note that it is necessary for the user to set the required value on the variable, possibly
    via the numpy interface.

    Constructor must be called by collectively all processes.

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

    def __init__(self, variable, indexSetsPerDof=None):
        if not isinstance( variable, uw.mesh.MeshVariable ):
            raise TypeError("Provided variable must be of class 'MeshVariable'.")
        self._variable = variable

        if isinstance( indexSetsPerDof, uw.container.IndexSet):
            indexSets = ( indexSetsPerDof, )
        elif isinstance( indexSetsPerDof, (list,tuple)):
            indexSets = indexSetsPerDof
        else:
            raise TypeError("You must provide the required indexSet as an 'IndexSet' \n"+
                             "item, or as a list or tuple of 'IndexSet' items.")
        for guy in indexSets:
            if not isinstance( guy, (uw.container.IndexSet,type(None)) ):
                raise TypeError("Provided list must only contain objects of 'NoneType' or type 'IndexSet'.")
        self._indexSets = indexSets

        if variable.nodeDofCount != len(self._indexSets):
            raise ValueError("Provided variable has a nodeDofCount of {}, however you have ".format(variable.nodeDofCount)+
                             "provided {} index set(s). You must provide an index set for each degree ".format(len(self.indexSets))+
                             "of freedom of your variable, but no more.")

        # ok, lets setup the c array
        libUnderworld.StGermain._PythonVC_SetupIndexSetArray(self._cself,len(self._indexSets))

        # now, lets add the indexSet objects
        for position,set in enumerate(self._indexSets):
            if set:
                libUnderworld.StGermain._PythonVC_SetIndexSetAtArrayPosition( self._cself, set._cself, position );

        super(DirichletCondition,self).__init__()

    @property
    def indexSets(self):
        """
        Tuple or list of IndexSet objects. One set for each degree of freedom of the associated
        variable. These sets flag which nodes/DOFs are to be considered Dirichlet.
        """
        return self._indexSets

    @property
    def variable(self):
        """
        Variable for which this condition applies.
        """
        return self._variable

class NeumannCondition(_SystemCondition):
    """
    This class defines Neumann conditions for a differential equation.
    Neumann conditions specifiy a field's flux along a boundary.

    As such the user specifices the field's flux as a uw.Function and the nodes where this flux
    is to be applied - similar to uw.conditions.DirichletCondtion

    Parameters
    ----------
    flux : uw.Function
        See uw.Function for details

    """
    _objectsDict = { "_pyvc": "PythonVC" }
    _selfObjectName = "_pyvc"

    def __init__(self, flux, variable=None, nodeIndexSet=None, **kwargs ):

        if nodeIndexSet == None:
            raise ValueError("No 'nodeIndexSet' provided to apply Neumann conditions")

        _flux = uw.function.Function._CheckIsFnOrConvertOrThrow(flux)
        if not isinstance( _flux, uw.function.Function):
            raise ValueError( "Provided 'flux' must be of or convertible to 'Function' class." )
        self._flux = _flux

        if not isinstance( variable, uw.mesh.MeshVariable ):
            raise TypeError("Provided variable must be of class 'MeshVariable'.")
        self._variable = variable

        if isinstance( nodeIndexSet, uw.container.IndexSet):
            indexSet = nodeIndexSet
        else:
            raise TypeError("Provided 'nodeIndexSet' must be of type 'IndexSet', " \
            "currently it is: ", type(nodeIndexSet)  )

        self._indexSet = indexSet

        # ok, lets setup the c array, only 1 of them
        libUnderworld.StGermain._PythonVC_SetupIndexSetArray(self._cself,1 )

        # now, lets add the indexSet objects
        libUnderworld.StGermain._PythonVC_SetIndexSetAtArrayPosition( self._cself, self._indexSet._cself, 0 );

        # call parent
        super(NeumannCondition,self).__init__()

    @property
    def flux(self):
        """
        Gradient Field for which this condition applies.
        """
        return self._flux

    @property
    def indexSet(self):
        """
        Node IndexSet objects. Represents the nodes, of the given variable's mesh,
        that will be flagged to have this Neumann condition.
        """
        return self._indexSet

    @property
    def variable(self):
        """
        Variable for which this condition applies.
        """
        return self._variable
