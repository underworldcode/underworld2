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

#include "StGermain/pcu/src/pcu.h"
#include "StGermain.h"
#include "LibStGermainSuite.h"

typedef struct {
} LibStGermainSuiteData;


void LibStGermainSuite_Setup( LibStGermainSuiteData* data ) {
}

void LibStGermainSuite_Teardown( LibStGermainSuiteData* data ) {
}
   

void LibStGermainSuite_TestXMLLibraryPath( LibStGermainSuiteData* data ) {
   Stg_Object* testDirectory;
   /* Testing entries in xmlDictionary */
   testDirectory = (Stg_Object*)Stg_ObjectList_Get( Project_XMLSearchPaths,"StGermain" );
   pcu_check_true(testDirectory != NULL);
}


void LibStGermainSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, LibStGermainSuiteData );
   pcu_suite_setFixtures( suite, LibStGermainSuite_Setup, LibStGermainSuite_Teardown );
   pcu_suite_addTest( suite, LibStGermainSuite_TestXMLLibraryPath );
}


