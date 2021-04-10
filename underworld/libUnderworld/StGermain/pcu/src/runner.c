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
#include <mpi.h>
#include "types.h"
#include "runner.h"
#include "listener.h"
#include "suite.h"
#include "test.h"
#include "source.h"

static int pcu_nsuites;
static pcu_suite_t* pcu_suites;

/* Is global */
pcu_suite_t* pcu_cursuite;

static int pcu_nnames;
static char** pcu_names;

/* A dynamically managed array of ptrs to listeners. New listerer ptrs added by calling
 * pcu_runner_associateListener(). User-level code is responsible for creating and deleting listeners,
 * pcu_runner just manages the array */
static pcu_listener_t** pcu_lsnrs;
static int pcu_nlsnrs;
static int pcu_lsnrsarraysize;
static const int PCU_LSNRS_ARRAY_INC = 5;
 
int PCU_PRINT_DOCS=0;

void pcu_runner_searchHierarchy( pcu_suite_t* suite );

void pcu_runner_init( int argc, char* argv[] ) {
   /* Clear global values. */
   pcu_nsuites = 0;
   pcu_suites = NULL;
   pcu_cursuite = NULL;
   pcu_nlsnrs = 0;
   pcu_lsnrs = NULL;

   /* Extract suite names from the command line. */
   if ( argc > 1 && 0 == strcmp( argv[1], "--withDocs" ) ) {
      pcu_nnames = argc - 2;
      pcu_names = argv + 2;
      PCU_PRINT_DOCS = 1;
   }
   else {
      pcu_nnames = argc - 1;
      pcu_names = argv + 1;
   }

}

void pcu_runner_finalise() {
   pcu_suite_t* tmp;

   while( pcu_suites ) {
      pcu_suite_clear( pcu_suites );
      tmp = pcu_suites->next;
      free( pcu_suites );
      pcu_suites = tmp;
   }

   /* Don't delete the associated listeners ... this remains user code responsibility - just delete the array of
      ptrs */
   free ( pcu_lsnrs );   
}

PCU_Runner_Status pcu_runner_run() {
   pcu_suite_t* cur;
   unsigned int totalPasses=0; 
   unsigned int totalTests=0; 
   PCU_Runner_Status returnStatus;
   int ii=0;

   for( ii=0; ii<pcu_nlsnrs; ii++ ) {
      pcu_lsnrs[ii]->runbegin( pcu_lsnrs[ii], pcu_nsuites );
   }

   cur = pcu_suites;
   while( cur ) {
      pcu_suite_run( cur, pcu_lsnrs, pcu_nlsnrs );
      totalPasses += cur->npassed;
      totalTests += cur->ntests;
      cur = cur->next;
   }

   for( ii=0; ii<pcu_nlsnrs; ii++ ) {
      pcu_lsnrs[ii]->runend( pcu_lsnrs[ii], pcu_nsuites, totalPasses, totalTests );
   }

   if ( totalPasses == totalTests ) {
      returnStatus = PCU_RUNNER_ALLPASS;
   }
   else {
      returnStatus = PCU_RUNNER_FAILS;
   }
   
   return returnStatus;
}

void _pcu_runner_addSuite( const char* name, void (initfunc)( pcu_suite_t* ), const char* moduleDir ) {
   pcu_suite_t* suite;

   assert( initfunc );
   assert( name );
   assert( moduleDir );

   /* Setup the new suite. */
   suite = (pcu_suite_t*)malloc( sizeof(pcu_suite_t) );
   suite->name = strdup( name );
   suite->moduleDir = strdup( moduleDir );
   suite->ntests = 0;
   suite->tests = NULL;
   suite->npassed = 0;
   suite->curtest = NULL;
   suite->lsnrs = NULL;
   suite->next = NULL;
   suite->nsubsuites = 0;
   suite->subsuites = NULL;
   suite->setup = NULL;
   suite->teardown = NULL;
   suite->data = NULL;
   initfunc( suite );

   /* Don't add the suite if it's not in our list of names. */
   pcu_runner_searchHierarchy( suite );
}

void pcu_runner_associateListener( pcu_listener_t* lsnr ) {
	if (pcu_nlsnrs >= pcu_lsnrsarraysize) {
		pcu_lsnrsarraysize += PCU_LSNRS_ARRAY_INC;
		pcu_lsnrs = (pcu_listener_t**)realloc( pcu_lsnrs, sizeof(pcu_listener_t*) * pcu_lsnrsarraysize );
	}
	pcu_lsnrs[pcu_nlsnrs] = lsnr;
	pcu_nlsnrs++;
}


void pcu_runner_searchHierarchy( pcu_suite_t* suite ) {
   if( pcu_nnames ) {
      int ii;

      for( ii = 0; ii < pcu_nnames; ii++ ) {
	 if( !strcmp( pcu_names[ii], suite->name ) )
	    break;
      }
      if( ii == pcu_nnames ) {
         pcu_suite_t* cur;

         cur = suite->subsuites;
         while( cur ) {
            pcu_runner_searchHierarchy( cur );
            cur = cur->next;
         }
         pcu_suite_clear( suite );
         free( suite );
         return;
      }
   }

   /* Add to the list of current suites. */
   if( pcu_suites ) {
      pcu_suite_t* cur;

      cur = pcu_suites;
      while( cur->next )
	 cur = cur->next;
      cur->next = suite;
   }
   else
      pcu_suites = suite;
   pcu_nsuites++;
}


