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
    Orders how a MeshVariable's degrees of freedom are aranged withing numerical
    matrices and vectors. We call this equation ordering.
    An EqNumber is required when building a
    uw.sle.AssembledMatrix, uw.sle.SolutionVector or uw.sle.AssembledVector
    """

    def __init__(self, meshVariable, removeBCs=True, **kwargs):
        """
        Parameters:
        -----------
        meshVariable : MeshVariable
            The MeshVariable used to create an equation odering.

        removeBCs : Bool, optional
            Determines if the MeshVariable's boundary conditions are included in the ordering.
            Hence it effects the size of the matrix or vector and the algorithm used to handle boundary conditions.

    Example
    -------
    >>> linearMesh = uw.mesh.FeMesh_Cartesian( elementType='Q1/dQ0', elementRes=(4,4), minCoord=(0.,0.), maxCoord=(1.,1.) )
    >>> tField = uw.mesh.MeshVariable( linearMesh, 1 )
    >>> teqNum = uw.systems.sle.EqNumber( tField )
    
    """

    def __init__(self, meshVariable, removeBCs=True, **kwargs):
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
        return self._meshVariable
