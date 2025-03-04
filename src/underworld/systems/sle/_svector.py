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

class SolutionVector(_stgermain.StgCompoundComponent):
    """
    The SolutionVector manages the numerical solution vectors used by Underworld's equation systems.
    Interface between meshVariables and systems.

    Parameters
    ----------
    meshVariable: underworld.mesh.MeshVariable
        MeshVariable object for which this SLE vector corresponds.
    eqNumber:     underworld.systems.sle.EqNumber
        Equation numbering object corresponding to this vector.
    
    Example
    -------
    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> tField = uw.mesh.MeshVariable( linearMesh, 1 )
    >>> eqNum = uw.systems.sle.EqNumber( tField )
    >>> sVector = uw.systems.sle.SolutionVector(tField, eqNum )

    """
    _objectsDict = { "_vector": "SolutionVector" }
    _selfObjectName = "_vector"

    def __init__(self, meshVariable, eqNumber, **kwargs):
        if not isinstance(meshVariable, uw.mesh.MeshVariable):
            raise TypeError("'meshVariable' object passed in must be of type 'MeshVariable'")
        self._meshVariable = meshVariable
        if not isinstance(eqNumber, uw.systems.sle.EqNumber):
            raise TypeError("'eqNumber' object passed in must be of type 'EqNumber'")
        self._eqNumber = eqNumber

        if not eqNumber.meshVariable == meshVariable:
            raise ValueError("Supplied 'eqNumber' doesn't correspond to the supplied 'meshVariable'")

        # build parent
        super(SolutionVector,self).__init__(**kwargs)

        # setup the 'c' SolutionVector to reference the eqNum 'c' object
        self._cself.eqNum = self._eqNumber._cself


    @property
    def meshVariable(self):
        return self._meshVariable

    @property
    def eqNumber(self):
        return self._eqNumber

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SolutionVector,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._vector.name ]["FeVariable"] = self._meshVariable._cself.name
