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
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "IndentFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <stdarg.h>   /* subsequent files need this */


const Type IndentFormatter_Type = "IndentFormatter";
const char IndentFormatter_IndentChar = '\t';

/** Makes the string which forms the identation for printing. */
void IndentFormatter_MakeTag( IndentFormatter* formatter );


StreamFormatter* IndentFormatter_New()
{
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(IndentFormatter);
	Type                                    type = IndentFormatter_Type;
	Stg_Class_DeleteFunction*            _delete = _IndentFormatter_Delete;
	Stg_Class_PrintFunction*              _print = _IndentFormatter_Print;
	Stg_Class_CopyFunction*                _copy = _IndentFormatter_Copy;
	StreamFormatter_FormatFunction*      _format = _LineFormatter_Format;
	char                              _character = IndentFormatter_IndentChar;

	return (StreamFormatter*)_IndentFormatter_New(  INDENTFORMATTER_PASSARGS  );
}

StreamFormatter* IndentFormatter_New2( char _character )
{
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(IndentFormatter);
	Type                                    type = IndentFormatter_Type;
	Stg_Class_DeleteFunction*            _delete = _IndentFormatter_Delete;
	Stg_Class_PrintFunction*              _print = _IndentFormatter_Print;
	Stg_Class_CopyFunction*                _copy = _IndentFormatter_Copy;
	StreamFormatter_FormatFunction*      _format = _LineFormatter_Format;

	return (StreamFormatter*)_IndentFormatter_New(  INDENTFORMATTER_PASSARGS  );
}

void IndentFormatter_Init( IndentFormatter* self )
{
	self->_sizeOfSelf = sizeof(IndentFormatter);
	self->type = IndentFormatter_Type;
	self->_delete = _IndentFormatter_Delete;
	self->_print = _IndentFormatter_Print;
	self->_copy = _IndentFormatter_Copy;
	
	_IndentFormatter_Init( self, _LineFormatter_Format, IndentFormatter_IndentChar );
}

IndentFormatter* _IndentFormatter_New(  INDENTFORMATTER_DEFARGS  )
{
	IndentFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(IndentFormatter) );
	self = (IndentFormatter*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
                                                                                
	_IndentFormatter_Init( self, _format, _character );

	return self;
}

void _IndentFormatter_Init(
	IndentFormatter*		self,
	StreamFormatter_FormatFunction*	_format,
	char				_character )
{
	_LineFormatter_Init( (LineFormatter*)self, _format );
	
	self->_indent = 0;
	self->_character = _character;
}

void _IndentFormatter_Delete( void* formatter )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	/* Stg_Class_Delete parent */
	_LineFormatter_Delete( self );
}
void _IndentFormatter_Print( void* formatter, Stream* stream )
{
	/*IndentFormatter* self = (IndentFormatter*) formatter;*/

	#if DEBUG
		assert( formatter );
		assert( stream );
	#endif
	
	/* General info */
	printf( "IndentFormatter (ptr): %p\n", formatter );
	
	_LineFormatter_Print( formatter, stream );
}

void* _IndentFormatter_Copy( void* indentFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	IndentFormatter*	self = (IndentFormatter*)indentFormatter;
	IndentFormatter*	newIndentFormatter;
	
	newIndentFormatter = (IndentFormatter*)_LineFormatter_Copy( self, dest, deep, nameExt, ptrMap );
	
	IndentFormatter_SetIndent( newIndentFormatter, self->_indent );
	
	return newIndentFormatter;
}

void IndentFormatter_SetIndent( void* formatter, Index indent )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent = indent;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_Increment( void* formatter )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent++;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_Decrement( void* formatter )
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent = ( self->_indent <= 0 ) ? 0 : self->_indent - 1;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_Zero( void* formatter)
{
	IndentFormatter* self = (IndentFormatter*)formatter;
	
	self->_indent = 0;
	IndentFormatter_MakeTag( self );
}

void IndentFormatter_MakeTag( IndentFormatter* formatter )
{
	int i;
	
	/* Release previous tag if existing. */
	if ( formatter->_tag != NULL )
	{
		Memory_Free( formatter->_tag );
	}
	
	/* Stop if no indentation. */
	if ( formatter->_indent <= 0 )
	{
		formatter->_tag = NULL;
		return;
	}
	
	/* Produce tag. */
	formatter->_tag = Memory_Alloc_Array( char, formatter->_indent + 1, "INDENTFORMATTERDUDE" );
	for ( i = 0; i < formatter->_indent; ++i )
	{
		formatter->_tag[i] = formatter->_character;
	}
	formatter->_tag[i] = '\0';
}





