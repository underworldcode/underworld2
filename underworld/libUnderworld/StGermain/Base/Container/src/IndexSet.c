/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "IndexSet.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type IndexSet_Type = "IndexSet";

/** Macro to calculate container membership */
#define IS_MEMBER( indexSet, index ) \
	((indexSet)->_container[(index) / (sizeof(char) * 8 )] & (1 << ((index) % (sizeof(char) * 8))))

IndexSet* IndexSet_New( IndexSet_Index size ) {
	/* Variables set in this function */
	SizeT                        _sizeOfSelf = sizeof(IndexSet);
	Type                                type = IndexSet_Type;
	Stg_Class_DeleteFunction*        _delete = _IndexSet_Delete;
	Stg_Class_PrintFunction*          _print = _IndexSet_Print;
	Stg_Class_CopyFunction*            _copy = _IndexSet_Copy;
	IndexSet_DuplicateFunction*   _duplicate = _IndexSet_Duplicate;

	return _IndexSet_New(  INDEXSET_PASSARGS  );
}

void IndexSet_Init( IndexSet* self, IndexSet_Index size ) {
	/* General info */
	self->type = IndexSet_Type;
	self->_sizeOfSelf = sizeof(IndexSet);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _IndexSet_Delete;
	self->_print = _IndexSet_Print;
	self->_copy = _IndexSet_Copy;
	self->_duplicate = _IndexSet_Duplicate;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* IndexSet info */
	_IndexSet_Init( self, size );
}


IndexSet* _IndexSet_New(  INDEXSET_DEFARGS  )
{
	IndexSet* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IndexSet) );
	self = (IndexSet*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	self->_duplicate = _duplicate;
	
	/* IndexSet info */
	_IndexSet_Init( self, size );
	
	return self;
}


void _IndexSet_Init( IndexSet* self, IndexSet_Index size ) {
	/* General and Virtual info should already be set */
	
	/* IndexSet info */
	self->size = size;
	self->_containerSize = self->size / (sizeof(char) * 8) + (self->size % (sizeof(char) * 8) ? 1 : 0);
	self->_container = Memory_Alloc_Array( BitField, self->_containerSize, "IndexSet->_container");
	memset( self->_container, 0, sizeof(char) * self->_containerSize );
	self->membersCount = 0;
	self->error = Journal_Register( ErrorStream_Type, self->type );
}


void _IndexSet_Delete( void* indexSet ) {
	IndexSet* self = (IndexSet*)indexSet;
	
	if( self->_container ) {
		Memory_Free( self->_container );
	}
	
	/* Stg_Class_Delete parent class */
	_Stg_Class_Delete( self );
}


void _IndexSet_Print( void* indexSet, Stream* stream ) {
	IndexSet* self = (IndexSet*)indexSet;
	
	/* Set the Journal for printing informations */
	Stream* indexSetStream = Journal_Register( InfoStream_Type, "IndexSetStream");
	
	/* General info */
	Journal_Printf( indexSetStream, "IndexSet (%p):\n", self );
	
	/* Virtual info */
	
	/* IndexSet */
	Journal_Printf( indexSetStream, "\tsize: %u\n", self->size );
	Journal_Printf( indexSetStream, "\t_containerSize: %lu\n", self->_containerSize );
	Journal_Printf( indexSetStream, "\t_container: %p\n", self->_container );

	if( self->_container ) {
		IndexSet_Index		index_I;
		
		for( index_I = 0; index_I < self->size; index_I++ ) {
			Index			containerIndex;
			char			indexMask;
			
			containerIndex = index_I / (sizeof(char) * 8);
			indexMask = 1 << (index_I % (sizeof(char) * 8));
			
			if( self->_container[containerIndex] & indexMask ) {
				Journal_Printf( indexSetStream, "\t\tindex %u: In set.\n", index_I );
			}
			else {
				Journal_Printf( indexSetStream, "\t\tindex %u: Not in set.\n", index_I );
			}
		}
	}
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
}


