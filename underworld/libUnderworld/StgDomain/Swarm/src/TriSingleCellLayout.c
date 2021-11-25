/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>

#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>
#include <StgDomain/Mesh/src/Mesh.h>

#include "types.h"

#include "CellLayout.h"
#include "TriSingleCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

const Type TriSingleCellLayout_Type = "TriSingleCellLayout";


void _TriSingleCellLayout_Init( TriSingleCellLayout* self, Dictionary* dictionary, int dim ) { 
	/* General and Virtual info should already be set */
	
	/* SingleCellInfo info */
	self->dictionary = dictionary;
	self->dim = dim;
}

TriSingleCellLayout* TriSingleCellLayout_New( Name name, AbstractContext* context, int dim, Dictionary* dictionary ) { 
	TriSingleCellLayout* self = _TriSingleCellLayout_DefaultNew( name );

	self->isConstructed = True;
	_CellLayout_Init( (CellLayout*)self );
	_TriSingleCellLayout_Init( self, dictionary, dim );

	return self;
}

TriSingleCellLayout* _TriSingleCellLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                _sizeOfSelf = sizeof(TriSingleCellLayout);
	Type                                                        type = TriSingleCellLayout_Type;
	Stg_Class_DeleteFunction*                                _delete = _TriSingleCellLayout_Delete;
	Stg_Class_PrintFunction*                                  _print = _TriSingleCellLayout_Print;
	Stg_Class_CopyFunction*                                    _copy = _TriSingleCellLayout_Copy;
	Stg_Component_DefaultConstructorFunction*    _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_TriSingleCellLayout_DefaultNew;
	Stg_Component_ConstructFunction*                      _construct = _TriSingleCellLayout_AssignFromXML;
	Stg_Component_BuildFunction*                              _build = _TriSingleCellLayout_Build;
	Stg_Component_InitialiseFunction*                    _initialise = _TriSingleCellLayout_Initialise;
	Stg_Component_ExecuteFunction*                          _execute = _TriSingleCellLayout_Execute;
	Stg_Component_DestroyFunction*                          _destroy = _TriSingleCellLayout_Destroy;
	AllocationType                                nameAllocationType = NON_GLOBAL;
	CellLayout_CellCountFunction*                    _cellLocalCount = _TriSingleCellLayout_CellLocalCount;
	CellLayout_CellCountFunction*                   _cellShadowCount = _TriSingleCellLayout_CellShadowCount;
	CellLayout_PointCountFunction*                       _pointCount = _TriSingleCellLayout_PointCount;
	CellLayout_InitialisePointsFunction*           _initialisePoints = _TriSingleCellLayout_InitialisePoints;
	CellLayout_MapElementIdToCellIdFunction*   _mapElementIdToCellId = _TriSingleCellLayout_MapElementIdToCellId;
	CellLayout_IsInCellFunction*                           _isInCell = _TriSingleCellLayout_IsInCell;
	CellLayout_CellOfFunction*                               _cellOf = _TriSingleCellLayout_CellOf;
	CellLayout_GetShadowInfoFunction*                 _getShadowInfo = _TriSingleCellLayout_GetShadowInfo;
	Dictionary*                                           dictionary = NULL;

	return (TriSingleCellLayout*)_TriSingleCellLayout_New(  TRISINGLECELLLAYOUT_PASSARGS  );
}

TriSingleCellLayout* _TriSingleCellLayout_New(  TRISINGLECELLLAYOUT_DEFARGS  ) {
	TriSingleCellLayout* self;
	
	/* Allocate memory */
	self = (TriSingleCellLayout*)_CellLayout_New(  CELLLAYOUT_PASSARGS  );
	
	/* General info */
	self->dictionary = dictionary;
	
	/* Virtual info */
	
	/* TriSingleCellLayout info */
	
	return self;
}

void _TriSingleCellLayout_Delete( void* triSingleCellLayout ) {
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	/* Stg_Class_Delete parent class */
	_CellLayout_Delete( self );
}

void _TriSingleCellLayout_Print( void* triSingleCellLayout, Stream* stream ) {
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	/* Set the Journal for printing informations */
	Stream* triSingleCellLayoutStream = stream;
	
	/* General info */
	Journal_Printf( triSingleCellLayoutStream, "TriSingleCellLayout (ptr): %p\n", self ); 
	
	/* Parent class info */
	_CellLayout_Print( self, stream );
	
	/* Virtual info */
	
	/* TriSingleCellLayout info */
	Journal_Printf( triSingleCellLayoutStream, "self->dim: %u", self->dim );
}

