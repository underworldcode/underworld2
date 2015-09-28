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

class AssembledVector(_stgermain.StgCompoundComponent):
    """
    """
    _objectsDict = { "_vector": "ForceVector" }
    _selfObjectName = "_vector"

#    def __init__(self, feVariable, assemblyTerms=[], **kwargs):
    def __init__(self, feVariable, **kwargs):
        """
        Class initialiser.
        
        Parameter
        ---------
        feVariable    : fevariable.FeVariable
            The FeVariable for which the SLE vector should be created
            
        Returns
        -------
        AssembledVector: The constructed AssembledVector object.

        """
        
        if not isinstance(feVariable, uw.fevariable.FeVariable):
            raise TypeError("'feVariable' object passed in must be of type 'FeVariable'")
        self._feVariable = feVariable

#        if not isinstance(assemblyTerms, (tuple,list)):
#            raise TypeError("'assemblyTerms' object passed in must be of type 'list' or 'tuple'")
#        self._assemblyTerms = assemblyTerms

        # build parent
        super(AssembledVector,self).__init__(**kwargs)
        

    @property
    def feVariable(self):
        """    
        feVariable (FeVariable): FeVariable object for which this SLE vector corresponds.
        """
        return self._feVariable

    @property
    def petscVector(self):
        """    
        petscVector (swig petsc vector): Underlying PETSc vector object.
        """
        return self._cself.vector


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AssembledVector,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._vector.name ]["FeVariable"] = self._feVariable._cself.name
        componentDictionary[ self._vector.name ][       "dim"] = self._feVariable.feMesh.generator.dim

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
