/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_MPIFile_h__
#define __StGermain_Base_IO_MPIFile_h__

#include <mpi.h>

	/** Textual name for MPIFile class. */
	extern const Type MPIFile_Type;
	
	/** \def __MPIFile See MPIFile. */
	#define __MPIFile \
		/* General info */ \
		__File \
		MPI_File mpiFile;
	struct MPIFile { __MPIFile };


	/** Creates a new MPIFile with no opened file. File_Read/Write/Append() must be called. */
	File* MPIFile_New();

	/** Creates a new MPIFile, opening the file given by fileName. Returns NULL if system is unable to open the given file. */
	File* MPIFile_New2( const char* const fileName );

	/** Initialises an instance of MPIFile. Will not open any files. */
	void MPIFile_Init( MPIFile* self );


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MPIFILE_DEFARGS \
                STG_CLASS_DEFARGS

	#define MPIFILE_PASSARGS \
                STG_CLASS_PASSARGS

	MPIFile* _MPIFile_New(  MPIFILE_DEFARGS  );
		
	/** Init interface. */
	void _MPIFile_Init( MPIFile* self );
	
	/** Stg_Class_Delete interface. */
	void _MPIFile_Delete( void* cfile );

	/** Print interface. */
	void _MPIFile_Print( void* cfile, Stream* stream );
	
	
	/** Implementation for Read function. */
	Bool _MPIFile_Read( void* file, const char* const fileName );

	
	/** Implementation for Write function. */
	Bool _MPIFile_Write( void* file, const char* const fileName );

	/** Implementation for Append function. */
	Bool _MPIFile_Append( void* file, const char* const fileName );

	/** Implementation for Close function. */
	Bool _MPIFile_Close( void* file );

	/** Implementation for Flush function. */
	Bool _MPIFile_Flush( void* file );
	

#endif /* __StGermain_Base_IO_MPIFile_h__ */

