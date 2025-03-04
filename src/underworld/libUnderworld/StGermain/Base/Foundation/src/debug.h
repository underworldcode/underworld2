/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_debug_hh__
#define __StGermain_Base_Foundation_debug_hh__

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "types.h"

#ifndef NDEBUG

#define ASSERT_FAIL	1

extern Bool	assert_jmpEnabled;
extern jmp_buf	assert_env;

#ifdef stgAssert
#undef stgAssert
#endif
#define stgAssert( e, file, line )						\
   (printf( "%s:%u: failed assertion `%s'\n", file, line, e ), abort(), 0)

#ifdef assert
#undef assert
#endif
#define assert( expr ) ((void)((expr) ? 0 : assert_jmpEnabled ?			\
			     (longjmp( assert_env, ASSERT_FAIL), 0 ) :		\
			     stgAssert( #expr, __FILE__, __LINE__)) )
#else

#ifdef assert
#undef assert
#endif
#define assert( expr ) ((void)0)

#endif

#endif /*__StGermain_Base_Foundation_debug_hh__*/
