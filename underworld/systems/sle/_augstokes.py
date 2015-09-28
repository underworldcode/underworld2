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
"""
from underworld._vector import *
from underworld._matrix import *
"""

class AugStokes(_stgermain.StgCompoundComponent):
    """
    The Augmented Lagrangian Stokes System.

    Builds following system:
    
    [K ,G][u]=[f]
    [G',0][p] [h]
    
    """
    _objectsDict = { "_augstokes": "AugLagStokes_SLE" }
    _selfObjectName = "_augstokes"

    _supportedDataTypes = ["char","short","int","float", "double"]

    def __init__(self, velocityFeVariable, pressureFeVariable, temperatureFeVariable=None, pic=True, bodyforces=[], **kwargs):
        """
        Args:
            
        See property docstrings for further information on each argument.
        
        """
        # build parent
        super(FeVariable,self).__init__(**kwargs)
        
        # now create a bunch of stuff
        Ra=1.0
        if temperatureFeVariable is None:
            #then build a temperature field
            mesh = velocityFeVariable._feMesh
            temperatureFeVariable = uw.fevariable.FeVariable( feMesh=mesh, nodeDofCount=1, dataType="double")

        #create our own int swarm here?
        
        thermalBuoyancyTerm = ThermalBuoyancy(temperatureFeVariable, intswarm, Ra)

        f = ForceVector(velocityFeVariable,[])
        h = ForceVector(pressureFeVariable,[])
        u = SolutionVector(velocityFeVariable)
        p = SolutionVector(pressureFeVariable)
        

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AugStokes,self)._add_to_stg_dict(componentDictionary)
        
        #componentDictionary[ self._augstokes.name ]["AugLagStokes_SLE"] = self._feVariable._cself.name



