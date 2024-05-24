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
#include <StgDomain/Geometry/src/Geometry.h>

#include "types.h"
#include "shortcuts.h"
#include "Decomp.h"
#include "Sync.h"
#include "MeshTopology.h"
#include "Mesh_ElementType.h"
#include "MeshClass.h"
#include "Mesh_CentroidType.h"


/* Textual name of this class */
const Type Mesh_CentroidType_Type = "Mesh_CentroidType";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Mesh_CentroidType* Mesh_CentroidType_New( Name name ) {
	/* Variables set in this function */
	SizeT                                                    _sizeOfSelf = sizeof(Mesh_CentroidType);
	Type                                                            type = Mesh_CentroidType_Type;
	Stg_Class_DeleteFunction*                                    _delete = _Mesh_CentroidType_Delete;
	Stg_Class_PrintFunction*                                      _print = _Mesh_CentroidType_Print;
	Stg_Class_CopyFunction*                                        _copy = NULL;
	Mesh_ElementType_UpdateFunc*                              updateFunc = Mesh_CentroidType_Update;
	Mesh_ElementType_ElementHasPointFunc*            elementHasPointFunc = Mesh_CentroidType_ElementHasPoint;
	Mesh_ElementType_GetMinimumSeparationFunc*  getMinimumSeparationFunc = Mesh_CentroidType_GetMinimumSeparation;
	Mesh_ElementType_GetCentroidFunc*                    getCentroidFunc = Mesh_CentroidType_GetCentroid;

	return _Mesh_CentroidType_New(  MESH_CENTROIDTYPE_PASSARGS  );
}

Mesh_CentroidType* _Mesh_CentroidType_New(  MESH_CENTROIDTYPE_DEFARGS  ) {
	Mesh_CentroidType* self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Mesh_CentroidType) );
	self = (Mesh_CentroidType*)_Mesh_ElementType_New(  MESH_ELEMENTTYPE_PASSARGS  );

	/* Virtual info */

	/* Mesh_CentroidType info */
	_Mesh_CentroidType_Init( self );

	return self;
}

void _Mesh_CentroidType_Init( Mesh_CentroidType* self ) {
	assert( self && Stg_CheckType( self, Mesh_CentroidType ) );

	self->elMesh = NULL;
	self->incArray = IArray_New();
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Mesh_CentroidType_Delete( void* centroidType ) {
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;

	Stg_Class_Delete( self->incArray );

	/* Delete the parent. */
	_Mesh_ElementType_Delete( self );
}

void _Mesh_CentroidType_Print( void* centroidType, Stream* stream ) {
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;
	Stream*			centroidTypeStream;

	centroidTypeStream = Journal_Register( InfoStream_Type, (Name)"Mesh_CentroidTypeStream"  );

	/* Print parent */
	Journal_Printf( stream, "Mesh_CentroidType (ptr): (%p)\n", self );
	_Mesh_ElementType_Print( self, stream );
}

void Mesh_CentroidType_Update( void* centroidType ) {
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;

	assert( self && Stg_CheckType( self, Mesh_CentroidType ) );
}

Bool Mesh_CentroidType_ElementHasPoint( void* centroidType, unsigned elInd, double* point, 
					MeshTopology_Dim* dim, unsigned* ind )
{
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;

	assert( self && Stg_CheckType( self, Mesh_CentroidType ) );

	return Mesh_ElementHasPoint( self->elMesh, elInd, point, dim, ind );
}

double Mesh_CentroidType_GetMinimumSeparation( void* centroidType, unsigned elInd, double* perDim ) {
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;
	Mesh_ElementType*	elType;

	assert( self && Stg_CheckType( self, Mesh_CentroidType ) );

	elType = Mesh_GetElementType( self->elMesh, elInd );

	return Mesh_ElementType_GetMinimumSeparation( elType, elInd, perDim );
}

void Mesh_CentroidType_GetCentroid( void* centroidType, unsigned element, double* centroid ) {
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;
	const int*		inc;

	assert( self && Stg_CheckType( self, Mesh_CentroidType ) );

	Mesh_GetIncidence( self->mesh, Mesh_GetDimSize( self->mesh ), element, MT_VERTEX, 
			   self->incArray );
	assert( IArray_GetSize( self->incArray ) == 1 );
	inc = IArray_GetPtr( self->incArray );
	memcpy( centroid, Mesh_GetVertex( self->mesh, inc[0] ), Mesh_GetDimSize( self->mesh ) * sizeof(unsigned) );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Mesh_CentroidType_SetElementMesh( void* centroidType, void* mesh ) {
	Mesh_CentroidType*	self = (Mesh_CentroidType*)centroidType;

	assert( self && Stg_CheckType( self, Mesh_CentroidType ) );
	assert( !mesh || Stg_CheckType( mesh, Mesh ) );

	self->elMesh = mesh;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


