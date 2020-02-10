/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>

#include "types.h"

#include "ElementType.h"
#include "dQ13DElementType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type dQ13DElType_Type = "dQ13DElType";

#define _dQ13DElType_NodeCount 8

void* _dQ13DElType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(dQ13DElType);
	Type                                                                                    type = dQ13DElType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _dQ13DElType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _dQ13DElType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _dQ13DElType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _dQ13DElType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _dQ13DElType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _dQ13DElType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _dQ13DElType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _dQ13DElType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _dQ13DElType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _dQ13DElType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = NULL;

	return _dQ13DElType_New(  DQ13DELTYPE_PASSARGS  );
}

dQ13DElType* dQ13DElType_New( Name name ) {
	dQ13DElType* self = _dQ13DElType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _dQ13DElType_NodeCount );
	_dQ13DElType_Init( self );

	return self;
}

dQ13DElType* _dQ13DElType_New(  DQ13DELTYPE_DEFARGS  ) {
	dQ13DElType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(dQ13DElType) );
	self = (dQ13DElType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	return self;
}

void _dQ13DElType_Init( dQ13DElType* self ) {
	Dimension_Index dim_I=0;

	/* General and Virtual info should already be set */
	for ( dim_I = 0; dim_I < 3; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] =  1;
		self->elLocalLength  [dim_I] =  self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}
}

void _dQ13DElType_Delete( void* elementType ) {
	dQ13DElType* self = (dQ13DElType*)elementType;

	/* Stg_Class_Delete parent*/
	_ElementType_Delete( self  );
}

void _dQ13DElType_Print( void* elementType, Stream* stream ) {
	dQ13DElType* self = (dQ13DElType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* trilinearInnerElTypeStream = stream;
	
	/* General info */
	Journal_Printf( trilinearInnerElTypeStream, "dQ13DElType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, trilinearInnerElTypeStream );

}

void _dQ13DElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
	
}
	
void _dQ13DElType_Initialise( void* elementType, void *data ){
	
}
	
void _dQ13DElType_Execute( void* elementType, void *data ){
	
}
	
void _dQ13DElType_Destroy( void* elementType, void *data ){
  dQ13DElType* self = (dQ13DElType*)elementType;
  Memory_Free( self->evaluatedShapeFunc );
  Memory_Free( self->GNi );
  _ElementType_Destroy( self, data );
}

void _dQ13DElType_Build( void* elementType, void *data ) {
  dQ13DElType* self = (dQ13DElType*)elementType;
  self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
  self->GNi = Memory_Alloc_2DArray( double, 3, self->nodeCount, (Name)"localShapeFuncDerivitives"  ); 
}

void _dQ13DElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
  double xi, et, ze;
  double xi_m,et_m,xi_p,et_p,ze_m,ze_p;
  int I=0,J=1,K=2;
	
  xi = localCoord[I];
  et = localCoord[J];
  ze = localCoord[K];	
  
  xi_m = (0.5-xi);
  et_m = (0.5-et);
  xi_p = (0.5+xi);
  et_p = (0.5+et);
  ze_m = (0.5-ze);
  ze_p = (0.5+ze);

  evaluatedValues[0] = xi_m * et_m * ze_m;
  evaluatedValues[1] = xi_p * et_m * ze_m;
  evaluatedValues[2] = xi_p * et_p * ze_m;
  evaluatedValues[3] = xi_m * et_p * ze_m;
  evaluatedValues[4] = xi_m * et_m * ze_p;
  evaluatedValues[5] = xi_p * et_m * ze_p;
  evaluatedValues[6] = xi_p * et_p * ze_p;
  evaluatedValues[7] = xi_m * et_p * ze_p;

}



void _dQ13DElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives ){		
  double xi, et, ze;
  double xi_m,et_m,xi_p,et_p,ze_m,ze_p;
  int I=0,J=1,K=2;
	
  xi = localCoord[I];
  et = localCoord[J];
  ze = localCoord[K];

  xi_m = (0.5-xi);
  et_m = (0.5-et);
  xi_p = (0.5+xi);
  et_p = (0.5+et);
  ze_m = (0.5-ze);
  ze_p = (0.5+ze);
	                       
  evaluatedDerivatives[I][0] = -et_m * ze_m; 
  evaluatedDerivatives[I][1] =  et_m * ze_m; 
  evaluatedDerivatives[I][2] =  et_p * ze_m; 
  evaluatedDerivatives[I][3] = -et_p * ze_m; 
  evaluatedDerivatives[I][4] = -et_m * ze_p; 
  evaluatedDerivatives[I][5] =  et_m * ze_p; 
  evaluatedDerivatives[I][6] =  et_p * ze_p; 
  evaluatedDerivatives[I][7] = -et_p * ze_p; 
	                             
  evaluatedDerivatives[J][0] = -xi_m * ze_m; 
  evaluatedDerivatives[J][1] = -xi_p * ze_m; 
  evaluatedDerivatives[J][2] =  xi_p * ze_m; 
  evaluatedDerivatives[J][3] =  xi_m * ze_m; 
  evaluatedDerivatives[J][4] = -xi_m * ze_p; 
  evaluatedDerivatives[J][5] = -xi_p * ze_p; 
  evaluatedDerivatives[J][6] =  xi_p * ze_p; 
  evaluatedDerivatives[J][7] =  xi_m * ze_p; 

  evaluatedDerivatives[K][0] = -xi_m * et_m; 
  evaluatedDerivatives[K][1] = -xi_p * et_m; 
  evaluatedDerivatives[K][2] = -xi_p * et_p; 
  evaluatedDerivatives[K][3] = -xi_m * et_p; 
  evaluatedDerivatives[K][4] =  xi_m * et_m; 
  evaluatedDerivatives[K][5] =  xi_p * et_m; 
  evaluatedDerivatives[K][6] =  xi_p * et_p; 
  evaluatedDerivatives[K][7] =  xi_m * et_p; 
}

