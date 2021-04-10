/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h> 
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h" 
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "DecompSuite.h"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} DecompSuiteData;

void DecompSuite_Setup( DecompSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void DecompSuite_Teardown( DecompSuiteData* data ) {
}

void DecompSuite_TestDecomp( DecompSuiteData* data ) {
   Decomp* decomp;
   int     nLocs, *locs, *ranks;
   int     l_i, g_i;

   nLocs = 10;
   locs = MemArray( int, nLocs, "testDecomp" );

   decomp = Decomp_New();
   for( l_i = 0; l_i < nLocs; l_i++ )
      locs[l_i] = data->rank * nLocs + l_i;
   pcu_check_noassert( Decomp_SetLocals( decomp, nLocs, locs ) );
   for( g_i = 0; g_i < data->nProcs * nLocs; g_i++ ) {
      if( g_i >= data->rank * nLocs && g_i < (data->rank + 1) * nLocs ) {
         pcu_check_true( IMap_Map( decomp->owners, g_i ) == data->rank );
      }
      else {
         pcu_check_true( !IMap_Has( decomp->owners, g_i ) );
      }
   }

   for( l_i = 0; l_i < nLocs; l_i++ ) {
      locs[l_i] = (data->rank * nLocs + nLocs / 2 + l_i) % (data->nProcs * nLocs);
   }
   pcu_check_noassert( Decomp_SetLocals( decomp, nLocs, locs ) );
   for( g_i = 0; g_i < data->nProcs * nLocs; g_i++ ) {
      if( g_i >= data->rank * nLocs && g_i < (data->rank + 1) * nLocs ) {
         if( g_i < data->rank * nLocs + nLocs / 2 ) {
            if( data->rank > 0 ) {
               pcu_check_true( IMap_Map( decomp->owners, g_i ) == data->rank - 1 );
            }
            else {
               pcu_check_true( IMap_Map( decomp->owners, g_i ) == data->nProcs - 1 );
            }
         }
         else {
            pcu_check_true( IMap_Map( decomp->owners, g_i ) == data->rank );
         }
      }
      else {
         pcu_check_true( !IMap_Has( decomp->owners, g_i ) );
      }
   }

   locs = MemRearray( locs, int, data->nProcs * nLocs, "testDecomp" );
   ranks = MemArray( int, data->nProcs * nLocs, "testDecomp" );
   for( g_i = 0; g_i < data->nProcs * nLocs; g_i++ )
      locs[g_i] = g_i;
   pcu_check_noassert( Decomp_FindOwners( decomp, data->nProcs * nLocs, locs, ranks ) );

   Stg_Class_Delete( decomp );
   MemFree( locs );
   MemFree( ranks );
}

void DecompSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, DecompSuiteData );
   pcu_suite_setFixtures( suite, DecompSuite_Setup, DecompSuite_Teardown );
   pcu_suite_addTest( suite, DecompSuite_TestDecomp );
}


