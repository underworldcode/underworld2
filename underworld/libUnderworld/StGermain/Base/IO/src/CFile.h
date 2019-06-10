/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_CFile_h__
#define __StGermain_Base_IO_CFile_h__

	#include <stdio.h>
	#include "File.h"
	/** Textual name for CFile class. */
	extern const Type CFile_Type;
	
	/** \def __CFile See CFile. */
	#define __CFile \
		/* General info */ \
		__File  \
		/* open a binary file? */ \
		Bool   binary;
	struct CFile { __CFile };


	/** Creates a new CFile with no opened file. File_Read/Write/Append() must be called. */
	File* CFile_New();

	/** Creates a new CFile for reading, opening the file given by fileName. Returns NULL if system is unable to open the given file. */
	File* CFile_NewRead( char* fileName );

	/** Creates a new CFile for writing, opening the file given by fileName. Returns NULL if system is unable to open the given file. */
	File* CFile_NewWrite( char* fileName );

	/** Creates a new binary CFile with no opened file. File_Open() must be called. */
	File* CFileBinary_New();
	
	/** Initialises an instance of CFile. Will not open any files. */
	void CFile_Init( CFile* self );


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CFILE_DEFARGS \
                STG_CLASS_DEFARGS, \
                Bool  binary

	#define CFILE_PASSARGS \
                STG_CLASS_PASSARGS, \
	        binary

	CFile* _CFile_New(  CFILE_DEFARGS  );
		
	/** Init interface. */
	void _CFile_Init( CFile* self );
	
	/** Stg_Class_Delete interface. */
	void _CFile_Delete( void* cfile );

	/** Print interface. */
	void _CFile_Print( void* cfile, Stream* stream );
	
	
	/** Implementation for Read function. */
	Bool _CFile_Read( void* file, char* fileName );

	/** Implementation for Write function. */
	Bool _CFile_Write( void* file, char* fileName );

	/** Implementation for Append function. */
	Bool _CFile_Append( void* file, char* fileName );

	/** Implementation for Close function. */
	Bool _CFile_Close( void* file );

	/** Implementation for Flush function. */
	Bool _CFile_Flush( void* file );
	
	/** Obtain the FILE pointer */
	FILE* CFile_Ptr( void* file );


#endif /* __StGermain_Base_IO_CFile_h__ */



