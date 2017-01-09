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
import libUnderworld
from _svector import SolutionVector

class AssembledVector(SolutionVector):
    """
    Vector object, generally assembled as a result of the FEM
    framework.
    
    See parent class for parameters.

    """
    _objectsDict = { "_vector": "ForceVector" }
    _selfObjectName = "_vector"


    def __init__(self, meshVariable, eqNum, **kwargs):
        super(AssembledVector,self).__init__(meshVariable, eqNum, **kwargs)

    @property
    def meshVariable(self):
        return self._meshVariable

    @property
    def petscVector(self):
        """
        petscVector (swig petsc vector): Underlying PETSc vector object.
        """
        return self._cself.vector


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AssembledVector,self)._add_to_stg_dict(componentDictionary)
        #
        componentDictionary[ self._vector.name ][       "dim"] = self._meshVariable.mesh.generator.dim

#    def _setup(self):
#        # add terms to vector
#        for term in self._assemblyTerms:
#            term._cself.forceVector = self._cself
#            libUnderworld.StgFEM.ForceVector_AddForceTerm( self._cself, term._cself )
#
#    def AddTerm(self, assemblyTerm):
#        self._assemblyTerms.append(assemblyTerm)
#        assemblyTerm._cself.forceVector = self._cself
#        libUnderworld.StgFEM.ForceVector_AddForceTerm(self._cself, assemblyTerm._cself)
