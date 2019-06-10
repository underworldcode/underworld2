/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StGermain_Utils_Progress_h__
#define __StGermain_Utils_Progress_h__

#define __Progress				\
   __Stg_Class					\
   int rank;					\
   Stream* strm;				\
   char* title;					\
   Bool printTitle;				\
   char* preStr;				\
   int width;					\
   int start;					\
   int end;					\
   int pos;					\
   int perc;
struct Progress { __Progress };

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PROGRESS_DEFARGS \
                STG_CLASS_DEFARGS

	#define PROGRESS_PASSARGS \
                STG_CLASS_PASSARGS

Progress* Progress_New();
Progress* _Progress_New(  PROGRESS_DEFARGS  );
void _Progress_Init( void* self );
void _Progress_Delete( void* self );
void _Progress_Print( void* self, Stream* stream );

void Progress_SetStream( void* self, Stream* strm );
void Progress_SetTitle( void* self, const char* str );
void Progress_SetPrefix( void* self, const char* str );
void Progress_SetRange( void* self, int start, int end );
void Progress_Restart( void* self );
void Progress_Update( void* self );
void Progress_Increment( void* self );

#endif /* __StGermain_Utils_Progress_h__ */

