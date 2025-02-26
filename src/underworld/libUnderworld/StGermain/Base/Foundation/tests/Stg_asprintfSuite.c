/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <string.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/Foundation/forwardDecl.h" /* For Journal stuff */
#include "Stg_asprintfSuite.h"

typedef struct {
} Stg_asprintfSuiteData;

void Stg_asprintfSuite_Setup( Stg_asprintfSuiteData* data ) {
}

void Stg_asprintfSuite_Teardown( Stg_asprintfSuiteData* data ) {
}

void Stg_asprintfSuite_TestPrint( Stg_asprintfSuiteData* data ) {
	const char*  fiftyBytes = "01234567890123456789012345678901234567890123456789";
	char*        testString;
	char*        testStringPtr;
   unsigned int offset=0;

	/* Stress testing Stg_asprintf beyond the default alloc number of bytes */
	Stg_asprintf( &testString, "%s%s%s%s", fiftyBytes, fiftyBytes, fiftyBytes, fiftyBytes );
	pcu_check_true( 200 == strlen( testString ) );
   for ( offset=0; offset < 200; offset+=50 ) {
      testStringPtr = testString + offset;
      pcu_check_true( 0 == strncmp( fiftyBytes, testStringPtr, 50 ) );
   }
	Memory_Free( testString );
}

void Stg_asprintfSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, Stg_asprintfSuiteData );
   pcu_suite_setFixtures( suite, Stg_asprintfSuite_Setup, Stg_asprintfSuite_Teardown );
   pcu_suite_addTest( suite, Stg_asprintfSuite_TestPrint );
}


