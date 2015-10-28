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
    This class constructs a surface or volume integral of the provided function over a 
    given mesh.
    
    Calculate volume of mesh:
    
    >>> import underworld as uw
    >>> mesh = uw.mesh.FeMesh_Cartesian(minCoord=(0.,0.), maxCoord=(1.,1.))
    >>> volumeIntegral = uw.utils.Integral(fn=1.,mesh=mesh)
    >>> volumeIntegral.evaluate()
    [1.0]
    
    Calculate surface area of mesh:
    
    >>> surfaceIntegral = uw.utils.Integral(fn=1.,mesh=mesh, integrationType='surface', surfaceIndexSet=mesh.specialSets["AllWalls_VertexSet"])
    >>> surfaceIntegral.evaluate()
    [4.0]
    
    """
    _objectsDict = { "_integral": "Fn_Integrate" }
    _selfObjectName = "_integral"

    def __init__(self, fn, mesh, integrationType="volume", surfaceIndexSet=None, integrationSwarm=None, feMesh=None, **kwargs):
        """
        Parameters
        ----------
        mesh : uw.mesh.FeMesh
            The FeMesh the swarm is supported by.
        integrationSwarm : uw.swarm.IntegrationSwarm (optional)
            User provided integration swarm.
        integrationType : str
            Type of integration to perform.  Options are "volume" or "surface".
        surfaceIndexSet : uw.mesh.FeMesh_IndexSet
            Must be provided where integrationType is "surface".
            This IndexSet determines which surface is to be integrated over.
            Note that surface integration over interior nodes is not supported.
        
        """
        
        if feMesh:  # DEPRECATE
            raise ValueError("This parameter has been renamed to 'mesh'.")
        
        if not isinstance(mesh, uw.mesh.FeMesh):
            raise TypeError("'feMesh' object passed in must be of type 'FeMesh'")
        self._mesh = mesh
        self._cself.mesh = self._mesh._cself
        
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
                self._cself.isSurfaceIntegral = False
                integrationSwarm = uw.swarm.GaussIntegrationSwarm(mesh)
            else:
                self._cself.isSurfaceIntegral = True
                if not surfaceIndexSet:
                    raise RuntimeError("For surface integration, you must provide a 'surfaceIndexSet'.")
                if not isinstance(surfaceIndexSet, uw.mesh.FeMesh_IndexSet ):
                    raise TypeError("'surfaceIndexSet' must be of type 'FeMesh_IndexSet'.")
                if surfaceIndexSet.object != mesh:
                    raise ValueError("'surfaceIndexSet' mesh does not appear to correspond to mesh provided to Integral object.")
                if surfaceIndexSet.topologicalIndex != 0:
                    raise ValueError("'surfaceIndexSet' must correspond to vertex objects.")
                # check that nodes are boundary nodes
                try:
                    allBoundaryNodes = mesh.specialSets['AllWalls_VertexSet']
                except:
                    raise ValueError("Mesh does not appear to provide a 'AllWalls_VertexSet' special set. This is required for surface integration.")
                for guy in surfaceIndexSet:
                    inSet = int(guy) in allBoundaryNodes
                    if not inSet:
                        raise ValueError("Your surfaceIndexSet appears to contain node(s) which do not belong to the mesh boundary. Surface integration across internal nodes is not currently supported.")
                # create feVariable
                deltaFeVariable = uw.fevariable.FeVariable(mesh, 1)
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
                integrationSwarm = uw.swarm.GaussBorderIntegrationSwarm(mesh)
        else:
            if not isinstance(integrationSwarm, uw.swarm.IntegrationSwarm):
                raise TypeError("'integrationSwarm' object passed in must be of type 'IntegrationSwarm'")

        self._integrationSwarm = integrationSwarm
        self._cself.integrationSwarm = integrationSwarm._cself
        self._cself.dim = mesh.dim

        # lets setup fn tings
        libUnderworld.Underworld._Fn_Integrate_SetFn( self._cself, self._fn._fncself)
        
        super(Integral,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        pass

    def evaluate(self):
        """
        Perform integration.
        
        Returns
        -------
        result : list of floats
            Integration result. For vector integrals, a vector is returned.
        
        """
        val = libUnderworld.Underworld.Fn_Integrate_Integrate( self._cself )
        result = []
        for ii in range(0,val.size()):
            result.append(val.value(ii))
        return result

    def integrate(self): # DEPRECATE
        raise RuntimeError("This method has been renamed to 'evaluate'.")
    
    @property
    def maskFn(self):
        """
        The integration mask used where surface integration is performed.
        """
        if not self._maskFn:
            raise RuntimeError("No mask function appears to have been set.\n"+
                               "Note that mask functions are only set for surface integration.")
        return self._maskFn
