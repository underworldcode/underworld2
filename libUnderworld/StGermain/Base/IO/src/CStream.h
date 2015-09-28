/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_CStream_h__
#define __StGermain_Base_IO_CStream_h__
	
	/** Textual name for CStream class. */
	extern const Type CStream_Type;
	
	
	/** \def __CStream See CStream. */
	#define __CStream \
		/* General info */ \
		__Stream
	struct CStream { __CStream };


	/** Create a new CStream */
	Stream* CStream_New( Name name );

	/** Inits a CStream. */
	void CStream_Init( CStream* self, Name name );

	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CSTREAM_DEFARGS \
                STREAM_DEFARGS

	#define CSTREAM_PASSARGS \
                STREAM_PASSARGS

	CStream* _CStream_New(  CSTREAM_DEFARGS  );

	/** Init interface. */
	void _CStream_Init( CStream *self );
	
	/** Stg_Class_Delete interface. */
	void _CStream_Delete( void* cStream );
	
	/** Print interface. */
	void _CStream_Print( void* cStream, Stream* stream );


	/** Printf() implementation. */
	SizeT _CStream_Printf( Stream* stream, char *fmt, va_list args );
	
	/** Write() implementation. */
	SizeT _CStream_Write( Stream* stream, void *data, SizeT elem_size, SizeT num_elems );
	
	/** Dump() implementation. Performs no operation for CStreams. */
	Bool _CStream_Dump( Stream* stream, void *data );
	
	/** SetFile() implementation. */
	Bool _CStream_SetFile( Stream* stream, File* file );
	
	
#endif /* __IO_CStreamFile_h__ */




