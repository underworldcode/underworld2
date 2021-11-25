/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "Set.h"
#include "PtrSet.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type PtrSet_Type = "PtrSet";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

PtrSet* PtrSet_New(
		Dictionary*					dictionary )
{
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(PtrSet);
	Type                                    type = PtrSet_Type;
	Stg_Class_DeleteFunction*            _delete = _PtrSet_Delete;
	Stg_Class_PrintFunction*              _print = _PtrSet_Print;
	Stg_Class_CopyFunction*                _copy = NULL;
	Set_UnionFunc*                    _unionFunc = _PtrSet_Union;
	Set_IntersectionFunc*      _intersectionFunc = _PtrSet_Intersection;
	Set_SubtractionFunc*        _subtractionFunc = _PtrSet_Subtraction;
	SizeT                            elementSize = 0;
	BTree_compareFunction*           compareFunc = _PtrSet_CompareData;
	BTree_dataCopyFunction*         dataCopyFunc = NULL;
	BTree_dataDeleteFunction*     dataDeleteFunc = _PtrSet_DeleteData;

	return _PtrSet_New(  PTRSET_PASSARGS  );
}


void PtrSet_Init(
		PtrSet*						self,
		Dictionary*					dictionary )
{
	/* General info */
	self->type = PtrSet_Type;
	self->_sizeOfSelf = sizeof(PtrSet);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _PtrSet_Delete;
	self->_print = _PtrSet_Print;
	self->_copy = NULL;
	self->_unionFunc = _PtrSet_Union;
	self->_intersectionFunc = _PtrSet_Intersection;
	self->_subtractionFunc = _PtrSet_Subtraction;
	_Set_Init( (Set*)self, 0, _PtrSet_CompareData, NULL, _PtrSet_DeleteData );
	
	/* PtrSet info */
	_PtrSet_Init( self );
}


PtrSet* _PtrSet_New(  PTRSET_DEFARGS  )
{
	PtrSet*	self;
	
	/* allocate memory */
	assert( _sizeOfSelf >= sizeof(PtrSet) );
	self = (PtrSet*)_Set_New(  SET_PASSARGS  );
	
	/* general info */
	
	/* virtual info */
	
	/* PtrSet info */
	_PtrSet_Init( self );
	
	return self;
}


void _PtrSet_Init(
		PtrSet*						self )
{
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _PtrSet_Delete( void* ptrPtrSet ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	
	/* delete the class itself */
	
	/* delete parent */
	_Set_Delete( self );
}


void _PtrSet_Print( void* ptrPtrSet, Stream* stream ) {
	PtrSet*		self = (PtrSet*)ptrPtrSet;
	Stream*		myStream = Journal_Register( InfoStream_Type, "PtrSetStream" );

	/* print parent */
	_Set_Print( self, stream );
	
	/* general info */
	Journal_Printf( myStream, "PtrSet (ptr): (%p)\n", self );
	Journal_Printf( myStream, "\tdictionary (ptr): %p\n", self->dictionary );
	
	/* virtual info */
	
	/* PtrSet info */
}


void* _PtrSet_Union( void* ptrPtrSet, void* operand ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	PtrSet*	results;

	results = PtrSet_New( self->dictionary );

	BTree_ParseTree( self->_btree, _Set_BTreeUnion, results );
	BTree_ParseTree( ((PtrSet*)operand)->_btree, _Set_BTreeUnion, results );

	return results;
}


void* _PtrSet_Intersection( void* ptrPtrSet, void* operand ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	PtrSet*	results;
	void*	pack[2];

	results = PtrSet_New( self->dictionary );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Set_BTreeIntersection, pack );

	return results;
}


void* _PtrSet_Subtraction( void* ptrPtrSet, void* operand ) {
	PtrSet*	self = (PtrSet*)ptrPtrSet;
	PtrSet*	results;
	void*	pack[2];

	results = PtrSet_New( self->dictionary );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Set_BTreeSubtraction, pack );

	return results;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

int _PtrSet_CompareData( void* left, void* right ) {
	if( (ArithPointer)left < (ArithPointer)right ) {
		return -1;
	}
	else if( (ArithPointer)left > (ArithPointer)right ) {
		return 1;
	}
	else {
		return 0;
	}
}


void _PtrSet_DeleteData( void* data ) {
}