void* _IndexSet_Copy( void* indexSet, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	newIndexSet;
	
	newIndexSet = (IndexSet*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Virtual methods */
	newIndexSet->_duplicate = self->_duplicate;
	
	newIndexSet->size = self->size;
	newIndexSet->_containerSize = self->_containerSize;
	newIndexSet->_container = Memory_Alloc_Array( char, newIndexSet->_containerSize, "IndexSet->_container" );
	memcpy( newIndexSet->_container, self->_container, sizeof(char) * newIndexSet->_containerSize );
	newIndexSet->membersCount = self->membersCount;
	newIndexSet->error = self->error;
	
	return (void*)newIndexSet;
}


void IndexSet_Add( void* indexSet, Index index ) {
	IndexSet* self = (IndexSet*)indexSet;

    Journal_Firewall( index < self->size, self->error, "Error- %s: index %u outside current size %d. Aborting.",
        __func__, index, self->size);
	
	self->_container[index / (sizeof(char) * 8)] |= (1 << (index % (sizeof(char) * 8)));
	self->membersCount = (unsigned)-1;
}


void IndexSet_Remove( void* indexSet, Index index ) {
	IndexSet* self = (IndexSet*)indexSet;

    Journal_Firewall( index < self->size, self->error, "Error- %s: index %u outside current size %d. Aborting.",
        __func__, index, self->size);
	
	self->_container[index / (sizeof(char) * 8)] &= ~(1 << (index % (sizeof(char) * 8)));
	self->membersCount = (unsigned)-1;
}

Bool IndexSet_IsMember( void* indexSet, Index index ) {
	IndexSet* self = (IndexSet*)indexSet;
	Bool retFlag = False;

    Journal_Firewall( index < self->size, self->error, "Error- %s: index %u outside current size %d. Aborting.",
        __func__, index, self->size);
	
	if ( 0 == IS_MEMBER( self, index ) ) {
		retFlag = False;
	}
	else {
		retFlag = True;
	}
	
	return retFlag;
}

IndexSet_Index IndexSet_GetIndexOfNthMember( void* indexSet, const Index nthMember ) {
	IndexSet*		self = (IndexSet*)indexSet;
	int		membersSoFar = -1;
	IndexSet_Index	index_I;
	
	for( index_I = 0; index_I < self->size; index_I++ ) {
		if( IS_MEMBER( self, index_I ) ) {
			membersSoFar++;
			if ( nthMember == membersSoFar ) {
				return index_I;
			}
		}
	}

	return IndexSet_Invalid( self );
}


IndexSet_Index IndexSet_UpdateMembersCount( void* indexSet ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;

	if (self->membersCount == (unsigned)-1 ) {
		self->membersCount = 0;
		for( index_I = 0; index_I < self->size; index_I++ ) {
			if( IS_MEMBER( self, index_I ) ) {
				self->membersCount++;
			}
		}
	}

	return self->membersCount;
}


void IndexSet_GetMembers( void* indexSet, IndexSet_Index* countPtr, Index** arrayPtr ) {
	IndexSet*		self = (IndexSet*)indexSet;
	
	*countPtr = IndexSet_UpdateMembersCount( self );
	if( *countPtr ) {
		*arrayPtr = Memory_Alloc_Array( Index, (*countPtr), "IndexSet members" );
		IndexSet_GetMembers2( self, *arrayPtr );
	}
	else {
		*arrayPtr = NULL;
	}
}	


void IndexSet_GetMembers2( void* indexSet, Index* const array ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;
	unsigned int		array_I;

	for( array_I = 0, index_I = 0; index_I < self->size; index_I++ ) {
		if( IS_MEMBER( self, index_I ) ) {
			array[array_I] = index_I;
			array_I++;
		}
	}
}


void IndexSet_GetVacancies( void* indexSet, IndexSet_Index* countPtr, Index** arrayPtr ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;
	unsigned int		array_I;

	IndexSet_UpdateMembersCount( self );
	*countPtr = self->size - self->membersCount;
	
	*arrayPtr = Memory_Alloc_Array( Index, (*countPtr), "IndexSet vacancies" );
	for( array_I = 0, index_I = 0; index_I < self->size; index_I++ ) {
		if( !IS_MEMBER( self, index_I) ) {
			(*arrayPtr)[array_I] = index_I;
			array_I++;
		}
	}
}

void IndexSet_Invert( void* indexSet ) {
	IndexSet*		self = (IndexSet*)indexSet;
	IndexSet_Index		index_I;
	
	for( index_I = 0; index_I < self->size; index_I++ ) {
		if( IS_MEMBER( self, index_I) )
			IndexSet_Remove( indexSet, index_I);
		else
            IndexSet_Add( indexSet, index_I);
	}
}


void IndexSet_Merge_OR(void* indexSet, void* indexSetTwo )
{
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	secondSet = (IndexSet*)indexSetTwo;
	Index		size;
	Index		i;
	
	size = self->size <= secondSet->size ? self->_containerSize : secondSet->_containerSize;
	
	for (i = 0; i < size; i++)
		self->_container[i] |= secondSet->_container[i];
	
	self->membersCount = (unsigned int)-1;
}


void IndexSet_Merge_AND(void* indexSet, void* indexSetTwo )
{
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	secondSet = (IndexSet*)indexSetTwo;
	Index		size;
	Index		i;
	
	size = self->size <= secondSet->size ? self->_containerSize : secondSet->_containerSize;
	
	for (i = 0; i < size; i++)
		self->_container[i] &= secondSet->_container[i];

	/* As specified in header file description, if first given IndexSet is larger, zero out the
	 * remaining entries since this is an AND operation */
	for( i = size; i < self->_containerSize; i++) {
		self->_container[i] &= 0x00;
	}
	
	self->membersCount = (unsigned int)-1;
}


void IndexSet_AddAll( void* indexSet )
{
	IndexSet*	self = (IndexSet*)indexSet;
	Index		i;
	
	for( i = 0; i < self->_containerSize; i++)
		self->_container[i] |= 0xFF;

	self->membersCount = self->size;
}


void IndexSet_RemoveAll( void* indexSet )
{
	IndexSet*	self = (IndexSet*)indexSet;
	Index		i;
	
	for( i = 0; i < self->_containerSize; i++)
		self->_container[i] &= 0x00;

	self->membersCount = 0;
}


IndexSet* IndexSet_Duplicate( void* indexSet ) {
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	newSelf = 0;
	
	if( self  ) {
		newSelf = _IndexSet_New( self->_sizeOfSelf, self->type, self->_delete, self->_print, NULL, self->_duplicate, self->size );
		self->_duplicate( self, newSelf );
	}
	return newSelf;
}


void _IndexSet_Duplicate( void* indexSet, void* newIndexSet ){
	/* self->containerSize and self->container are set by _IndexSet_Init */
	IndexSet*	self = (IndexSet*)indexSet;
	IndexSet*	newSet = (IndexSet*)newIndexSet;
	memcpy( ((IndexSet*)newIndexSet)->_container, ((IndexSet*)indexSet)->_container, sizeof(char)*((IndexSet*)indexSet)->_containerSize );
	newSet->membersCount = self->membersCount;
}