void* _TriSingleCellLayout_Copy( void* triSingleCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TriSingleCellLayout*	self = (TriSingleCellLayout*)triSingleCellLayout;
	TriSingleCellLayout*	newTriSingleCellLayout;
	
	newTriSingleCellLayout = _CellLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newTriSingleCellLayout->dictionary = self->dictionary;
	newTriSingleCellLayout->dim = self->dim;
	
	return (void*)newTriSingleCellLayout;
}
	
void _TriSingleCellLayout_AssignFromXML( void *triSingleCellLayout, Stg_ComponentFactory *cf, void* data ){
	TriSingleCellLayout	*self = (TriSingleCellLayout*)triSingleCellLayout;
	Dimension_Index		dim = 0;

	_CellLayout_AssignFromXML( self, cf, data );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0 );
	assert( dim );

	_TriSingleCellLayout_Init( (TriSingleCellLayout* )self, cf->rootDict, dim );
}
	
void _TriSingleCellLayout_Build( void* triSingleCellLayout, void* data ){
	
}
	
void _TriSingleCellLayout_Initialise( void* triSingleCellLayout, void* data ){
	
}
	
void _TriSingleCellLayout_Execute( void* triSingleCellLayout, void* data ){
	
}
	
void _TriSingleCellLayout_Destroy( void* triSingleCellLayout, void* data ){
	TriSingleCellLayout	*self = (TriSingleCellLayout*)triSingleCellLayout;

	_CellLayout_Destroy( self, data );	
}

Cell_Index _TriSingleCellLayout_CellLocalCount( void* triSingleCellLayout ) {
	/* There is only one cell... */
	return 1;
}


Cell_Index _TriSingleCellLayout_CellShadowCount( void* triSingleCellLayout ) {
	/* No shadow cells */
	return 0;
}


Cell_PointIndex _TriSingleCellLayout_PointCount( void* triSingleCellLayout, Cell_Index cellIndex ) {
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	switch( self->dim ) {
		case 1:
			return 2;
		case 2:
			return 3;
		case 3:
			return 4;
		default:
			assert( 0 );
	}
	return 0;
}


void _TriSingleCellLayout_InitialisePoints( 
		void*			triSingleCellLayout, 
		Cell_Index		cellIndex, 
		Cell_PointIndex		pointCount, 
		Cell_Points		points ) 
{
	TriSingleCellLayout* self = (TriSingleCellLayout*)triSingleCellLayout;
	
	switch( self->dim ) {
		case 1:
			assert( 0 );
		case 2:
			points[0] = Memory_Alloc_Array( double, self->dim, "points[0]" );
			points[1] = Memory_Alloc_Array( double, self->dim, "points[1]" );
			points[2] = Memory_Alloc_Array( double, self->dim, "points[2]" );
			
			(points[0])[0] = 0.0f;
			(points[0])[1] = 0.0f;
			(points[1])[0] = 1.0f;
			(points[1])[1] = 0.0f;
			(points[2])[0] = 0.0f;
			(points[2])[1] = 1.0f;
			break;
		case 3:
			assert( 0 );
		default:
			assert( 0 );
	}
}


Cell_Index _TriSingleCellLayout_MapElementIdToCellId( void* cellLayout, Element_DomainIndex element_dI ) {
	
	/* Always 0: see the header comment */
	return 0;
}


Bool _TriSingleCellLayout_IsInCell( void* triSingleCellLayout, Cell_Index cellIndex, void* particle ) {
	assert( 0 );
	return 0;
}


Cell_Index _TriSingleCellLayout_CellOf( void* triSingleCellLayout, void* particle ) {
	assert( 0 );
	return 0;
}


ShadowInfo* _TriSingleCellLayout_GetShadowInfo( void* triSingleCellLayout ) {
  /*TriSingleCellLayout*      self = (TriSingleCellLayout*)triSingleCellLayout; */

	/* TODO: this should return a shadow info with at least nbr info for my processors */
	Journal_Firewall( 0, Swarm_Warning, "Error: %s not implemented yet!\n", __func__ );
	return NULL;
}


