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
import _assembledvector
import libUnderworld

class AssembledMatrix(_stgermain.StgCompoundComponent):
    """
    """
    _objectsDict = { "_matrix": "StiffnessMatrix" }
    _selfObjectName = "_matrix"

#    def __init__(self, meshVariableRow, meshVariableCol, assemblyTerms=[], rhs=None, rhs_T=None, **kwargs):
    def __init__(self, meshVariableRow, meshVariableCol, rhs=None, rhs_T=None, allowZeroContrib=False, **kwargs):
        """
        Args:
            meshVariableRow (MeshVariable)
            meshVariableCol (MeshVariable)

        See property docstrings for further information on each argument.
        
        """
        
        if not isinstance(meshVariableRow, uw.meshvariable.MeshVariable):
            raise TypeError("'meshvariableRow' object passed in must be of type 'MeshVariable'")

        if not isinstance(meshVariableCol, uw.meshvariable.MeshVariable):
            raise TypeError("'meshvariableCol' object passed in must be of type 'MeshVariable'")
        self._meshVariableRow = meshVariableRow
        self._meshVariableCol = meshVariableCol


        if rhs and not isinstance(rhs, _assembledvector.AssembledVector):
            raise TypeError("'rhs' object passed in must be of type 'AssembledVector'")

        if rhs_T and not isinstance(rhs_T, _assembledvector.AssembledVector):
            raise TypeError("'rhs_T' object passed in must be of type 'AssembledVector'")
        self._rhs   = rhs
        self._rhs_T = rhs_T
        
        if not isinstance( allowZeroContrib, bool ):
            raise TypeError("'allowZeroContrib' must be of type 'bool'.")
        self.allowZeroContrib = allowZeroContrib
        
#        if not isinstance(assemblyTerms, (tuple,list)):
#            raise TypeError("'assemblyTerms' object passed in must be of type 'list' or 'tuple'")
#        self._assemblyTerms = assemblyTerms

        # build parent
        super(AssembledMatrix,self).__init__(**kwargs)
        

    @property
    def meshVariableRow(self):
        """    
        meshVariableRow (MeshVariable): MeshVariable object for matrix row.
        """
        return self._meshVariableRow
    @property
    def meshVariableCol(self):
        """    
        meshVariableCol (MeshVariable): MeshVariable object for matrix xol.
        """
        return self._meshVariableCol

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AssembledMatrix,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._matrix.name ][   "RowVariable"] = self._meshVariableRow._cself.name
        componentDictionary[ self._matrix.name ]["ColumnVariable"] = self._meshVariableCol._cself.name
        if self.allowZeroContrib == False:
            componentDictionary[ self._matrix.name ]["allowZeroElementContributions"] = "False"
        else:
            componentDictionary[ self._matrix.name ]["allowZeroElementContributions"] = "True"
        componentDictionary[ self._matrix.name ]["dim"] = self._meshVariableCol._mesh.generator.dim
        if self._rhs:
            componentDictionary[ self._matrix.name ][         "RHS"] = self._rhs._cself.name
        if self._rhs_T:
            componentDictionary[ self._matrix.name ]["transposeRHS"] = self._rhs_T._cself.name


#    def _setup(self):
#        # add terms to vector
#        for term in self._assemblyTerms:
#            term._cself.stiffnessMatrix = self._cself
#            libUnderworld.StgFEM.StiffnessMatrix_AddStiffnessMatrixTerm( self._cself, term._cself )
#
#    def AddTerm(self, assemblyTerm):
#        self._assemblyTerms.append(assemblyTerm)
#        assemblyTerm._cself.stiffnessMatrix = self._cself
#        libUnderworld.StgFEM.StiffnessMatrix_AddStiffnessMatrixTerm(self._cself, assemblyTerm._cself)

