/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "ConstantWeightsSuite.h"

typedef struct {
   PICelleratorContext* context;
   Stg_ComponentFactory* cf;
} ConstantWeightsSuiteData;

void constantFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   *value = -3.0;
}
void linearFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 2.0 + 2.2 * x - y;
}
void shapeFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 1 + x + y + x * y;
}
void polynomialFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 11 + 2*x*x + 3*x*x*x*y + y + x*x*x + 2*y*y;
}
void quadraticFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];

   *value = 1 + x + x * x;
}
void exponentialFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 5*exp(2*x*x*x + 2*y*y*y) * (1-x) * (1+y);
}
void exponentialInterface( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = (double) (y <= 0.1 * exp( 2*x ));
}
void circleInterface( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = (double) (x*x + y*y <= 1.0);
}

void WeightsSuite_TestElementIntegral(
   PICelleratorContext* context,
   Name                 funcName,
   Index                count,   // was SampleSize - defaults to 5000
   double               meanTolerance,
   double               stdDevTolerance,
   double               expectedMean,
   double               expectedStdDev ) 
{
   Swarm*             integrationSwarm = (Swarm*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"integrationSwarm" );
   Swarm*             materialSwarm    = (Swarm*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"materialPoints" );
   FeMesh*            mesh             = (FeMesh*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"linearMesh" );
   WeightsCalculator* weights          = (WeightsCalculator*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"weights"  );
   FeVariable*        feVariable;
   Element_LocalIndex lElement_I       = 0;
   double             analyticValue    = 0.0;
   double             integral         = 0.0;
   double             error;
   double             errorSquaredSum  = 0.0;
   double             errorSum         = 0.0;
   double             mean;
   double             standardDeviation;
   Index              loop_I;
   void*              data;
   double             differenceMean, differenceStdDev;   

   /* Create FeVariable */
   feVariable = FeVariable_New_Full(
      "feVariable", 
      (DomainContext*)context,
      mesh,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL, 
      1,
      context->dim,
      False,
      False,
      MPI_COMM_WORLD,
      context->fieldVariable_Register );

   Journal_Firewall( (funcName!=NULL), Journal_Register( Error_Type, (Name)"ConstantWeightsSuite"  ),
      "Error, function name input to %s is NULL", __func__ );

   if ( strcasecmp( funcName, "ShapeFunction" ) == 0 ) {
      feVariable->_interpolateWithinElement = shapeFunction;
      analyticValue = 4.0;
   }
   else if ( strcasecmp( funcName, "ConstantFunction" ) == 0 ) {
      feVariable->_interpolateWithinElement = constantFunction;
      analyticValue = -12.0;
   }
   else if ( strcasecmp( funcName, "LinearFunction" ) == 0 ) {
      feVariable->_interpolateWithinElement = linearFunction;
      analyticValue = 8.0;
   }
   else if ( strcasecmp( funcName, "QuadraticFunction" ) == 0 ) {
      feVariable->_interpolateWithinElement = quadraticFunction;
      analyticValue = 16.0/3.0;
   }
   else if ( strcasecmp( funcName, "PolynomialFunction" ) == 0 ) {
      feVariable->_interpolateWithinElement = polynomialFunction;
      analyticValue = 148.0/3.0;
   }
   else if ( strcasecmp( funcName, "ExponentialFunction" ) == 0 ) {
      feVariable->_interpolateWithinElement = exponentialFunction;
      analyticValue = 0.0 /*TODO*/;
      abort();
   }
   else if ( strcasecmp( funcName, "ExponentialInterface" ) == 0 ) {
      feVariable->_interpolateWithinElement = exponentialInterface;
      analyticValue = 0.05 * (exp(2) - exp(-2)) + 2.0;
   }
   else if ( strcasecmp( funcName, "CircleInterface" ) == 0 ) {
      feVariable->_interpolateWithinElement = circleInterface;
      analyticValue = M_PI;
   }
   else 
      Journal_Firewall( False,
         Journal_Register( Error_Type, (Name)"ConstantWeightsSuite"  ),
         "Cannot understand function name '%s'\n", funcName );

   for ( loop_I = 0 ; loop_I < count ; loop_I++ ) {
      Swarm_Random_Seed( (long)loop_I );
      /* Layout Particles */
      _Swarm_InitialiseParticles( materialSwarm, data );

      _IntegrationPointsSwarm_UpdateHook( NULL, integrationSwarm );
      
      WeightsCalculator_CalculateCell( weights, integrationSwarm, lElement_I );

      /* Evaluate Integral */
      integral = FeVariable_IntegrateElement( feVariable, integrationSwarm, lElement_I );

      /* Calculate Error */
      error = fabs( integral - analyticValue )/fabs( analyticValue );
      errorSum += error;
      errorSquaredSum += error*error;
   }

   /* Calculate Mean and Standard Deviation */
   mean = errorSum / (double)count;
   standardDeviation = sqrt( errorSquaredSum / (double)count - mean * mean );

   //printf( "Func: %s - Mean = %g; SD = %g\n", funcName, mean, standardDeviation );

   /* compare the mean and standard deviation */
   differenceMean = fabs(mean - expectedMean);
   differenceStdDev = fabs(standardDeviation - expectedStdDev);
   pcu_check_le( differenceMean, meanTolerance );
   pcu_check_le( differenceStdDev, stdDevTolerance );
}

