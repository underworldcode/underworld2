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
#include "File.h"
#include "Stream.h" 
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "PathUtils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>


/* Private functions */
static void Dictionary_Entry_Value_SetValueUnsignedInt( Dictionary_Entry_Value* self, unsigned int value );
static void Dictionary_Entry_Value_SetValueInt( Dictionary_Entry_Value* self, unsigned int value );
static void Dictionary_Entry_Value_SetValueDouble( Dictionary_Entry_Value* self, double value );
static void Dictionary_Entry_Value_SetValueUnsignedLong( Dictionary_Entry_Value* self, unsigned long value );
static void Dictionary_Entry_Value_SetValueString( Dictionary_Entry_Value* self, const char* const value );
static void Dictionary_Entry_Value_SetValueBool( Dictionary_Entry_Value* self, Bool value );
static void Dictionary_Entry_Value_SetValueNewStruct( Dictionary_Entry_Value* self);
static void Dictionary_Entry_Value_SetValueStruct( Dictionary_Entry_Value* self, void* value );
static void Dictionary_Entry_Value_SetValueNewList( Dictionary_Entry_Value* self );
static void Dictionary_Entry_Value_DeleteContents( Dictionary_Entry_Value* self);


Dictionary_Entry_Value* Dictionary_Entry_Value_FromUnsignedInt( unsigned int value ) {
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Signed Int" );
	
	Dictionary_Entry_Value_InitFromUnsignedInt( self, value );
	
	return self;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_FromInt( int value ) {
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Int" );
	
	Dictionary_Entry_Value_InitFromInt( self, value );
	
	return self;
}

void Dictionary_Entry_Value_InitFromUnsignedInt( Dictionary_Entry_Value* self, unsigned int value ) {
	Dictionary_Entry_Value_SetValueUnsignedInt( self, value );
	self->next = 0;
}

void Dictionary_Entry_Value_InitFromInt( Dictionary_Entry_Value* self, int value ) {
	Dictionary_Entry_Value_SetValueInt( self, value );
	self->next = 0;
}

static void Dictionary_Entry_Value_SetValueUnsignedInt( Dictionary_Entry_Value* self, unsigned int value ) {
	self->as.typeUnsignedInt = value;
	self->type = Dictionary_Entry_Value_Type_UnsignedInt;
}

static void Dictionary_Entry_Value_SetValueInt( Dictionary_Entry_Value* self, unsigned int value ) {
	self->as.typeInt = value;
	self->type = Dictionary_Entry_Value_Type_Int;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_FromDouble( double value ) {
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Double" );
	
	Dictionary_Entry_Value_InitFromDouble( self, value );
	
	return self;
}

void Dictionary_Entry_Value_InitFromDouble( Dictionary_Entry_Value* self, double value ) {
	Dictionary_Entry_Value_SetValueDouble( self, value );
	self->next = 0;
}


static void Dictionary_Entry_Value_SetValueDouble( Dictionary_Entry_Value* self, double value ) {
	self->as.typeDouble = value;
	self->type = Dictionary_Entry_Value_Type_Double;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_FromUnsignedLong( unsigned long value ) {
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Unsigned Long" );
	
	Dictionary_Entry_Value_InitFromUnsignedLong( self, value );
	
	return self;
}

void Dictionary_Entry_Value_InitFromUnsignedLong( Dictionary_Entry_Value* self, unsigned long value ) {
	Dictionary_Entry_Value_SetValueUnsignedLong( self, value );
	self->next = 0;
}

static void Dictionary_Entry_Value_SetValueUnsignedLong( Dictionary_Entry_Value* self, unsigned long value ) {
	self->as.typeUnsignedLong = value;
	self->type = Dictionary_Entry_Value_Type_UnsignedLong;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_FromString( const char* const value ) {
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value String" );
	
	Dictionary_Entry_Value_InitFromString( self, value );
	
	return self;
}
	
void Dictionary_Entry_Value_InitFromString( Dictionary_Entry_Value* self, const char* const value ) {
	Dictionary_Entry_Value_SetValueString( self, value );
	self->next = 0;
}

static void Dictionary_Entry_Value_SetValueString( Dictionary_Entry_Value* self, const char* const value ) {
	self->as.typeString = StG_Strdup( value );
	self->type = Dictionary_Entry_Value_Type_String;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_FromStringTo( char* string, char type ) {
	Dictionary_Entry_Value* retValue = Memory_Alloc( Dictionary_Entry_Value, "Return Value" );
	
	/* need to create the value temporarily so it can be converted if necessary */
	retValue->type = Dictionary_Entry_Value_Type_String;

	if ( string ) {
		retValue->as.typeString = ExpandEnvironmentVariables( string );
	}
	else {
		retValue->as.typeString = string;
	}
	
	switch (type) {
		case Dictionary_Entry_Value_Type_String:
			Dictionary_Entry_Value_InitFromString( retValue, retValue->as.typeString );
			break;
		case Dictionary_Entry_Value_Type_Double:
			Dictionary_Entry_Value_InitFromDouble( retValue, Dictionary_Entry_Value_AsDouble( retValue ) );
			break;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			Dictionary_Entry_Value_InitFromUnsignedInt( retValue, Dictionary_Entry_Value_AsUnsignedInt( retValue ) );
			break;
		case Dictionary_Entry_Value_Type_Int:
			Dictionary_Entry_Value_InitFromInt( retValue, Dictionary_Entry_Value_AsInt( retValue ) );
			break;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			Dictionary_Entry_Value_InitFromUnsignedLong( retValue, Dictionary_Entry_Value_AsUnsignedLong( retValue ) );
			break;
		case Dictionary_Entry_Value_Type_Bool:
			Dictionary_Entry_Value_InitFromBool( retValue, Dictionary_Entry_Value_AsBool( retValue ) );
			break;
		case Dictionary_Entry_Value_Type_Struct:
			Dictionary_Entry_Value_InitNewStruct( retValue );
			break;
		case Dictionary_Entry_Value_Type_List:
			Dictionary_Entry_Value_InitNewList( retValue );
			break;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: type '%d' is invalid.\n", __func__, type );
		}
	}		
	
	return retValue;
}


Dictionary_Entry_Value* Dictionary_Entry_Value_FromBool( Bool value ) {
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Bool" );
	
	Dictionary_Entry_Value_InitFromBool( self, value );
	
	return self;
}

void Dictionary_Entry_Value_InitFromBool( Dictionary_Entry_Value* self, Bool value ) {
	Dictionary_Entry_Value_SetValueBool( self, value );
	self->next = 0;
}

static void Dictionary_Entry_Value_SetValueBool( Dictionary_Entry_Value* self, Bool value ) {
	self->as.typeBool = value;
	if( self->as.typeBool != False && self->as.typeBool != True ) {
		self->as.typeBool = True;
	}
	self->type = Dictionary_Entry_Value_Type_Bool;
}


Dictionary_Entry_Value* Dictionary_Entry_Value_NewStruct( void )
{
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Struct" );
	
	Dictionary_Entry_Value_InitNewStruct( self );
	return self;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_FromStruct( void* dictionary )
{
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value Struct" );
	
	Dictionary_Entry_Value_InitFromStruct( self, dictionary );
	return self;
}

void Dictionary_Entry_Value_InitNewStruct( Dictionary_Entry_Value* self )
{
	Dictionary_Entry_Value_SetValueNewStruct( self );
	self->next = 0;
}

void Dictionary_Entry_Value_InitFromStruct( Dictionary_Entry_Value* self, void* dictionary )
{
	Dictionary_Entry_Value_SetValueStruct( self, dictionary );
	self->next = 0;
}

static void Dictionary_Entry_Value_SetValueNewStruct( Dictionary_Entry_Value* self ) {
	Dictionary_Entry_Value_SetValueStruct( self, Dictionary_New() );
}

static void Dictionary_Entry_Value_SetValueStruct( Dictionary_Entry_Value* self, void* value )
{
	self->as.typeStruct = (Dictionary*) value;
	self->type = Dictionary_Entry_Value_Type_Struct;
}

Dictionary_Entry_Value* Dictionary_Entry_Value_NewList( void )
{
	Dictionary_Entry_Value* self = Memory_Alloc( Dictionary_Entry_Value, "Entry Value List" );
	
	Dictionary_Entry_Value_InitNewList( self );
	return self;
}

void Dictionary_Entry_Value_InitNewList( Dictionary_Entry_Value* self )
{
	Dictionary_Entry_Value_SetValueNewList( self );
	self->next = 0;
}

static void Dictionary_Entry_Value_SetValueNewList( Dictionary_Entry_Value* self )
{
	self->as.typeList = Memory_Alloc( Dictionary_Entry_Value_List, "Entry Value List" );
	self->as.typeList->first = NULL;
	self->as.typeList->last = NULL;
	self->as.typeList->count = 0;
	self->as.typeList->encoding = Default;
	self->type = Dictionary_Entry_Value_Type_List;
}


void Dictionary_Entry_Value_AddMember( Dictionary_Entry_Value* self, Dictionary_Entry_Key name,	Dictionary_Entry_Value* value )
{
	self->as.typeStruct->add( self->as.typeStruct, name, value);
}

void Dictionary_Entry_Value_AddMemberWithSource( Dictionary_Entry_Value* self, Dictionary_Entry_Key name,
							Dictionary_Entry_Value* value, Dictionary_Entry_Source source )
{  
   /* yo */
   self->as.typeStruct->addWithSource( self->as.typeStruct, name, value, NULL, source);
}

void Dictionary_Entry_Value_AddElement( Dictionary_Entry_Value* self, Dictionary_Entry_Value* element )
{
	/* check type - convert to a list if not so... */
	if (Dictionary_Entry_Value_Type_List != self->type) {
		Dictionary_Entry_Value* copy = NULL;
		copy = Dictionary_Entry_Value_Copy( self, True );
		Dictionary_Entry_Value_SetNewList( self );	
		Dictionary_Entry_Value_AddElement( self, copy );
	}
	
	if (!self->as.typeList->first) {
		self->as.typeList->first = element;
	} else {
		self->as.typeList->last->next = element;
	}
	self->as.typeList->last = element;
	self->as.typeList->count++;
}


void Dictionary_Entry_Value_AddElementWithSource(
		Dictionary_Entry_Value* self,
		Dictionary_Entry_Value* element,
		Dictionary_Entry_Source source )
{
	/* check type - convert to a list if not so... */
	if (Dictionary_Entry_Value_Type_List != self->type) {
		Dictionary_Entry_Value* copy = NULL;
		copy = Dictionary_Entry_Value_Copy( self, True );

		Dictionary_Entry_Value_SetNewList( self );	
		Dictionary_Entry_Value_AddElementWithSource( self, copy, source );
	}
	
	if (!self->as.typeList->first) {
		self->as.typeList->first = element;
	} else {
		self->as.typeList->last->next = element;
	}
	self->as.typeList->last = element;
	self->as.typeList->count++;
}


void Dictionary_Entry_Value_Delete( Dictionary_Entry_Value* self ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Memory_Free( self );
}


static void Dictionary_Entry_Value_DeleteContents( Dictionary_Entry_Value* self ) {
	Dictionary_Entry_Value* cur         = NULL;
	Dictionary_Entry_Value* next        = NULL;
	Stream*                 errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_String:
		   /* CHECK THIS! SZ 2/8/2010 */
			/*Journal_Firewall( self->as.typeString != NULL, errorStream, "In func %s: self->as.typeString is NULL.\n", __func__ );*/
		  if( self->as.typeString )
			 Memory_Free( self->as.typeString );

			break;
		case Dictionary_Entry_Value_Type_Struct:
			Journal_Firewall( self->as.typeStruct != NULL, errorStream, "In func %s: self->as.typeStruct is NULL.\n", __func__ );
			Stg_Class_Delete( self->as.typeStruct );
			break;
		case Dictionary_Entry_Value_Type_List:
			cur = self->as.typeList->first;
			while ( cur ) {
				next = cur->next;
				Dictionary_Entry_Value_Delete( cur );
				cur = next;
			}	
			Memory_Free( self->as.typeList );
			break;
		case Dictionary_Entry_Value_Type_Double:
		case Dictionary_Entry_Value_Type_UnsignedInt:
		case Dictionary_Entry_Value_Type_Int:
		case Dictionary_Entry_Value_Type_UnsignedLong:
		case Dictionary_Entry_Value_Type_Bool:
			break;
		default:
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
	};
}


void Dictionary_Entry_Value_Print( Dictionary_Entry_Value* self, Stream* stream ) {
	Dictionary_Index index;
	
	if( !self ) {
		return;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_String:
			Journal_Printf( stream, "\"%s\"", self->as.typeString );
			return;
		case Dictionary_Entry_Value_Type_Double:
			Journal_Printf( stream, "%g", self->as.typeDouble );
			return;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			Journal_Printf( stream, "%u", self->as.typeUnsignedInt );
			return;
		case Dictionary_Entry_Value_Type_Int:
			Journal_Printf( stream, "%d", self->as.typeInt );
			return;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			Journal_Printf( stream, "%ld", self->as.typeUnsignedLong );
			return;
		case Dictionary_Entry_Value_Type_Bool:
			Journal_Printf( stream, "%s", Dictionary_Entry_Value_AsString( self ) );
			return;
		case Dictionary_Entry_Value_Type_List:
			if (self->as.typeList->first) {
				Dictionary_Entry_Value* cur = self->as.typeList->first;
				Dictionary_Entry_Value_Print( cur, stream );
				cur = cur->next;
				
				while (cur) {
					Journal_Printf( stream, ", " );
					Dictionary_Entry_Value_Print( cur, stream );
					cur = cur->next;
				}
			}
			return;
		case Dictionary_Entry_Value_Type_Struct:
			Stream_Indent( stream );
			for( index = 0; index < self->as.typeStruct->count; index++ ) {
				Journal_Printf( stream, "\n");
				Journal_Printf( stream, "%s: ", self->as.typeStruct->entryPtr[index]->key );
				Dictionary_Entry_Value_Print( self->as.typeStruct->entryPtr[index]->value, stream );
			}
			Stream_UnIndent( stream );
			return;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
}


void Dictionary_Entry_Value_SetFrom( Dictionary_Entry_Value* self, void* value, const char type) {
	Dictionary_Entry_Value_DeleteContents( self );
	
	switch (type) {
		case Dictionary_Entry_Value_Type_String:
			Dictionary_Entry_Value_SetValueString( self, *( (char**) value) );
			break;
		case Dictionary_Entry_Value_Type_Double:
			Dictionary_Entry_Value_SetValueDouble( self, *( (double*) value) );
			break;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			Dictionary_Entry_Value_SetValueUnsignedInt( self, *( (unsigned int*) value) );
			break;
		case Dictionary_Entry_Value_Type_Int:
			Dictionary_Entry_Value_SetValueInt( self, *( (unsigned int*) value) );
			break;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			Dictionary_Entry_Value_SetValueUnsignedLong( self, *( (unsigned long*) value) );
			break;
		case Dictionary_Entry_Value_Type_Bool:
			Dictionary_Entry_Value_SetValueBool( self, *( (Bool*) value) );
			break;
		case Dictionary_Entry_Value_Type_Struct:
			Dictionary_Entry_Value_SetValueStruct( self, value );
			break;
		case Dictionary_Entry_Value_Type_List:
			Dictionary_Entry_Value_SetValueNewList( self );
			break;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
}


void Dictionary_Entry_Value_SetFromUnsignedInt( Dictionary_Entry_Value* self, unsigned int value ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueUnsignedInt( self, value );
}

void Dictionary_Entry_Value_SetFromInt( Dictionary_Entry_Value* self, int value ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueInt( self, value );
}

void Dictionary_Entry_Value_SetFromDouble( Dictionary_Entry_Value* self, double value ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueDouble( self, value );
}

void Dictionary_Entry_Value_SetFromUnsignedLong( Dictionary_Entry_Value* self, unsigned long value ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueUnsignedLong( self, value );
}

void Dictionary_Entry_Value_SetFromString( Dictionary_Entry_Value* self, char* value ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueString( self, value );
}


void Dictionary_Entry_Value_SetFromStringKeepCurrentType( Dictionary_Entry_Value* self, char* string ) {
	Dictionary_Entry_Value_Type currType = self->type;
	Dictionary_Entry_Value_DeleteContents( self );
	self->type = Dictionary_Entry_Value_Type_String;
	self->as.typeString = string;
	
	switch (currType) {
		case Dictionary_Entry_Value_Type_String:
			Dictionary_Entry_Value_SetValueString( self, string );
			break;
		case Dictionary_Entry_Value_Type_Double:
			Dictionary_Entry_Value_SetValueDouble( self, Dictionary_Entry_Value_AsDouble( self ) );
			break;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			Dictionary_Entry_Value_SetValueUnsignedInt( self, Dictionary_Entry_Value_AsUnsignedInt( self ) );
			break;
		case Dictionary_Entry_Value_Type_Int:
			Dictionary_Entry_Value_SetValueInt( self, Dictionary_Entry_Value_AsInt( self ) );
			break;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			Dictionary_Entry_Value_SetValueUnsignedLong( self, Dictionary_Entry_Value_AsUnsignedLong( self ) );
			break;
		case Dictionary_Entry_Value_Type_Bool:
			Dictionary_Entry_Value_SetValueBool( self, Dictionary_Entry_Value_AsBool( self ) );
			break;
		case Dictionary_Entry_Value_Type_Struct:
			Dictionary_Entry_Value_SetValueNewStruct( self );
			break;
		case Dictionary_Entry_Value_Type_List:
			Dictionary_Entry_Value_SetValueNewList( self );
			break;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
}


void Dictionary_Entry_Value_SetFromBool( Dictionary_Entry_Value* self, Bool value ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueBool( self, value );
}


void Dictionary_Entry_Value_SetNewStruct( Dictionary_Entry_Value* self )
{
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueNewStruct( self );
}


void Dictionary_Entry_Value_SetNewList( Dictionary_Entry_Value* self )
{
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueNewList( self );
}

void Dictionary_Entry_Value_SetFromStruct( Dictionary_Entry_Value* self, void* dictionary ) {
	Dictionary_Entry_Value_DeleteContents( self );
	Dictionary_Entry_Value_SetValueStruct( self, dictionary );
}

unsigned int Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Entry_Value* self ) {
	if( !self ) {
		return 0;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			/* Do nothing (later will print a warning) */
			return 0;
		case Dictionary_Entry_Value_Type_List:
			/* returns the first element as an unsigned int */
			if (self->as.typeList->first) {
				return Dictionary_Entry_Value_AsUnsignedInt( self->as.typeList->first );
			} else {	
				return 0;
			}	
		case Dictionary_Entry_Value_Type_String:
			return strtoul( self->as.typeString, 0, 0 );
		case Dictionary_Entry_Value_Type_Double:
			return (unsigned int)self->as.typeDouble;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			return self->as.typeUnsignedInt;
		case Dictionary_Entry_Value_Type_Int:
			return self->as.typeInt;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return self->as.typeUnsignedLong;
		case Dictionary_Entry_Value_Type_Bool:
			return (unsigned int)self->as.typeBool;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return 0;
}

int Dictionary_Entry_Value_AsInt( Dictionary_Entry_Value* self ) {
	if( !self ) {
		return 0;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			/* Do nothing (later will print a warning) */
			return 0;
		case Dictionary_Entry_Value_Type_List:
			/* returns the first element as an unsigned int */
			if (self->as.typeList->first) {
				return Dictionary_Entry_Value_AsInt( self->as.typeList->first );
			} else {	
				return 0;
			}	
		case Dictionary_Entry_Value_Type_String:
			return strtoul( self->as.typeString, 0, 0 );
		case Dictionary_Entry_Value_Type_Double:
			return (unsigned int)self->as.typeDouble;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			return self->as.typeUnsignedInt;
		case Dictionary_Entry_Value_Type_Int:
			return self->as.typeInt;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return self->as.typeUnsignedLong;
		case Dictionary_Entry_Value_Type_Bool:
			return (unsigned int)self->as.typeBool;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return 0;
}

double Dictionary_Entry_Value_AsDouble( Dictionary_Entry_Value* self ) {
	if( !self ) {
		return 0.0f;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			/* Do nothing (later will print a warning) */
			return 0;
		case Dictionary_Entry_Value_Type_List:
			/* returns the first element as an unsigned int */
			if (self->as.typeList->first) {
				return Dictionary_Entry_Value_AsDouble( self->as.typeList->first );
			} else {	
				return 0;
			}	
		case Dictionary_Entry_Value_Type_String:
			return strtod( self->as.typeString, 0 );
		case Dictionary_Entry_Value_Type_Double:
			return self->as.typeDouble;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			return (double)self->as.typeUnsignedInt;
		case Dictionary_Entry_Value_Type_Int:
			return self->as.typeInt;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return self->as.typeUnsignedLong;
		case Dictionary_Entry_Value_Type_Bool:
			return (double)self->as.typeBool;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	
	return 0.0f;
}

unsigned long Dictionary_Entry_Value_AsUnsignedLong( Dictionary_Entry_Value* self ) {
	if( !self ) {
		return 0.0;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			/* Do nothing (later will print a warning) */
			return 0;
		case Dictionary_Entry_Value_Type_List:
			/* returns the first element as an unsigned long */
			if (self->as.typeList->first) {
				return Dictionary_Entry_Value_AsUnsignedLong( self->as.typeList->first );
			} else {	
				return 0;
			}	
		case Dictionary_Entry_Value_Type_String:
			return strtod( self->as.typeString, 0 );
		case Dictionary_Entry_Value_Type_Double:
			return self->as.typeDouble;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			return (double)self->as.typeUnsignedInt;
		case Dictionary_Entry_Value_Type_Int:
			return self->as.typeInt;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return self->as.typeUnsignedLong;
		case Dictionary_Entry_Value_Type_Bool:
			return (double)self->as.typeBool;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	
	return 0.0;
}

char* Dictionary_Entry_Value_AsString( Dictionary_Entry_Value* self ) {
	static char buf[256];
	
	if( !self ) {
		strcpy( buf, "" );
		return buf;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			strcpy( buf, "" );
			return buf;
		case Dictionary_Entry_Value_Type_List:
			/* returns the first element as a string */
			if (self->as.typeList->first) {
				return Dictionary_Entry_Value_AsString( self->as.typeList->first );
			} else {	
				return 0;
			}	
		case Dictionary_Entry_Value_Type_String:
			return self->as.typeString;
		case Dictionary_Entry_Value_Type_Double:
			sprintf( buf, "%g", self->as.typeDouble );
			return buf;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			sprintf( buf, "%u", self->as.typeUnsignedInt );
			return buf;
		case Dictionary_Entry_Value_Type_Int:
			sprintf( buf, "%d", self->as.typeInt );
			return buf;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			sprintf( buf, "%ld", self->as.typeUnsignedLong );
			return buf;
		case Dictionary_Entry_Value_Type_Bool:
			if (True == self->as.typeBool) {
				 sprintf( buf, "true" );
			}	 
			else if (False == self->as.typeBool) {
				 sprintf( buf, "false" );
			}	 
			return buf;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return buf;
}


Bool Dictionary_Entry_Value_AsBool( Dictionary_Entry_Value* self ) {
	if( !self ) {
		return (Bool)0;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			/* Do nothing (later will print a warning) */
			return (Bool)0;
		case Dictionary_Entry_Value_Type_List:
			/* returns the first element as an unsigned int */
			if (self->as.typeList->first) {
				return Dictionary_Entry_Value_AsBool( self->as.typeList->first );
			} else {	
				return (Bool)0;
			}	
		case Dictionary_Entry_Value_Type_String:
			if( !strcmp( "1", self->as.typeString ) ) {
				return True;
			}
			else if( !strcmp( "0", self->as.typeString ) ) {
				return False;
			}
			else if( !strcasecmp( "true", self->as.typeString ) ) {
				return True;
			}
			else if( !strcasecmp( "false", self->as.typeString ) ) {
				return False;
			}
			else if( !strcasecmp( "t", self->as.typeString ) ) {
				return True;
			}
			else if( !strcasecmp( "f", self->as.typeString ) ) {
				return False;
			}
			else if( !strcasecmp( "yes", self->as.typeString ) ) {
				return True;
			}
			else if( !strcasecmp( "no", self->as.typeString ) ) {
				return False;
			}
			else if( !strcasecmp( "y", self->as.typeString ) ) {
				return True;
			}
			else if( !strcasecmp( "n", self->as.typeString ) ) {
				return False;
			}
			else if( !strcasecmp( "on", self->as.typeString ) ) {
				return True;
			}
			else if( !strcasecmp( "off", self->as.typeString ) ) {
				return False;
			}
			else {
				Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
				Journal_Firewall( False, errorStream, "In func %s: Cannot convert string '%s' to Bool.\n", __func__, self->as.typeString );
			}
		case Dictionary_Entry_Value_Type_Double:
			if( (Bool)self->as.typeDouble ) {
				return True;
			}
			else {
				return False;
			}
		case Dictionary_Entry_Value_Type_UnsignedInt:
			if( (Bool)self->as.typeUnsignedInt ) {
				return True;
			}
			else {
				return False;
			}
		case Dictionary_Entry_Value_Type_Int:
			if( (Bool)self->as.typeInt ) {
				return True;
			}
			else {
				return False;
			}
		case Dictionary_Entry_Value_Type_UnsignedLong:
			if( (Bool)self->as.typeUnsignedLong ) {
				return True;
			}
			else {
				return False;
			}
		case Dictionary_Entry_Value_Type_Bool:
			return self->as.typeBool;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return False;
}


Bool Dictionary_Entry_Value_AsBool_with_Default( Dictionary_Entry_Value* self, Bool Default ) {
  if(self==NULL)
    return Default;
  else
    return Dictionary_Entry_Value_AsBool(self);
}


Dictionary* Dictionary_Entry_Value_AsDictionary( Dictionary_Entry_Value* self ) {
	if( !self ) {
		return 0;
	}
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			return self->as.typeStruct;
		case Dictionary_Entry_Value_Type_List:
			return 0;
		case Dictionary_Entry_Value_Type_String:
			return 0;
		case Dictionary_Entry_Value_Type_Double:
			return 0;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			return 0;
		case Dictionary_Entry_Value_Type_Int:
			return 0;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return 0;
		case Dictionary_Entry_Value_Type_Bool:
			return 0;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return 0;
}


Dictionary_Entry_Value* Dictionary_Entry_Value_GetFirstElement( Dictionary_Entry_Value* self )
{
	return Dictionary_Entry_Value_GetElement( self, 0 );
}


Dictionary_Entry_Value* Dictionary_Entry_Value_GetElement( Dictionary_Entry_Value* self, Index index )
{
	Index currIndex = 0;
	Dictionary_Entry_Value* currElement = NULL;
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_List:
			if ( index > self->as.typeList->count ) {
				return NULL;
			}
			
			currElement = self->as.typeList->first;
			while ( currIndex++ < index ) {
				currElement = currElement->next;
			}	
			
			return currElement;
		case Dictionary_Entry_Value_Type_Struct:
		case Dictionary_Entry_Value_Type_String:
		case Dictionary_Entry_Value_Type_Bool:
		case Dictionary_Entry_Value_Type_Double:
		case Dictionary_Entry_Value_Type_UnsignedInt:
		case Dictionary_Entry_Value_Type_Int:
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return self;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return self;
}


Index Dictionary_Entry_Value_GetCount( Dictionary_Entry_Value* self )
{
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_List:
			return self->as.typeList->count;
		case Dictionary_Entry_Value_Type_Struct:
			return self->as.typeStruct->count;
		case Dictionary_Entry_Value_Type_String:
		case Dictionary_Entry_Value_Type_Bool:
		case Dictionary_Entry_Value_Type_Double:
		case Dictionary_Entry_Value_Type_UnsignedInt:
		case Dictionary_Entry_Value_Type_Int:
			return 1;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return 1;
}


void Dictionary_Entry_Value_SetEncoding( Dictionary_Entry_Value* self, const Encoding encoding ) {
	if ( Dictionary_Entry_Value_Type_List == self->type ) {
		self->as.typeList->encoding = encoding;
	}	
}


Encoding Dictionary_Entry_Value_GetEncoding( Dictionary_Entry_Value* self ) {
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_List:
			return self->as.typeList->encoding;
		case Dictionary_Entry_Value_Type_Struct:
		case Dictionary_Entry_Value_Type_String:
		case Dictionary_Entry_Value_Type_Bool:
		case Dictionary_Entry_Value_Type_Double:
		case Dictionary_Entry_Value_Type_UnsignedInt:
		case Dictionary_Entry_Value_Type_Int:
		case Dictionary_Entry_Value_Type_UnsignedLong:
			return Default;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}
	return Default;
}


Dictionary_Entry_Value* Dictionary_Entry_Value_GetMember( Dictionary_Entry_Value* self, Dictionary_Entry_Key name) {
	Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );

	Journal_Firewall( self->as.typeStruct != NULL, 
			errorStream, "In func %s: self->as.typeStruct is NULL.\n", __func__, self->as.typeStruct );
	if ( Dictionary_Entry_Value_Type_Struct != self->type ) {
		return NULL;
	}
	
	return self->as.typeStruct->get( self->as.typeStruct, name );
}


void Dictionary_Entry_Value_SetMember( Dictionary_Entry_Value* self,
	Dictionary_Entry_Key name, Dictionary_Entry_Value* member ) {
	Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			self->as.typeStruct->set( self->as.typeStruct, name, member );
			return;
		case Dictionary_Entry_Value_Type_List:
		case Dictionary_Entry_Value_Type_String:
		case Dictionary_Entry_Value_Type_Bool:
		case Dictionary_Entry_Value_Type_Double:
		case Dictionary_Entry_Value_Type_UnsignedInt:
		case Dictionary_Entry_Value_Type_Int:
		case Dictionary_Entry_Value_Type_UnsignedLong:
			/* should print a warning once journal set up */
			Journal_Printf( errorStream, "Func %s does not support Dictionary_Entry_Values of type '%d'.\n", __func__, self->type );
			return;
		default:
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
	};
}


void Dictionary_Entry_Value_SetMemberWithSource( Dictionary_Entry_Value* self,
	Dictionary_Entry_Key name, Dictionary_Entry_Value* member, Dictionary_Entry_Source source ) {
	Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
	
	switch( self->type ) {
		case Dictionary_Entry_Value_Type_Struct:
			self->as.typeStruct->setWithSource( self->as.typeStruct, name, member, source );
			return;
		case Dictionary_Entry_Value_Type_List:
		case Dictionary_Entry_Value_Type_String:
		case Dictionary_Entry_Value_Type_Bool:
		case Dictionary_Entry_Value_Type_Double:
		case Dictionary_Entry_Value_Type_UnsignedInt:
		case Dictionary_Entry_Value_Type_Int:
		case Dictionary_Entry_Value_Type_UnsignedLong:
			/* should print a warning once journal set up */
			Journal_Printf( errorStream, "Func %s does not support Dictionary_Entry_Values of type '%d'.\n", __func__, self->type );
			return;
		default:
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
	};
}


Bool Dictionary_Entry_Value_CompareFull( Dictionary_Entry_Value* self, Dictionary_Entry_Value* dev, Bool strictTypeCheck ) {
	Bool         retValue = True; 
	Stream*      errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );

	if ( strictTypeCheck ) {
		if ( self->type != dev->type ) {
			return False;
		}
	}

	switch( self->type ) {
		case Dictionary_Entry_Value_Type_String: {
			/* Comparing as strings is tricky. When both are strings it's fine. If the dev to compare to is stored
			 * natively as a number, we should convert the first to a number also for comparison. Otherwise, you
			 * can get false Negatives when the entries are numerically the same, but use different notation
			 * (e.g. scientific vs decimal) */ 	
			switch( dev->type ) {
				case Dictionary_Entry_Value_Type_String:
					if ( 0 != strcmp( self->as.typeString, dev->as.typeString ) ) 
						retValue = False;
					break;
				case Dictionary_Entry_Value_Type_Double:
					if ( dev->as.typeDouble != Dictionary_Entry_Value_AsDouble( self ) )
						retValue = False;
					break;
				case Dictionary_Entry_Value_Type_UnsignedInt:
					if ( dev->as.typeUnsignedInt != Dictionary_Entry_Value_AsUnsignedInt( self ) )
						retValue = False;
					break;
				case Dictionary_Entry_Value_Type_Int:
					if ( dev->as.typeInt != Dictionary_Entry_Value_AsInt( self ) )
						retValue = False;
					break;
				case Dictionary_Entry_Value_Type_UnsignedLong:
					if ( dev->as.typeUnsignedLong != Dictionary_Entry_Value_AsUnsignedLong( self ) )
						retValue = False;
					break;
				case Dictionary_Entry_Value_Type_Bool:
					if ( dev->as.typeBool != Dictionary_Entry_Value_AsBool( self ) )
				case Dictionary_Entry_Value_Type_Struct:
				case Dictionary_Entry_Value_Type_List:
					retValue = False;
					break;
				default:
					Journal_Firewall( False, errorStream, "In func %s: dev->type '%d' is invalid.\n", __func__, dev->type );
			}
			break;
		}
		case Dictionary_Entry_Value_Type_Double:
			if ( self->as.typeDouble != Dictionary_Entry_Value_AsDouble( dev ) )
				retValue = False;
			break;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			if ( self->as.typeUnsignedInt != Dictionary_Entry_Value_AsUnsignedInt( dev ) )
				retValue = False;
			break;
		case Dictionary_Entry_Value_Type_Int:
			if ( self->as.typeInt != Dictionary_Entry_Value_AsInt( dev ) )
				retValue = False;
			break;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			if ( self->as.typeUnsignedLong != Dictionary_Entry_Value_AsUnsignedLong( dev ) )
				retValue = False;
			break;
		case Dictionary_Entry_Value_Type_Bool:
			if ( self->as.typeBool != Dictionary_Entry_Value_AsBool( dev ) )
				retValue = False;
			break;
		case Dictionary_Entry_Value_Type_Struct:
			if ( dev->type != Dictionary_Entry_Value_Type_Struct ) {
				retValue = False;
				break;
			}
			retValue = Dictionary_CompareAllEntriesFull( self->as.typeStruct, dev->as.typeStruct, strictTypeCheck );
			break;
		case Dictionary_Entry_Value_Type_List: {
			Dictionary_Entry_Value* cur1 = NULL;
			Dictionary_Entry_Value* cur2 = NULL;

			if ( dev->type != Dictionary_Entry_Value_Type_List ) {
				retValue = False;
				break;
			}
			if ( self->as.typeList->count != dev->as.typeList->count ) {
				retValue = False;
				break;
			}
			cur1 = self->as.typeList->first;
			cur2 = dev->as.typeList->first;
			while ( cur1 ) {
				retValue = Dictionary_Entry_Value_CompareFull( cur1, cur2, strictTypeCheck );
				if ( retValue == False ) break;
				cur1 = cur1->next;
				cur2 = cur2->next;
			}	
			break;
		}
		default:
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
	};

	return retValue;
}


Dictionary_Entry_Value* Dictionary_Entry_Value_Copy(
		Dictionary_Entry_Value*  self,
		Bool deep )
{
	Dictionary_Entry_Value* copy = NULL;
	
	switch (self->type) {
		case Dictionary_Entry_Value_Type_String:
			copy = Dictionary_Entry_Value_FromString( self->as.typeString );
			break;
		case Dictionary_Entry_Value_Type_Double:
			copy = Dictionary_Entry_Value_FromDouble( self->as.typeDouble );
			break;
		case Dictionary_Entry_Value_Type_UnsignedInt:
			copy = Dictionary_Entry_Value_FromUnsignedInt( self->as.typeUnsignedInt );
			break;
		case Dictionary_Entry_Value_Type_Int:
			copy = Dictionary_Entry_Value_FromInt( self->as.typeInt );
			break;
		case Dictionary_Entry_Value_Type_UnsignedLong:
			copy = Dictionary_Entry_Value_FromUnsignedLong( self->as.typeUnsignedLong );
			break;
		case Dictionary_Entry_Value_Type_Bool:
			copy = Dictionary_Entry_Value_FromBool( self->as.typeBool );
			break;
		case Dictionary_Entry_Value_Type_List:
			if ( False == deep ) {
				Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
				Journal_Firewall( False, errorStream, "In func %s: Shallow copy operation of list DEV not supported.\n", __func__ );
			}
			else {
				Dictionary_Entry_Value* cur = self->as.typeList->first;
				Dictionary_Entry_Value* copiedEntry = NULL;

				copy = Dictionary_Entry_Value_NewList();
				while ( cur ) {
					copiedEntry = Dictionary_Entry_Value_Copy( cur, True );
					Dictionary_Entry_Value_AddElement( copy, copiedEntry );
					cur = cur->next;
				}	
			}
			break;
		case Dictionary_Entry_Value_Type_Struct:
			if ( False == deep ) {
				copy = Dictionary_Entry_Value_FromStruct( self->as.typeStruct );
			}
			else {
				Dictionary* copiedDict;
				copiedDict = (Dictionary*)Stg_Class_Copy( self->as.typeStruct,
					NULL, True, NULL, NULL );
				copy = Dictionary_Entry_Value_FromStruct( copiedDict );
			}
			break;
		default: {
			Stream* errorStream = Journal_Register( Error_Type, "Dictionary_Entry_Value" );
			Journal_Firewall( False, errorStream, "In func %s: self->type '%d' is invalid.\n", __func__, self->type );
		}
	}

	return copy;
}


