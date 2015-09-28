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

#include "SobolGeneratorSuite.h"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} SobolGeneratorSuiteData;

void SobolGeneratorSuite_Setup( SobolGeneratorSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void SobolGeneratorSuite_Teardown( SobolGeneratorSuiteData* data ) {
}

void SobolGeneratorSuite_TestSobolGenerator( SobolGeneratorSuiteData* data ) {
   int             procToWatch;
   Stream*         stream;
   SobolGenerator* sobolGenerator;
   Index           index;
   Index           sobol_I;
   int             bit_I;
   double          result;
   char            output_file[PCU_PATH_MAX];
   char            rightmostBit_file[PCU_PATH_MAX];
   char            expected_name[PCU_PATH_MAX];
   char            expected_file[PCU_PATH_MAX];

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {
      stream = Journal_Register( Info_Type, (Name)"SobolGeneratorStream" );
      Stream_RedirectFile( stream, "testSobolGeneratorRightmostBit.dat" );
   
      Journal_Printf( stream, " *********************** Testing _SobolGenerator_FindRightmostZeroBit *******************\n" );
      for ( index = 0 ; index < 30 ; index++ ) {
         for ( bit_I = sizeof( Index ) * 4 - 1 ; bit_I >= 0 ; bit_I-- )
            Journal_Printf( stream, "%u", index & 1 << bit_I ? 1 : 0 );
         Journal_Printf( stream, " number %u: %u\n", index, _SobolGenerator_FindRightmostZeroBit( index ) );
      }

      /* constructor  */
      for ( sobol_I = 0 ; sobol_I < 100 ; sobol_I++ ) {
         sprintf( output_file, "testSobolGenerator.%03u.dat", sobol_I );
         Stream_RedirectFile( stream, output_file );
         sobolGenerator = SobolGenerator_NewFromTable( output_file );

         Journal_Printf( stream," ****************** Testing SobolGenerator_GetDirectionalNumber ***************\n" );
         for ( index = 0 ; index < 30 ; index++ )
            SobolGenerator_GetDirectionalNumber( sobolGenerator, index );
   
         /* Checking up to 200000 numbers - this number is arbitary - 
         * it's only limited because we don't want file size to be huge
         * This number is intentionally over 25535 = 2^16 - 1 because there was a time when numbers repeated after this */
         for ( index = 0 ; index < 200000 ; index++ ) {
            result = SobolGenerator_GetNextNumber(sobolGenerator);
   
            assert( fabs( result - SobolGenerator_GetNumberByIndex(sobolGenerator, index)) < 1e-8 );

            /* Only dump subset of data - this output criterion is completely arbitary */
            if ( index % 773 == 3 )
               Journal_Printf( stream, "%.4g\n", result );
         }
         sprintf( expected_name, "testSobolGeneratorOutput.%03u-%03u.expected", sobolGenerator->polynomialDegree, sobolGenerator->polynomialCoefficient );

         pcu_filename_expected( "testSobolGeneratorRightmostBitOutput.expected", rightmostBit_file );
         pcu_check_fileEq( "testSobolGeneratorRightmostBit.dat", rightmostBit_file );

         pcu_filename_expected( expected_name, expected_file );
         pcu_check_fileEq( output_file, expected_file ); 

         remove( output_file );

         Stg_Class_Delete( sobolGenerator );
      }
      remove( "testSobolGeneratorRightmostBit.dat" );
   }
}

void SobolGeneratorSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, SobolGeneratorSuiteData );
   pcu_suite_setFixtures( suite, SobolGeneratorSuite_Setup, SobolGeneratorSuite_Teardown );
   pcu_suite_addTest( suite, SobolGeneratorSuite_TestSobolGenerator );
}




