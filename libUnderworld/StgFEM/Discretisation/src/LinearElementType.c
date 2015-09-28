/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "types.h"
#include "ElementType.h"
#include "LinearElementType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

const Type LinearElementType_Type = "LinearElementType";

#define _LinearElementType_NodeCount 4

LinearElementType* LinearElementType_New( Name name ) {
	LinearElementType* self = _LinearElementType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _LinearElementType_NodeCount );
	_LinearElementType_Init( self );

	return self;
}

void* _LinearElementType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(LinearElementType);
	Type                                                                                    type = LinearElementType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _LinearElementType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _LinearElementType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _LinearElementType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _LinearElementType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _LinearElementType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _LinearElementType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _LinearElementType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _LinearElementType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _LinearElementType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _LinearElementType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_JacobianDeterminantSurfaceFunction*                  _jacobianDeterminantSurface = _LinearElementType_JacobianDeterminantSurface;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = _ElementType_SurfaceNormal;

	return _LinearElementType_New(  BILINEARELEMENTTYPE_PASSARGS  );
}

LinearElementType* _LinearElementType_New(  BILINEARELEMENTTYPE_DEFARGS  ) {
	LinearElementType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LinearElementType) );
	self = (LinearElementType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	return self;
}

void _LinearElementType_Init( LinearElementType* self ) {
	Dimension_Index dim, dim_I=0;
	/* General and Virtual info should already be set */
	
	/* set the dimensionality of the element */
	dim = self->dim = 1;

	for ( dim_I = 0; dim_I < dim; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] = 1;
		self->elLocalLength[dim_I] = self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}

	self->triInds = Memory_Alloc_2DArray( unsigned, dim, 3, (Name)"LinearElementType::triInds" );
/*
	self->triInds[0][0] = 0; self->triInds[0][1] = 1; self->triInds[0][2] = 2;
	self->triInds[1][0] = 1; self->triInds[1][1] = 3; self->triInds[1][2] = 2;
*/
}

void _LinearElementType_Delete( void* elementType ) {
	LinearElementType* self = (LinearElementType*)elementType;

	/* Stg_Class_Delete parent */
	_ElementType_Delete( self  );
}

void _LinearElementType_Print( void* elementType, Stream* stream ) {
	LinearElementType* self = (LinearElementType*)elementType;
	Dimension_Index dim_I=0;

	/* General info */
	Journal_Printf( stream, "LinearElementType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, stream );
	
	/* Virtual info */
	
	/* LinearElementType info */
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

void _LinearElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ) {
}
	
void _LinearElementType_Initialise( void* elementType, void *data ){
	LinearElementType*	self	= (LinearElementType*) elementType;

	self->faceNodes = Memory_Alloc_2DArray( unsigned, 2, 1, (Name)"node indices for element faces"  );

	self->faceNodes[0][0] = 0;
	self->faceNodes[1][0] = 1;
}
	
void _LinearElementType_Execute( void* elementType, void *data ){
}
	
void _LinearElementType_Destroy( void* elementType, void *data ){
	LinearElementType*	self	= (LinearElementType*) elementType;

	FreeArray( self->triInds );

	Memory_Free( self->faceNodes );	
	Memory_Free( self->evaluatedShapeFunc );
	Memory_Free( self->GNi );
	
	_ElementType_Destroy( self, data );
}

void _LinearElementType_Build( void* elementType, void *data ) {
	LinearElementType*	self	= (LinearElementType*) elementType;

	self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
	self->GNi = Memory_Alloc_2DArray( double, self->dim, self->nodeCount, (Name)"localShapeFuncDerivitives"  );
}

/*
 - Shape function definitions
 - Local node numbering convention for billinear element (xi, eta)
 - Local coordinate domain spans  -1 <= xi,eta <= 1

  eta
   |
3-----2
|  |__|___xi
|     |
0-----1

*/
void _LinearElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	double xi, eta;
	
	xi  = localCoord[0];
	eta = localCoord[1];
	
	evaluatedValues[0] = 0.5*( 1.0-xi );
	evaluatedValues[1] = 0.5*( 1.0+xi );
}


void _LinearElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives ) 
{		
	double xi, eta;
	
	xi  = localCoord[0];
	eta = localCoord[1];
	
	/* derivatives wrt xi */
	evaluatedDerivatives[0][0] = - 0.5;
	evaluatedDerivatives[0][1] =   0.5;
}


#if 0
/*
** Calculates the barycenter of a triangle with respect to some point.
*/

void _LinearElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		_mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord )
{
	LinearElementType*	self = (LinearElementType*)elementType;
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

double _LinearElementType_JacobianDeterminantSurface( 
		void* elementType, 
		void* _mesh, 
		unsigned element_I,
		const double localCoord[],
		unsigned face_I, 
		unsigned norm ) 
{
	LinearElementType*	self		= (LinearElementType*) elementType;
	Mesh*			mesh		= (Mesh*)_mesh;
	unsigned		surfaceDim	= ( norm + 1 ) % 2;
	double			x[2];
	double			detJac;
	Index			nodes[2];

        /* TODO */
        abort();

	ElementType_GetFaceNodes( self, mesh, element_I, face_I, 2, nodes );

	x[0] = Mesh_GetVertex( mesh, nodes[0] )[surfaceDim];
	x[1] = Mesh_GetVertex( mesh, nodes[1] )[surfaceDim];

	detJac = 0.5 * ( x[1] - x[0] );

	return fabs( detJac );
}



