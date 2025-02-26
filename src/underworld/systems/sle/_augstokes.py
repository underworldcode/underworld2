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

    def __init__(self, velocityMeshVariable, pressureMeshVariable, temperatureMeshVariable=None, pic=True, bodyforces=[], **kwargs):
        # build parent
        super(MeshVariable,self).__init__(**kwargs)
        
        # now create a bunch of stuff
        Ra=1.0
        if temperatureMeshVariable is None:
            #then build a temperature field
            mesh = velocityMeshVariable._mesh
            temperatureMeshVariable = uw.mesh.MeshVariable( mesh=mesh, nodeDofCount=1, dataType="double")

        #create our own int swarm here?
        
        thermalBuoyancyTerm = ThermalBuoyancy(temperatureMeshVariable, intswarm, Ra)

        f = ForceVector(velocityMeshVariable,[])
        h = ForceVector(pressureMeshVariable,[])
        u = SolutionVector(velocityMeshVariable)
        p = SolutionVector(pressureMeshVariable)
        

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AugStokes,self)._add_to_stg_dict(componentDictionary)
        
        #componentDictionary[ self._augstokes.name ]["AugLagStokes_SLE"] = self._meshVariable._cself.name



