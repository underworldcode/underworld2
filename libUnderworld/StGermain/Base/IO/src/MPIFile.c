/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "File.h"
#include "MPIFile.h"
#include "Stream.h"
#include "Journal.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


const Type MPIFile_Type = "MPIFile";


File* MPIFile_New()
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(MPIFile);
	Type                              type = MPIFile_Type;
	Stg_Class_DeleteFunction*      _delete = _MPIFile_Delete;
	Stg_Class_PrintFunction*        _print = _MPIFile_Print;
	Stg_Class_CopyFunction*          _copy = NULL;

	return (File*)_MPIFile_New(  MPIFILE_PASSARGS  );
}

File* MPIFile_New2( const char* const fileName )
{
	File* result = MPIFile_New();

	if ( !File_Write( result, fileName ) )
	{
		/* File could not be opened successfully. Return cleanly. */
		Stg_Class_Delete( result );
		result = NULL;
	}
	
	return result;
}

MPIFile* _MPIFile_New(  MPIFILE_DEFARGS  )
{
	MPIFile* self;
	
	self = (MPIFile*)_Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	_MPIFile_Init( self );
	
	return self;
}
	
void MPIFile_Init( MPIFile* self )
{
	/* Set virtual info. */
	self->_sizeOfSelf = sizeof(MPIFile);
	self->type = MPIFile_Type;
	self->_delete = _MPIFile_Delete;
	self->_print = _MPIFile_Print;
	self->_copy = NULL;
	
	_MPIFile_Init( self );
}
	
void _MPIFile_Init( MPIFile* self )
{
	_File_Init( (File*)self, _MPIFile_Read, _MPIFile_Write, _MPIFile_Append, _MPIFile_Close, _MPIFile_Flush );
}
	
void _MPIFile_Delete( void* cfile )
{
	MPIFile* self = (MPIFile*)cfile;
	
	_File_Delete( self );
}
void _MPIFile_Print( void* cfile, Stream* stream )
{
	MPIFile* self = (MPIFile*)cfile;
	
	_File_Print( self, stream );
}

	
Bool _MPIFile_Read( void* file, const char* const fileName )
{
	Journal_Firewall( 
		0,
		Journal_Register( Error_Type, MPIFile_Type ), 
		"Feature not yet implemented! %s:%s", 
		__FILE__, __LINE__ );

	return False;
}
	
Bool _MPIFile_Write( void* file, const char* const fileName )
{
	MPIFile*   self = (MPIFile*) file;
	int        fileOpenResult = 0;

	/* Remove the file */
        remove( fileName );

	fileOpenResult = MPI_File_open( MPI_COMM_WORLD, (char*)fileName,
				MPI_MODE_CREATE | MPI_MODE_WRONLY | MPI_MODE_EXCL, 
				MPI_INFO_NULL, &(self->mpiFile) );

	if (fileOpenResult != MPI_SUCCESS) {
		char         errorString[2000];
		int          errorStringLength = 0;
		Stream*      errorStream = Journal_Register( Error_Type, MPIFile_Type );
		int          myRank = 0;

		MPI_Comm_rank( MPI_COMM_WORLD, &myRank );
		MPI_Error_string( fileOpenResult, errorString, &errorStringLength);
		Journal_Printf( errorStream, "%3d: %s\n", myRank, errorString );
		MPI_Abort(MPI_COMM_WORLD, fileOpenResult );
	}

	self->fileHandle = &(self->mpiFile);
	return True;	
}
	
Bool _MPIFile_Append( void* file, const char* const fileName )
{
	MPIFile*   self = (MPIFile*) file;
	int        fileOpenResult = 0;

	fileOpenResult = MPI_File_open( MPI_COMM_WORLD, (char*)fileName, MPI_MODE_WRONLY, MPI_INFO_NULL,
		&(self->mpiFile) );

	if (fileOpenResult != MPI_SUCCESS) {
		char         errorString[2000];
		int          errorStringLength = 0;
		Stream*      errorStream = Journal_Register( Error_Type, MPIFile_Type );
		int          myRank = 0;

		MPI_Comm_rank( MPI_COMM_WORLD, &myRank );
		MPI_Error_string( fileOpenResult, errorString, &errorStringLength);
		Journal_Printf( errorStream, "%3d: %s\n", myRank, errorString );
		MPI_Abort(MPI_COMM_WORLD, fileOpenResult );
	}

	self->fileHandle = &(self->mpiFile);
	return True;	
}


Bool _MPIFile_Close( void* file )
{
	MPIFile* self = (MPIFile*) file;
	
	MPI_File_close( &(self->mpiFile) );
	
	return True;
}

Bool _MPIFile_Flush( void* file )
{

	/* No flush */
	
	return True;
}


