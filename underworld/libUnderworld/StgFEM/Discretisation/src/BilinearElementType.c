/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>

#include "types.h"

#include "ElementType.h"
#include "BilinearElementType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

const Type BilinearElementType_Type = "BilinearElementType";

#define _BilinearElementType_NodeCount 4

BilinearElementType* BilinearElementType_New( Name name ) {
	BilinearElementType* self = _BilinearElementType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _BilinearElementType_NodeCount );
	_BilinearElementType_Init( self );

	return self;
}

void* _BilinearElementType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(BilinearElementType);
	Type                                                                                    type = BilinearElementType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _BilinearElementType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _BilinearElementType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _BilinearElementType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _BilinearElementType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _BilinearElementType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _BilinearElementType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _BilinearElementType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _BilinearElementType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _BilinearElementType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _BilinearElementType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = _ElementType_SurfaceNormal;

	return _BilinearElementType_New(  BILINEARELEMENTTYPE_PASSARGS  );
}

BilinearElementType* _BilinearElementType_New(  BILINEARELEMENTTYPE_DEFARGS  ) {
	BilinearElementType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(BilinearElementType) );
	self = (BilinearElementType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	return self;
}

void _BilinearElementType_Init( BilinearElementType* self ) {
	Dimension_Index dim, dim_I=0;
	/* General and Virtual info should already be set */
	
	/* set the dimensionality of the element */
	dim = self->dim = 2;

	for ( dim_I = 0; dim_I < dim; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] = 1;
		self->elLocalLength[dim_I] = self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}

	self->triInds = Memory_Alloc_2DArray( unsigned, dim, 3, (Name)"BilinearElementType::triInds" );
	self->triInds[0][0] = 0; self->triInds[0][1] = 1; self->triInds[0][2] = 2;
	self->triInds[1][0] = 1; self->triInds[1][1] = 3; self->triInds[1][2] = 2;
}

void _BilinearElementType_Delete( void* elementType ) {
	BilinearElementType* self = (BilinearElementType*)elementType;

	/* Stg_Class_Delete parent */
	_ElementType_Delete( self  );
}

void _BilinearElementType_Print( void* elementType, Stream* stream ) {
	BilinearElementType* self = (BilinearElementType*)elementType;
	Dimension_Index dim_I=0;

	/* General info */
	Journal_Printf( stream, "BilinearElementType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, stream );
	
	/* Virtual info */
	
	/* BilinearElementType info */
	Journal_Printf( stream, "self->minElLocalCoord: (", self );
	for ( dim_I = 0; dim_I < 2; dim_I++ ) {
		Journal_Printf( stream, "%0.5f,", self->minElLocalCoord[dim_I] );
	}
	Journal_Printf( stream, ")\n", self );
	Journal_Printf( stream, "self->maxElLocalCoord: (", self );
	for ( dim_I = 0; dim_I < 2; dim_I++ ) {
		Journal_Printf( stream, "%0.5f,", self->maxElLocalCoord[dim_I] );
	}	
	Journal_Printf( stream, ")\n", self );
	Journal_Printf( stream, "self->elLocalLength: (", self );
	for ( dim_I = 0; dim_I < 2; dim_I++ ) {
		Journal_Printf( stream, "%0.5f,", self->elLocalLength[dim_I] );
	}
	Journal_Printf( stream, ")\n", self );
}

void _BilinearElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ) {
}
	
void _BilinearElementType_Initialise( void* elementType, void *data ){
	BilinearElementType*	self	= (BilinearElementType*) elementType;

	self->faceNodes = Memory_Alloc_2DArray( unsigned, 4, 2, (Name)"node indices for element faces"  );

	self->faceNodes[0][0] = 0; self->faceNodes[0][1] = 1;
	self->faceNodes[1][0] = 2; self->faceNodes[1][1] = 3;
	self->faceNodes[2][0] = 0; self->faceNodes[2][1] = 2;
	self->faceNodes[3][0] = 1; self->faceNodes[3][1] = 3;
}
	
