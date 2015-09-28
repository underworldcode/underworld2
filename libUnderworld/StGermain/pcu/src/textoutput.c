/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <mpi.h>
#include "types.h"
#include "listener.h"
#include "test.h"
#include "suite.h"
#include "textoutput.h"
#include "test.h"
#include "source.h"
#include "checks.h"
#include <setjmp.h>

extern int PCU_PRINT_DOCS;
extern jmp_buf pcu_rollback_env;

typedef struct {
	int rank;
} textoutputdata_t;

void printsuitestatus( pcu_listener_t* lsnr, pcu_suite_t* suite, int final );
void printteststatus( pcu_listener_t* lsnr, pcu_suite_t* suite, char* testname, int final );
void printsources( pcu_listener_t* lsnr, pcu_suite_t* suite );
void printrunbegin( pcu_listener_t* lsnr, int nsuites );
void printrunsummary( pcu_listener_t* lsnr, int nsuites, int totalPasses, int totalTests );

void pcu_textoutput_suitebegin( pcu_listener_t* lsnr, pcu_suite_t* suite ) {
	printsuitestatus( lsnr, suite, 0 );
   /*printstatus( lsnr, suite, 0 );*/
}

void pcu_textoutput_suiteend( pcu_listener_t* lsnr, pcu_suite_t* suite ) {
   printsuitestatus( lsnr, suite, 1 );
   printsources( lsnr, suite );
}

void pcu_textoutput_testbegin( pcu_listener_t* lsnr, pcu_test_t* test ) {
}

void pcu_textoutput_testend( pcu_listener_t* lsnr, pcu_test_t* test ) {
   printteststatus( lsnr, test->suite, test->name, 0 );
}

void pcu_textoutput_checkdone( pcu_listener_t* lsnr, pcu_source_t* src ) {
	if( !src->result )
      longjmp( pcu_rollback_env, 1);
}

void pcu_textoutput_runbegin( pcu_listener_t* lsnr, int nsuites ) {
   printrunbegin( lsnr, nsuites );
}

void pcu_textoutput_runend( pcu_listener_t* lsnr, int nsuites, int totalPasses, int totalTests ) {
   printrunsummary( lsnr, nsuites, totalPasses, totalTests );
}

pcu_listener_t* pcu_textoutput_create( const char* projectName, int printdocs ) {
   pcu_listener_t* lsnr;

   lsnr = (pcu_listener_t*)malloc( sizeof(pcu_listener_t) );
   lsnr->suitebegin = pcu_textoutput_suitebegin;
   lsnr->suiteend = pcu_textoutput_suiteend;
   lsnr->testbegin = pcu_textoutput_testbegin;
   lsnr->testend = pcu_textoutput_testend;
   lsnr->checkdone = pcu_textoutput_checkdone;
   lsnr->runbegin = pcu_textoutput_runbegin;
   lsnr->runend = pcu_textoutput_runend;
   lsnr->data = malloc( sizeof(textoutputdata_t) );
   assert( projectName );
   lsnr->projectName = strdup( projectName );
   assert( printdocs == 1 || printdocs == 0 );
   lsnr->printdocs = printdocs;
   MPI_Comm_rank( MPI_COMM_WORLD, &((textoutputdata_t*)lsnr->data)->rank );

   return lsnr;
}

void pcu_textoutput_destroy( pcu_listener_t* lsnr ) {
   free( lsnr->projectName );

   if( lsnr->data )
      free( lsnr->data );
   free( lsnr );
}

void printsuitestatus( pcu_listener_t* lsnr, pcu_suite_t* suite, int final ) { 
   if( ((textoutputdata_t*)lsnr->data)->rank )
      return;

	if( final ) {
		printf( "[PCU] Status: %s\n", suite->npassed == suite->ntests ? "PASSED" : "FAILED" ); 
	}
	else {
		printf( "------------------------------------------------------------------------\n" );
		printf( "[PCU] Testing '%s':\n", suite->name );  
	}
}

void printteststatus( pcu_listener_t* lsnr, pcu_suite_t* suite, char* testname, int final ) {
   if( ((textoutputdata_t*)lsnr->data)->rank )
      return;

	printf( "[PCU]     Test Case: '%s', Passes: (%d/%d)\n", testname, suite->npassed, suite->ntests );
}

void printsources( pcu_listener_t* lsnr, pcu_suite_t* suite ) {
   pcu_test_t* test;
   pcu_source_t* src;
   int nfails;

   if( ((textoutputdata_t*)lsnr->data)->rank )
      return;

   nfails = 0;
   test = suite->tests;
   while( test ) {
      if( lsnr->printdocs ) {
         if (test->globalresult ) {
            printf( " * (P) Test %s: ", test->name );
         }
         else {
            printf( " * (F) Test %s: ", test->name );
         }
         if ( test->docString ) {
            printf( "%s\n", test->docString );
         }
         else {
            printf( "(undocumented)\n" );
         }
      }
      src = test->srcs;
      while( src ) {
	 if( !src->result ) {
	    printf( "\n\tCheck '%s' failed:\n", src->type );
	    printf( "\t\tLocation: \t%s:%d\n", src->file, src->line );
	    printf( "\t\tTest name: \t%s\n", src->test->name );
	    printf( "\t\tExpression: \t%s\n", src->expr );
            if( src->msg )
               printf( "\t\tMessage: \t%s\n", src->msg );
	    printf( "\t\tRank: \t\t%d\n", src->rank );
	    nfails++;
	 }
	 src = src->next;
      }
      test = test->next;
   }

   if( nfails )
     printf( "\n" );
}


void printrunbegin( pcu_listener_t* lsnr, int nsuites ) {
   printf( "-----------------------------------------------------------\n" );
   printf( "[PCU] Project %s, running %d test suites:\n", lsnr->projectName, nsuites );
   printf( "-----------------------------------------------------------\n" );
}

void printrunsummary( pcu_listener_t* lsnr, int nsuites, int totalPasses, int totalTests ) {
   if ( nsuites >= 1 ) {
      if( ((textoutputdata_t*)lsnr->data)->rank == 0 ) {
         printf( "-----------------------------------------------------------\n" );
         printf( "[PCU] Total Passes: (%d/%d)\n", totalPasses, totalTests );
         printf( "-----------------------------------------------------------\n" );
      }
   }
}
