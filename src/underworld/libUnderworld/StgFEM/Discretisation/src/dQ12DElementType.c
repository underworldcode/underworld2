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
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>

#include "types.h"

#include "ElementType.h"
#include "dQ12DElementType.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type dQ12DElType_Type = "dQ12DElType";

#define _dQ12DElType_NodeCount 4

void* _dQ12DElType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(dQ12DElType);
	Type                                                                                    type = dQ12DElType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _dQ12DElType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _dQ12DElType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _dQ12DElType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _dQ12DElType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _dQ12DElType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _dQ12DElType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _dQ12DElType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _dQ12DElType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _dQ12DElType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _dQ12DElType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = NULL;

	return _dQ12DElType_New(  DQ12DELTYPE_PASSARGS  );
}

dQ12DElType* dQ12DElType_New( Name name ) {
	dQ12DElType* self = _dQ12DElType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _dQ12DElType_NodeCount );
	_dQ12DElType_Init( self );

	return self;
}

dQ12DElType* _dQ12DElType_New(  DQ12DELTYPE_DEFARGS  ) {
	dQ12DElType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(dQ12DElType) );
	self = (dQ12DElType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	return self;
}

void _dQ12DElType_Init( dQ12DElType* self ) {
	Dimension_Index dim_I=0;

	/* General and Virtual info should already be set */
	for ( dim_I = 0; dim_I < 2; dim_I++ ) {
		self->minElLocalCoord[dim_I] = -1;
		self->maxElLocalCoord[dim_I] =  1;
		self->elLocalLength  [dim_I] =  self->maxElLocalCoord[dim_I] - self->minElLocalCoord[dim_I];
	}
}

void _dQ12DElType_Delete( void* elementType ) {
	dQ12DElType* self = (dQ12DElType*)elementType;

	/* Stg_Class_Delete parent*/
	_ElementType_Delete( self  );
}

void _dQ12DElType_Print( void* elementType, Stream* stream ) {
	dQ12DElType* self = (dQ12DElType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* trilinearInnerElTypeStream = stream;
	/* General info */
	Journal_Printf( trilinearInnerElTypeStream, "dQ12DElType (ptr): %p\n", self );	
	/* Print parent */
	_ElementType_Print( self, trilinearInnerElTypeStream );

}

void _dQ12DElType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
	
}
	
void _dQ12DElType_Initialise( void* elementType, void *data ){
	
}
	
void _dQ12DElType_Execute( void* elementType, void *data ){
	
}
	
void _dQ12DElType_Destroy( void* elementType, void *data ){
	dQ12DElType* self = (dQ12DElType*)elementType;
	Memory_Free( self->evaluatedShapeFunc );
    Memory_Free( self->GNi );
	_ElementType_Destroy( self, data );
}

void _dQ12DElType_Build( void* elementType, void *data ) {
	dQ12DElType* self = (dQ12DElType*)elementType;
    self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
    self->GNi = Memory_Alloc_2DArray( double, 2, self->nodeCount, (Name)"localShapeFuncDerivitives"  ); 
}

void _dQ12DElType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
  double xi, et;
  double xi_m,et_m,xi_p,et_p;
  int I=0,J=1;
	
  xi = localCoord[I];
  et = localCoord[J];
	
  xi_m = (0.5-xi);
  et_m = (0.5-et);
  xi_p = (0.5+xi);
  et_p = (0.5+et);
  
  evaluatedValues[0] = xi_m * et_m;
  evaluatedValues[1] = xi_p * et_m;
  evaluatedValues[2] = xi_p * et_p;
  evaluatedValues[3] = xi_m * et_p;
}



void _dQ12DElType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives ){		
  double xi, et;
  double xi_m,et_m,xi_p,et_p;
  int I=0,J=1;

  xi  = localCoord[I];
  et  = localCoord[J];

  xi_m = (0.5-xi);
  et_m = (0.5-et);
  xi_p = (0.5+xi);
  et_p = (0.5+et);

  evaluatedDerivatives[I][0] = -et_m; 
  evaluatedDerivatives[I][1] =  et_m; 
  evaluatedDerivatives[I][2] =  et_p; 
  evaluatedDerivatives[I][3] = -et_p; 
  
  evaluatedDerivatives[J][0] = -xi_m;	
  evaluatedDerivatives[J][1] = -xi_p;	
  evaluatedDerivatives[J][2] =  xi_p;	
  evaluatedDerivatives[J][3] =  xi_m;	
}
