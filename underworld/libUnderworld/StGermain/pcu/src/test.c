/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>
#include "types.h"
#include "test.h"
#include "source.h"
#include "suite.h"
#include "listener.h"

/* Need to access this for docstring functions */
extern pcu_suite_t* pcu_cursuite;

void pcu_test_gathersources( pcu_test_t* test );


void pcu_docstring( const char* docString ) {
   assert( pcu_cursuite );
   pcu_test_doc( pcu_cursuite->curtest, docString );
}

void pcu_test_doc( pcu_test_t* test, const char* docString ) {
   assert( test );
   assert( docString );
   
   test->docString = strdup( docString );
}

void pcu_test_run( pcu_test_t* test, pcu_listener_t** lsnrs, int nlsnrs ) {
   int rank;
   int passed;
   pcu_source_t* src;
   int ii=0;

   MPI_Comm_rank( MPI_COMM_WORLD, &rank );

   /* Must have a listener. */
   assert(nlsnrs > 0);
   for (ii=0; ii<nlsnrs; ii++) {
      assert( lsnrs[ii] );
   }


   /* Begin this test. */
   for (ii=0; ii<nlsnrs; ii++) {
      lsnrs[ii]->testbegin( lsnrs[ii], test );
   }	

   /* Run the test. */
   assert( test->func );
   assert( test->suite );
   test->func( test->suite->data );

   /* Need to collect information from all ranks to
      determine if the test passed. */
   passed = 1;
   src = test->srcs;
   while( src ) {
      if( !src->result ) {
	 passed = 0;
	 break;
      }
      src = src->next;
   }
   MPI_Reduce( &passed, &test->globalresult, 1, MPI_INT, MPI_LAND, 
	       0, MPI_COMM_WORLD ); /* did anyone fail? */

   /* Update the suite's passed count. */
   if( rank == 0 && test->globalresult )
      test->suite->npassed++;

   /* Gather up all the sources onto the master rank. */
   pcu_test_gathersources( test );

   /* End the test. */
   for (ii=0; ii<nlsnrs; ii++) {
      lsnrs[ii]->testend( lsnrs[ii], test );
   }   
}

pcu_source_t* pcu_test_addSource( pcu_test_t* test, pcu_source_t* src ) {
   assert( test );
   assert( src );
   if( test->srcs ) {
      test->lastSrc->next = src;
      test->lastSrc = src;
   }
   else {
      test->srcs = src;
      test->lastSrc = src;
   }
   test->nsrcs++;

   return src;
}

void pcu_test_gathersources( pcu_test_t* test ) {
   int rank, nranks;
   int buflen;
   pcu_source_t* cur;
   void* buf;
   void* ptr;
   int totalsize;
   void* totalbuf;
   int* alllens;
   int* disps;
   int ii;

   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   MPI_Comm_size( MPI_COMM_WORLD, &nranks );

   /* Pack all our sources. */
   buflen = 0;
   cur = test->srcs;
   while( cur ) {
      buflen += pcu_source_getPackLen( cur );
      cur = cur->next;
   }
   if( buflen )
      buf = malloc( buflen );
   else
      buf = NULL;
   ptr = buf;
   while( test->srcs ) {
      pcu_source_pack( test->srcs, ptr );
      ptr += pcu_source_getPackLen( test->srcs );
      cur = test->srcs->next;
      pcu_source_clear( test->srcs );
      free( test->srcs );
      test->srcs = cur;
   }
   test->nsrcs = 0;
   test->srcs = NULL;

   /* Gather them all up. */
   totalsize = 0;
   MPI_Allreduce( &buflen, &totalsize, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD );
   if( totalsize ) {
      if( rank == 0 ) {
         totalbuf = malloc( totalsize );
         alllens = (int*)malloc( nranks * sizeof(int) );
         disps = (int*)malloc( nranks * sizeof(int) );
      }
      MPI_Gather( &buflen, 1, MPI_INT, alllens, 1, MPI_INT, 0, MPI_COMM_WORLD );
      if( rank == 0 ) {
         disps[0] = 0;
         for( ii = 1; ii < nranks; ii++ )
            disps[ii] = disps[ii - 1] + alllens[ii - 1];
      }
      MPI_Gatherv( buf, buflen, MPI_BYTE, totalbuf, alllens, disps, 
                   MPI_BYTE, 0, MPI_COMM_WORLD );
      if( buf )
         free( buf );

      if( rank == 0 ) {
         /* Free arrays. */
         if( alllens )
            free( alllens );
         if( disps )
            free( disps );

         /* Unpack sources into the list. */
         ptr = totalbuf;
         while( ptr < totalbuf + totalsize ) {
            cur = (pcu_source_t*)malloc( sizeof(pcu_source_t) );
            pcu_source_init( cur );
            pcu_source_unpack( cur, ptr );
            cur->test = test;
            ptr += pcu_source_getPackLen( cur );
            pcu_test_addSource( test, cur );
         }

         /* Free global buffer. */
         if( totalbuf )
            free( totalbuf );
      }
   }
}


