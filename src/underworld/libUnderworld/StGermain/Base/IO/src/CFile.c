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
#include "File.h"
#include "CFile.h"
#include "Stream.h"
#include "Journal.h"

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


const Type CFile_Type = "CFile";


File* CFile_New()
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(CFile);
	Type                              type = CFile_Type;
	Stg_Class_DeleteFunction*      _delete = _CFile_Delete;
	Stg_Class_PrintFunction*        _print = _CFile_Print;
	Stg_Class_CopyFunction*          _copy = NULL;
	Bool                            binary = False;

	return (File*)_CFile_New(  CFILE_PASSARGS  );
}

File* CFileBinary_New()
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(CFile);
	Type                              type = CFile_Type;
	Stg_Class_DeleteFunction*      _delete = _CFile_Delete;
	Stg_Class_PrintFunction*        _print = _CFile_Print;
	Stg_Class_CopyFunction*          _copy = NULL;
	Bool                            binary = True;

	return (File*)_CFile_New(  CFILE_PASSARGS  );
}


File* CFile_NewRead( char* fileName )
{
	File* result = CFile_New();

	if ( !File_Read( result, fileName ) )
	{
		/* File could not be opened successfully. Return cleanly. */
		Stg_Class_Delete( result );
		result = NULL;
	}
	
	return result;
}

File* CFile_NewWrite( char* fileName )
{
	File* result = CFile_New();

	if ( !File_Write( result, fileName ) )
	{
		/* File could not be opened successfully. Return cleanly. */
		Stg_Class_Delete( result );
		result = NULL;
	}
	
	return result;
}

CFile* _CFile_New(  CFILE_DEFARGS  )
{
	CFile* self;
	
	self = (CFile*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	self->binary = binary;
	
	_CFile_Init( self );
	
	return self;
}
	
void CFile_Init( CFile* self )
{
	/* Set virtual info. */
	self->_sizeOfSelf = sizeof(CFile);
	self->type = CFile_Type;
	self->_delete = _CFile_Delete;
	self->_print = _CFile_Print;
	self->_copy = NULL;
	
	_CFile_Init( self );
}
	
void _CFile_Init( CFile* self )
{
	_File_Init( (File*)self, (File_ReadFunction*)_CFile_Read, (File_WriteFunction*)_CFile_Write,
		(File_AppendFunction*)_CFile_Append, _CFile_Close, _CFile_Flush );
}
	
void _CFile_Delete( void* cfile )
{
	CFile* self = (CFile*)cfile;
	
	_File_Delete( self );
}

void _CFile_Print( void* cfile, Stream* stream )
{
	CFile* self = (CFile*)cfile;
	
	_File_Print( self, stream );
}

	
Bool _CFile_Read( void* file, char* fileName ) {
	CFile* self = (CFile*)file;
	FILE* filePtr;
	
	if( (filePtr = self->binary ? fopen( fileName, "rb" ) : fopen( fileName, "r" )) == NULL ) {
		return False;
	}

	self->fileHandle = (void*)filePtr;
	
	return True;	
}
	
Bool _CFile_Write( void* file, char* fileName )
{
	CFile* self = (CFile*) file;
	FILE* filePtr;
	
	if(!self->binary)
		filePtr = fopen( fileName, "w" );
	else
		filePtr = fopen( fileName, "wb" );
	
	if ( filePtr == NULL )
	{
		return False;
	}

	self->fileHandle = (void*) filePtr;
	
	return True;	
}
	
Bool _CFile_Append( void* file, char* fileName )
{
	CFile* self = (CFile*) file;
	FILE* filePtr;
	
	if(!self->binary)
		filePtr = fopen( fileName, "a" );
	else
		filePtr = fopen( fileName, "ab" );
	
	if ( filePtr == NULL )
	{
		return False;
	}

	self->fileHandle = (void*) filePtr;
	
	return True;	
}
Bool _CFile_Close( void* file )
{
	CFile* self = (CFile*) file;
	if ( self->fileHandle != NULL )
	{
		return (Bool) (fclose(  (FILE*) self->fileHandle ) == 0);
	}
	return False;
}

Bool _CFile_Flush( void* file )
{
	CFile* self = (CFile*) file;
	if ( self->fileHandle != NULL )
	{
		return (Bool) (fflush( (FILE*) self->fileHandle ) == 0);
	}
	return False;
}

FILE* CFile_Ptr( void* file ) {
	CFile* self = (CFile*)file;
	return (FILE*)self->fileHandle;
}


