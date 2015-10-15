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
#include "LinearTriangleElementType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const Type LinearTriangleElementType_Type = "LinearTriangleElementType";

#define _LinearTriangleElementType_NodeCount 3

LinearTriangleElementType* LinearTriangleElementType_New( Name name ) {
	LinearTriangleElementType* self = _LinearTriangleElementType_DefaultNew( name );

	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, _LinearTriangleElementType_NodeCount );
	_LinearTriangleElementType_Init( self );

	return self;
}

void* _LinearTriangleElementType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(LinearTriangleElementType);
	Type                                                                                    type = LinearTriangleElementType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _LinearTriangleElementType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _LinearTriangleElementType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = _LinearTriangleElementType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _LinearTriangleElementType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _LinearTriangleElementType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _LinearTriangleElementType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _LinearTriangleElementType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _LinearTriangleElementType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _LinearTriangleElementType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _LinearTriangleElementType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_JacobianDeterminantSurfaceFunction*                  _jacobianDeterminantSurface = _ElementType_JacobianDeterminantSurface;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = NULL;

	return _LinearTriangleElementType_New(  LINEARTRIANGLEELEMENTTYPE_PASSARGS  );
}

LinearTriangleElementType* _LinearTriangleElementType_New(  LINEARTRIANGLEELEMENTTYPE_DEFARGS  ) {
	LinearTriangleElementType* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LinearTriangleElementType) );
	self = (LinearTriangleElementType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* LinearTriangleElementType info */
	
	return self;
}

void _LinearTriangleElementType_Init( LinearTriangleElementType* self ) {
	self->dim = 1;
}

void _LinearTriangleElementType_Delete( void* elementType ) {
	LinearTriangleElementType* self = (LinearTriangleElementType*)elementType;

	/* Stg_Class_Delete parent*/
	_ElementType_Delete( self );
}

void _LinearTriangleElementType_Print( void* elementType, Stream* stream ) {
	LinearTriangleElementType* self = (LinearTriangleElementType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* linearTriangleElementTypeStream = stream;
	
	/* General info */
	Journal_Printf( linearTriangleElementTypeStream, "LinearTriangleElementType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, linearTriangleElementTypeStream );
	
	/* Virtual info */
	
	/* LinearTriangleElementType info */
}

void _LinearTriangleElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
	LinearTriangleElementType* self = (LinearTriangleElementType*)elementType;

	_LinearTriangleElementType_Init( self );
}
	
void _LinearTriangleElementType_Build( void* elementType, void *data ){
}
	
void _LinearTriangleElementType_Initialise( void* elementType, void *data ){
}
	
void _LinearTriangleElementType_Execute( void* elementType, void *data ){
}
	
void _LinearTriangleElementType_Destroy( void* elementType, void *data ){
	LinearTriangleElementType* self = (LinearTriangleElementType*)elementType;

	_ElementType_Destroy( self, data );
}
	
/*

 - Shape function definitions
 - Local node numbering convention for linearTriangle element (xi, eta)
 - Local coordinate domain spans  0 <= xi,eta <= 1

    eta
     |
     |____ xi
 
  eta
   |
   |
2 \ 
|  \
|   \_____xi
|    \ 
0-----\1


*/
void _LinearTriangleElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues) {
	double xi, eta, zeta;
	
	xi   = localCoord[0];
	eta  = localCoord[1];
	zeta = localCoord[2];	
	
	evaluatedValues[0] = 1.0 - xi - eta;
	evaluatedValues[1] = xi;
	evaluatedValues[2] = eta;	
}


void _LinearTriangleElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives )
{		
	double xi, eta, zeta;
	
	xi   = localCoord[0];
	eta  = localCoord[1];
	zeta = localCoord[2];	
		
	/* derivatives wrt xi */
	evaluatedDerivatives[0][0] = -1.0;
	evaluatedDerivatives[0][1] =  1.0;
	evaluatedDerivatives[0][2] =  0.0;
	
	/* derivatives wrt eta */	
	evaluatedDerivatives[1][0] = - 1.0;
	evaluatedDerivatives[1][1] =   0.0;
	evaluatedDerivatives[1][2] =   1.0;
}

