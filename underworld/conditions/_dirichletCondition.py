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
    pass

class DirichletCondition(_SystemCondition):
    """
    The DirichletCondition class provides the required functionality to imposed Dirichlet
    conditions on your differential equation system.
    
    The user is simply required to flag which nodes/DOFs should be considered by the system
    to be a Dirichlet condition. The values at the Dirichlet nodes/DOFs is then left 
    untouched by the system.
    
    Note that it is necessary for the user to set the required value on the variable, possibly
    via the numpy interface.
    
    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> velocityField = uw.fevariable.FeVariable( linearMesh, 2 )
    >>> velocityField.data[:] = [0.,0.]  # set velocity zero everywhere, which will of course include the boundaries.
    >>> IWalls = linearMesh.specialSets["MinI_VertexSet"] + linearMesh.specialSets["MaxI_VertexSet"]  # get some wall index sets
    >>> JWalls = linearMesh.specialSets["MinJ_VertexSet"] + linearMesh.specialSets["MaxJ_VertexSet"]
    >>> freeSlipBC = uw.conditions.DirichletCondition(velocityField, (IWalls,JWalls) )  # this will give free slip sides
    >>> noSlipBC = uw.conditions.DirichletCondition(velocityField, (IWalls+JWalls,IWalls+JWalls) )  # this will give no slip sides
    
    """
    _objectsDict = { "_pyvc": "PythonVC" }
    _selfObjectName = "_pyvc"

    def __init__(self, variable, nodeIndexSets):
        """
        Class initialiser.
        
        Parameters
        ----------
        variable : uw.fevariable.FeVariable
            This is the variable for which the Direchlet condition applies.
        nodeIndexSets : list, tuple, IndexSet
            The index set(s) which flag nodes/DOFs as Dirichlet conditions.
            Note that the user must provide an index set for each degree of
            freedom of the variable.  So for a vector variable of rank 2 (say Vx & Vy),
            two index sets must be provided (say VxDofSet, VyDofSet).
        """

        if not isinstance( variable, uw.fevariable.FeVariable ):
            raise TypeError("Provided variable must be of class 'FeVariable'.")
        self._variable = variable

        if isinstance( nodeIndexSets, uw.container.IndexSet):
            indexSets = ( nodeIndexSets, )
        elif isinstance( nodeIndexSets, (list,tuple)):
            indexSets = nodeIndexSets
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

    def _add_to_stg_dict(self,componentDict):
        pass

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