void ConstantWeightsSuite_Setup( ConstantWeightsSuiteData* data ) {
   char xmlInputFilename[PCU_PATH_MAX];

   pcu_filename_input( "testConstantWeights.xml", xmlInputFilename );
   data->cf = stgMainInitFromXML( xmlInputFilename, MPI_COMM_WORLD, NULL );
   data->context = (PICelleratorContext*)LiveComponentRegister_Get( data->cf->LCRegister, (Name)"context"  );
   // Stg_ComponentFactory_BuildComponents( data->cf, NULL );
   
   stgMainBuildAndInitialise( data->cf );
}

void ConstantWeightsSuite_Teardown( ConstantWeightsSuiteData* data ) {
   stgMainDestroy( data->cf );
}

void ConstantWeightsSuite_TestConstantFunction( ConstantWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "ConstantFunction", 1000,
      1e-10, /* --mean-tolerance */
      1e-10, /* --standardDeviation-tolerance */
      2.96059e-16, /* --mean-expectedValue */
      4.98509e-23 /* --standardDeviation-expectedValue */ );
}

void ConstantWeightsSuite_TestLinearFunction( ConstantWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "LinearFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.119677, /* --mean-expectedValue */
      0.0921005 /* --standardDeviation-expectedValue */ );
}

void ConstantWeightsSuite_TestQuadraticFunction( ConstantWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "QuadraticFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0840621, /* --mean-expectedValue */
      0.0627759 /* --standardDeviation-expectedValue */ );
}

void ConstantWeightsSuite_TestPolynomialFunction( ConstantWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "PolynomialFunction", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.0174753, /* --mean-expectedValue */
      0.0137385 /* --standardDeviation-expectedValue */ );
}

void ConstantWeightsSuite_TestCircleInterface( ConstantWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "CircleInterface", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.091215, /* --mean-expectedValue */
      0.066458 /* --standardDeviation-expectedValue */ );
}
   
void ConstantWeightsSuite_TestExponentialInterface( ConstantWeightsSuiteData* data ) {
   WeightsSuite_TestElementIntegral( data->context, "ExponentialInterface", 1000,
      0.000001, /* --mean-tolerance */
      0.000001, /* --standardDeviation-tolerance */
      0.155483, /* --mean-expectedValue */
      0.116796 /* --standardDeviation-expectedValue */ );
}
   
void ConstantWeightsSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ConstantWeightsSuiteData );
   pcu_suite_setFixtures( suite, ConstantWeightsSuite_Setup, ConstantWeightsSuite_Teardown );
   pcu_suite_addTest( suite, ConstantWeightsSuite_TestConstantFunction );
   pcu_suite_addTest( suite, ConstantWeightsSuite_TestLinearFunction );
   pcu_suite_addTest( suite, ConstantWeightsSuite_TestQuadraticFunction );
   pcu_suite_addTest( suite, ConstantWeightsSuite_TestPolynomialFunction );
   pcu_suite_addTest( suite, ConstantWeightsSuite_TestExponentialInterface );
   pcu_suite_addTest( suite, ConstantWeightsSuite_TestCircleInterface );
}
