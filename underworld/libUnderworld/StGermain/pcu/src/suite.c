/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "listener.h"
#include "test.h"
#include "source.h"
#include "suite.h"
#include "checks.h"
#include <setjmp.h>

extern pcu_suite_t* pcu_cursuite;

extern jmp_buf pcu_rollback_env;

void pcu_suite_run( pcu_suite_t* suite, pcu_listener_t** lsnrs, int nlsnrs ) {
   int ii=0;
   /* Must have a listener. */
   assert(nlsnrs > 0);
   for (ii=0; ii<nlsnrs; ii++) {
      assert( lsnrs[ii] );
   }

   /* Temporarily set the listener, we need this so assert
      macros can work properly. */
   assert( suite );
   suite->lsnrs = lsnrs;
   suite->npassed = 0;

   /* Run all sub-suites first. */
   if( suite->subsuites ) {
      pcu_suite_t* sub;

      sub = suite->subsuites;
      while( sub ) {
         pcu_suite_run( sub, lsnrs, nlsnrs );
         sub = sub->next;
      }
   }

   /* Set this as the active suite. */
   pcu_cursuite = suite;

   /* Begin this suite. */
   for (ii=0; ii<nlsnrs; ii++) {
      lsnrs[ii]->suitebegin( lsnrs[ii], suite );
   }   

   /* Temporarily set the current test, once again needed
      for assert macros. */
   suite->curtest = suite->tests;

   /* Loop over all the tests. */
   while( suite->curtest ) {
      /* Run the test. */
      if( suite->setup )
         suite->setup( suite->data );
			if( !setjmp(pcu_rollback_env) )
				pcu_test_run( suite->curtest, lsnrs, nlsnrs );
      if( suite->teardown )
         suite->teardown( suite->data );

      /* Move to the next test. */
      suite->curtest = suite->curtest->next;
   }

   /* End the suite. */
   for (ii=0; ii<nlsnrs; ii++) {
      lsnrs[ii]->suiteend( lsnrs[ii], suite );
   }   

   /* Clear temporary settings. */
   suite->curtest = NULL;
   suite->lsnrs = NULL;
   pcu_cursuite = NULL;
}

void _pcu_suite_setFixtures( pcu_suite_t* suite, 
			     pcu_fixture_t* setup, pcu_fixture_t* teardown )
{
   assert( suite );
   suite->setup = setup;
   suite->teardown = teardown;
}

void _pcu_suite_setData( pcu_suite_t* suite, int size ) {
   if( suite->data )
      free( suite->data );
   if( size )
      suite->data = malloc( size );
   else
      suite->data = NULL;
}

void _pcu_suite_addTest( pcu_suite_t* suite, pcu_testfunc_t* func, const char* name ) {
   pcu_test_t* test;

   /* Extract test name. */
   /* TODO */

   /* Create the new test. */
   test = (pcu_test_t*)malloc( sizeof(pcu_test_t) );
   test->name = strdup( name );
   test->suite = suite;
   test->func = func;
   test->next = NULL;
   test->nsrcs = 0;
   test->srcs = NULL;
   test->lastSrc = NULL;
   test->docString = NULL; /* Don't document when test created - user needs to call explicit function to
                              Add a document string */

   /* Add the new test. */
   if( suite->tests ) {
      pcu_test_t* cur = suite->tests;

      while( cur->next )
	 cur = cur->next;
      cur->next = test;
   }
   else
      suite->tests = test;
   suite->ntests++;
}

void _pcu_suite_addSubSuite( pcu_suite_t* suite, const char* name,
                             void (initfunc)( pcu_suite_t* ),
                             const char* moduleDir )
{
   pcu_suite_t* subsuite;

   assert( initfunc );

   /* Setup the new suite. */
   subsuite = (pcu_suite_t*)malloc( sizeof(pcu_suite_t) );
   subsuite->name = strdup( name );
   subsuite->moduleDir = strdup( moduleDir );
   subsuite->ntests = 0;
   subsuite->tests = NULL;
   subsuite->npassed = 0;
   subsuite->curtest = NULL;
   subsuite->lsnrs = NULL;
   subsuite->next = NULL;
   subsuite->nsubsuites = 0;
   subsuite->subsuites = NULL;
   subsuite->setup = NULL;
   subsuite->teardown = NULL;
   subsuite->data = NULL;
   initfunc( subsuite );

   /* Add to our list. */
   if( suite->subsuites ) {
      pcu_suite_t* cur;

      cur = suite->subsuites;
      while( cur->next )
	 cur = cur->next;
      cur->next = subsuite;
   }
   else
     suite->subsuites = subsuite;

   suite->nsubsuites++;
}

void pcu_suite_clear( pcu_suite_t* suite ) {
   pcu_test_t* tst;
   pcu_source_t* src;
   pcu_suite_t* sub;

   free( suite->name );
   free( suite->moduleDir );

   while( suite->tests ) {
      while( suite->tests->srcs ) {
         src = suite->tests->srcs->next;
         pcu_source_clear( suite->tests->srcs );
         free( suite->tests->srcs );
         suite->tests->srcs = src;
      }
      free( suite->tests->name );
      if ( suite->tests->docString ) {
         free( suite->tests->docString );
      }

      tst = suite->tests->next;
      free( suite->tests );
      suite->tests = tst;
   }
   suite->ntests = 0;
   suite->curtest = NULL;

   while( suite->subsuites ) {
      pcu_suite_clear( suite->subsuites );
      sub = suite->subsuites->next;
      free( suite->subsuites );
      suite->subsuites = sub;
   }

   if( suite->data ) {
      free( suite->data );
      suite->data = NULL;
   }
}



