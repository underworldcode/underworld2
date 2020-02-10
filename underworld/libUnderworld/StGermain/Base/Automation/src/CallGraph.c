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
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "CallGraph.h"

#include <string.h>


/* Textual name of this class */
const Type Stg_CallGraph_Type = "Stg_CallGraph";

static const Type _Stg_CallGraph_Stack_Type = "_Stg_CallGraph_Stack";
static const Type _Stg_CallGraph_Entry_Type = "_Stg_CallGraph_Entry";
static const Index _Stg_CallGraph_Table_Delta = 256; /* TODO: ascertain whether this is a good for an initial & delta size */

/* Call graph singleton. The code in this file (this class) shouldn't use this global variable. It is a global variable ONLY
    because adding it to the context and making the appropriate changes to EntryPoints and Components (and where ever else
    push and pop may end up, is a big deal. */
Stg_CallGraph* stgCallGraph = 0;


Stg_CallGraph* Stg_CallGraph_New() {
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(Stg_CallGraph);
	Type                              type = Stg_CallGraph_Type;
	Stg_Class_DeleteFunction*      _delete = _Stg_CallGraph_Delete;
	Stg_Class_PrintFunction*        _print = _Stg_CallGraph_Print;
	Stg_Class_CopyFunction*          _copy = _Stg_CallGraph_Copy;

	return _Stg_CallGraph_New(  STG_CALLGRAPH_PASSARGS  );
}

void Stg_CallGraph_Init( void* callGraph ) {
	Stg_CallGraph* self = (Stg_CallGraph*)callGraph;
	
	/* General info */
	self->type = Stg_CallGraph_Type;
	self->_sizeOfSelf = sizeof(Stg_CallGraph);
	self->_deleteSelf = False;
	
	/* Virtual info */
	self->_delete = _Stg_CallGraph_Delete;
	self->_print = _Stg_CallGraph_Print;
	self->_copy = _Stg_CallGraph_Copy;
	_Stg_Class_Init( (Stg_Class*)self );
	
	/* Stg_CallGraph info */
	_Stg_CallGraph_Init( self );
};

