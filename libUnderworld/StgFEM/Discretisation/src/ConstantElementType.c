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
#include "ConstantElementType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

const Type ConstantElementType_Type = "ConstantElementType";
#define _ConstantElementType_NodeCount 1

ConstantElementType* ConstantElementType_New( Name name ) {
	ConstantElementType* self = ConstantElementType_DefaultNew( name );

	self->isConstructed = True;	
	_ElementType_Init( (ElementType*)self, _ConstantElementType_NodeCount );
	_ConstantElementType_Init( self );	

	return self;
}

void* ConstantElementType_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(ConstantElementType);
	Type                                                                                    type = ConstantElementType_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _ConstantElementType_Delete;
	Stg_Class_PrintFunction*                                                              _print = _ConstantElementType_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = ConstantElementType_DefaultNew;
	Stg_Component_ConstructFunction*                                                  _construct = _ConstantElementType_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _ConstantElementType_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _ConstantElementType_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _ConstantElementType_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _ConstantElementType_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = _ConstantElementType_SF_allNodes;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = _ConstantElementType_SF_allLocalDerivs_allNodes;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ConstantElementType_ConvertGlobalCoordToElLocal;
	ElementType_JacobianDeterminantSurfaceFunction*                  _jacobianDeterminantSurface = _ElementType_JacobianDeterminantSurface;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = NULL;

	return _ConstantElementType_New(  CONSTANTELEMENTTYPE_PASSARGS  );
}

ConstantElementType* _ConstantElementType_New(  CONSTANTELEMENTTYPE_DEFARGS  ) {
	ConstantElementType*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(ConstantElementType) );
	self = (ConstantElementType*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* ConstantElementType info */

	return self;
}

void _ConstantElementType_Init( ConstantElementType* self ) {
	self->dim = 0;
}

void _ConstantElementType_Delete( void* elementType ) {
	ConstantElementType* self = (ConstantElementType*)elementType;

	/* Stg_Class_Delete parent*/
	_ElementType_Delete( self );
}

void _ConstantElementType_Print( void* elementType, Stream* stream ) {
	ConstantElementType* self = (ConstantElementType*)elementType;
	
	/* Set the Journal for printing informations */
	Stream* constantElementTypeStream = stream;
	
	/* General info */
	Journal_Printf( constantElementTypeStream, "ConstantElementType (ptr): %p\n", self );
	
	/* Print parent */
	_ElementType_Print( self, constantElementTypeStream );
	
	/* Virtual info */
	
	/* ConstantElementType info */
}

void _ConstantElementType_AssignFromXML( void* elementType, Stg_ComponentFactory *cf, void* data ){
	ConstantElementType* self = (ConstantElementType*)elementType;

	_ConstantElementType_Init( self );
}
	
void _ConstantElementType_Initialise( void* elementType, void *data ){
}
	
void _ConstantElementType_Execute( void* elementType, void *data ){
}
	
void _ConstantElementType_Destroy( void* elementType, void *data ){
	ConstantElementType* self = (ConstantElementType*)elementType;

	_ElementType_Destroy( self, data );
}

void _ConstantElementType_Build( void* elementType, void *data ) {

}

/*

 - Shape function definition
 - Constant shape function in 2d/3d has only one node at centroid of element.
 - Node id is 0 in both cases.
 - Local coordinate domain spans  -1 <= xi,eta <= 1 in 2d
 - Local coordinate domain spans  -1 <= xi,eta,zeta <= 1 in 3d

*/
void _ConstantElementType_SF_allNodes( void* elementType, const double localCoord[], double* const evaluatedValues ) {
	evaluatedValues[0] = 1.0;
}


/*
Since we use only have one constant shape func for 2d and 3d quads, then
if we want to return the zero derivatives we need to pass in "dim" so we know
whether to fill in GNi[0][0] = GNi[1][0] = 0.0 + GNi[2][0] = 0.0 if dim == 3
Should just return error if we try to take deriv of constant. No one would want
to do this!
*/
void _ConstantElementType_SF_allLocalDerivs_allNodes( void* elementType, const double localCoord[],
		double** const evaluatedDerivatives )
{		
    Journal_Firewall( 0, NULL, "Taking derivative of constant shape function not currently supported." );
}


void _ConstantElementType_ConvertGlobalCoordToElLocal(
		void*		elementType,
		void*		mesh, 
		unsigned	element, 
		const double*	globalCoord,
		double*		elLocalCoord )
{
	/* See header file function introduction for explanation... */
	elLocalCoord[0] = elLocalCoord[1] = elLocalCoord[2] = 0;
}

