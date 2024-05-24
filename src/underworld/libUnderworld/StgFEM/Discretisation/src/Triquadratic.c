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

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include "Discretisation.h"

#include "Triquadratic.h"

/* Textual name of this class */
const Type Triquadratic_Type = "Triquadratic";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

#define TRIQUADRATICNODECOUNT 27

Triquadratic* Triquadratic_New( Name name ) {
	/* Variables set in this function */
	SizeT                                                                            _sizeOfSelf = sizeof(Triquadratic);
	Type                                                                                    type = Triquadratic_Type;
	Stg_Class_DeleteFunction*                                                            _delete = _Triquadratic_Delete;
	Stg_Class_PrintFunction*                                                              _print = _Triquadratic_Print;
	Stg_Class_CopyFunction*                                                                _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                                _defaultConstructor = (void* (*)(Name))_Triquadratic_New;
	Stg_Component_ConstructFunction*                                                  _construct = _Triquadratic_AssignFromXML;
	Stg_Component_BuildFunction*                                                          _build = _Triquadratic_Build;
	Stg_Component_InitialiseFunction*                                                _initialise = _Triquadratic_Initialise;
	Stg_Component_ExecuteFunction*                                                      _execute = _Triquadratic_Execute;
	Stg_Component_DestroyFunction*                                                      _destroy = _Triquadratic_Destroy;
	AllocationType                                                            nameAllocationType = NON_GLOBAL;
	ElementType_EvaluateShapeFunctionsAtFunction*                      _evaluateShapeFunctionsAt = Triquadratic_EvalBasis;
	ElementType_EvaluateShapeFunctionLocalDerivsAtFunction*  _evaluateShapeFunctionLocalDerivsAt = Triquadratic_EvalLocalDerivs;
	ElementType_ConvertGlobalCoordToElLocalFunction*                _convertGlobalCoordToElLocal = _ElementType_ConvertGlobalCoordToElLocal;
	ElementType_SurfaceNormalFunction*                                            _surfaceNormal = _ElementType_SurfaceNormal;

	return _Triquadratic_New(  TRIQUADRATIC_PASSARGS  );
}

Triquadratic* _Triquadratic_New(  TRIQUADRATIC_DEFARGS  ) {
	Triquadratic*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Triquadratic) );
	self = (Triquadratic*)_ElementType_New(  ELEMENTTYPE_PASSARGS  );

	/* Virtual info */

	/* Triquadratic info */
	self->isConstructed = True;
	_ElementType_Init( (ElementType*)self, TRIQUADRATICNODECOUNT );
	_Triquadratic_Init( self );

	return self;
}

