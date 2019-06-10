/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_File_h__
#define __StGermain_Base_IO_File_h__

	/** Textual name for File class. */
	extern const Type File_Type;

	/** Search paths for these files */
	extern Stg_ObjectList* _stgFilePaths;

	typedef File* (File_Constructor) (void);
	typedef Bool (File_ReadFunction) (void* file, const char* const fileName );
	typedef Bool (File_WriteFunction) (void* file, const char* const fileName );
	typedef Bool (File_AppendFunction) (void* file, const char* const fileName );
	typedef Bool (File_CloseFunction) (void* file );
	typedef Bool (File_FlushFunction) (void* file );
		
	/** \def __File See File */
	#define __File \
		__Stg_Object					\
								\
		/* Virtual functions */				\
		File_ReadFunction*	_read; 		\
		File_WriteFunction*	_write; 	\
		File_AppendFunction*	_append; 	\
		File_CloseFunction*	_close; 	\
		File_FlushFunction*	_flush; 	\
								\
		/* File members */			\
		void*				fileHandle;	/* Generic file handle for any file. */ \
		Bool				_opened; \
		int				_lastOpenedAs;
	struct File { __File };


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define JOURNALFILE_DEFARGS \
                STG_CLASS_DEFARGS, \
                File_ReadFunction*     _read, \
                File_WriteFunction*    _write, \
                File_AppendFunction*   _append, \
                File_CloseFunction*    _close, \
                File_FlushFunction*    _flush

	#define JOURNALFILE_PASSARGS \
                STG_CLASS_PASSARGS, \
	        _read,   \
	        _write,   \
	        _append, \
	        _close,  \
	        _flush 

	File* _File_New(  JOURNALFILE_DEFARGS  );
	
	/** Init interface. */
	void _File_Init(
		File*			self,
		File_ReadFunction*	_read,
		File_WriteFunction*	_write,
		File_AppendFunction*	_append,
		File_CloseFunction*	_close,
		File_FlushFunction*	_flush );
	
	/** Stg_Class_Delete interface. */
	void _File_Delete( void* file );

	/** Print interface. */
	void _File_Print( void* file, Stream* stream );		
	

	/** Open a file for reading. If successful, sets the name for this instance to the file name and returns True.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_Read( void* file, const char* const fileName );
	
	
	/** Open a file for writing. If successful, sets the name for this instance to the file name and returns True.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_Write( void* file, const char* const fileName );
	
	
	/** Reopen a file for writing. It will use the previous name it was opened with.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_Reopen( void* file );
	
	/** Macro to improve readability */
	#define File_ReopenIfClosed File_Reopen
	
	/** Opens a file for writing, if the file exists then append future print statements to this file.
	 ** If successful, sets the name for this instance to the file name and returns True.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_Append( void* file, const char* const fileName );

	/** Close a file. If successful, returns True.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_Close( void* file );

	/** Checks whether a file is opened.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_IsOpened( void* file );

	/** Flush a file, outputting all un-written data from input buffers to device/file. If successful, returns True.
	 **
	 ** @param file An instance of a File sub-class.
	 **/
	Bool File_Flush( void* file );

	/** Add a default lookup fail - where read-opens fail because file is not found, will lookup.
	 **
	 ** @param file An instance of a File sub-class.
	 ** @param directory Directory/path to add to look up
	 **/
	void File_AddPath( char* directory );


#endif /* __StGermain_Base_IO_File_h__ */

