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
#include <StgDomain/Shape/src/Shape.h>

#include "types.h"
#include "shortcuts.h"
#include "Grid.h"


/* Textual name of this class */
const Type Grid_Type = "Grid";

/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Grid* Grid_New() {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(Grid);
	Type                              type = Grid_Type;
	Stg_Class_DeleteFunction*      _delete = _Grid_Delete;
	Stg_Class_PrintFunction*        _print = _Grid_Print;
	Stg_Class_CopyFunction*          _copy = NULL;

	return _Grid_New(  GRID_PASSARGS  );
}

Grid* _Grid_New(  GRID_DEFARGS  ) {
	Grid*	self;

	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(Grid) );
	self = (Grid*)_Stg_Class_New(  STG_CLASS_PASSARGS  );

	/* Virtual info */

	/* Grid info */
	_Grid_Init( self );

	return self;
}

void _Grid_Init( Grid* self ) {
	self->nDims = 0;
	self->sizes = NULL;
	self->basis = NULL;
	self->nPoints = 0;

	self->map = NULL;
	self->invMap = NULL;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Grid_Delete( void* grid ) {
	Grid*	self = (Grid*)grid;

	FreeArray( self->sizes );
	FreeArray( self->basis );

	/* Delete the parent. */
	_Stg_Class_Delete( self );
}

void _Grid_Print( void* grid, Stream* stream ) {
	Grid*	self = (Grid*)grid;

	/* Set the Journal for printing informations */
	Stream* gridStream;
	gridStream = Journal_Register( InfoStream_Type, (Name)"GridStream"  );

	/* Print parent */
	Journal_Printf( stream, "Grid (ptr): (%p)\n", self );
	_Stg_Class_Print( self, stream );
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void Grid_SetNumDims( void* grid, unsigned nDims ) {
	Grid*	self = (Grid*)grid;

	/* Sanity check. */
	assert( self );

	/* If we're changing dimensions, kill everything and begin again. */
	KillArray( self->sizes );
	KillArray( self->basis );

	/* Set dimensions. */
	self->nDims = nDims;
}

void Grid_SetSizes( void* grid, unsigned* sizes ) {
	Grid*	self = (Grid*)grid;

	/* Sanity check. */
	assert( self );
	assert( !self->nDims || sizes );
#ifndef NDEBUG
	{
		unsigned	d_i;

		for( d_i = 0; d_i < self->nDims; d_i++ )
			assert( sizes[d_i] );
	}
#endif

	KillArray( self->sizes );
	KillArray( self->basis );

	/* Copy the sizes, allocate arrays and build basis. */
	if( self->nDims ) {
		unsigned	d_i;

      if(self->sizes) FreeArray( self->sizes );
      if(self->basis) FreeArray( self->basis );
		   
		self->sizes = Memory_Alloc_Array( unsigned, self->nDims, "Grid::sizes" );
		self->basis = Memory_Alloc_Array( unsigned, self->nDims, "Grid::basis" );
		memcpy( self->sizes, sizes, self->nDims * sizeof(unsigned) );

		/* Build basis. */
		self->basis[0] = 1;
		self->nPoints = sizes[0];
		for( d_i = 1; d_i < self->nDims; d_i++ ) {
			self->basis[d_i] = self->basis[d_i - 1] * self->sizes[d_i - 1];
			self->nPoints *= sizes[d_i];
		}
	}
}

void Grid_SetMapping( void* grid, HashTable* mapping, HashTable* inverse ) {
	Grid*	self = (Grid*)grid;

	assert( self );

	self->map = mapping;
	self->invMap = inverse;
}

unsigned Grid_GetNumDims( void* grid ) {
	Grid*	self = (Grid*)grid;

	assert( self );

	return self->nDims;
}

unsigned* Grid_GetSizes( void* grid ) {
	Grid*	self = (Grid*)grid;

	assert( self );

	return self->sizes;
}

unsigned Grid_GetNumPoints( void* grid ) {
	Grid*	self = (Grid*)grid;

	assert( self );

	return self->nPoints;
}

void Grid_GetMapping( void* grid, HashTable** mapping, HashTable** inverse ) {
	Grid*	self = (Grid*)grid;

	assert( self );

	*mapping = self->map;
	*inverse = self->invMap;
}

void Grid_Lift( void* grid, unsigned ind, unsigned* params ) {
	Grid*		self = (Grid*)grid;
	unsigned	rem;
	unsigned	d_i;

	/* Sanity check. */
	assert( self );
	assert( self->nDims );
	assert( self->sizes && self->basis );
	assert( params );

	/*
	** Take a one dimensional array index and lift it into a regular mesh topological
	** space.
	*/

	if( self->map ) {
#ifndef NDEBUG
		unsigned*	remPtr;

		assert( remPtr = HashTable_FindEntry( self->map, &ind, sizeof(unsigned), unsigned ) );
		rem = *remPtr;
#else
		rem = *HashTable_FindEntry( self->map, &ind, sizeof(unsigned), unsigned );
#endif
	}
	else
		rem = ind;

	for( d_i = self->nDims; d_i > 0; d_i-- ) {
		unsigned	dimInd = d_i - 1;
		div_t		divRes;

		divRes = div( rem, self->basis[dimInd] );
		params[dimInd] = divRes.quot;
		rem = divRes.rem;

		/* Ensure this is a valid lifting. */
		assert( params[dimInd] < self->sizes[dimInd] );
	}
}

unsigned Grid_Project( void* grid, unsigned* params ) {
	Grid*		self = (Grid*)grid;
	unsigned	ind = 0;
	unsigned	d_i;

	/* Sanity check. */
	assert( self );
	assert( self->nDims );
	assert( self->sizes && self->basis );
	assert( params );

	/*
	** Project an n-dimensional set of topological indices into a one-dimensional, unique space.
	*/

	for( d_i = 0; d_i < self->nDims; d_i++ ) {
		assert( params[d_i] < self->sizes[d_i] );
		ind += params[d_i] * self->basis[d_i];
	}

	if( self->invMap ) {
#ifndef NDEBUG
		unsigned*	indPtr;

		assert( indPtr = HashTable_FindEntry( self->invMap, &ind, sizeof(unsigned), unsigned ) );
		ind = *indPtr;
#else
		ind = *HashTable_FindEntry( self->invMap, &ind, sizeof(unsigned), unsigned );
#endif
	}

	return ind;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/


