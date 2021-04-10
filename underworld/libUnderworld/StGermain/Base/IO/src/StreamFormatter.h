/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_StreamFormatter_h__
#define __StGermain_Base_IO_StreamFormatter_h__


	/** Textual name for StreamFormatter class. */
	extern const Type StreamFormatter_Type;

	typedef char* (StreamFormatter_FormatFunction) ( void* formatter, const char* const fmt );

	/** \def __StreamFormatter See StreamFormatter */
	#define __StreamFormatter \
		__Stg_Class						\
								\
		/* Virtual functions */				\
		StreamFormatter_FormatFunction*	_format;	\
								\
		/* Members */					\
		Stream*				_stream;
	struct StreamFormatter { __StreamFormatter };


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STREAMFORMATTER_DEFARGS \
                STG_CLASS_DEFARGS, \
                StreamFormatter_FormatFunction*  _format

	#define STREAMFORMATTER_PASSARGS \
                STG_CLASS_PASSARGS, \
	        _format

	StreamFormatter* _StreamFormatter_New(  STREAMFORMATTER_DEFARGS  );
	
	/** Init interface. */
	void _StreamFormatter_Init(
		StreamFormatter*		self,
		StreamFormatter_FormatFunction*	_format );
			
	/** Stg_Class_Delete interface. */
	void _StreamFormatter_Delete( void* formatter );

	/** Print interface. */
	void _StreamFormatter_Print( void* formatter, Stream* stream );
	
	/** Copy interface. */	
	void* _StreamFormatter_Copy( void* streamFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );


	/** Formats a Printf() output string. */
	char* StreamFormatter_Format( void* formatter, const char* const fmt );


	extern StreamFormatter_Buffer* stgStreamFormatter_Buffer;

	/* A buffer swap singleton for improve formatting efficiency. */
	#define __StreamFormatter_Buffer \
		char* buffer1; \
		char* buffer2; \
		Index length1; \
		Index length2; \
		char** current; \
		Index* lengthPtr; 
	struct StreamFormatter_Buffer { __StreamFormatter_Buffer };

	StreamFormatter_Buffer* StreamFormatter_Buffer_New();
	void StreamFormatter_Buffer_Delete( StreamFormatter_Buffer* buffer );
	char* StreamFormatter_Buffer_AllocNext( Index size );

#endif /* __StGermain_Base_IO_StreamFormatter_h__ */