Stg_CallGraph* _Stg_CallGraph_New(  STG_CALLGRAPH_DEFARGS  )
{
	Stg_CallGraph* self;
	
	/* Allocate memory */
	Journal_Firewall( 
		_sizeOfSelf >= sizeof(Stg_CallGraph), 
		Journal_Register( Error_Type, Stg_CallGraph_Type ), 
		"Attempting to construct an object that is smaller than this class" );
	self = (Stg_CallGraph*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	/* General info */
	
	/* Virtual functions */
	
	/* Stg_CallGraph info */
	_Stg_CallGraph_Init( self );
	
	return self;
}


void _Stg_CallGraph_Init( Stg_CallGraph* self ) {
	/* General and Virtual info should already be set */
	
	/* Stg_CallGraph info */
	self->_stack = 0;
	self->_tableSize = _Stg_CallGraph_Table_Delta; 
	self->_tableCount = 0;
	self->table = Memory_Alloc_Array( _Stg_CallGraph_Entry, self->_tableSize, _Stg_CallGraph_Entry_Type );
}


static void _Stg_CallGraph_Delete_Stack( _Stg_CallGraph_Stack* stack ) {
	if( stack ) {
		_Stg_CallGraph_Delete_Stack( stack->pop );
		Memory_Free( stack );
	}
}

void _Stg_CallGraph_Delete( void* callGraph ) {
	Stg_CallGraph* self = (Stg_CallGraph*)callGraph;
	
	/* Stg_CallGraph info */
	_Stg_CallGraph_Delete_Stack( self->_stack );
	if( self->table ) {
		Memory_Free( self->table );
	}
	self->_tableCount = self->_tableSize = 0;
	
	/* Delete parent class */
	_Stg_Class_Delete( self );
}


static void _Stg_CallGraph_Print_Stack( _Stg_CallGraph_Stack* stack, Stream* stream ) {
	if( stack ) {
		Journal_Printf( (void*) stream, "\t\tfunction pointer (ptr): %p\n", stack->functionPtr );
		Journal_Printf( (void*) stream, "\t\tfunction name: \"%s\"\n", stack->name );
		Journal_Printf( (void*) stream, "\t\tfunction started timestamp: %g\n", stack->time );
		Journal_Printf( (void*) stream, "\t\tfunction memory stamp at start: %u\n", stack->memory );
		Journal_Printf( (void*) stream, "\t\tfunction table entry: %u\n", stack->entry_I );
		_Stg_CallGraph_Print_Stack( stack->pop, stream );
	}
}

void _Stg_CallGraph_Print( void* callGraph, Stream* stream ) {
	Stg_CallGraph*	self = (Stg_CallGraph*)callGraph;
	Index		i;
	
	/* General info */
	Journal_Printf( (void*) stream, "Stg_CallGraph (ptr): %p\n", self );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	
	/* Stg_CallGraph */
	Journal_Printf( (void*) stream, "\t_stack (ptr): %p\n", self->_stack );
	_Stg_CallGraph_Print_Stack( self->_stack, stream );
	
	Journal_Printf( (void*) stream, "\t_tableSize: %u\n", self->_tableSize );
	Journal_Printf( (void*) stream, "\t_tableCount: %u\n", self->_tableCount );
	Journal_Printf( (void*) stream, "\ttable (ptr): %p\n", self->table );
	for( i = 0; i < self->_tableCount; i++ ) {
		Journal_Printf( (void*) stream, 
			"\t\tfunction (ptr): %p, parent (ptr): %p, name: \"%s\", parent name: \"%s\", time: %g, memory: %u, called: %u, returned: %u\n",
			self->table[i].functionPtr, 
			self->table[i].parentPtr, 
			self->table[i].name,
			self->table[i].parentName,
			self->table[i].time, 
			self->table[i].memory,
			self->table[i].called,
			self->table[i].returned );
	}
}


static void _Stg_CallGraph_Copy_Table( _Stg_CallGraph_Entry* dst, _Stg_CallGraph_Entry* src, Index size, Index count ) {
	memcpy( dst, src, sizeof(_Stg_CallGraph_Entry) * count );
}

static void _Stg_CallGraph_Copy_Stack_Deep( _Stg_CallGraph_Stack** dst, _Stg_CallGraph_Stack* src, PtrMap* map ) {
	if( dst && !src ) {
		*dst = src;
	}
	else if( dst && (*dst = (_Stg_CallGraph_Stack*) PtrMap_Find( map, src )) == NULL ) {
		*dst = Memory_Alloc( _Stg_CallGraph_Stack, "Stg_CallGraph->_stack" );
		(*dst)->functionPtr = src->functionPtr;
		(*dst)->name = src->name;
		(*dst)->time = src->time;
		(*dst)->memory = src->memory;
		(*dst)->entry_I = src->entry_I;
		(*dst)->pop = 0;
		_Stg_CallGraph_Copy_Stack_Deep( &((*dst)->pop), src->pop, map );
		PtrMap_Append( map, *dst, src );
	}
	
}

static void _Stg_CallGraph_Copy_Stack_ShallowIncr( _Stg_CallGraph_Stack* stack ) {
	_Stg_CallGraph_Copy_Stack_ShallowIncr( stack->pop );
}

static void _Stg_CallGraph_Copy_Stack_Shallow( _Stg_CallGraph_Stack** dst, _Stg_CallGraph_Stack* src ) {
	*dst = src;
	_Stg_CallGraph_Copy_Stack_ShallowIncr( *dst );
}

void* _Stg_CallGraph_Copy( void* callGraph, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	Stg_CallGraph*	self = (Stg_CallGraph*)callGraph;
	Stg_CallGraph*	newCallGraph;
	PtrMap*		map = ptrMap;
	Bool		ownMap= False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newCallGraph = (Stg_CallGraph*) _Stg_Class_Copy( callGraph, dest, deep, nameExt, ptrMap );
	
	/* Virtual methods */
	newCallGraph->_tableSize = self->_tableSize;
	newCallGraph->_tableCount = self->_tableCount;

	if( deep ) {
		if( self->table && (newCallGraph->table = (_Stg_CallGraph_Entry*) PtrMap_Find( map, self->table )) == NULL ) {
			newCallGraph->table = Memory_Alloc_Array( _Stg_CallGraph_Entry, newCallGraph->_tableSize, 
				"Stg_CallGraph->table" );
			_Stg_CallGraph_Copy_Table( newCallGraph->table, self->table, newCallGraph->_tableSize, 
				newCallGraph->_tableCount );
			PtrMap_Append( map, newCallGraph->table, self->table );
		}
		
		_Stg_CallGraph_Copy_Stack_Deep( &newCallGraph->_stack, self->_stack, map );
	}
	else {
		newCallGraph->table = self->table;
		_Stg_CallGraph_Copy_Stack_Shallow( &newCallGraph->_stack, self->_stack );
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}

	return newCallGraph;
}



void Stg_CallGraph_Push( void* callGraph, void* functionPtr, Type name ) {
	Stg_CallGraph*		self = (Stg_CallGraph*)callGraph;
	_Stg_CallGraph_Stack*	newStack;
	Bool			found;
	void*			parentPtr;
	Type			parentName;
	
	/* Push onto the stack */
	parentPtr = self->_stack ? self->_stack->functionPtr : 0;
	parentName = self->_stack ? self->_stack->name : 0;
	newStack = Memory_Alloc( _Stg_CallGraph_Stack, _Stg_CallGraph_Stack_Type );
	newStack->functionPtr = functionPtr;
	newStack->name = name;
	newStack->time = 0.0f; /* TODO: Actual time stamp */
	newStack->memory = 0; /* TODO: Actual memory stamp */
	newStack->pop = self->_stack;
	self->_stack = newStack;
	
	/* Search for the function in the table */
	found = False;
	for( newStack->entry_I = 0; newStack->entry_I < self->_tableCount; newStack->entry_I++ ) {
		/* Each function/parent-pointer - function/parent name, is treated as a distinct call graph entry. This way, if a
		    function is called by more than one distinct function, we have that information (the user can combine this
		    info if they see fit. And in the case of entry points, the same function pointer is used for all entry points,
		    as the name is the distinquisher */
		if( 
			self->table[newStack->entry_I].functionPtr == functionPtr &&
			self->table[newStack->entry_I].parentPtr == parentPtr &&
			self->table[newStack->entry_I].name == name && 
			self->table[newStack->entry_I].parentName == parentName ) 
		{
			found = True;
			break;
		}
	}
	
	if( found ) {
		self->table[newStack->entry_I].called += 1;
	}
	/* else, add to the table */
	else {
		/* We may need to allocate some more memory */
		if( self->_tableCount >= self->_tableSize ) {
			_Stg_CallGraph_Entry* newTable;
			SizeT newSize;
			
			Journal_Firewall( 
				self->_tableCount == self->_tableSize, 
				Journal_Register( Error_Type, Stg_CallGraph_Type ), 
				"Somehow the table entry counter is greater than the memory allocation" );

			
			newSize = self->_tableSize + _Stg_CallGraph_Table_Delta;
			newTable = Memory_Alloc_Array( _Stg_CallGraph_Entry, newSize, _Stg_CallGraph_Entry_Type );
			/* safe to do, we want to copy ptr values */
			memcpy( newTable, self->table, self->_tableSize * sizeof(_Stg_CallGraph_Entry) ); 
			Memory_Free( self->table );
			self->table = newTable;
			self->_tableSize = newSize;
		}
		
		newStack->entry_I = self->_tableCount;
		self->_tableCount += 1;
		
		self->table[newStack->entry_I].functionPtr = functionPtr;
		self->table[newStack->entry_I].name = name;
		self->table[newStack->entry_I].parentPtr = parentPtr;
		self->table[newStack->entry_I].parentName = parentName;
		self->table[newStack->entry_I].time = 0.0f;
		self->table[newStack->entry_I].memory = 0;
		self->table[newStack->entry_I].called = 1;
		self->table[newStack->entry_I].returned = 0;
	}
}

void Stg_CallGraph_Pop( void* callGraph ) {
	Stg_CallGraph*		self = (Stg_CallGraph*)callGraph;
	_Stg_CallGraph_Stack*	oldStack;
	
	Journal_Firewall( 
		self->_stack != 0, 
		Journal_Register( Error_Type, Stg_CallGraph_Type ), 
		"Attempting to pop and empty stack" );
	
	/* Record the accumulative delta time & memory */
	self->table[self->_stack->entry_I].time += 0 /* TODO: Actual time stamp */ - self->_stack->time;
	self->table[self->_stack->entry_I].memory += 0 /* TODO: Actual memory stamp */ - self->_stack->memory;
	self->table[self->_stack->entry_I].returned += 1;
	
	/* Pop the stack */
	oldStack = self->_stack;
	self->_stack = oldStack->pop;
	Memory_Free( oldStack );
}


