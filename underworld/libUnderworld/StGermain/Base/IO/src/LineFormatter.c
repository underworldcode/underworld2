/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <mpi.h>
#include <stdarg.h>  


const Type LineFormatter_Type = "LineFormatter";

LineFormatter* _LineFormatter_New(  LINEFORMATTER_DEFARGS  )
{
	LineFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LineFormatter) );
	self = (LineFormatter*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
                                                                                
	_LineFormatter_Init( self, _format );

	return self;
}

void _LineFormatter_Init(
	LineFormatter*			self,
	StreamFormatter_FormatFunction*	_format )
{
	_StreamFormatter_Init( (StreamFormatter*)self, _format );
	self->_newLine = True;
	self->_tag = NULL;
}

void _LineFormatter_Delete( void* formatter )
{
	LineFormatter* self = (LineFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	if ( self->_tag != NULL )
	{
		Memory_Free( self->_tag );
	}

	/* Stg_Class_Delete parent */
	_StreamFormatter_Delete( self );
}
void _LineFormatter_Print( void* formatter, Stream* stream )
{
	LineFormatter* self = (LineFormatter*) formatter;

	#if DEBUG
		assert( self );
		assert (stream);
	#endif
	
	/* General info */
	printf( "LineFormatter (ptr): %p\n", formatter );
	
	/* Print parent */
	_Stg_Class_Print( self, stream );
	
	/* print _newLine and _tag */
}

void* _LineFormatter_Copy( void* lineFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	LineFormatter*	self = (LineFormatter*)lineFormatter;
	LineFormatter*	newLineFormatter;
	
	/* Create new instance. Copy virtual info */
	newLineFormatter = (LineFormatter*)_StreamFormatter_Copy( self, dest, deep, nameExt, ptrMap );
	
	/* Copy member info. */
	newLineFormatter->_newLine = self->_newLine;
	if ( self->_tag != NULL ) {
		newLineFormatter->_tag = StG_Strdup( self->_tag );
	}
	else {
		newLineFormatter->_tag = NULL;
	}
	
	return newLineFormatter;
}

char* _LineFormatter_Format( void* formatter, const char* const fmt )
{
	LineFormatter* self = (LineFormatter*)formatter;
	char* result;
	char* currentLine;
	int numInsert;
	int fmtLength;
	int prependLength;
	int newLength;

			
	char* destPtr;
	char* srcPtr;
		
	#if DEBUG
		assert( self );
	#endif

	if ( self->_tag == NULL )
	{
		/* No tag to add, so return same format */
		return (char*)fmt;
	}

	/* count number of inserts */
	numInsert = 0;
	currentLine = strchr( fmt, '\n' );
	while ( currentLine != NULL )
	{
		numInsert++;	
		currentLine = strchr( currentLine + 1, '\n' );
	}
	if ( self->_newLine )
	{
		numInsert++;
	}
	
	fmtLength = strlen( fmt );
	if ( fmtLength > 0 && fmt[fmtLength - 1] == '\n' )
	{
		numInsert--;
	}

	prependLength = strlen( self->_tag );
	
	/* Calculate length of new string */
	if ( numInsert > 0 )
	{
		newLength = fmtLength + ( prependLength * numInsert );
	}
	else
	{
		newLength = fmtLength;
	}
	
/*	result = Memory_Alloc_Array( char, newLength + 1, Memory_IgnoreName );*/
	result = StreamFormatter_Buffer_AllocNext( newLength + 1 );
	
	destPtr = result;
	srcPtr = (char*)fmt;
	
	/* If we were upto a new line, begin with a prepend. */
	if ( self->_newLine )
	{
		sprintf( destPtr, "%s", self->_tag );
		destPtr += prependLength;
	}
	
	self->_newLine = False;
	
	while ( *srcPtr != '\0' )
	{
		*destPtr = *srcPtr;
		
		destPtr++;
		
		if ( *srcPtr == '\n' )
		{
			/* if not the last endline */
			if ( (srcPtr - fmt) != (fmtLength - 1) )
			{
				sprintf( destPtr, "%s", self->_tag );
				destPtr += prependLength;
			}
			else
			{
				/* If we are at the last line, and it was a new line, prepend the next time this is called. */
				self->_newLine = True;
			}
		}
		srcPtr++;
	}
	*destPtr = '\0';
	
	return result;	
}



