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
#include "BilinearInnerElType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type BilinearInnerElType_Type = "BilinearInnerElType";

#define _BilinearInnerElType_NodeCount 3

BilinearInnerElType* BilinearInnerElType_New( Name name ) {
	BilinearInnerElType* self = BilinearInnerElType_DefaultNew( name );

	self->isConstructed = True;	
	_ElementType_Init( (ElementType*)self, _BilinearInnerElType_NodeCount );
	_BilinearInnerElType_Init( self );

	return self;
}

void* BilinearInnerElType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(BilinearInnerElType);
	Type                                                                                    type = BilinearInnerElType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _BilinearInnerElType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _BilinearInnerElType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = BilinearInnerElType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _BilinearInnerElType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _BilinearInnerElType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _BilinearInnerElType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _BilinearInnerElType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _BilinearInnerElType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _BilinearInnerElType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _BilinearInnerElType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_JacobianDeterminantSurfaceFunction*                  _jacobianDeterminantSurface = _ElementType_JacobianDeterminantSurface;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = _BilinearInnerElType_SurfaceNormal;

	return _BilinearInnerElType_New(  BILINEARINNERELTYPE_PASSARGS  );
}

BilinearInnerElType* _BilinearInnerElType_New(  BILINEARINNERELTYPE_DEFARGS  ) {
	BilinearInnerElType*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(BilinearInnerElType) );
	self = (BilinearInnerElType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	return self;
}

void _BilinearInnerElType_Init( BilinearInnerElType* self ) {
	Dimension_Index dim_I=0;
	/* General and Virtual info should already be set */
	
	/* BilinearInnerElType info */
	self->isConstructed = True;
	for ( dim_I = 0; dim_I < 2; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] = 1;
		self->elLocalLength[dim_I] = self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}

	self->triInds = Memory_Alloc_2DArray( unsigned, 2, 3, (Name)"BilinearInnerElType::triInds" );
	self->triInds[0][0] = 0; self->triInds[0][1] = 1; self->triInds[0][2] = 2;
	self->triInds[1][0] = 1; self->triInds[1][1] = 3; self->triInds[1][2] = 2;
}

void _BilinearInnerElType_Delete( void* elementType ) {
	BilinearInnerElType* self = (BilinearInnerElType*)elementType;

	/* Stg_Class_Delete parent */
	_ElementType_Delete( self  );
}

void _BilinearInnerElType_Print( void* elementType, Stream* stream ) {
	BilinearInnerElType* self = (BilinearInnerElType*)elementType;
	Dimension_Index dim_I=0;

	/* General info */
	Journal_Printf( stream, "BilinearInnerElType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, stream );
	
	/* Virtual info */
	
	/* BilinearInnerElType info */
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

void _BilinearInnerElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
}
	
void _BilinearInnerElType_Initialise( void* elementType, void *data ){
}
	
void _BilinearInnerElType_Execute( void* elementType, void *data ){
}
	
void _BilinearInnerElType_Destroy( void* elementType, void *data ){
	BilinearInnerElType* self = (BilinearInnerElType*)elementType;

	FreeArray( self->triInds );
        Memory_Free( self->evaluatedShapeFunc );
        Memory_Free( self->GNi );

	_ElementType_Destroy( self, data );
}

void _BilinearInnerElType_Build( void* elementType, void *data ) {
	BilinearInnerElType* self = (BilinearInnerElType*)elementType;

	self->dim=2;
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
void _BilinearInnerElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	double xi, eta;
	
	xi  = localCoord[0];
	eta = localCoord[1];

	evaluatedValues[0] = - xi - 0.5*eta + 0.25;
	evaluatedValues[1] = xi - 0.5*eta + 0.25;
	evaluatedValues[2] = eta + 0.5;
}


void _BilinearInnerElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives ) 
{		
	double xi, eta;
	
	xi  = localCoord[0];
	eta = localCoord[1];

	evaluatedDerivatives[0][0] = - 1.0;
	evaluatedDerivatives[0][1] = 1.0;
	evaluatedDerivatives[0][2] = 0.0;

	evaluatedDerivatives[1][0] = - 0.5;
	evaluatedDerivatives[1][1] = - 0.5;
	evaluatedDerivatives[1][2] = 1.0;
}

int _BilinearInnerElType_SurfaceNormal( void* elementType, unsigned element_I, unsigned dim, double* xi, double* norm ) {
	Stream*	errStream = Journal_Register( ErrorStream_Type, (Name)ElementType_Type  );

	Journal_Printf( errStream, "surface normal function not yet implemented for this element type.\n" );
	assert( 0 );

	norm = NULL;

	return -1;
}



