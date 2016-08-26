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

class EqNumber(_stgermain.StgClass):
    """
    The SolutionVector manages the numerical solution vectors used by Underworld's equation systems.
    Interface between meshVariables and systems.
    """

    def __init__(self, meshVariable, removeBCs=True, **kwargs):
        """
        Parameters:
        -----------
            meshVariable (MeshVariable)

        See property docstrings for further information on each argument.

        >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
        >>> tField = uw.mesh.MeshVariable( linearMesh, 1 )
        >>> teqNum = uw.systems.sle.EqNumber( tField )
        """

        if not isinstance(meshVariable, uw.mesh.MeshVariable):
            raise TypeError("'meshVariable' object passed in must be of type 'MeshVariable'")
        self._meshVariable = meshVariable
        if not isinstance(removeBCs, bool):
            raise TypeError("'removeBCs' must be of type 'bool'")
        self._removeBCs=removeBCs

        # build parent
        super(EqNumber,self).__init__( uw.libUnderworld.StgFEM._FeEquationNumber_Create( meshVariable._cself, removeBCs ) , **kwargs)

    @property
    def meshVariable(self):
        """
        meshVariable (MeshVariable): MeshVariable object for which this SLE vector corresponds.
        """
        return self._meshVariable
