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

    Notes
    -----
    low level stuff that should be reworked post alpha
    """
    _objectsDict = { "_svector": "SolutionVector" }
    _selfObjectName = "_svector"

    def __init__(self, meshVariable, **kwargs):
        """
        Parameters:
        -----------
            meshVariable (MeshVariable)

        See property docstrings for further information on each argument.
        
        >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> tField = uw.meshvariable.MeshVariable( linearMesh, 1 )
        >>> uw.libUnderworld.StgFEM._FeVariable_CreateNewEqnNumber( tField._cself )
        >>> sVector = uw.systems.sle.SolutionVector(tField)
        """
        
        if not isinstance(meshVariable, uw.meshvariable.MeshVariable):
            raise TypeError("'meshVariable' object passed in must be of type 'MeshVariable'")
        self._meshVariable = meshVariable
        
        # build parent
        super(SolutionVector,self).__init__(**kwargs)
        

    @property
    def meshVariable(self):
        """    
        meshVariable (MeshVariable): MeshVariable object for which this SLE vector corresponds.
        """
        return self._meshVariable

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(SolutionVector,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._svector.name ]["FeVariable"] = self._meshVariable._cself.name

