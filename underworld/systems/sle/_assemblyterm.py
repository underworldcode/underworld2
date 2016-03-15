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
        _fn = uw.function.Function._CheckIsFnOrConvertOrThrow(value)
        if not isinstance( _fn, uw.function.Function):
            raise ValueError( "Provided 'fn' must be of or convertible to 'Function' class." )
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
    Assembly term for a Neumann condition
    """
    _objectsDict = { "_assemblyterm": "VectorSurfaceAssemblyTerm_NA__Fn__ni" }

    def __init__(self, fluxCond, mesh=None, **kwargs):
        """
        """
        # build parent
        super(VectorSurfaceAssemblyTerm_NA__Fn__ni,self).__init__(**kwargs)

        if not isinstance(fluxCond, uw.conditions.NeumannCondition):
            raise ValueError( "Provided 'fluxCond' must be a NeumannCondition class." )
        self._fluxCond = fluxCond

        self._fn = fluxCond.gradientField
        self._set_fn_function = libUnderworld.Underworld._VectorSurfaceAssemblyTerm_NA__Fn__ni_SetFn

        # pass the NeumannConditions to the SurfaceAssemblyTerm so it knows which nodes to assemble the flux contribution
        libUnderworld.Underworld._VectorSurfaceAssemblyTerm_SetBNodes( self._cself, self._fluxCond._cself )

        if mesh:
            if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
                raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
            # set directly
            self._cself.geometryMesh = mesh._cself
            self._mesh = mesh

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(VectorSurfaceAssemblyTerm_NA__Fn__ni,self)._add_to_stg_dict(componentDictionary)

class VectorAssemblyTerm_NA__Fn(VectorAssemblyTerm):
    """
    """
    _objectsDict = { "_assemblyterm": "VectorAssemblyTerm_NA__Fn" }

    def __init__(self, fn, mesh=None, **kwargs):
        """
        """
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

    def __init__(self, fn, **kwargs):
        """
        """
        # build parent
        super(ConstitutiveMatrixTerm,self).__init__(**kwargs)

        self._set_fn_function = libUnderworld.Underworld._ConstitutiveMatrixCartesian_SetFn
        self._fn = fn

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(ConstitutiveMatrixTerm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ]["dim"] = self._integrationSwarm._mesh.dim

class MatrixAssemblyTerm_NA_i__NB_i__Fn(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "MatrixAssemblyTerm_NA_i__NB_i__Fn" }

    def __init__(self, fn, **kwargs):
        """
        """
        # build parent
        super(MatrixAssemblyTerm_NA_i__NB_i__Fn,self).__init__(**kwargs)

        self._set_fn_function = libUnderworld.Underworld.MatrixAssemblyTerm_NA_i__NB_i__Fn_SetFn
        self._fn = fn


    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MatrixAssemblyTerm_NA_i__NB_i__Fn,self)._add_to_stg_dict(componentDictionary)

class LumpedMassMatrixVectorTerm(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "LumpedMassMatrixForceTerm" }
    pass

class AdvDiffResidualVectorTerm(VectorAssemblyTerm):
    _objectsDict = { "_assemblyterm": "AdvDiffResidualForceTerm" }

    def __init__( self, velocityField, fn, **kwargs ):
        # build parent
        super(AdvDiffResidualVectorTerm,self).__init__(**kwargs)

        if not isinstance( velocityField, uw.mesh.MeshVariable):
            raise TypeError( "Provided 'velocityField' must be of 'MeshVariable' class." )
        self._velocityField = velocityField

        self._set_fn_function = libUnderworld.Underworld._SUPGVectorTerm_NA__Fn_SetFn
        self._fn = fn

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(AdvDiffResidualVectorTerm,self)._add_to_stg_dict(componentDictionary)

        componentDictionary[ self._cself.name ][     "VelocityField"] = self._velocityField._cself.name
        componentDictionary[ self._cself.name ][  "UpwindXiFunction"] = "DoublyAsymptoticAssumption"


class MassMatrixTerm(MatrixAssemblyTerm):
    _objectsDict = { "_assemblyterm": "MassMatrixTerm" }

    def __init__( self, mesh, **kwargs ):
        super(MassMatrixTerm,self).__init__(**kwargs)

        if not isinstance( mesh, uw.mesh.FeMesh_Cartesian ):
            raise TypeError( "The provided mesh must be of FeMesh_Cartesian class.")
        self._mesh = mesh

    def _add_to_stg_dict(self,componentDictionary):
        # call parents method
        super(MassMatrixTerm,self)._add_to_stg_dict(componentDictionary)
        #Need GeometryMesh to to be able to calculate detJac in PressureMatrixTerm function
        componentDictionary[ self._cself.name ]["GeometryMesh"] = self._mesh._cself.name
