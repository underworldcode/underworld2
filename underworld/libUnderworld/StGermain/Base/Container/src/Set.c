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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* Textual name of this class */
const Type Set_Type = "Set";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

Set* Set_New_all(
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(Set);
	Type                                    type = Set_Type;
	Stg_Class_DeleteFunction*            _delete = _Set_Delete;
	Stg_Class_PrintFunction*              _print = _Set_Print;
	Stg_Class_CopyFunction*                _copy = NULL;
	Set_UnionFunc*                    _unionFunc = _Set_Union;
	Set_IntersectionFunc*      _intersectionFunc = _Set_Intersection;
	Set_SubtractionFunc*        _subtractionFunc = _Set_Subtraction;

	return _Set_New(  SET_PASSARGS  );
}


void Set_Init(
		Set*						self,
		Dictionary*					dictionary, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	/* General info */
	self->type = Set_Type;
	self->_sizeOfSelf = sizeof(Set);
	self->_deleteSelf = False;
	self->dictionary = dictionary;
	
	/* Virtual info */
	self->_delete = _Set_Delete;
	self->_print = _Set_Print;
	self->_copy = NULL;
	self->_unionFunc = _Set_Union;
	self->_intersectionFunc = _Set_Intersection;
	self->_subtractionFunc = _Set_Subtraction;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* Set info */
	_Set_Init( self, elementSize, compareFunc, dataCopyFunc, dataDeleteFunc );
}


Set* _Set_New(  SET_DEFARGS  )
{
	Set*	self;
	
	/* allocate memory */
	assert( _sizeOfSelf >= sizeof(Set) );
	self = (Set*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* general info */
	self->dictionary = dictionary;
	
	/* virtual info */
	self->_unionFunc = _unionFunc;
	self->_intersectionFunc = _intersectionFunc;
	self->_subtractionFunc = _subtractionFunc;
	
	/* Set info */
	_Set_Init( self, elementSize, compareFunc, dataCopyFunc, dataDeleteFunc );
	
	return self;
}


void _Set_Init(
		Set*						self, 
		SizeT						elementSize, 
		BTree_compareFunction*				compareFunc, 
		BTree_dataCopyFunction*				dataCopyFunc, 
		BTree_dataDeleteFunction*			dataDeleteFunc )
{
	/* TODO: convert to journal */
	assert( compareFunc && dataDeleteFunc );

	self->_elSize = elementSize;
	self->_btree = BTree_New( compareFunc, dataCopyFunc, dataDeleteFunc, NULL, BTREE_ALLOW_DUPLICATES );
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Set_Delete( void* set ) {
	Set*	self = (Set*)set;
	
	/* delete the class itself */
	Stg_Class_Delete( self->_btree );
	
	/* delete parent */
	_Stg_Class_Delete( self );
}


void _Set_Print( void* set, Stream* stream ) {
	Set*		self = (Set*)set;
	Stream*		myStream = Journal_Register( InfoStream_Type, "SetStream" );

	/* print parent */
	_Stg_Class_Print( self, stream );
	
	/* general info */
	Journal_Printf( myStream, "Set (ptr): (%p)\n", self );
	Journal_Printf( myStream, "\tdictionary (ptr): %p\n", self->dictionary );
	
	/* virtual info */
	
	/* Set info */
}


void* _Set_Union( void* set, void* operand ) {
	Set*	self = (Set*)set;
	Set*	results;

	results = Set_New_all( self->dictionary, 
			       self->_elSize, 
			       self->_btree->compareFunction, 
			       self->_btree->dataCopyFunction, 
			       self->_btree->dataDeleteFunction );

	BTree_ParseTree( self->_btree, _Set_BTreeUnion, results );
	BTree_ParseTree( ((Set*)operand)->_btree, _Set_BTreeUnion, results );

	return results;
}


void* _Set_Intersection( void* set, void* operand ) {
	Set*	self = (Set*)set;
	Set*	results;
	void*	pack[2];

	results = Set_New_all( self->dictionary, 
			       self->_elSize, 
			       self->_btree->compareFunction, 
			       self->_btree->dataCopyFunction, 
			       self->_btree->dataDeleteFunction );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Set_BTreeIntersection, pack );

	return results;
}


void* _Set_Subtraction( void* set, void* operand ) {
	Set*	self = (Set*)set;
	Set*	results;
	void*	pack[2];

	results = Set_New_all( self->dictionary, 
			       self->_elSize, 
			       self->_btree->compareFunction, 
			       self->_btree->dataCopyFunction, 
			       self->_btree->dataDeleteFunction );

	pack[0] = operand;
	pack[1] = results;
	BTree_ParseTree( self->_btree, _Set_BTreeSubtraction, pack );

	return results;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

Bool Set_Insert( void* set, void* data ) {
	Set*      self = (Set*)set;

	if( BTree_FindNode( self->_btree, data ) != NULL ) {
		return False;
	}

	BTree_InsertNode( self->_btree, data, self->_elSize );

	return True;
}


void Set_Traverse( void* set, BTree_parseFunction* func, void* args ) {
	Set*      self = (Set*)set;

	BTree_ParseTree( self->_btree, func, args );
}


Bool Set_Exists( void* set, void* data ) {
	Set*      self = (Set*)set;

	return (BTree_FindNode( self->_btree, data ) != NULL) ? True : False;
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Private Functions
*/

void _Set_BTreeUnion( void* data, void* set ) {
	Set_Insert( (Set*)set, data );
}


void _Set_BTreeIntersection( void* data, void* pack ) {
	if( BTree_FindNode( ((Set**)pack)[0]->_btree, data ) != NULL ) {
		Set_Insert( ((Set**)pack)[1], data );
	}
}


void _Set_BTreeSubtraction( void* data, void* pack ) {
	if( BTree_FindNode( ((Set**)pack)[0]->_btree, data ) == NULL ) {
		Set_Insert( ((Set**)pack)[1], data );
	}
}


