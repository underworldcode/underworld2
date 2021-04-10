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
#include "Journal.h"
#include <stdarg.h>  
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "RankFormatter.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <mpi.h>


const Type RankFormatter_Type = "RankFormatter";
const char* PREPEND = ": ";


StreamFormatter* RankFormatter_New()
{
	/* Variables set in this function */
	SizeT                            _sizeOfSelf = sizeof(RankFormatter);
	Type                                    type = RankFormatter_Type;
	Stg_Class_DeleteFunction*            _delete = _RankFormatter_Delete;
	Stg_Class_PrintFunction*              _print = _RankFormatter_Print;
	Stg_Class_CopyFunction*                _copy = _LineFormatter_Copy;
	StreamFormatter_FormatFunction*      _format = _LineFormatter_Format;

	return (StreamFormatter*)_RankFormatter_New(  RANKFORMATTER_PASSARGS  );
}
	
void RankFormatter_Init( RankFormatter* self )
{
	self->_sizeOfSelf = sizeof(RankFormatter);
	self->type = RankFormatter_Type;
	self->_delete = _RankFormatter_Delete;
	self->_print = _RankFormatter_Print;
	self->_copy = _LineFormatter_Copy;

	_LineFormatter_Init( (LineFormatter*)self, _LineFormatter_Format );
	_RankFormatter_Init( self, _LineFormatter_Format );
}

RankFormatter* _RankFormatter_New(  RANKFORMATTER_DEFARGS  )
{
	RankFormatter* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(RankFormatter) );
	self = (RankFormatter*)_LineFormatter_New(  LINEFORMATTER_PASSARGS  );
	
	_RankFormatter_Init( self, _format );

	return self;
}

void _RankFormatter_Init(
	RankFormatter*			self,
	StreamFormatter_FormatFunction*	_format )
{
	int rank, rankTemp;
	int rankDigits;
	int size;
	int prependLength;
	
	/* If there is only one processor, do not print rank */
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	if ( size == 1 )
	{
		return;
	}

	/* Calculate length of rank string */
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	rankTemp = rank;
	rankDigits = 0;
	do
	{
		rankDigits++;
		rankTemp /= 10; /* Base 10 */
	} while ( rankTemp > 0 );
	
	prependLength = rankDigits + strlen( PREPEND );

	self->_tag = Memory_Alloc_Array( char, prependLength + 1, "RankFormatter->_tag" );

	sprintf( self->_tag, "%d%s", rank, PREPEND );
}

void _RankFormatter_Delete( void* formatter )
{
	RankFormatter* self = (RankFormatter*)formatter;
	
	#if DEBUG
		assert( self );
	#endif

	/* Stg_Class_Delete parent */
	_LineFormatter_Delete( self );
}
void _RankFormatter_Print( void* formatter, Stream* stream )
{
	#if DEBUG
		RankFormatter* self = (RankFormatter*) formatter;

		assert( self );
		assert (stream);
	#endif
	
	/* General info */
	printf( "RankFormatter (ptr): %p\n", formatter );
	
	_LineFormatter_Print( formatter, stream );
}




