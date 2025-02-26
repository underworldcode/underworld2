/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_MPIStream_h__
#define __StGermain_Base_IO_MPIStream_h__
	
	/** Textual name for MPIStream class. */
	extern const Type MPIStream_Type;
	
	
	/** \def __MPIStream See MPIStream. */
	#define __MPIStream \
		/* General info */ \
		__Stream
	struct MPIStream { __MPIStream };


	/** Create a new MPIStream */
	Stream* MPIStream_New( Name name );

	/** Inits a MPIStream. */
	void MPIStream_Init( MPIStream* self, Name name );

	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MPISTREAM_DEFARGS \
                STREAM_DEFARGS

	#define MPISTREAM_PASSARGS \
                STREAM_PASSARGS

	MPIStream* _MPIStream_New(  MPISTREAM_DEFARGS  );

	/** Init interface. */
	void _MPIStream_Init( MPIStream *self );
	
	/** Stg_Class_Delete interface. */
	void _MPIStream_Delete( void* cStream );
	
	/** Print interface. */
	void _MPIStream_Print( void* cStream, Stream* stream );


	/** Printf() implementation. */
	SizeT _MPIStream_Printf( Stream* stream, char *fmt, va_list args );
	
	/** Write() implementation. */
	SizeT _MPIStream_Write( Stream* stream, void *data, SizeT elem_size, SizeT num_elems );
	
	/** Dump() implementation. Performs no operation for MPIStreams. */
	Bool _MPIStream_Dump( Stream* stream, void *data );
	
	/** SetFile() implementation. */
	Bool _MPIStream_SetFile( Stream* stream, File* file );
	
	/** Sets the offset in bytes of this an mpi stream so that each rank may print to their own position in file */
	Bool MPIStream_SetOffset( Stream* stream, SizeT sizeToWrite, MPI_Comm communicator ) ;
	SizeT MPIStream_WriteAllProcessors( Stream* stream, void *data, SizeT elem_size, SizeT num_elems, MPI_Comm communicator ) ;
	
#endif /* __IO_MPIStreamFile_h__ */




