/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_IndentFormatter_h__
#define __StGermain_Base_IO_IndentFormatter_h__


	/** Textual name for IndentFormatter class. */
	extern const Type IndentFormatter_Type;


	/** \def __IndentFormatter See IndentFormatter. */
	#define __IndentFormatter \
		__LineFormatter \
		Index	_indent; \
		char	_character;
	struct IndentFormatter { __IndentFormatter };


	/** Creates a new IndentFormatter. */
	StreamFormatter* IndentFormatter_New();

	/** Creates a new IndentFormatter which indents with the given char. */
	StreamFormatter* IndentFormatter_New2( char _character );

	/** Initialise a IndentFormatter. */
	void IndentFormatter_Init( IndentFormatter* self );


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define INDENTFORMATTER_DEFARGS \
                STG_CLASS_DEFARGS, \
                StreamFormatter_FormatFunction*     _format, \
                char                             _character

	#define INDENTFORMATTER_PASSARGS \
                STG_CLASS_PASSARGS, \
	        _format,    \
	        _character

	IndentFormatter* _IndentFormatter_New(  INDENTFORMATTER_DEFARGS  );

	/** Init interface. */
	void _IndentFormatter_Init(
		IndentFormatter*		self,
		StreamFormatter_FormatFunction*	_format,
		char				_character );

	/** Stg_Class_Delete interface. */
	void _IndentFormatter_Delete( void* formatter );

	/** Print interface. */
	void _IndentFormatter_Print( void* formatter, Stream* stream );

	/** Copy implementation. */
	void* _IndentFormatter_Copy( void* indentFormatter, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );


	/** Set the value of indentation. */
	void IndentFormatter_SetIndent( void* formatter, Index indent );

	/** Increase the level of indentation by 1. */
	void IndentFormatter_Increment( void* formatter );

	/** Decrease the level of indentation by 1. */
	void IndentFormatter_Decrement( void* formatter );

	/** Sets the level of indentation to zero. */
	void IndentFormatter_Zero( void* formatter);
	
	/** Sets the character to indent with */
	void IndentFormatter_SetCharacter( char _character );


#endif /* __StGermain_Base_IO_IndentFormatter_h__ */

