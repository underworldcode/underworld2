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
    Interface between feVariables and systems.

    Notes
    -----
    low level stuff that should be reworked post alpha
    """
    _objectsDict = { "_svector": "SolutionVector" }
    _selfObjectName = "_svector"

    def __init__(self, feVariable, **kwargs):
        """
        Parameters:
        -----------
            feVariable (FeVariable)

        See property docstrings for further information on each argument.
        
        >>> linearMesh = uw.mesh.FeMesh_Cartesian()
        >>> tField = uw.fevariable.FeVariable( linearMesh, 1 )
        >>> uw.libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( tField._cself )
        >>> sVector = uw.systems.sle.SolutionVector(tField)
        """
        
        if not isinstance(feVariable, uw.fevariable.FeVariable):
            raise TypeError("'feVariable' object passed in must be of type 'FeVariable'")
        self._feVariable = feVariable
        
        # build parent
        super(SolutionVector,self).__init__(**kwargs)
        

    @property
    def feVariable(self):
        """    
        feVariable (FeVariable): FeVariable object for which this SLE vector corresponds.
        """
        return self._feVariable

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SolutionVector,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._svector.name ]["FeVariable"] = self._feVariable._cself.name

