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
#include "File.h"
#include "Journal.h"
#include "Stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type File_Type = "File";

Stg_ObjectList* _stgFilePaths=NULL;

File* _File_New(  JOURNALFILE_DEFARGS  )
{
	File* self;
	
	assert( _sizeOfSelf >= sizeof(File) );
	self = (File*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	_File_Init( self, _read, _write, _append, _close, _flush );
	
	return self;
}
	
void _File_Init(
	File*			self,
	File_ReadFunction*	_read,
	File_WriteFunction*	_write,
	File_AppendFunction*	_append,
	File_CloseFunction*	_close,
	File_FlushFunction*	_flush )
{
	_Stg_Object_Init( (Stg_Object*)self, "", NON_GLOBAL );
	
	self->_read = _read;
	self->_write = _write;
	self->_append = _append;
	self->_close = _close;
	self->_flush = _flush;
	self->_lastOpenedAs = 0;
	
	self->fileHandle = NULL;
	self->_opened = False;
}

void _File_Delete( void* file )
{
	/* Close if opened */
	File_Close( file );
	_Stg_Object_Delete( file );
}

void _File_Print( void* file, Stream* stream )
{
	File* self = (File*)file;
                                                                                
        Journal_Printf( stream, "File (ptr): %p\n", self );
         
        _Stg_Object_Print( self, stream );
         
        Journal_Printf( stream, "\tfileHandle (ptr): %p\n", self->fileHandle );
}

Bool File_Read( void* file, const char* const fileName ) {
	File* self = (File*)file;
	Bool result;
	
	if( !(result = self->_read( self, fileName )) ) {
		/* If the file failed to read, try prepending the search paths (if its not an absolute path) */
		if( strlen(fileName) >= 1 && fileName[0] != '/' ) {
			int i;

			for( i = 0; i < Stg_ObjectList_Count( _stgFilePaths ); i++ ) {
				char* path = (char*)Stg_ObjectAdaptor_Object( (Stg_ObjectAdaptor*)Stg_ObjectList_At( _stgFilePaths, i ) );
				char newFileName[FILENAME_MAX];

				strncpy( newFileName, path, FILENAME_MAX-1 );
				strncat( newFileName, "/", FILENAME_MAX-1 - 1 );
				strncat( newFileName, fileName, FILENAME_MAX-1 - 1 - strlen(path) );
				if( (result = self->_read( self, newFileName )) ) {
					break;
				}
			}
		}
	}
	
	if( result ) {
		/* File opened, set the name. */
		Stg_Object_SetName( self, (char*)fileName );
		self->_opened = True;
		self->_lastOpenedAs = 1;
	}
	
	return result;
}

Bool File_Write( void* file, const char* const fileName )
{
	File* self = (File*)file;
	
	Bool result = self->_write( self, fileName );
	
	if ( result )
	{
		/* File opened, set the name. */
		Stg_Object_SetName( self, (char*)fileName );
		self->_opened = True;
		self->_lastOpenedAs = 2;
	}
	
	return result;
}

Bool File_Reopen( void* file ) {
	File* self = (File*)file;

	Bool result;

	if ( self->_opened ) {
		return True;
	}
	switch( self->_lastOpenedAs ) {
		case 1:
			result = File_Read( self, self->name ); /* in particular honors search paths */
			break;
		case 0:
		default:
			/* This assumption is made because prior to adding "read" this was the only/default action*/
			Journal_Printf( 
				Journal_Register( Error_Type, File_Type ), 
				"Assuming %s is to be reopened for a write!!! Inform developers if wrong. %s:%u\n",
				self->name, __FILE__, __LINE__ );
		case 3:
			/* Treat the same as... */
		case 2:
			result = File_Write( self, self->name );
			break;
	}
	if ( result ) {
		self->_opened = True;
	}
	return result;
}

Bool File_Append( void* file, const char* const fileName ) {
	File* self = (File*)file;
	
	Bool result = self->_append( self, fileName );
	
	if ( result )
	{
		/* File opened, set the name. */
		Stg_Object_SetName( self, (char*)fileName );
		self->_opened = True;
		self->_lastOpenedAs = 3;
	}
	
	return result;
}
Bool File_Close( void* file )
{
	File* self = (File*)file;
	Bool result = False;
	if ( self->_opened ) {
		result = self->_close( file );
		if ( result ) {
			self->_opened = False;
		}
	}
	return result;
}

Bool File_IsOpened( void* file ) {
	File* self = (File*)file;
	return self->_opened;
}

Bool File_Flush( void* file )
{
	File* self = (File*)file;
	stJournal->flushCount++;
	return self->_flush( self );
}

static void deleteInputPathItem( void* ptr ) {
   /* Correct way to delete result of stgParseInputPathCmdLineArg items */
   Memory_Free( ptr );
}


/** add a path to the search paths */
void File_AddPath( char* directory ) {
	Bool found;
	Index dir_i;

	/* Check if it is a valid path */
	if( !directory ) {
		return;
	}

	/* Check if dictionary already exists */
	if( _stgFilePaths == NULL ) {
		_stgFilePaths = Stg_ObjectList_New();
	}
	
	/* Add path to global list */
	found = False;
	for( dir_i =  0; dir_i < _stgFilePaths->count; dir_i++ ){
		if( strcmp( directory, (char*)Stg_ObjectList_ObjectAt( _stgFilePaths, dir_i ) ) == 0 ) {
			found = True;
		}
	}
	
	if( !found ) {
		Stg_ObjectList_PointerAppend( _stgFilePaths, StG_Strdup( directory ), directory, deleteInputPathItem, 0, 0 ); 
	}
}

