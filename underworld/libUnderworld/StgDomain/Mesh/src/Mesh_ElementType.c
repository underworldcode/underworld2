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
#include "Mesh.h"


/* Textual name of this class */
const Type Mesh_ElementType_Type = "Mesh_ElementType";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh_ElementType* _Mesh_ElementType_New(  MESH_ELEMENTTYPE_DEFARGS  ) {
	Mesh_ElementType*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh_ElementType) );
	self = (Mesh_ElementType*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	/* Virtual info */
	self->updateFunc = updateFunc;
	self->elementHasPointFunc = elementHasPointFunc;
	self->getMinimumSeparationFunc = getMinimumSeparationFunc;
	self->getCentroidFunc = getCentroidFunc;

	/* Mesh_ElementType info */
	_Mesh_ElementType_Init( self );

	return self;
}

void _Mesh_ElementType_Init( Mesh_ElementType* self ) {
	assert( self && Stg_CheckType( self, Mesh_ElementType ) );

	self->mesh = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_ElementType_Delete( void* elementType ) {
	Mesh_ElementType*	self = (Mesh_ElementType*)elementType;

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _Mesh_ElementType_Print( void* elementType, Stream* stream ) {
	Mesh_ElementType*	self = (Mesh_ElementType*)elementType;
	Stream*			elementTypeStream;

	elementTypeStream = Journal_Register( InfoStream_Type, (Name)"Mesh_ElementTypeStream"  );

	/* Print parent */
	Journal_Printf( stream, "Mesh_ElementType (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}

void _Mesh_ElementType_GetCentroid( void* elementType, unsigned element, double* centroid ) {
	Mesh_ElementType*	self = (Mesh_ElementType*)elementType;
	Mesh*			mesh;
	IArray*			inc;
	unsigned		nIncVerts;
	const int		*incVerts;
	unsigned		nDims;
	double			denom;
	unsigned		d_i, v_i;

	assert( self );

	mesh = self->mesh;
	nDims = Mesh_GetDimSize( mesh );
	inc = IArray_New();
	Mesh_GetIncidence( mesh, nDims, element, MT_VERTEX, inc );
	nIncVerts = (unsigned)IArray_GetSize( inc );
	incVerts = IArray_GetPtr( inc );

	assert( nIncVerts );
	denom = 1.0 / (double)nIncVerts;

	for( d_i = 0; d_i < nDims; d_i++ ) {
		centroid[d_i] = Mesh_GetVertex( mesh, incVerts[0] )[d_i];
		for( v_i = 1; v_i < nIncVerts; v_i++ )
			centroid[d_i] += Mesh_GetVertex( mesh, incVerts[v_i] )[d_i];
		centroid[d_i] *= denom;
	}

	Stg_Class_Delete( inc );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Mesh_ElementType_SetMesh( void* elementType, void* mesh ) {
	Mesh_ElementType*	self = (Mesh_ElementType*)elementType;

	assert( self && Stg_CheckType( self, Mesh_ElementType ) );
	assert( !mesh || Stg_CheckType( mesh, Mesh ) );

	self->mesh = mesh;
	Mesh_ElementType_Update( self );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


