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
import underworld.mesh as mesh
import numpy as np
import weakref
from libUnderworld import *

class ForceVector(_stgermain.StgCompoundComponent):
    """
    """
    _objectsDict = { "_vector": "ForceVector" }
    _selfObjectName = "_vector"

    def __init__(self, feVariable, **kwargs):
        """
        Parameters
        ----------
            feVariable : FeVariable

        See property docstrings for further information on each argument.
        
        """
        
        if not isinstance(feVariable, fevariable.FeVariable):
            raise TypeError("'feVariable' object passed in must be of type 'FeVariable'")
        self._feVariable = feVariable
        
        # build parent
        super(Vector,self).__init__(**kwargs)
        

    @property
    def feVariable(self):
        """    
        feVariable (FeVariable): FeVariable object for which this SLE vector corresponds.
        """
        return self._feVariable

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(FeVariable,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._vector.name ]["FeVariable"] = self._feVariable._cself.name

    def AddTerm(assemblyTerm):
        """
        One of:
        
        ThermalBuoyancyForceTerm
        MassMatrixTerm
        LumpedMassMatrixForceTerm
        AdvDiffResidualForceTerm

        """
        # make a call to underlying c-function 'ForceVector_AddTerm'
        _stgermain.StgFEM.ForceVector_AddTerm(self._cself, assemblyTerm._cself)


