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
#include "LinkedListNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type LinkedListNode_Type = "LinkedListNode";

LinkedListNode* LinkedListNode_New( void )
{
	LinkedListNode* self;
	
	/* Allocate memory */
	self = Memory_Alloc( LinkedListNode, "LinkedListNode" );
	memset ( self, 0, sizeof ( LinkedListNode ) );

	assert( self );
	/* General info */
	
	/* Virtual functions */
	
	self->next = NULL;
	_LinkedListNode_Init( self );
	
	return self;
}

void _LinkedListNode_Init( LinkedListNode* self ){
	
	/* General info */
	
	/* Virtual info */
	
	/* ListNode info */
		assert( self );
	self->data = NULL;
	self->sizeOfData = 0;
}


