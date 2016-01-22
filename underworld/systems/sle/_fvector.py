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

    def __init__(self, meshVariable, **kwargs):
        """
        Parameters
        ----------
            meshVariable : MeshVariable

        See property docstrings for further information on each argument.
        
        """
        
        if not isinstance(meshVariable, mesh.MeshVariable):
            raise TypeError("'meshVariable' object passed in must be of type 'MeshVariable'")
        self._meshVariable = meshVariable
        
        # build parent
        super(Vector,self).__init__(**kwargs)
        

    @property
    def meshVariable(self):
        """    
        meshVariable (MeshVariable): MeshVariable object for which this SLE vector corresponds.
        """
        return self._meshVariable

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MeshVariable,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._vector.name ]["FeVariable"] = self._meshVariable._cself.name

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


