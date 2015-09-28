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

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "types.h"

#include "CellLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type CellLayout_Type = "CellLayout";

CellLayout* _CellLayout_New(  CELLLAYOUT_DEFARGS  ) {
	CellLayout* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(CellLayout) );
	self = (CellLayout*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	self->_cellLocalCount = _cellLocalCount;
	self->_cellShadowCount = _cellShadowCount;
	self->_pointCount = _pointCount;
	self->_initialisePoints = _initialisePoints;
	self->_mapElementIdToCellId = _mapElementIdToCellId;
	self->_isInCell = _isInCell;
	self->_cellOf = _cellOf;
	self->_getShadowInfo = _getShadowInfo;
	
	return self;
}

void _CellLayout_Init( CellLayout* self, AbstractContext* context ) {
	/* General and Virtual info should already be set */

	self->context = context;	
}

void _CellLayout_Delete( void* cellLayout ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	/* _Stg_Component_Delete parent*/
	_Stg_Component_Delete( self );
}

void _CellLayout_Print( void* cellLayout, Stream* stream ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	/* Set the Journal for printing informations */
	Stream* cellLayoutStream = stream;
	
	/* General info */
	Journal_Printf( cellLayoutStream, "CellLayout (ptr): %p\n", self );
	
	/* Print parent */
	_Stg_Component_Print( self, cellLayoutStream );
	
	/* Virtual info */
	Journal_Printf( cellLayoutStream, "\t_cellLocalCount (func ptr): %p\n", self->_cellLocalCount );
	Journal_Printf( cellLayoutStream, "\t_cellShadowCount (func ptr): %p\n", self->_cellShadowCount );
	Journal_Printf( cellLayoutStream, "\t_pointCount (func ptr): %p\n", self->_pointCount );
	Journal_Printf( cellLayoutStream, "\t_initialisePoints (func ptr): %p\n", self->_initialisePoints );
	Journal_Printf( cellLayoutStream, "\t_mapElementIdToCellId (func ptr): %p\n", self->_mapElementIdToCellId );
	Journal_Printf( cellLayoutStream, "\t_isInCell (func ptr): %p\n", self->_isInCell );
	Journal_Printf( cellLayoutStream, "\t_cellOf (func ptr): %p\n", self->_cellOf );
	Journal_Printf( cellLayoutStream, "\t_getShadowInfo (func ptr): %p\n", self->_getShadowInfo );
	
	/* CellLayout info */
}

void _CellLayout_Destroy( void* cellLayout, void* data ) {
}

void* _CellLayout_Copy( void* cellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	CellLayout*	self = (CellLayout*)cellLayout;
	CellLayout*	newCellLayout;
	
	newCellLayout = _Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Virtual methods */
	newCellLayout->_cellLocalCount = self->_cellLocalCount;
	newCellLayout->_cellShadowCount = self->_cellShadowCount;
	newCellLayout->_pointCount = self->_pointCount;
	newCellLayout->_initialisePoints = self->_initialisePoints;
	newCellLayout->_mapElementIdToCellId = self->_mapElementIdToCellId;
	newCellLayout->_isInCell = self->_isInCell;
	newCellLayout->_cellOf = self->_cellOf;
	newCellLayout->_getShadowInfo = self->_getShadowInfo;
	
	return (void*)newCellLayout;
}

void _CellLayout_AssignFromXML( void* cellLayout, Stg_ComponentFactory *cf, void* data ) {
	CellLayout*			self = (CellLayout*)cellLayout;
	AbstractContext*	context;

	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
   if( !context  )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, False, data  );

	_CellLayout_Init( self, context );
}

Cell_Index CellLayout_CellDomainCount( void* cellLayout ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_cellLocalCount( self ) + self->_cellShadowCount( self );
}

Cell_Index CellLayout_CellLocalCount( void* cellLayout ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_cellLocalCount( self );
}

Cell_Index CellLayout_CellShadowCount( void* cellLayout ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_cellShadowCount( self );
}

Cell_PointIndex CellLayout_PointCount( void* cellLayout, Cell_Index cell_I ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_pointCount( self, cell_I );
}

void CellLayout_InitialiseCellPoints( void* cellLayout, Cell_Index cell_I, Cell_PointIndex pointCount, Cell_Points points ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	self->_initialisePoints( self, cell_I, pointCount, points );
}


Cell_Index CellLayout_MapElementIdToCellId( void* cellLayout, unsigned element_dI ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_mapElementIdToCellId( self, element_dI );
}


Bool CellLayout_IsInCell( void* cellLayout, Cell_Index cellIndex, void* particle ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_isInCell( self, cellIndex, particle );
}

	
Cell_Index CellLayout_CellOf( void* cellLayout, void* particle ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_cellOf( self, particle );
}


ShadowInfo* CellLayout_GetShadowInfo( void* cellLayout ) {
	CellLayout* self = (CellLayout*)cellLayout;
	
	return self->_getShadowInfo( self );
}


