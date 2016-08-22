/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "Discretisation.h"

/* Textual name of this class */
const Type FeMesh_Type = "FeMesh";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

FeMesh* FeMesh_New( Name name ) {
   FeMesh* self = _FeMesh_DefaultNew( name );
   _Mesh_Init( (Mesh*)self );
	/* FeMesh info */
	_FeMesh_Init( self, NULL, NULL, False ); /* this is a useless Init() */

   return self;
}

FeMesh* _FeMesh_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(FeMesh);
	Type                                                      type = FeMesh_Type;
	Stg_Class_DeleteFunction*                              _delete = _FeMesh_Delete;
	Stg_Class_PrintFunction*                                _print = _FeMesh_Print;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void* (*)(Name))_FeMesh_New;
	Stg_Component_ConstructFunction*                    _construct = _FeMesh_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _FeMesh_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _FeMesh_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _FeMesh_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _FeMesh_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	/* The following terms are parameters that have been passed into or defined in this function but are being set before being passed onto the parent */
	Stg_Class_CopyFunction*        _copy = NULL;

   return _FeMesh_New(  FEMESH_PASSARGS  );
}

FeMesh* _FeMesh_New(  FEMESH_DEFARGS  ) {
	FeMesh*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(FeMesh) );
	self = (FeMesh*)_Mesh_New(  MESH_PASSARGS  );

	return self;
}

void _FeMesh_Init( FeMesh* self, ElementType* elType, const char* family, Bool elementMesh ) {
	Stream*	stream;

	assert( self && Stg_CheckType( self, FeMesh ) );

	stream = Journal_Register( Info_Type, (Name)self->type  );
	Stream_SetPrintingRank( stream, 0 );

	self->feElType = elType;
	self->feElFamily = family;
	self->elementMesh = elementMesh;

   /* checkpoint non-constant meshes */
   if ( self->feElFamily && strcmp( self->feElFamily, "constant" ) ){
      self->isCheckpointedAndReloaded = True;
   }
	
	self->inc = IArray_New();
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _FeMesh_Delete( void* feMesh ) {
	FeMesh*	self = (FeMesh*)feMesh;
	/* Delete the parent. */
	_Mesh_Delete( self );
}

void _FeMesh_Print( void* feMesh, Stream* stream ) {
	FeMesh*	self = (FeMesh*)feMesh;
	
	/* Print parent */
	Journal_Printf( stream, "FeMesh (ptr): (%p)\n", self );
	_Mesh_Print( self, stream );
}

void _FeMesh_AssignFromXML( void* feMesh, Stg_ComponentFactory* cf, void* data ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	_Mesh_AssignFromXML( self, cf, data );

        self->useFeAlgorithms = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"UseFeAlgorithms", True );

	_FeMesh_Init( self, NULL, Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"elementType", "linear"  ), 
		Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"isElementMesh", False )  );
}

void _FeMesh_Build( void* feMesh, void* data ) {
	FeMesh*		self = (FeMesh*)feMesh;
	Stream*		stream;
	ElementType*	elType;

	assert( self );

	stream = Journal_Register( Info_Type, (Name)self->type  );

	_Mesh_Build( self, data );

    /* add this check incase the FeMesh hasn't been provided with a generator yet, in which case there's nothing to do */
	if( !Mesh_GetDimSize( self ) ){
        self->isBuilt = False;
        return;
    }

	Stream_Indent( stream );
//	Journal_Printf( stream, "Assigning FeMesh element types...\n" );
	Stream_Indent( stream );

	if( !strcmp( self->feElFamily, "Q2" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)Triquadratic_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)Biquadratic_New( "" );
		else
			abort();
	}
	else if( !strcmp( self->feElFamily, "Q1" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)TrilinearElementType_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)BilinearElementType_New( "" );
		else if( nDims == 1 )
                    elType = (ElementType*)LinearElementType_New( "" );
                else
			abort();
	}
	else if( !strcmp( self->feElFamily, "DQ1" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)dQ13DElType_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)dQ12DElType_New( "" );
        else
			abort();
	}
	else if( !strcmp( self->feElFamily, "DPC1" ) ) {
		unsigned	nDims;

		nDims = Mesh_GetDimSize( self );
		if( nDims == 3 )
			elType = (ElementType*)TrilinearInnerElType_New( "" );
		else if( nDims == 2 )
			elType = (ElementType*)BilinearInnerElType_New( "" );
		else
			abort();
	}
	else if( !strcmp( self->feElFamily, "DQ0" ) ) {
		elType = (ElementType*)ConstantElementType_New( "" );
	}
	else
		abort();
	FeMesh_SetElementType( self, elType );
	if( self->feElType )
		Stg_Component_Build( self->feElType, data, False );

    if( !self->elementMesh && self->useFeAlgorithms ) {
        /* We need to swap to the FeMesh element type because the
          geometric versions do not produce the same results. */
        Stg_Class_Delete( self->elTypes[0] );
        self->elTypes[0] = 
           (FeMesh_ElementType*)FeMesh_ElementType_New();
        Mesh_ElementType_SetMesh( self->elTypes[0], self );
        Mesh_ElementType_Update( self->elTypes[0] );
    }

