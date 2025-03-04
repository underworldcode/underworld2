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
#include <string.h>
#include <mpi.h>
#include <sys/stat.h>
#include <unistd.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "PathUtilsSuite.h"

typedef struct {
   int rank;
} PathUtilsSuiteData;

void PathUtilsSuite_Setup( PathUtilsSuiteData* data ) {
   MPI_Comm_rank( MPI_COMM_WORLD, &data->rank );
}

void PathUtilsSuite_Teardown( PathUtilsSuiteData* data ) {
}

void PathUtilsSuite_TestPathJoin( PathUtilsSuiteData* data ) {
   char     searchPaths[1024];
   unsigned len;
   
   PathJoin( searchPaths, 1, "." );
   len = strlen( searchPaths );
   searchPaths[len] = ':';
   
   PathJoin( &searchPaths[len + 1], 2,  ".", "data" );
   len = strlen( searchPaths );
   searchPaths[len] = ':';
   
   PathJoin( &searchPaths[len + 1], 6, "..", "..", "..", "does", "not", "exist" );
   
   pcu_check_streq( searchPaths, ".:./data:../../../does/not/exist" );
}

void PathUtilsSuite_TestFindFile( PathUtilsSuiteData* data ) {
   char*       searchPaths = NULL;
   char        fullPath[1024];
   const char* subDir = "./testSubDir";
   const char* subDirFilename = "./testSubDir/subDirTest.xml";
   const char* currDirFilename = "./currDirTest.xml";


   Stg_asprintf( &searchPaths, ".:%s:/does/not/exist", subDir );
   /* Create necessary test files/dirs */
   if (data->rank==0) {
      FILE* subDirFile = NULL;
      FILE* currDirFile = NULL;

      currDirFile = fopen( currDirFilename, "w" );
      fputs( "test.\n", currDirFile );
      fclose( currDirFile );
      mkdir( subDir, 0755 );
      subDirFile = fopen( subDirFilename, "w" );
      fputs( "test.\n", subDirFile );
      fclose( subDirFile );
   }
   MPI_Barrier(MPI_COMM_WORLD);

   /* try and open some files using the search path */
   /* Only do this using proc 0 - for why, see warning in Doxygen comment for the function. */
   if (data->rank==0) {
      /* This first test is to make sure it can handle files preceded with ./ */
      FindFile( fullPath, currDirFilename, searchPaths );
      pcu_check_streq( fullPath, currDirFilename );

      FindFile( fullPath, "currDirTest.xml", searchPaths );
      pcu_check_streq( fullPath, currDirFilename );
      
      FindFile( fullPath, "subDirTest.xml", searchPaths );
      pcu_check_streq( fullPath, subDirFilename );
      
      FindFile( fullPath, "nofile.man", searchPaths );
      pcu_check_streq( fullPath, "" );
      
      FindFile( fullPath, "/Users/luke/Projects/StGermain/env_vars", searchPaths );
      pcu_check_streq( fullPath, "" );
   }

   if (data->rank==0) {
      remove( currDirFilename );
      remove( subDirFilename );
      rmdir( subDir );
   }
}

void PathUtilsSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, PathUtilsSuiteData );
   pcu_suite_setFixtures( suite, PathUtilsSuite_Setup, PathUtilsSuite_Teardown );
   pcu_suite_addTest( suite, PathUtilsSuite_TestPathJoin );
   pcu_suite_addTest( suite, PathUtilsSuite_TestFindFile );
}


