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
#include "TrilinearInnerElType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type TrilinearInnerElType_Type = "TrilinearInnerElType";

#define _TrilinearInnerElType_NodeCount 4

void* _TrilinearInnerElType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(TrilinearInnerElType);
	Type                                                                                    type = TrilinearInnerElType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _TrilinearInnerElType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _TrilinearInnerElType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _TrilinearInnerElType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _TrilinearInnerElType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _TrilinearInnerElType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _TrilinearInnerElType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _TrilinearInnerElType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _TrilinearInnerElType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _TrilinearInnerElType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _TrilinearInnerElType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = NULL;

	return _TrilinearInnerElType_New(  TRILINEARINNERELTYPE_PASSARGS  );
}

TrilinearInnerElType* TrilinearInnerElType_New( Name name ) {
	TrilinearInnerElType* self = _TrilinearInnerElType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _TrilinearInnerElType_NodeCount );
	_TrilinearInnerElType_Init( self );

	return self;
}

TrilinearInnerElType* _TrilinearInnerElType_New(  TRILINEARINNERELTYPE_DEFARGS  ) {
	TrilinearInnerElType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(TrilinearInnerElType) );
	self = (TrilinearInnerElType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* TrilinearInnerElType info */
	
	return self;
}

void _TrilinearInnerElType_Init( TrilinearInnerElType* self ) {
	Dimension_Index dim_I=0;

	/* General and Virtual info should already be set */
	
	/* TriInnerEllementType info */
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] = 1;
		self->elLocalLength[dim_I] = self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}

	/* Set up the tetrahedral indices. */
	self->tetInds = Memory_Alloc_2DArray( unsigned, 10, 4, (Name)"Mesh_HexType::tetInds" );
	self->tetInds[0][0] = 0; self->tetInds[0][1] = 1; self->tetInds[0][2] = 2; self->tetInds[0][3] = 4;
	self->tetInds[1][0] = 1; self->tetInds[1][1] = 2; self->tetInds[1][2] = 3; self->tetInds[1][3] = 7;
	self->tetInds[2][0] = 1; self->tetInds[2][1] = 4; self->tetInds[2][2] = 5; self->tetInds[2][3] = 7;
	self->tetInds[3][0] = 2; self->tetInds[3][1] = 4; self->tetInds[3][2] = 6; self->tetInds[3][3] = 7;
	self->tetInds[4][0] = 1; self->tetInds[4][1] = 2; self->tetInds[4][2] = 4; self->tetInds[4][3] = 7;
	self->tetInds[5][0] = 0; self->tetInds[5][1] = 1; self->tetInds[5][2] = 3; self->tetInds[5][3] = 5;
	self->tetInds[6][0] = 0; self->tetInds[6][1] = 4; self->tetInds[6][2] = 5; self->tetInds[6][3] = 6;
	self->tetInds[7][0] = 0; self->tetInds[7][1] = 2; self->tetInds[7][2] = 3; self->tetInds[7][3] = 6;
	self->tetInds[8][0] = 3; self->tetInds[8][1] = 5; self->tetInds[8][2] = 6; self->tetInds[8][3] = 7;
	self->tetInds[9][0] = 0; self->tetInds[9][1] = 3; self->tetInds[9][2] = 5; self->tetInds[9][3] = 6;
}

void _TrilinearInnerElType_Delete( void* elementType ) {
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;

	/* Stg_Class_Delete parent*/
	_ElementType_Delete( self  );
}

void _TrilinearInnerElType_Print( void* elementType, Stream* stream ) {
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* trilinearInnerElTypeStream = stream;
	
	/* General info */
	Journal_Printf( trilinearInnerElTypeStream, "TrilinearInnerElType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, trilinearInnerElTypeStream );
	
	/* Virtual info */
	
	/* TrilinearInnerElType info */
}

void _TrilinearInnerElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
	
}
	
void _TrilinearInnerElType_Initialise( void* elementType, void *data ){
	
}
	
void _TrilinearInnerElType_Execute( void* elementType, void *data ){
	
}
	
void _TrilinearInnerElType_Destroy( void* elementType, void *data ){
	TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;
	
	FreeArray( self->tetInds );
    Memory_Free( self->evaluatedShapeFunc );
    Memory_Free( self->GNi );

	_ElementType_Destroy( self, data );
}