//	Journal_Printf( stream, "... FE element types are '%s',\n", elType->type );
//	Journal_Printf( stream, "... done.\n" );
	Stream_UnIndent( stream );
	Stream_UnIndent( stream );
}

void _FeMesh_Initialise( void* feMesh, void* data ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	_Mesh_Initialise( self, data );

	if( self->feElType )
		Stg_Component_Initialise( self->feElType, data, False );
}

void _FeMesh_Execute( void* feMesh, void* data ) {
}

void _FeMesh_Destroy( void* feMesh, void* data ) {
	FeMesh*	self = (FeMesh*)feMesh;
   
	FeMesh_Destruct( self );
	Stg_Class_Delete( self->inc );
   _Mesh_Destroy( self, data );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void FeMesh_SetElementType( void* feMesh, ElementType* elType ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

   if( self->feElType ) Stg_Class_Delete( self->feElType );
	self->feElType = elType;
}

void FeMesh_SetElementFamily( void* feMesh, const char* family ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	self->feElFamily = (char*)family;
}

ElementType* FeMesh_GetElementType( void* feMesh, unsigned element ) {
	FeMesh*	self = (FeMesh*)feMesh;

	assert( self );

	return self->feElType;
}

unsigned FeMesh_GetNodeLocalSize( void* feMesh ) {
	return Mesh_GetLocalSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetNodeRemoteSize( void* feMesh ) {
	return Mesh_GetRemoteSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetNodeDomainSize( void* feMesh ) {
	return Mesh_GetDomainSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetNodeGlobalSize( void* feMesh ) {
	return Mesh_GetGlobalSize( feMesh, MT_VERTEX );
}

unsigned FeMesh_GetElementLocalSize( void* feMesh ) {
	return Mesh_GetLocalSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementRemoteSize( void* feMesh ) {
	return Mesh_GetRemoteSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementDomainSize( void* feMesh ) {
	return Mesh_GetDomainSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementGlobalSize( void* feMesh ) {
	return Mesh_GetGlobalSize( feMesh, Mesh_GetDimSize( feMesh ) );
}

unsigned FeMesh_GetElementNodeSize( void* feMesh, unsigned element ) {
	return Mesh_GetIncidenceSize( feMesh, Mesh_GetDimSize( feMesh ), element, MT_VERTEX );
}

unsigned FeMesh_GetNodeElementSize( void* feMesh, unsigned node ) {
	return Mesh_GetIncidenceSize( feMesh, MT_VERTEX, node, Mesh_GetDimSize( feMesh ) );
}

void FeMesh_GetElementNodes( void* feMesh, unsigned element, IArray* inc ) {
	Mesh_GetIncidence( feMesh, Mesh_GetDimSize( feMesh ), element, MT_VERTEX, inc );
}

void FeMesh_GetNodeElements( void* feMesh, unsigned node, IArray* inc ) {
	Mesh_GetIncidence( feMesh, MT_VERTEX, node, Mesh_GetDimSize( feMesh ), inc );
}

unsigned FeMesh_ElementDomainToGlobal( void* feMesh, unsigned domain ) {
	return Mesh_DomainToGlobal( feMesh, Mesh_GetDimSize( feMesh ), domain );
}

Bool FeMesh_ElementGlobalToDomain( void* feMesh, unsigned global, unsigned* domain ) {
	return Mesh_GlobalToDomain( feMesh, Mesh_GetDimSize( feMesh ), global, domain );
}

unsigned FeMesh_NodeDomainToGlobal( void* feMesh, unsigned domain ) {
	return Mesh_DomainToGlobal( feMesh, MT_VERTEX, domain );
}

Bool FeMesh_NodeGlobalToDomain( void* feMesh, unsigned global, unsigned* domain ) {
	return Mesh_GlobalToDomain( feMesh, MT_VERTEX, global, domain );
}

void FeMesh_CoordGlobalToLocal( void* feMesh, unsigned element, const double* global, double* local ) {
	FeMesh*		self = (FeMesh*)feMesh;
	ElementType*	elType;

	assert( self );
	assert( element < FeMesh_GetElementDomainSize( self ) );
	assert( global );
	assert( local );

	elType = FeMesh_GetElementType( self, element );
	ElementType_ConvertGlobalCoordToElLocal( elType, self, element, global, local );
}

void FeMesh_CoordLocalToGlobal( void* feMesh, unsigned element, const double* local, double* global ) {
	FeMesh*		self = (FeMesh*)feMesh;
	unsigned	nDims;
	ElementType*	elType;
	double*		basis;
	unsigned	nElNodes, *elNodes;
	double		dimBasis;
	double*		vert;
	unsigned	n_i, d_i;

	assert( self );
	assert( element < FeMesh_GetElementDomainSize( self ) );
	assert( global );
	assert( local );

	nDims = Mesh_GetDimSize( self );
	elType = FeMesh_GetElementType( self, element );
	FeMesh_GetElementNodes( self, element, self->inc );
	nElNodes = IArray_GetSize( self->inc );
	elNodes = IArray_GetPtr( self->inc );
	basis = AllocArray( double, nElNodes );
	ElementType_EvaluateShapeFunctionsAt( elType, local, basis );

	memset( global, 0, nDims * sizeof(double) );
	for( n_i = 0; n_i < nElNodes; n_i++ ) {
		dimBasis = basis[n_i];
		vert = Mesh_GetVertex( self, elNodes[n_i] );
		for( d_i = 0; d_i < nDims; d_i++ )
			global[d_i] += dimBasis * vert[d_i];
	}

	FreeArray( basis );
}

void FeMesh_EvalBasis( void* feMesh, unsigned element, double* localCoord, double* basis ) {
	FeMesh*		self = (FeMesh*)feMesh;
	ElementType*	elType;

	assert( self );
	assert( localCoord );

	elType = FeMesh_GetElementType( self, element );
	ElementType_EvaluateShapeFunctionsAt( elType, localCoord, basis );
}

void FeMesh_EvalLocalDerivs( void* feMesh, unsigned element, double* localCoord, double** derivs ) {
	FeMesh*		self = (FeMesh*)feMesh;
	ElementType*	elType;

	assert( self );
	assert( localCoord );
	assert( derivs );

	elType = FeMesh_GetElementType( self, element );
	ElementType_EvaluateShapeFunctionLocalDerivsAt( elType, localCoord, derivs );
}

void FeMesh_EvalGlobalDerivs( void* feMesh, unsigned element, double* localCoord, double** derivs, double* jacDet ) {
	FeMesh*		self = (FeMesh*)feMesh;
	unsigned	nDims;
	ElementType*	elType;
	double		jd;

	assert( self );
	assert( localCoord );
	assert( derivs );

	nDims = Mesh_GetDimSize( self );
	elType = FeMesh_GetElementType( self, element );
	ElementType_ShapeFunctionsGlobalDerivs( elType, self, element, localCoord, nDims, 
						&jd, derivs );
	if( jacDet )
		*jacDet = jd;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void FeMesh_Destruct( FeMesh* self ) {
   Stg_Class_Delete( self->feElType );
	self->feElFamily = NULL;
	/* Disabling the killing of this object from within this
	component as this will be destroyed by the LiveComponentRegister_DestroyAll function 101109 */
	/*KillObject( self->feElType );*/ 
}
