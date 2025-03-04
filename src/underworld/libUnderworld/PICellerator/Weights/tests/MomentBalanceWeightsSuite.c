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
#include "MomentBalanceWeightsSuite.h"

typedef struct {
   PICelleratorContext*  context;
   Stg_ComponentFactory* cf;
} MomentBalanceWeightsSuiteData;

void MomentBalanceWeightsSuite_Setup( MomentBalanceWeightsSuiteData* data ) {
   char xmlInputFilename[PCU_PATH_MAX];

   pcu_filename_input( "testMomentBalanceWeights.xml", xmlInputFilename );
   data->cf = stgMainInitFromXML( xmlInputFilename, MPI_COMM_WORLD, NULL );
   data->context = (PICelleratorContext*) LiveComponentRegister_Get( data->cf->LCRegister, (Name)"context" );
   stgMainBuildAndInitialise( data->cf );
} 

void MomentBalanceWeightsSuite_Teardown( MomentBalanceWeightsSuiteData* data ) {
   stgMainDestroy( data->cf );
}

void MomentBalanceWeightsSuite_TestConstantFunction( MomentBalanceWeightsSuiteData* data  ) {
   WeightsSuite_TestElementIntegral( data->context, "ConstantFunction", 1000,
      1e-10, /* --mean-tolerance */
      1e-10, /* --standardDeviation-tolerance */
      0.0, /* --mean-expectedValue */
      0.0 /* --standardDeviation-expectedValue */ );
}
void MomentBalanceWeightsSuite_TestLinearFunction ( MomentBalanceWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "LinearFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.00363688, /* --mean-expectedValue */
      0.029866 /* --standardDeviation-expectedValue */ );
}
void MomentBalanceWeightsSuite_TestQuadraticFunction ( MomentBalanceWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "QuadraticFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0446896, /* --mean-expectedValue */
      0.0351938 /* --standardDeviation-expectedValue */ );
}

void MomentBalanceWeightsSuite_TestPolynomialFunction( MomentBalanceWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "PolynomialFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0166968, /* --mean-expectedValue */
      0.0130415 /* --standardDeviation-expectedValue */ );
}

void MomentBalanceWeightsSuite_TestCircleInterface( MomentBalanceWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "CircleInterface", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0978135, /* --mean-expectedValue */
      0.0686067 /* --standardDeviation-expectedValue */ );
}

void MomentBalanceWeightsSuite_TestExponentialInterface( MomentBalanceWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "ExponentialInterface", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0946707, /* --mean-expectedValue */
      0.0752869 /* --standardDeviation-expectedValue */ );
}

void MomentBalanceWeightsSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, MomentBalanceWeightsSuiteData );
   pcu_suite_setFixtures( suite, MomentBalanceWeightsSuite_Setup, MomentBalanceWeightsSuite_Teardown );
   pcu_suite_addTest( suite, MomentBalanceWeightsSuite_TestConstantFunction );
   pcu_suite_addTest( suite, MomentBalanceWeightsSuite_TestLinearFunction );
   pcu_suite_addTest( suite, MomentBalanceWeightsSuite_TestQuadraticFunction );
   pcu_suite_addTest( suite, MomentBalanceWeightsSuite_TestPolynomialFunction );
   pcu_suite_addTest( suite, MomentBalanceWeightsSuite_TestCircleInterface );
   pcu_suite_addTest( suite, MomentBalanceWeightsSuite_TestExponentialInterface );
}
