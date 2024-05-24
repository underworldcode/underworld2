/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_RankFormatter_h__
#define __StGermain_Base_IO_RankFormatter_h__


	/** Textual name for RankFormatter class. */
	extern const Type RankFormatter_Type;

	/** \def __RankFormatter See RankFormatter */
	#define __RankFormatter \
		__LineFormatter
	struct RankFormatter { __RankFormatter };


	/** Creates a new RankFormatter, automatically determining the current rank. */
	StreamFormatter* RankFormatter_New();
	
	/** Init a new RankFormatter, automatically determining the current rank. */
	void RankFormatter_Init( RankFormatter* self );


	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define RANKFORMATTER_DEFARGS \
                LINEFORMATTER_DEFARGS

	#define RANKFORMATTER_PASSARGS \
                LINEFORMATTER_PASSARGS

	RankFormatter* _RankFormatter_New(  RANKFORMATTER_DEFARGS  );

	/** Init interface. */
	void _RankFormatter_Init(
		RankFormatter*			self,
		StreamFormatter_FormatFunction*	_format );
			
	/** Stg_Class_Delete interface. */
	void _RankFormatter_Delete( void* formatter );

	/** Print interface. */
	void _RankFormatter_Print( void* formatter, Stream* stream );


#endif /* __StGermain_Base_IO_RankFormatter_h__ */




