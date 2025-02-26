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
#include "Dictionary_Entry_Value.h"
#include "Dictionary_Entry.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Dictionary_Entry* Dictionary_Entry_New( Dictionary_Entry_Key key, Dictionary_Entry_Value* value )
{
	Dictionary_Entry* self;
	
	self = Memory_Alloc( Dictionary_Entry, "Entry" );
	Dictionary_Entry_Init( self, key, value );
	
	return self;
}

Dictionary_Entry* Dictionary_Entry_NewWithSource( Dictionary_Entry_Key key, Dictionary_Entry_Value* value, Dictionary_Entry_Source source )
{
	Dictionary_Entry* self;
	
	self = Memory_Alloc( Dictionary_Entry, "Entry" );
	assert( key );
	assert( value );
	
	self->key = StG_Strdup( key );
	self->value = value;
	if( source != NULL )
		self->source = StG_Strdup( source );
	else
		self->source = NULL;

	return self;
}

void Dictionary_Entry_Init( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Value* value )
{
	assert( key );
	assert( value );

	self->key = StG_Strdup( key );
	self->value = value;
	self->source = NULL;
   self->units = NULL;
}

void Dictionary_Entry_Delete( Dictionary_Entry* self )
{
	Memory_Free( self->key );
	Dictionary_Entry_Value_Delete( self->value );
	if( self->source != NULL )
		Memory_Free( self->source );
	Memory_Free( self );
}

void Dictionary_Entry_Print( Dictionary_Entry* self, Stream* stream )
{
	Journal_Printf( stream, "%s: ", self->key );
	Dictionary_Entry_Value_Print( self->value, stream );
	if( self->source != NULL )
		Journal_Printf( stream, " (original source file: %s)", self->source );
}

Bool Dictionary_Entry_Compare( Dictionary_Entry* self, Dictionary_Entry_Key key )
{
	return (Bool)!strcmp( self->key, key );
}

Bool Dictionary_Entry_CompareWithSource( Dictionary_Entry* self, Dictionary_Entry_Key key, Dictionary_Entry_Source source )
{
	if( self->source == NULL )
	{
		if( source == NULL )
			return (Bool)!strcmp( self->key, key );
		else
			return False;
	}
	else
	{
		if( source == NULL )
			return False;
		else
			return (Bool)( !strcmp( self->key, key ) && !strcmp( self->source, source ) );
	}
}

void Dictionary_Entry_Set( Dictionary_Entry* self, Dictionary_Entry_Value* value )
{
	assert( value );
	Dictionary_Entry_Value_Delete( self->value );
	self->value = value;
}

void Dictionary_Entry_SetWithSource( Dictionary_Entry* self, Dictionary_Entry_Value* value, Dictionary_Entry_Source source )
{
	assert( value );
	Dictionary_Entry_Value_Delete( self->value );
	self->value = value;

	if( self->source != NULL )
		Memory_Free( self->source );
	if( source != NULL )
		self->source = StG_Strdup( source );
}

void Dictionary_Entry_SetSource( Dictionary_Entry* self, Dictionary_Entry_Source source )
{
	if( self->source != NULL )
		Memory_Free( self->source );
	if( source != NULL )
		self->source = StG_Strdup( source );
}

Dictionary_Entry_Value* Dictionary_Entry_Get( Dictionary_Entry* self )
{
	return self->value;
}

Dictionary_Entry_Source Dictionary_Entry_GetSource( Dictionary_Entry* self )
{
	return self->source;
}


