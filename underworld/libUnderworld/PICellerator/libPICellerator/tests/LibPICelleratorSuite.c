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
#include <StGermain/StGermain.h>
#include "StgDomain/StgDomain.h"
#include <StgFEM/StgFEM.h>

#include "PICellerator/PICellerator.h"
#include "PICellerator/Init.h"
#include "PICellerator/Finalise.h"

#include "LibPICelleratorSuite.h"

typedef struct {
} LibPICelleratorSuiteData;

void LibPICelleratorSuite_Setup( LibPICelleratorSuiteData* data ) {
}

void LibPICelleratorSuite_Teardown( LibPICelleratorSuiteData* data ) {
}

void LibPICelleratorSuite_TestDirectoryStGermain( LibPICelleratorSuiteData* data ) {
   Stg_Object* testDirectoryStGermain;
   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain" );
   pcu_check_true( testDirectoryStGermain != NULL );
}

void LibPICelleratorSuite_TestDirectoryStgFEM( LibPICelleratorSuiteData * data ) {
   Stg_Object* testDirectoryStGermain;
   Stg_Object* testDirectoryStgFEM;

   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain" );
   testDirectoryStgFEM= Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StgFEM" );

   pcu_check_true( ( strcmp((char* )LIB_DIR, (char*)testDirectoryStGermain) ) || ( testDirectoryStgFEM != NULL ) );
}

void LibPICelleratorSuite_TestDirectoryPICellerator( LibPICelleratorSuiteData * data ) {
   Stg_Object* testDirectoryStGermain;
   Stg_Object* testDirectoryPICellerator;

   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain" );
   testDirectoryPICellerator= Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"PICellerator" );

   pcu_check_true( ( strcmp((char* )LIB_DIR, (char*)testDirectoryStGermain) ) || ( testDirectoryPICellerator != NULL ) );
}

void LibPICelleratorSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, LibPICelleratorSuiteData );
   pcu_suite_setFixtures( suite, LibPICelleratorSuite_Setup, LibPICelleratorSuite_Teardown);
   pcu_suite_addTest( suite, LibPICelleratorSuite_TestDirectoryStGermain );
   pcu_suite_addTest( suite, LibPICelleratorSuite_TestDirectoryStgFEM);
   pcu_suite_addTest( suite, LibPICelleratorSuite_TestDirectoryPICellerator);
}


