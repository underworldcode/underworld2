/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef pcu_checks_h
#define pcu_checks_h

/* For the streq macro */
#include <string.h>

#include "types.h"
#include "listener.h"
#include "suite.h"
#include "source.h"
#include "assert.h"

extern pcu_suite_t* pcu_cursuite;

#define _pcu_check_eval( expr, strexpr, msg, type )     \
   pcu_cursuite->lsnrs[0]->checkdone(                       \
      pcu_cursuite->lsnrs[0],                               \
      pcu_test_addSource(                               \
         pcu_cursuite->curtest,                         \
         pcu_source_create( (expr) ? 1 : 0,             \
                            type,                       \
                            __FILE__,                   \
                            __LINE__,                   \
                            #strexpr,                   \
                            msg,                        \
                            pcu_cursuite->curtest )     \
         )                                              \
      )

/* Use this version if you need manual control over the file and line to be printed in case of failure.
 * Also, the "exprstring" should be an actual string, rather than "strexpr" in the above function which will be stringified. */
#define _pcu_check_eval2( expr, exprstring, msg, type, file, line )     \
   pcu_cursuite->lsnrs[0]->checkdone(                       \
      pcu_cursuite->lsnrs[0],                               \
      pcu_test_addSource(                               \
         pcu_cursuite->curtest,                         \
         pcu_source_create( (expr) ? 1 : 0,             \
                            type,                       \
                            file,                   \
                            line,                   \
                            exprstring,              \
                            msg,                        \
                            pcu_cursuite->curtest )     \
         )                                              \
      )

#define pcu_check_true( expr )			\
	_pcu_check_eval( expr, expr, NULL, "true" )

#define pcu_check_gt( a, b )                                            \
   _pcu_check_eval( (a) > (b), (a) > (b), NULL, "greater than" )

#define pcu_check_lt( a, b )                                    \
   _pcu_check_eval( (a) < (b), (a) < (b), NULL, "less than" )

#define pcu_check_ge( a, b )                                            \
   _pcu_check_eval( (a) >= (b), (a) >= (b), NULL, "greater than or equal" )

#define pcu_check_le( a, b )                                            \
   _pcu_check_eval( (a) <= (b), (a) <= (b), NULL, "less than or equal" )

/** Check that two strings are equal */
#define pcu_check_streq( strA, strB ) \
   do { \
      /* The temporary ptrs are so ++ operations etc inside (strA) or (b) aren't done several times */ \
      const char* tempStr1 = (strA); \
      const char* tempStr2 = (strB); \
      _pcu_check_eval( tempStr1 != NULL, 0 == strcmp( strA, strB ), "First string passed to pcu_check_streq was NULL", "equal strings-preCheck" ); \
      _pcu_check_eval( tempStr2 != NULL, 0 == strcmp( strA, strB ), "Second string passed to pcu_check_streq was NULL", "equal strings-preCheck" ); \
      if ( tempStr1 && tempStr2 ) { \
         char  msgString[1000]; \
         sprintf( msgString, "Actual strings were- \"%s\", \"%s\"", tempStr1, tempStr2 ); \
         _pcu_check_eval( 0 == strcmp( tempStr1, tempStr2 ), 0 == strcmp( strA, strB ), msgString, "equal strings" ); \
      } \
   } while( 0 )

#define pcu_check_fileEq( fnameA, fnameB ) \
   _pcu_check_fileEq( (fnameA), (fnameB), #fnameA, #fnameB, pcu_cursuite, __FILE__, __LINE__ )

void _pcu_check_fileEq( const char* const fileName1, const char* const fileName2,
      const char* const fName1Expr, const char* const fName2Expr, pcu_suite_t* pcu_cursuite,
      const char* sourceFile, const unsigned int sourceLine );

#ifndef NDEBUG

#define pcu_check_noassert( stmnt )                                     \
   do {                                                                 \
      pcu_jump_ready = 1;                                               \
      if( setjmp( pcu_jump_env ) ) {                                    \
         _pcu_check_eval( 0, stmnt, pcu_assert_cur, "assertion free" ); \
         pcu_assert_cur = NULL;                                         \
      }                                                                 \
      else {                                                            \
         stmnt;                                                         \
      }                                                                 \
      pcu_jump_ready = 0;                                               \
   } while( 0 )

#define pcu_check_assert( stmnt )                                       \
   do {                                                                 \
      pcu_jump_ready = 1;                                               \
      if( setjmp( pcu_jump_env ) )                                      \
         pcu_jump_ready = 0;                                            \
      else {                                                            \
         stmnt;                                                         \
      }                                                                 \
      if( pcu_jump_ready ) {                                            \
         _pcu_check_eval( 0, stmnt, NULL, "assertion generated" );      \
         pcu_jump_ready = 0;                                            \
      }                                                                 \
   } while( 0 )

#else

#define pcu_check_noassert( stmnt ) stmnt
#define pcu_check_assert( stmnt ) assert( 0 )

#endif

#endif
