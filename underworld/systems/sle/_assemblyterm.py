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
import libUnderworld.libUnderworldPy.Function as _cfn
import _assembledvector
import _assembledmatrix
import libUnderworld

class AssemblyTerm(_stgermain.StgCompoundComponent):
    _objectsDict = { "_assemblyterm": None }
    _selfObjectName = "_assemblyterm"

    def __init__(self, integrationSwarm, extraInfo=None, **kwargs):
        super(AssemblyTerm,self).__init__(**kwargs)

        if not isinstance(integrationSwarm, uw.swarm.IntegrationSwarm):
            raise TypeError("'integrationSwarm' object passed in must be of type 'IntegrationSwarm'")
        self._integrationSwarm = integrationSwarm
        self._extraInfo = extraInfo

        self._fn = None
        self._set_fn_function = None

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AssemblyTerm,self)._add_to_stg_dict(componentDictionary)
        componentDictionary[ self._cself.name ][    "Swarm"] = self._integrationSwarm._cself.name
        componentDictionary[ self._cself.name ]["ExtraInfo"] = self._extraInfo

    @property
    def fn(self):
        return self._fn

    @fn.setter
    def fn(self, value):
        if not self._set_fn_function:
            raise RuntimeError("You cannot set a function for this assembly term.")
        _fn = uw.function._function.Function.convert(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of, or convertible to, 'Function' class." )
        self._fn = _fn
        self._set_fn_function( self._cself, self._fn._fncself )

    def _setup(self):
        # lets setup fn tings.. note that this uses the 'property' above
        if self._set_fn_function:
            self.fn = self._fn

class VectorAssemblyTerm(AssemblyTerm):
    def __init__(self, assembledObject, **kwargs):
        super(VectorAssemblyTerm,self).__init__(**kwargs)
        if not isinstance( assembledObject, _assembledvector.AssembledVector):
            raise TypeError("'assembledObject' passed in must be of type 'AssembledVector'")
        self._assembledObject = assembledObject

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorAssemblyTerm,self)._add_to_stg_dict(componentDictionary)
        componentDictionary[ self._cself.name ]["ForceVector"] = self._assembledObject._cself.name


class MatrixAssemblyTerm(AssemblyTerm):
    def __init__(self, assembledObject=None, **kwargs):
        super(MatrixAssemblyTerm,self).__init__(**kwargs)
        if not isinstance( assembledObject, (_assembledmatrix.AssembledMatrix, type(None))):
            raise TypeError("'assembledObject' passed in must be of type 'AssembledMatrix'")
        self._assembledObject = assembledObject

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MatrixAssemblyTerm,self)._add_to_stg_dict(componentDictionary)
        if self._assembledObject:
            componentDictionary[ self._cself.name ]["StiffnessMatrix"] = self._assembledObject._cself.name


