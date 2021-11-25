/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <StGermain/Base/Foundation/src/Foundation.h>
#include <StGermain/Base/IO/src/IO.h>

#include "types.h"
#include "BTreeNode.h"
#include "BTree.h"
#include "BTreeIterator.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type BTreeIterator_Type = "BTreeIterator";

BTreeIterator *_BTreeIterator_New(
		SizeT					_sizeOfSelf,
		Type					type,
		Stg_Class_DeleteFunction*	_delete,
		Stg_Class_PrintFunction*	_print,
		Stg_Class_CopyFunction*		_copy
		)
{
	BTreeIterator *self = NULL;

	/** BTreeIterator has to be malloced instead of using Class_New, because Class_New uses Memory_Alloc, 
	 *  but the Memory module will not have been initialized at this stage */
	
	self = (BTreeIterator*)malloc( _sizeOfSelf );
	memset( self, 0, _sizeOfSelf );
	
	self->_sizeOfSelf = _sizeOfSelf;
	self->_deleteSelf = True;
	self->type = type;
	self->_delete = _delete;
	self->_print = _print;
	self->_copy = _copy;
	
	return self;
}

BTreeIterator *BTreeIterator_New( BTree *tree )
{
	
	BTreeIterator* self;
	
	assert( tree );
	self = _BTreeIterator_New(
			sizeof(BTreeIterator),
			BTreeIterator_Type,
			_BTreeIterator_DeleteFunc,
			_BTreeIterator_PrintFunc,
			NULL
			);
	/* General info */
	
	/* Virtual functions */
	
	self->tree = tree;
	
	BTreeIterator_Init( self );	
	return self;
}

void _BTreeIterator_Init( BTreeIterator* self )
{

	assert(self);
	self->depth = 0;
}

void BTreeIterator_Init( BTreeIterator *self )
{
	_BTreeIterator_Init( self );
}

void *BTreeIterator_First( BTreeIterator *self  )
{
	BTreeNode *node = NULL;
	assert( self );
	assert( self->tree );

	self->depth = -1;

	node = self->tree->root;
	while( node != NIL ){
		self->depth++;
		self->stack[self->depth] = node;
		node = node->left;
	}
	
	if( self->depth < 0 ){
		return NULL;
	}
	return self->stack[self->depth]->data;
}

void *BTreeIterator_Next( BTreeIterator *self )
{
	BTreeNode *node = NULL;
	
	assert( self );
	assert( self->tree );

	if( self->depth < 0 ){
		return NULL;
	}
	
	node = self->stack[self->depth];
	self->depth--;
	node = node->right;
	
	while( node != NIL ){
		self->depth ++;
		self->stack[self->depth] = node;
		node = node->left;
	}

	if( self->depth < 0 ){
		return NULL;
	}
	else{
		return self->stack[self->depth]->data;
	}
}

void _BTreeIterator_DeleteFunc( void *self )
{
	if( self ){
		free( self );
		/* freeing the Iterator instead of using class_delete, because it was initially malloced */
	}
}

void _BTreeIterator_PrintFunc( void *self, Stream *myStream )
{
	BTreeIterator *iterator = NULL;

	iterator = (BTreeIterator*) self;
	assert( iterator );

	/* print parent */
	_Stg_Class_Print( (void*) iterator, myStream );

	/* general info */
	Journal_Printf( myStream, "BTreeIterator (ptr): (%p)\n", iterator );
	Journal_Printf( myStream, "\tTree (ptr): (%p)\n", iterator->tree );
	Journal_Printf( myStream, "\tDepth : %d\n", iterator->depth );
}


