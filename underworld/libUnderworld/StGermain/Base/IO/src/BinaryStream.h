/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_BinaryStream_h__
#define __StGermain_Base_IO_BinaryStream_h__
	
	/** Textual name for BinaryStream class. */
	extern const Type BinaryStream_Type;
	
	
	/** \def __BinaryStream See BinaryStream. */
	#define __BinaryStream \
		/* General info */ \
		__Stream
	struct BinaryStream { __BinaryStream };


	/** Create a new BinaryStream */
	Stream* BinaryStream_New( Name name );

	/** Inits a BinaryStream. */
	void _BinaryStream_Init( BinaryStream* self );

	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define BINARYSTREAM_DEFARGS \
                STREAM_DEFARGS

	#define BINARYSTREAM_PASSARGS \
                STREAM_PASSARGS

	BinaryStream* _BinaryStream_New(  BINARYSTREAM_DEFARGS  );

	/** Stg_Class_Delete interface. */
	void _BinaryStream_Delete( void* cStream );
	
	/** Print interface. */
	void _BinaryStream_Print( void* cStream, Stream* stream );


	/** Printf() implementation. */
	SizeT _BinaryStream_Printf( Stream* stream, char *fmt, va_list args );
	
	/** Write() implementation. */
	SizeT _BinaryStream_Write( Stream* stream, void *data, SizeT elem_size, SizeT num_elems );
	
	/** Dump() implementation. Performs no operation for BinaryStreams. */
	Bool _BinaryStream_Dump( Stream* stream, void *data );
	
	/** SetFile() implementation. */
	Bool _BinaryStream_SetFile( Stream* stream, File* file );
	
	SizeT BinaryStream_WriteAllProcessors( Name filename, void *data, SizeT elem_size, SizeT num_elems, MPI_Comm communicator ) ;
	
#endif /* __IO_BinaryStreamFile_h__ */