void _TrilinearInnerElType_Build( void* elementType, void *data ) {
    TrilinearInnerElType* self = (TrilinearInnerElType*)elementType;

    self->dim=3;
    self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
    self->GNi = Memory_Alloc_2DArray( double, self->dim, self->nodeCount, (Name)"localShapeFuncDerivitives"  );

}

#if 0
void _TrilinearInnerElType_ConvertGlobalCoordToElementLocal( void* elementType, Element_DomainIndex element,const Coord globalCoord, Coord localCoord ) 
{
	TrilinearInnerElType*	self = (TrilinearInnerElType*)elementType;
	Dimension_Index		dim_I;

	for ( dim_I=0; dim_I < 3; dim_I++ ) {
	}
}
#endif


/*

 - Shape function definitions
 - Local node numbering convention for billinear, trilinear element (xi, eta, zeta)
 - Local coordinate domain spans  -1 <= xi,eta,zeta <= 1

    eta
     |
     |____ xi
    /
   /
 zeta


  eta
   |
3-----2
|  |__|___xi
|     |
0-----1
(zeta = -1 plane)


  eta
   |
7-----6
|  |__|___xi
|     |
4-----5
(zeta = +1 plane)


*/
void _TrilinearInnerElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	double xi, eta, zeta;
	double fac;
	
	xi   = localCoord[0];
	eta  = localCoord[1];
	zeta = localCoord[2];	
	
	fac = 1.0/9.0;

	evaluatedValues[0] = fac*( - 4.0*xi - 20.0*eta + 12.0*zeta + 3.0 );
	evaluatedValues[1] = fac*(  16.0*xi - 64.0*eta + 60.0*zeta + 6.0 );
	evaluatedValues[2] = fac*(            36.0*eta - 36.0*zeta       );
	evaluatedValues[3] = fac*( -12.0*xi + 48.0*eta - 36.0*zeta       );
}



void _TrilinearInnerElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives )
{		
	double fac = 1.0/9.0;
	                       
	evaluatedDerivatives[0][0] = fac*( - 4.0 ); 
	evaluatedDerivatives[0][1] = fac*(  16.0 ); 
	evaluatedDerivatives[0][2] =         0.0  ; 
	evaluatedDerivatives[0][3] = fac*( -12.0 ); 
	                             
	evaluatedDerivatives[1][0] = fac*( -20.0 );	
	evaluatedDerivatives[1][1] = fac*( -64.0 );	
	evaluatedDerivatives[1][2] =         4.0  ;	
	evaluatedDerivatives[1][3] = fac*(  48.0 );	
	                            
	evaluatedDerivatives[2][0] = fac*(  12.0 );
	evaluatedDerivatives[2][1] = fac*(  60.0 );
	evaluatedDerivatives[2][2] =       - 4.0  ;
	evaluatedDerivatives[2][3] =       - 4.0  ;
}

/* get rid of this function and just use the superclass (elementType) version, as for BilinearInner class?? */
/*
void _TrilinearInnerElType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		_mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord )
{
	TrilinearInnerElType*	self = (TrilinearInnerElType*)elementType;
	Mesh*			mesh = (Mesh*)_mesh;
	unsigned		inside;
	double			bc[4];
	static double		lCrds[8][3] = {{-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0}, 
					       {-1.0, 1.0, -1.0}, {1.0, 1.0, -1.0}, 
					       {-1.0, -1.0, 1.0}, {1.0, -1.0, 1.0}, 
					       {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}};
	unsigned		nInc, *inc;
	unsigned		bc_i;

	Mesh_GetIncidence( mesh, MT_VOLUME, element, MT_VERTEX, &nInc, &inc );
	assert( nInc == 8 );

	insist( Simplex_Search3D( mesh->verts, inc, 10, self->tetInds, (double*)globalCoord, bc, &inside ), == True );

	elLocalCoord[0] = bc[0] * lCrds[self->tetInds[inside][0]][0];
	elLocalCoord[1] = bc[0] * lCrds[self->tetInds[inside][0]][1];
	elLocalCoord[2] = bc[0] * lCrds[self->tetInds[inside][0]][2];
	for( bc_i = 1; bc_i < 4; bc_i++ ) {
		elLocalCoord[0] += bc[bc_i] * lCrds[self->tetInds[inside][bc_i]][0];
		elLocalCoord[1] += bc[bc_i] * lCrds[self->tetInds[inside][bc_i]][1];
		elLocalCoord[2] += bc[bc_i] * lCrds[self->tetInds[inside][bc_i]][2];
	}
}*/

