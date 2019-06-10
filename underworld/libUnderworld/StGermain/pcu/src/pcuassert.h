/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_assert_h
#define pcu_assert_h

#include <setjmp.h>
#include <assert.h>
#include <stdlib.h>
#include "checks.h"

extern int pcu_jump_ready;
extern jmp_buf pcu_jump_env;
extern char* pcu_assert_cur;
extern jmp_buf pcu_rollback_env;


#ifndef NDEBUG

#define pcu_assert( expr )                                          \
   (pcu_jump_ready ?                                                \
       (!(expr) ?                                                   \
           (pcu_assert_cur = #expr, longjmp( pcu_jump_env, 1 ))     \
          :(void)0)                                                 \
      :(!(expr) ?                                                   \
          abort()                                                   \
         :(void)0 ))

#else

#define pcu_assert( expr ) assert( expr )

#endif

#define pcu_rollback( expr ) \
   longjmp( pcu_rollback_env, 1) \

#endif
