/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "StGermain/Base/Foundation/src/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Journal.h"
#include "StreamFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type StreamFormatter_Type = "StreamFormatter";

StreamFormatter* _StreamFormatter_New(  STREAMFORMATTER_DEFARGS  )
{
	StreamFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(StreamFormatter) );
	self = (StreamFormatter*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
                                                                                
	_StreamFormatter_Init( self, _format );

	return self;
}
void _StreamFormatter_Init(
	StreamFormatter*		self,
	StreamFormatter_FormatFunction*	_format )
{
	self->_format = _format;
	
	self->_stream = NULL;
}

void _StreamFormatter_Delete( void* formatter )
{
	StreamFormatter* self = (StreamFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	/* Stg_Class_Delete parent */
	_Stg_Class_Delete( self );
}
void _StreamFormatter_Print( void* formatter, Stream* stream )
{
	StreamFormatter* self = (StreamFormatter*) formatter;

	#if DEBUG
		assert( self );
		assert( stream );
	#endif
	
	/* General info */
	printf( "StreamFormatter (ptr): %p\n", formatter );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* Virtual info */
	printf( "\t_format (func ptr): %p\n", (void*)self->_format );
	printf( "\t_copy (func ptr): %p\n", (void*)self->_copy );
	printf( "\t_stream (ptr): %p\n", (void*)self->_stream );
}

void* _StreamFormatter_Copy( void* streamFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	StreamFormatter*	self = (StreamFormatter*)streamFormatter;
	StreamFormatter*	newStreamFormatter;
	
	newStreamFormatter = (StreamFormatter*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
	
	newStreamFormatter->_format = self->_format;
	newStreamFormatter->_stream = NULL;
	
	return newStreamFormatter;
}


char* StreamFormatter_Format( void* formatter, const char* const fmt )
{
	StreamFormatter* self = (StreamFormatter*)formatter;
	return self->_format( self, fmt );
}


StreamFormatter_Buffer* stgStreamFormatter_Buffer;

StreamFormatter_Buffer* StreamFormatter_Buffer_New() {
	StreamFormatter_Buffer* result = Memory_Alloc( StreamFormatter_Buffer, "StreamFormatter_Buffer" );
	result->buffer1 = NULL;
	result->buffer2 = NULL;
	result->length1 = 0;
	result->length2 = 0;
	result->current = &result->buffer1;
	result->lengthPtr = &result->length1;

	return result;
}
void StreamFormatter_Buffer_Delete( StreamFormatter_Buffer* buffer ) {
	if ( buffer->buffer1 != NULL ) {
		Memory_Free( buffer->buffer1 );
	}
	if ( buffer->buffer2 != NULL ) {
		Memory_Free( buffer->buffer2 );
	}

	Memory_Free( buffer );
}
char* StreamFormatter_Buffer_AllocNext( Index size ) {

	/* Buffer swap */
	if ( stgStreamFormatter_Buffer->current == &stgStreamFormatter_Buffer->buffer1 ) {
		stgStreamFormatter_Buffer->current = &stgStreamFormatter_Buffer->buffer2;
		stgStreamFormatter_Buffer->lengthPtr = &stgStreamFormatter_Buffer->length2;
	}
	else {
		stgStreamFormatter_Buffer->current = &stgStreamFormatter_Buffer->buffer1;
		stgStreamFormatter_Buffer->lengthPtr = &stgStreamFormatter_Buffer->length1;
	}

	/* Realloc/alloc as needed: make sure stats get entered */
	if ( size > *stgStreamFormatter_Buffer->lengthPtr ) {
		*stgStreamFormatter_Buffer->lengthPtr = size;
      if ( *stgStreamFormatter_Buffer->current == NULL ) {
         *stgStreamFormatter_Buffer->current = Memory_Alloc_Array( char, size, "StreamFormatter-Buffer-Current" );
      }
      else {
		   *stgStreamFormatter_Buffer->current = (char*)Memory_Realloc_Array( *stgStreamFormatter_Buffer->current, char, size );
      }
	}

	return *stgStreamFormatter_Buffer->current;
}


