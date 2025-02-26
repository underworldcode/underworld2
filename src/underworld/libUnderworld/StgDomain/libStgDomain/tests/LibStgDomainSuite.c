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
#include "StgDomain/src/StgDomain.h"

#include "LibStgDomainSuite.h"

typedef struct {
} LibStgDomainSuiteData;

void LibStgDomainSuite_Setup( LibStgDomainSuiteData* data ) {
}

void LibStgDomainSuite_Teardown( LibStgDomainSuiteData* data ) {
}

void LibStgDomainSuite_DirectoryStGermain( LibStgDomainSuiteData* data ) {
   Stg_Object* testDirectoryStGermain;

   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain" );
   pcu_check_true( testDirectoryStGermain != NULL );
}

void LibStgDomainSuite_DirectoryStgDomain( LibStgDomainSuiteData * data  ) {
   Stg_Object* testDirectoryStGermain;
   Stg_Object* testDirectoryStgDomain;

   testDirectoryStGermain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StGermain"  );
   testDirectoryStgDomain = Stg_ObjectList_Get( Project_XMLSearchPaths, (Name)"StgDomain" );

   pcu_check_true( ( strcmp((char* )LIB_DIR, (char*)testDirectoryStGermain) ) || ( testDirectoryStgDomain != NULL ) );
}

void LibStgDomainSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, LibStgDomainSuiteData );
   pcu_suite_setFixtures( suite, LibStgDomainSuite_Setup, LibStgDomainSuite_Teardown);
   pcu_suite_addTest( suite, LibStgDomainSuite_DirectoryStGermain );
   pcu_suite_addTest( suite, LibStgDomainSuite_DirectoryStgDomain );
}


