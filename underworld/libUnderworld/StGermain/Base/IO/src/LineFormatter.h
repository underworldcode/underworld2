/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __StGermain_Base_IO_LineFormatter_h__
#define __StGermain_Base_IO_LineFormatter_h__

	/** Textual name for LineFormatter class. */
	extern const Type LineFormatter_Type;


	/** \def __LineFormatter See LineFormatter */
	#define __LineFormatter \
		__StreamFormatter			/* Inherit StreamFormatter. */ \
							\
		Bool	_newLine;			/* Whether the current stream is at a new line. */ \
		char*	_tag;				/* The string to prepend at the start of every line. */
	struct LineFormatter { __LineFormatter };


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define LINEFORMATTER_DEFARGS \
                STG_CLASS_DEFARGS, \
                StreamFormatter_FormatFunction*  _format

	#define LINEFORMATTER_PASSARGS \
                STG_CLASS_PASSARGS, \
	        _format

	LineFormatter* _LineFormatter_New(  LINEFORMATTER_DEFARGS  );

	/** Init interface. */
	void _LineFormatter_Init(
		LineFormatter*			self,
		StreamFormatter_FormatFunction*	_format );
			
	/** Stg_Class_Delete interface. */
	void _LineFormatter_Delete( void* formatter );

	/** Print interface. */
	void _LineFormatter_Print( void* formatter, Stream* stream );
	
	/** Copy interface. */
	#define LineFormatter_Copy( self ) \
		(LineFormatter*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define LineFormatter_DeepCopy( self ) \
		(LineFormatter*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _LineFormatter_Copy( void* lineFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );


	/** Formats a output string of printf by prepending a tag in front of every line detected. */
	char* _LineFormatter_Format( void* formatter, const char* const fmt );


#endif /* __StGermain_Base_IO_LineFormatter_h__ */