void _Triquadratic_Init( Triquadratic* self ) {
	assert( self && Stg_CheckType( self, Triquadratic ) );

	self->dim = 3;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Triquadratic_Delete( void* elementType ) {
	Triquadratic*	self = (Triquadratic*)elementType;

	/* Delete the parent */
	_ElementType_Delete( self );
}

void _Triquadratic_Print( void* elementType, Stream* stream ) {
	Triquadratic*	self = (Triquadratic*)elementType;

	/* Set the Journal for printing informations */
	Stream* elementTypeStream;
	elementTypeStream = Journal_Register( InfoStream_Type, (Name)"TriquadraticStream"  );

	/* Print parent */
	Journal_Printf( stream, "Triquadratic (ptr): (%p)\n", self );
	_ElementType_Print( self, stream );
}

void _Triquadratic_AssignFromXML( void* elementType, Stg_ComponentFactory* cf, void* data ) {
}

void _Triquadratic_Build( void* elementType, void* data ) {
}

void _Triquadratic_Initialise( void* elementType, void* data ) {
	Triquadratic*	self 		= (Triquadratic*)elementType;
	unsigned**	faceNodes;

	faceNodes = Memory_Alloc_2DArray( unsigned, 6, 9, (Name)"node indices for element faces"  );

	faceNodes[0][0] =  0; faceNodes[0][1] =  1; faceNodes[0][2] =  2;
	faceNodes[0][3] =  9; faceNodes[0][4] = 10; faceNodes[0][5] = 11;
	faceNodes[0][6] = 18; faceNodes[0][7] = 19; faceNodes[0][8] = 20;

	faceNodes[1][0] =  6; faceNodes[1][1] =  7; faceNodes[1][2] =  8;
	faceNodes[1][3] = 15; faceNodes[1][4] = 16; faceNodes[1][5] = 17;
	faceNodes[1][6] = 24; faceNodes[1][7] = 25; faceNodes[1][8] = 26;

	faceNodes[2][0] =  0; faceNodes[2][1] =  9; faceNodes[2][2] = 18;
	faceNodes[2][3] =  3; faceNodes[2][4] = 12; faceNodes[2][5] = 21;
	faceNodes[2][6] =  6; faceNodes[2][7] = 15; faceNodes[2][8] = 24;

	faceNodes[3][0] =  2; faceNodes[3][1] = 11; faceNodes[3][2] = 20;
	faceNodes[3][3] =  5; faceNodes[3][4] = 14; faceNodes[3][5] = 23;
	faceNodes[3][6] =  8; faceNodes[3][7] = 17; faceNodes[3][8] = 26;

	faceNodes[4][0] =  0; faceNodes[4][1] =  1; faceNodes[4][2] =  2;
	faceNodes[4][3] =  3; faceNodes[4][4] =  4; faceNodes[4][5] =  5;
	faceNodes[4][6] =  6; faceNodes[4][7] =  7; faceNodes[4][8] =  8;

	faceNodes[5][0] = 18; faceNodes[5][1] = 19; faceNodes[5][2] = 20;
	faceNodes[5][3] = 21; faceNodes[5][4] = 22; faceNodes[5][5] = 23;
	faceNodes[5][6] = 24; faceNodes[5][7] = 25; faceNodes[5][8] = 26;

	self->faceNodes = faceNodes;

	self->evaluatedShapeFunc = Memory_Alloc_Array( double, self->nodeCount, "evaluatedShapeFuncs" );
	self->GNi = Memory_Alloc_2DArray( double, self->dim, self->nodeCount, (Name)"evaluatedShapeFuncDerivatives"  );
}

void _Triquadratic_Execute( void* elementType, void* data ) {
}

void _Triquadratic_Destroy( void* elementType, void* data ) {
	Triquadratic*	self 		= (Triquadratic*)elementType;

	Memory_Free( self->faceNodes );
	Memory_Free( self->evaluatedShapeFunc );
	Memory_Free( self->GNi );

	_ElementType_Destroy( elementType, data );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

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

void Triquadratic_EvalBasis( void* elementType, const double* localCoord, double* basis ) {
	double xi = localCoord[0], eta = localCoord[1], zeta = localCoord[2];
	double a0 = xi - 1.0, b0 = eta - 1.0, c0 = zeta - 1.0;
	double a1 = 1.0 - xi * xi, b1 = 1.0 - eta * eta, c1 = 1.0 - zeta * zeta;
	double a2 = xi + 1.0, b2 = eta + 1.0, c2 = zeta + 1.0;
	double m0 = 0.5 * xi;
	double m1 = 0.5 * eta;
	double m2 = 0.5 * zeta;
	double m3 = 0.25 * xi * eta;
	double m4 = 0.25 * xi * zeta;
	double m5 = 0.25 * eta * zeta;
	double m6 = 0.125 * xi * eta * zeta;

	basis[0]  = m6 * a0 * b0 * c0;
	basis[1]  = m5 * a1 * b0 * c0;
	basis[2]  = m6 * a2 * b0 * c0;

	basis[3]  = m4 * a0 * b1 * c0;
	basis[4]  = m2 * a1 * b1 * c0;
	basis[5]  = m4 * a2 * b1 * c0;

	basis[6]  = m6 * a0 * b2 * c0;
	basis[7]  = m5 * a1 * b2 * c0;
	basis[8]  = m6 * a2 * b2 * c0;


	basis[9]  = m3 * a0 * b0 * c1;
	basis[10] = m1 * a1 * b0 * c1;
	basis[11] = m3 * a2 * b0 * c1;

	basis[12] = m0 * a0 * b1 * c1;
	basis[13] =      a1 * b1 * c1;
	basis[14] = m0 * a2 * b1 * c1;

	basis[15] = m3 * a0 * b2 * c1;
	basis[16] = m1 * a1 * b2 * c1;
	basis[17] = m3 * a2 * b2 * c1;


	basis[18] = m6 * a0 * b0 * c2;
	basis[19] = m5 * a1 * b0 * c2;
	basis[20] = m6 * a2 * b0 * c2;

	basis[21] = m4 * a0 * b1 * c2;
	basis[22] = m2 * a1 * b1 * c2;
	basis[23] = m4 * a2 * b1 * c2;

	basis[24] = m6 * a0 * b2 * c2;
	basis[25] = m5 * a1 * b2 * c2;
	basis[26] = m6 * a2 * b2 * c2;
}

void Triquadratic_EvalLocalDerivs( void* elementType, const double* localCoord, double** derivs ) {
	double xi = localCoord[0], eta = localCoord[1], zeta = localCoord[2];
	double a0 = xi - 1.0, b0 = eta - 1.0, c0 = zeta - 1.0;
	double a1 = 1.0 - xi * xi, b1 = 1.0 - eta * eta, c1 = 1.0 - zeta * zeta;
	double a2 = xi + 1.0, b2 = eta + 1.0, c2 = zeta + 1.0;
	double m0 = 0.5 * xi;
	double m1 = 0.5 * eta;
	double m2 = 0.5 * zeta;
	double m3 = 0.25 * xi * eta;
	double m4 = 0.25 * xi * zeta;
	double m5 = 0.25 * eta * zeta;
	double a3 = xi - 0.5, b3 = eta - 0.5, c3 = zeta - 0.5;
	double a4 = -2.0 * xi, b4 = -2.0 * eta, c4 = -2.0 * zeta;
	double a5 = xi + 0.5, b5 = eta + 0.5, c5 = zeta + 0.5;

	/* first face perp. to zeta */
	derivs[0][0]  = a3 * m5 * b0 * c0;
	derivs[1][0]  = b3 * m4 * a0 * c0;
	derivs[2][0]  = c3 * m3 * a0 * b0;

	derivs[0][1]  = a4 * m5 * b0 * c0;
	derivs[1][1]  = b3 * m2 * a1 * c0;
	derivs[2][1]  = c3 * m1 * a1 * b0;

	derivs[0][2]  = a5 * m5 * b0 * c0;
	derivs[1][2]  = b3 * m4 * a2 * c0;
	derivs[2][2]  = c3 * m3 * a2 * b0;


	derivs[0][3]  = a3 * m2 * b1 * c0;
	derivs[1][3]  = b4 * m4 * a0 * c0;
	derivs[2][3]  = c3 * m0 * a0 * b1;

	derivs[0][4]  = a4 * m2 * b1 * c0;
	derivs[1][4]  = b4 * m2 * a1 * c0;
	derivs[2][4]  = c3 *      a1 * b1;

	derivs[0][5]  = a5 * m2 * b1 * c0;
	derivs[1][5]  = b4 * m4 * a2 * c0;
	derivs[2][5]  = c3 * m0 * a2 * b1;


	derivs[0][6]  = a3 * m5 * b2 * c0;
	derivs[1][6]  = b5 * m4 * a0 * c0;
	derivs[2][6]  = c3 * m3 * a0 * b2;

	derivs[0][7]  = a4 * m5 * b2 * c0;
	derivs[1][7]  = b5 * m2 * a1 * c0;
	derivs[2][7]  = c3 * m1 * a1 * b2;

	derivs[0][8]  = a5 * m5 * b2 * c0;
	derivs[1][8]  = b5 * m4 * a2 * c0;
	derivs[2][8]  = c3 * m3 * a2 * b2;

	/* second face perp. to zeta */
	derivs[0][9]  = a3 * m1 * b0 * c1;
	derivs[1][9]  = b3 * m0 * a0 * c1;
	derivs[2][9]  = c4 * m3 * a0 * b0;

	derivs[0][10] = a4 * m1 * b0 * c1;
	derivs[1][10] = b3 *      a1 * c1;
	derivs[2][10] = c4 * m1 * a1 * b0;

	derivs[0][11] = a5 * m1 * b0 * c1;
	derivs[1][11] = b3 * m0 * a2 * c1;
	derivs[2][11] = c4 * m3 * a2 * b0;


	derivs[0][12] = a3 *      b1 * c1;
	derivs[1][12] = b4 * m0 * a0 * c1;
	derivs[2][12] = c4 * m0 * a0 * b1;

	derivs[0][13] = a4 *      b1 * c1;
	derivs[1][13] = b4 *      a1 * c1;
	derivs[2][13] = c4 *      a1 * b1;

	derivs[0][14] = a5 *      b1 * c1;
	derivs[1][14] = b4 * m0 * a2 * c1;
	derivs[2][14] = c4 * m0 * a2 * b1;


	derivs[0][15] = a3 * m1 * b2 * c1;
	derivs[1][15] = b5 * m0 * a0 * c1;
	derivs[2][15] = c4 * m3 * a0 * b2;

	derivs[0][16] = a4 * m1 * b2 * c1;
	derivs[1][16] = b5 *      a1 * c1;
	derivs[2][16] = c4 * m1 * a1 * b2;

	derivs[0][17] = a5 * m1 * b2 * c1;
	derivs[1][17] = b5 * m0 * a2 * c1;
	derivs[2][17] = c4 * m3 * a2 * b2;

	/* back face perp. to zeta */
	derivs[0][18] = a3 * m5 * b0 * c2;
	derivs[1][18] = b3 * m4 * a0 * c2;
	derivs[2][18] = c5 * m3 * a0 * b0;

	derivs[0][19] = a4 * m5 * b0 * c2;
	derivs[1][19] = b3 * m2 * a1 * c2;
	derivs[2][19] = c5 * m1 * a1 * b0;

	derivs[0][20] = a5 * m5 * b0 * c2;
	derivs[1][20] = b3 * m4 * a2 * c2;
	derivs[2][20] = c5 * m3 * a2 * b0;


	derivs[0][21] = a3 * m2 * b1 * c2;
	derivs[1][21] = b4 * m4 * a0 * c2;
	derivs[2][21] = c5 * m0 * a0 * b1;

	derivs[0][22] = a4 * m2 * b1 * c2;
	derivs[1][22] = b4 * m2 * a1 * c2;
	derivs[2][22] = c5 *      a1 * b1;

	derivs[0][23] = a5 * m2 * b1 * c2;
	derivs[1][23] = b4 * m4 * a2 * c2;
	derivs[2][23] = c5 * m0 * a2 * b1;


	derivs[0][24] = a3 * m5 * b2 * c2;
	derivs[1][24] = b5 * m4 * a0 * c2;
	derivs[2][24] = c5 * m3 * a0 * b2;

	derivs[0][25] = a4 * m5 * b2 * c2;
	derivs[1][25] = b5 * m2 * a1 * c2;
	derivs[2][25] = c5 * m1 * a1 * b2;

	derivs[0][26] = a5 * m5 * b2 * c2;
	derivs[1][26] = b5 * m4 * a2 * c2;
	derivs[2][26] = c5 * m3 * a2 * b2;
}
