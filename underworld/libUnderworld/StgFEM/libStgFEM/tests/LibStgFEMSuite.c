/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pcu/pcu.h"
#include <StGermain/libStGermain/src/StGermain.h>

#include "StgDomain/libStgDomain/src/StgDomain.h"
#include "StgFEM/StgFEM.h"

#include "LibStgFEMSuite.h"

typedef struct {
} LibStgFEMSuiteData;

void LibStgFEMSuite_Setup( LibStgFEMSuiteData* data ) {
}

void LibStgFEMSuite_Teardown( LibStgFEMSuiteData* data ) {
}

void LibStgFEMSuite_DirectoryStGermain( LibStgFEMSuiteData* data ) {
   Stg_Object* testDirectoryStGermain;
   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain" );
   pcu_check_true( testDirectoryStGermain != NULL );
}

void LibStgFEMSuite_DirectoryStgFEM( LibStgFEMSuiteData * data ) {
   Stg_Object* testDirectoryStGermain;
   Stg_Object* testDirectoryStgFEM;

   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain" );
   testDirectoryStgFEM= Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StgFEM" );

   pcu_check_true( ( strcmp((char* )LIB_DIR, (char*)testDirectoryStGermain) ) || ( testDirectoryStgFEM != NULL ) );
}

void LibStgFEMSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, LibStgFEMSuiteData );
   pcu_suite_setFixtures( suite, LibStgFEMSuite_Setup, LibStgFEMSuite_Teardown);
   pcu_suite_addTest( suite, LibStgFEMSuite_DirectoryStGermain );
   pcu_suite_addTest( suite, LibStgFEMSuite_DirectoryStgFEM);
}