void _BilinearElementType_Execute( void* elementType, void *data ){
}
	
void _BilinearElementType_Destroy( void* elementType, void *data ){
	BilinearElementType*	self	= (BilinearElementType*) elementType;

	FreeArray( self->triInds );

	Memory_Free( self->faceNodes );	
	Memory_Free( self->evaluatedShapeFunc );
	Memory_Free( self->GNi );
	
	_ElementType_Destroy( self, data );
}

void _BilinearElementType_Build( void* elementType, void *data ) {
	BilinearElementType*	self	= (BilinearElementType*) elementType;

	self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
	self->GNi = Memory_Alloc_2DArray( double, self->dim, self->nodeCount, (Name)"localShapeFuncDerivitives"  );
}

/*
 - Shape function definitions
 - Local node numbering convention for billinear element (xi, eta)
 - Local coordinate domain spans  -1 <= xi,eta <= 1

  eta
   |
2-----3
|  |__|___xi
|     |
0-----1

*/
void _BilinearElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	double xi, eta;
	
	xi  = localCoord[0];
	eta = localCoord[1];
	
	evaluatedValues[0] = 0.25*( 1.0-xi )*( 1.0-eta );
	evaluatedValues[1] = 0.25*( 1.0+xi )*( 1.0-eta );
	evaluatedValues[2] = 0.25*( 1.0-xi )*( 1.0+eta );
	evaluatedValues[3] = 0.25*( 1.0+xi )*( 1.0+eta );
}


void _BilinearElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives ) 
{		
	double xi, eta;
	
	xi  = localCoord[0];
	eta = localCoord[1];
	
	/* derivatives wrt xi */
	evaluatedDerivatives[0][0] = - 0.25*( 1.0 - eta );
	evaluatedDerivatives[0][1] =   0.25*( 1.0 - eta );
	evaluatedDerivatives[0][3] =   0.25*( 1.0 + eta );
	evaluatedDerivatives[0][2] = - 0.25*( 1.0 + eta );
	
	/* derivatives wrt eta */
	evaluatedDerivatives[1][0] = - 0.25*( 1.0 - xi );
	evaluatedDerivatives[1][1] = - 0.25*( 1.0 + xi );
	evaluatedDerivatives[1][3] =   0.25*( 1.0 + xi );
	evaluatedDerivatives[1][2] =   0.25*( 1.0 - xi );
}


#if 0
/*
** Calculates the barycenter of a triangle with respect to some point.
*/

void _BilinearElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		_mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord )
{
	BilinearElementType*	self = (BilinearElementType*)elementType;
	Mesh*			mesh = (Mesh*)_mesh;
	unsigned		inside;
	double			bc[3];
	static double		lCrds[4][3] = {{-1.0, -1.0, 0.0}, {1.0, -1.0, 0.0}, 
					       {-1.0, 1.0, 0.0}, {1.0, 1.0, 0.0}};
	unsigned		nInc, *inc;
	unsigned		bc_i;

	Mesh_GetIncidence( mesh, MT_FACE, element, MT_VERTEX, self->inc );
	nInc = IArray_GetSize( self->inc );
	inc = IArray_GetPtr( self->inc );
	assert( nInc == 4 );

	insist( Simplex_Search2D( mesh->verts, inc, 2, self->triInds, (double*)globalCoord, bc, &inside ), == True );

	elLocalCoord[0] = bc[0] * lCrds[self->triInds[inside][0]][0];
	elLocalCoord[1] = bc[0] * lCrds[self->triInds[inside][0]][1];
	for( bc_i = 1; bc_i < 3; bc_i++ ) {
		elLocalCoord[0] += bc[bc_i] * lCrds[self->triInds[inside][bc_i]][0];
		elLocalCoord[1] += bc[bc_i] * lCrds[self->triInds[inside][bc_i]][1];
	}
}
#endif
