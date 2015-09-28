/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <Base/Foundation/Foundation.h>
#include <Base/IO/IO.h>

#include "types.h"
#include "ChunkArray.h"
#include "BTreeNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type BTreeNode_Type = "BTreeNode";

BTreeNode terminal = { BTREE_NODE_BLACK, NULL, 0, 0, NULL, NULL, NULL };

BTreeNode* BTreeNode_New( ChunkArray *pool )
{
	BTreeNode* self;
	
	/* Allocate memory */

	if( pool ){
		unsigned int objID = -1;
		assert( sizeof( BTreeNode ) == pool->elementSize );
		objID = ChunkArray_NewObjectID( BTreeNode, pool );
		
		self = ( BTreeNode* ) ChunkArray_ObjectAt(pool, objID);
		memset ( self, 0, sizeof ( BTreeNode ) );
		
		self->id = objID;
	}
	else{
		self = ( BTreeNode* ) malloc ( sizeof( BTreeNode ) );
		memset ( self, 0, sizeof ( BTreeNode ) );
	}

	assert ( self );
	/* General info */
	
	/* Virtual functions */
	
	/* NamedObject info */
	self->parent = NULL;
	self->left = NIL;
	self->right = NIL;
	self->color = BTREE_NODE_RED;
	_BTreeNode_Init( self );
	
	return self;
}

void _BTreeNode_Init( BTreeNode* self ){
	
	/* General info */
	
	/* Virtual info */
	
	/* BTreeNode info */
	assert ( self );
	self->data = NULL;
	self->sizeOfData = 0;
}



