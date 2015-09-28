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
import underworld.fevariable as fevariable
import underworld.function
import libUnderworld
import libUnderworld.libUnderworldPy.Function as _cfn

class Integral(_stgermain.StgCompoundComponent):
    """
    This class constructs an integral of the provided function over a mesh.
    """
    _objectsDict = { "_integral": "Fn_Integrate" }
    _selfObjectName = "_integral"

    def __init__(self, fn, feMesh, integrationType="volume", integrationSwarm=None, surfaceIndexSet=None,**kwargs):
        """
        Parameters
        ----------
        feMesh : uw.mesh.FeMesh
            The FeMesh the swarm is supported by. See Swarm.feMesh property docstring
            for further information.
        integrationSwarm : uw.swarm.IntegrationSwarm (optional)
            User provided integration swarm.
        integrationType : str
            Type of integration to perform.  Options are "volume" or "surface".
        surfaceIndexSet : uw.mesh.FeMesh_IndexSet
            Must be provided where integrationType is "surface"
            This IndexSet determines which surface is to be integrated over.
        
        """
        if not isinstance(feMesh, mesh.FeMesh):
            raise TypeError("'feMesh' object passed in must be of type 'FeMesh'")
        self._feMesh = feMesh
        self._cself.mesh = feMesh._cself
        
        self._maskFn = None
        
        self._fn = uw.function.Function._CheckIsFnOrConvertOrThrow(fn)

        if integrationType and integrationSwarm:
            raise RuntimeError("Either an 'integrationType' or an 'integrationSwarm' may be provided, but not both.\n"
                              +"You may need to set 'integrationType' to None.")
        
        if integrationType:
            if not isinstance( integrationType, str ):
                raise TypeError( "'integrationType' provided must be a string.")
            integrationType = integrationType.lower()
            if integrationType not in ["volume", "surface"]:
                raise ValueError( "'integrationType' string provided must be either 'volume' or 'surface'.")
            if integrationType == "volume":
                # set the dimensionality to be the same as the mesh for 'volume' integration
                self._cself.dim = feMesh.dim
                integrationSwarm = uw.swarm.GaussIntegrationSwarm(feMesh)
            else:
                raise RuntimeError("Surface integrals are currently disabled.")
                # set the dimensionality to be one less than the mesh for 'surface' integration
                self._cself.dim = feMesh.dim-1
                if not surfaceIndexSet:
                    raise RuntimeError("For surface integration, you must provide a 'surfaceIndexSet'.")
                if not isinstance(surfaceIndexSet, uw.mesh.FeMesh_IndexSet ):
                    raise TypeError("'surfaceIndexSet' must be of type 'FeMesh_IndexSet'.")
                if surfaceIndexSet.object != feMesh:
                    raise ValueError("'surfaceIndexSet' mesh does not appear to correspond to mesh provided to Integral object.")
                if surfaceIndexSet.object != feMesh:
                    raise ValueError("'surfaceIndexSet' mesh does not appear to correspond to mesh provided to Integral object.")
                if surfaceIndexSet.topologicalIndex != 0:
                    raise ValueError("'surfaceIndexSet' must correspond to vertex objects.")
                # create feVariable
                deltaFeVariable = uw.fevariable.FeVariable(feMesh, 1)
                # init to zero
                deltaFeVariable.data[:] = 0.
                # set to 1 on provided vertices
                deltaFeVariable.data[surfaceIndexSet.data] = 1.
                # replace fn with delta*fn
                # note that we need to use the condition so that we only capture border swarm particles
                # on the surface itself. for those directly adjacent, the deltaFeVariable will evaluate
                # to non-zero (but less than 1.), so we need to remove those from the integration as well.
                self._maskFn = underworld.function.branching.conditional(
                                                  [  ( deltaFeVariable > 0.999, 1. ),
                                                     (                    True, 0. )   ] )
                self._fn = self._fn * self._maskFn
                integrationSwarm = uw.swarm.GaussBorderIntegrationSwarm(feMesh)
        else:
            if not isinstance(integrationSwarm, uw.swarm.IntegrationSwarm):
                raise TypeError("'integrationSwarm' object passed in must be of type 'IntegrationSwarm'")

        self._integrationSwarm = integrationSwarm
        self._cself.integrationSwarm = integrationSwarm._cself

        # lets setup fn tings
        libUnderworld.Underworld._Fn_Integrate_SetFn( self._cself, self._fn._fncself)
        
        super(Integral,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        pass

    def integrate(self):
        """
        Perform integration
        """
        val = libUnderworld.Underworld.Fn_Integrate_Integrate( self._cself )
        result = []
        for ii in range(0,val.size()):
            result.append(val.value(ii))
        return result

    @property
    def maskFn(self):
        """
        The integration mask used where surface integration is performed.
        """
        if not self._maskFn:
            raise RuntimeError("No mask function appears to have been set.\n"+
                               "Note that mask functions are only set for surface integration.")
        return self._maskFn