class VectorSurfaceAssemblyTerm_NA__Fn__ni(VectorAssemblyTerm):
    """
    Build an assembly term for a surface integral.

    Parameters
    ----------
    nbc : underworld.conditions.NeumannCondition
        See uw.conditions.NeumannCondition for details
    integrationSwarm  : underworld.swarm.GaussBorderIntegrationSwarm
        Optional integration swarm to be used for numerical integration.
    surfaceGaussPoints : int
        The number of quadrature points per element face to use in surface 
        integration. Will be used to create a GaussBorderIntegrationSwarm in 
        the case the 'swarm' input is 'None'.
    """
    _objectsDict = { "_assemblyterm": "VectorSurfaceAssemblyTerm_NA__Fn__ni" }

    def __init__(self, nbc, integrationSwarm=None, surfaceGaussPoints=2, mesh=None, **kwargs):
        if not isinstance(nbc, uw.conditions.NeumannCondition):
            raise ValueError( "Provided 'nbc' must be a NeumannCondition class." )
        self._nbc = nbc
        mesh = nbc.variable.mesh

        if integrationSwarm != None:
            if not isinstance( integrationSwarm, uw.swarm.GaussBorderIntegrationSwarm):
                raise ValueError("Provided 'borderSwarm' must be of type uw.swarm.GaussBorderIntegrationSwarm")
        else: # no swarm provide, so we build one
            if not isinstance(surfaceGaussPoints, int):
                raise TypeError( "Provided 'surfaceGaussPoints' must be a positive integer")
            if surfaceGaussPoints < 1:
                raise ValueError( "Provided 'surfaceGaussPoints' must be a positive integer")
            integrationSwarm = uw.swarm.GaussBorderIntegrationSwarm( mesh=mesh,
                                                                         particleCount=surfaceGaussPoints )

        super(VectorSurfaceAssemblyTerm_NA__Fn__ni,self).__init__( integrationSwarm=integrationSwarm, **kwargs )

        # pass the NeumannConditions to the SurfaceAssemblyTerm so it knows which nodes to assemble the flux contribution
        libUnderworld.Underworld._VectorSurfaceAssemblyTerm_SetBNodes( self._cself, nbc._cself )

        ##### Now construct the additional rhs force like term. Ingredients required for a surface integral
        # 1) a gauss border swarm
        # 2) a mask function to only evaluate the flux only on nodes specified in the nbc.indexSets
        #####

        deltaMeshVariable = uw.mesh.MeshVariable(mesh, 1)
        deltaMeshVariable.data[:] = 0.
        # set a value 1.0 on provided vertices
        deltaMeshVariable.data[nbc.indexSet.data] = 1.0
        # note we use this condition to only capture border swarm particles
        # on the surface itself. for those directly adjacent, the deltaMeshVariable will evaluate
        # to non-zero (but less than 1.), so we need to remove those from the integration as well.
        maskFn = uw.function.branching.conditional(
                                          [  ( deltaMeshVariable > 0.999, 1. ),
                                             (                      True, 0. )   ] )

        self._fn = maskFn * nbc.flux
        self._set_fn_function = libUnderworld.Underworld._VectorSurfaceAssemblyTerm_NA__Fn__ni_SetFn

        if mesh:
            if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
                raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
            # set directly
            self._cself.geometryMesh = mesh._cself
            self._mesh = mesh

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorSurfaceAssemblyTerm_NA__Fn__ni,self)._add_to_stg_dict(componentDictionary)

class VectorAssemblyTerm_VEP__Fn(VectorAssemblyTerm):
    """
    Document!!  ... this is the viscoelastic force term assebler.  WIP.  use at own risk.
    """
    _objectsDict = { "_assemblyterm": "VectorAssemblyTerm_VEP" }

    def __init__(self, fn, mesh=None, **kwargs):
        # build parent
        super(VectorAssemblyTerm_VEP__Fn,self).__init__(**kwargs)

        self._set_fn_function = libUnderworld.Underworld._VectorAssemblyTerm_VEP_SetFneForce
        self._fn = fn

        if mesh:
            if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
                raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
            # set directly
            self._cself.geometryMesh = mesh._cself
            self._mesh = mesh

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorAssemblyTerm_VEP__Fn,self)._add_to_stg_dict(componentDictionary)


class VectorAssemblyTerm_NA__Fn(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "VectorAssemblyTerm_NA__Fn" }

    def __init__(self, fn, mesh=None, **kwargs):
        # build parent
        super(VectorAssemblyTerm_NA__Fn,self).__init__(**kwargs)

        self._set_fn_function = libUnderworld.Underworld._VectorAssemblyTerm_NA__Fn_SetFn
        self._fn = fn

        if mesh:
            if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
                raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
            # set directly
            self._cself.geometryMesh = mesh._cself
            self._mesh = mesh


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorAssemblyTerm_NA__Fn,self)._add_to_stg_dict(componentDictionary)

class GradientStiffnessMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "GradientStiffnessMatrixTerm" }
    pass

class PreconditionerMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "UzawaPreconditionerTerm" }
    pass

class ConstitutiveMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "ConstitutiveMatrixCartesian" }

    def __init__(self, fn_visc1=None, fn_visc2=None, fn_director=None, **kwargs):
        # build parent
        super(ConstitutiveMatrixTerm,self).__init__(**kwargs)

        # disable these, because this guy requires multiple functions
        self._set_fn_function = None
        self._fn = None

        if not fn_visc1:
            raise ValueError("You must provide a viscosity for the ConstitutiveMatrixTerm")
        self._fn_visc1    = fn_visc1
        self._fn_visc2    = fn_visc2
        self._fn_director = fn_director


    @property
    def fn_visc1(self):
        return self._fn_visc1

    @fn_visc1.setter
    def fn_visc1(self, value):
        _fn = uw.function.Function.convert(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn_visc1 = _fn
        libUnderworld.Underworld._ConstitutiveMatrixCartesian_Set_Fn_Visc1( self._cself, self._fn_visc1._fncself )

    @property
    def fn_visc2(self):
        return self._fn_visc2

    @fn_visc2.setter
    def fn_visc2(self, value):
        _fn = uw.function.Function.convert(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn_visc2 = _fn
        libUnderworld.Underworld._ConstitutiveMatrixCartesian_Set_Fn_Visc2( self._cself, self._fn_visc2._fncself )

    @property
    def fn_director(self):
        return self._fn_director

    @fn_director.setter
    def fn_director(self, value):
        _fn = uw.function.Function.convert(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn_director = _fn
        libUnderworld.Underworld._ConstitutiveMatrixCartesian_Set_Fn_Director( self._cself, self._fn_director._fncself )

    def _setup(self):
        # lets setup fn tings.. note that this uses the 'property' above
        self.fn_visc1    = self._fn_visc1
        if self._fn_visc2:
            self.fn_visc2    = self._fn_visc2
        if self._fn_director:
            self.fn_director = self._fn_director


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(ConstitutiveMatrixTerm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ]["dim"] = self._integrationSwarm._mesh.dim

class MatrixAssemblyTerm_NA_i__NB_i__Fn(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "MatrixAssemblyTerm_NA_i__NB_i__Fn" }

    def __init__(self, fn, **kwargs):
        # build parent
        super(MatrixAssemblyTerm_NA_i__NB_i__Fn,self).__init__(**kwargs)

        self._set_fn_function = libUnderworld.Underworld.MatrixAssemblyTerm_NA_i__NB_i__Fn_SetFn
        self._fn = fn


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MatrixAssemblyTerm_NA_i__NB_i__Fn,self)._add_to_stg_dict(componentDictionary)

class MatrixAssemblyTerm_NA__NB__Fn(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "MatrixAssemblyTerm_NA__NB__Fn" }

    def __init__(self, fn, mesh, **kwargs):
        # build parent
        super(MatrixAssemblyTerm_NA__NB__Fn,self).__init__(**kwargs)

        self._set_fn_function = libUnderworld.Underworld.MatrixAssemblyTerm_NA__NB__Fn_SetFn
        self._fn = fn

        if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
            raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
        # set mesh directly
        self._cself.geometryMesh = mesh._cself
        self._geometryMesh = mesh

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MatrixAssemblyTerm_NA__NB__Fn,self)._add_to_stg_dict(componentDictionary)

class LumpedMassMatrixVectorTerm(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "LumpedMassMatrixForceTerm" }
    pass

class AdvDiffResidualVectorTerm(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "AdvDiffResidualForceTerm" }

    def __init__( self, velocityField, diffusivity, sourceTerm, **kwargs ):
        # build parent
        super(AdvDiffResidualVectorTerm,self).__init__(**kwargs)

        if not isinstance( velocityField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        self._velocityField = velocityField

        self._diffFn = uw.function.Function.convert(diffusivity)
        if not isinstance( self._diffFn, uw.function.Function ):
            raise TypeError( "Provided 'diffusivity' must be of the type, or convertible to, 'Function' class ")

        # if sourceTerm is None make it 0.0 fn object
        self._sourceFn = uw.function.Function.convert(sourceTerm)
        if self._sourceFn == None:
            self._sourceFn = uw.function.misc.constant(0.0)

        self._set_fn_function = None
        self._fn = None

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AdvDiffResidualVectorTerm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][     "VelocityField"] = self._velocityField._cself.name
        componentDictionary[ self._cself.name ][  "UpwindXiFunction"] = "DoublyAsymptoticAssumption"

    def _setup(self):
        # lets override parent _setup definition because we use 2 function objects
        libUnderworld.Underworld._SUPGVectorTerm_NA__Fn_SetDiffusivityFn( self._cself, self._diffFn._fncself )
        libUnderworld.Underworld._SUPGVectorTerm_NA__Fn_SetSourceFn( self._cself, self._sourceFn._fncself )
