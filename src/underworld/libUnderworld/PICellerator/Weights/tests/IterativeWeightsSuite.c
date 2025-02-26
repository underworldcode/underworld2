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

#include "pcu/pcu.h"
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include "ConstantWeightsSuite.h"
#include "IterativeWeightsSuite.h"

typedef struct {
   PICelleratorContext*  context;
   Stg_ComponentFactory* cf;
} IterativeWeightsSuiteData;

void IterativeWeightsSuite_Setup( IterativeWeightsSuiteData* data ) {
   char xmlInputFilename[PCU_PATH_MAX];

   pcu_filename_input( "testIterativeWeights.xml", xmlInputFilename );
   data->cf = stgMainInitFromXML( xmlInputFilename, MPI_COMM_WORLD, NULL );
   data->context = (PICelleratorContext*)LiveComponentRegister_Get( data->cf->LCRegister, (Name)"context" );
   stgMainBuildAndInitialise( data->cf );
} 

void IterativeWeightsSuite_Teardown( IterativeWeightsSuiteData* data ) {
   stgMainDestroy( data->cf );
}

void IterativeWeightsSuite_TestConstantFunction( IterativeWeightsSuiteData* data  ) {
   WeightsSuite_TestElementIntegral( data->context, "ConstantFunction", 1000,
      1e-10, /* --mean-tolerance */
      1e-10, /* --standardDeviation-tolerance */
      0.0, /* --mean-expectedValue */
      0.0 /* --standardDeviation-expectedValue */ );
}
void IterativeWeightsSuite_TestLinearFunction ( IterativeWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "LinearFunction", 1000,
      1e-4, /* --mean-tolerance */
      1e-4, /* --standardDeviation-tolerance */
      0.0, /* --mean-expectedValue */
      0.0 /* --standardDeviation-expectedValue */ );
}
void IterativeWeightsSuite_TestQuadraticFunction ( IterativeWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "QuadraticFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0430721, /* --mean-expectedValue */
      0.0326016 /* --standardDeviation-expectedValue */ );
}

void IterativeWeightsSuite_TestPolynomialFunction( IterativeWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "PolynomialFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0175259, /* --mean-expectedValue */
      0.013522 /* --standardDeviation-expectedValue */ );
}

void IterativeWeightsSuite_TestCircleInterface( IterativeWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "CircleInterface", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.10172, /* --mean-expectedValue */
      0.070065 /* --standardDeviation-expectedValue */ );
}

void IterativeWeightsSuite_TestExponentialInterface( IterativeWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "ExponentialInterface", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.088927, /* --mean-expectedValue */
      0.06681 /* --standardDeviation-expectedValue */ );
}

void IterativeWeightsSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, IterativeWeightsSuiteData );
   pcu_suite_setFixtures( suite, IterativeWeightsSuite_Setup, IterativeWeightsSuite_Teardown );
   pcu_suite_addTest( suite, IterativeWeightsSuite_TestConstantFunction );
   pcu_suite_addTest( suite, IterativeWeightsSuite_TestLinearFunction );
   pcu_suite_addTest( suite, IterativeWeightsSuite_TestQuadraticFunction );
   pcu_suite_addTest( suite, IterativeWeightsSuite_TestPolynomialFunction );
   pcu_suite_addTest( suite, IterativeWeightsSuite_TestCircleInterface );
   pcu_suite_addTest( suite, IterativeWeightsSuite_TestExponentialInterface );
}


