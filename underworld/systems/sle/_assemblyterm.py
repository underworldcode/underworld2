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
        if not isinstance(integrationSwarm, uw.swarm.IntegrationSwarm):
            raise TypeError("'integrationSwarm' object passed in must be of type 'IntegrationSwarm'")
        self._integrationSwarm = integrationSwarm
        self._extraInfo = extraInfo

        super(AssemblyTerm,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AssemblyTerm,self)._add_to_stg_dict(componentDictionary)
        componentDictionary[ self._cself.name ][    "Swarm"] = self._integrationSwarm._cself.name
        componentDictionary[ self._cself.name ]["ExtraInfo"] = self._extraInfo


class VectorAssemblyTerm(AssemblyTerm):
    def __init__(self, assembledObject, **kwargs):
        if not isinstance( assembledObject, _assembledvector.AssembledVector):
            raise TypeError("'assembledObject' passed in must be of type 'AssembledVector'")
        self._assembledObject = assembledObject
        super(VectorAssemblyTerm,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorAssemblyTerm,self)._add_to_stg_dict(componentDictionary)
        componentDictionary[ self._cself.name ]["ForceVector"] = self._assembledObject._cself.name


class MatrixAssemblyTerm(AssemblyTerm):
    def __init__(self, assembledObject=None, **kwargs):
        if not isinstance( assembledObject, (_assembledmatrix.AssembledMatrix, type(None))):
            raise TypeError("'assembledObject' passed in must be of type 'AssembledMatrix'")
        self._assembledObject = assembledObject
        super(MatrixAssemblyTerm,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MatrixAssemblyTerm,self)._add_to_stg_dict(componentDictionary)
        if self._assembledObject:
            componentDictionary[ self._cself.name ]["StiffnessMatrix"] = self._assembledObject._cself.name


class VectorAssemblyTerm_NA__Fn(VectorAssemblyTerm):
    """
    """
    _objectsDict = { "_assemblyterm": "VectorAssemblyTerm_NA__Fn" }

    def __init__(self, fn, **kwargs):
        """
        """
        
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(fn)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn = _fn

        # build parent
        super(VectorAssemblyTerm_NA__Fn,self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorAssemblyTerm_NA__Fn,self)._add_to_stg_dict(componentDictionary)

    def _setup(self):
        # lets setup fn tings
        libUnderworld.Underworld._VectorAssemblyTerm_NA__Fn_SetFn( self._cself, self._fn._fncself )

    @property
    def fn(self):
        return self._fn

    @fn.setter
    def fn(self, value):
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn = _fn
        libUnderworld.Underworld._VectorAssemblyTerm_NA__Fn_SetFn( self._cself, self._fn._fncself )



class GradientStiffnessMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "GradientStiffnessMatrixTerm" }
    pass

class PreconditionerMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "UzawaPreconditionerTerm" }
    pass

class ConstitutiveMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "ConstitutiveMatrixCartesian" }

    def __init__(self, fn, **kwargs):
        """
        """
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(fn)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn = _fn

        # build parent
        super(ConstitutiveMatrixTerm,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(ConstitutiveMatrixTerm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ]["dim"] = self._integrationSwarm._feMesh.dim

    def _setup(self):
        # lets setup fn tings
        libUnderworld.Underworld._ConstitutiveMatrixCartesian_SetFn( self._cself, self._fn._fncself )

    @property
    def fn(self):
        return self._fn

    @fn.setter
    def fn(self, value):
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn = _fn
        libUnderworld.Underworld._ConstitutiveMatrixCartesian_SetFn( self._cself, self._fn._fncself )


class MatrixAssemblyTerm_NA_i__NB_i__Fn(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "MatrixAssemblyTerm_NA_i__NB_i__Fn" }

    def __init__(self, fn, **kwargs):
        """
        """
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(fn)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn = _fn

        # build parent
        super(MatrixAssemblyTerm_NA_i__NB_i__Fn,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MatrixAssemblyTerm_NA_i__NB_i__Fn,self)._add_to_stg_dict(componentDictionary)

    def _setup(self):
        # lets setup fn tings
        libUnderworld.Underworld.MatrixAssemblyTerm_NA_i__NB_i__Fn_SetFn( self._cself, self._fn._fncself )

    @property
    def fn(self):
        return self._fn

    @fn.setter
    def fn(self, value):
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
        self._fn = _fn
        libUnderworld.Underworld.MatrixAssemblyTerm_NA_i__NB_i__Fn_SetFn( self._cself, self._fn._fncself )


class LumpedMassMatrixVectorTerm(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "LumpedMassMatrixForceTerm" }
    pass

class AdvDiffResidualVectorTerm(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "AdvDiffResidualForceTerm" }

    def __init__( self, velocityField, diffusivity, **kwargs ):

        if not isinstance( velocityField, uw.meshvariable.MeshVariable):
            raise TypeError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        self._velocityField = velocityField

        if not isinstance( diffusivity, (int,float) ):
            raise TypeError( "Provided 'diffusivity' must be 'float' type." )
        if float(diffusivity) <= 0:
            raise ValueError( "Provided 'diffusivity' must take positive values." )
        self._diffusivity = diffusivity

        # build parent
        super(AdvDiffResidualVectorTerm,self).__init__(**kwargs)


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AdvDiffResidualVectorTerm,self)._add_to_stg_dict(componentDictionary)
        
        componentDictionary[ self._cself.name ][     "VelocityField"] = self._velocityField._cself.name
        componentDictionary[ self._cself.name ]["defaultDiffusivity"] = self._diffusivity
        componentDictionary[ self._cself.name ][  "UpwindXiFunction"] = "DoublyAsymptoticAssumption"


class PressureMassMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "PressMassMatrixTerm" }

    def __init__( self, mesh, **kwargs ):

        if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
            raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
        self._mesh = mesh

        super(PressureMassMatrixTerm,self).__init__(**kwargs)

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(PressureMassMatrixTerm,self)._add_to_stg_dict(componentDictionary)
        #Need GeometryMesh to to be able to calculate detJac in PressureMatrixTerm function
        componentDictionary[ self._cself.name ]["GeometryMesh"] = self._mesh._cself.name

class VelocityMassMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "VelocityMassMatrixTerm" }
    pass

