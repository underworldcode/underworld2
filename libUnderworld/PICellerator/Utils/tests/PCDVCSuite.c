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
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
//#include "PICellerator/PopulationControl/PopulationControl.h"
//#include "PICellerator/Weights/Weights.h"
#include <PICellerator/PICellerator.h>
#include "PCDVCSuite.h"

#define CURR_MODULE_NAME "PCDVCSuite"

typedef struct {
   PICelleratorContext*  context;
   Stg_ComponentFactory* cf;
   char                  inputFile[PCU_PATH_MAX];
   Stream*               stream; 
} PCDVCSuiteData;


void PCDVCSuite_Setup( PCDVCSuiteData* data ) {
   pcu_filename_input( "testPCDVC.xml", data->inputFile );

   data->context = _PICelleratorContext_DefaultNew( "context" );
   data->cf = stgMainInitFromXML( data->inputFile, MPI_COMM_WORLD, data->context );
   data->stream = Journal_Register( Info_Type, (Name)CURR_MODULE_NAME ); 
   stgMainBuildAndInitialise( data->cf );
}

void PCDVCSuite_Teardown( PCDVCSuiteData* data ) {
   stgMainDestroy( data->cf );
}

extern void _IntegrationPointsSwarm_UpdateHook( void* timeIntegrator, void* swarm );

void ConstantFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   *value = -3.0;
}

void LinearFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 2.0 + 2.2 * x - y;
}

void ShapeFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 1 + x + y + x * y;
}

void PolynomialFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 11 + 2*x*x + 3*x*x*x*y + y + x*x*x + 2*y*y;
}

void QuadraticFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];

   *value = 1 + x + x * x;
}

void ExponentialFunction( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = 5*exp(2*x*x*x + 2*y*y*y) * (1-x) * (1+y);
}

void ExponentialInterface( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = (double) (y <= 0.1 * exp( 2*x ));
}

void CircleInterface( void* feVariable, Element_DomainIndex dElement_I, Coord xi, double* value ) {
   double x = xi[0];
   double y = xi[1];

   *value = (double) (x*x + y*y <= 1.0);
}

void compareAgainstReferenceSolution(PICelleratorContext* context, Stream* stream, double mean, double standardDeviation, char* expFile) {
   double meanTolerance, stdDevTolerance;
   double expectedMean, expectedStdDev;
   char   expectedFile[PCU_PATH_MAX];
   FILE*  expectedfp;

   pcu_filename_expected( expFile, expectedFile );
   expectedfp = fopen( expectedFile, "r" );

   fscanf( expectedfp, "%lf %lf", &meanTolerance, &expectedMean );
   pcu_check_true( fabs( expectedMean - mean ) < meanTolerance );

   fscanf( expectedfp, "%lf %lf", &stdDevTolerance, &expectedStdDev );
   pcu_check_true( fabs( expectedStdDev - standardDeviation ) < stdDevTolerance );
   
   fclose( expectedfp );
}

void PCDVCSuite_TestCircleInterface( PCDVCSuiteData* data ) {
   Swarm*             integrationSwarm = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"integrationSwarm" );
   Swarm*             materialSwarm    = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"materialPoints" );
   FeMesh*            mesh             = (FeMesh*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"linearMesh" );
   //WeightsCalculator* weights          = (WeightsCalculator*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"weights" );
   FeVariable*        feVariable;
   Element_LocalIndex lElement_I       = 0;
   double             analyticValue    = 0.0;
   double             integral         = 0.0;
   double             error;
   double             errorSquaredSum  = 0.0;
   double             errorSum         = 0.0;
   Index              loop_I;
   Index              count            = Dictionary_GetUnsignedInt_WithDefault( data->context->dictionary, "SampleSize", 5000 );
   void*              generic          = NULL;
   IntegrationPoint*  intParticle;
   GlobalParticle*    materialPoint;
   double             mean;
   double             standardDeviation;

   /* Create FeVariable */
   feVariable = FeVariable_New_Full(
      "feVariable",
      (DomainContext*)data->context,
      mesh,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL, 
      1,
      data->context->dim,
      False,
      False,
      MPI_COMM_WORLD,
      data->context->fieldVariable_Register );

   feVariable->_interpolateWithinElement = CircleInterface;
   analyticValue = M_PI;

   for ( loop_I = 0 ; loop_I < count ; loop_I++ ) {
      /* Layout Particles */
      Swarm_Random_Seed( (long)loop_I );
      _Swarm_InitialiseParticles( materialSwarm, generic );

      _IntegrationPointsSwarm_UpdateHook( NULL, integrationSwarm );

      /* The following function should not be called here as it is already called ultimately via the above function */
      /* calling this function again causes a first iteration in Lloyd's algorithm for the Voronoi cells which we don't want here */
      //WeightsCalculator_CalculateCell( weights, integrationSwarm, lElement_I );
      if( loop_I % 10 == 0 ) {
         int i;

         for( i = 0 ; i < integrationSwarm->cellParticleCountTbl[0] ; i++ ) {
            intParticle = (IntegrationPoint*)Swarm_ParticleInCellAt( integrationSwarm, 0, i );
            materialPoint = (GlobalParticle*)Swarm_ParticleInCellAt( materialSwarm, 0, i );
            /* printf("In %s M(%10.7lf %10.7lf)I(%10.7lf %10.7lf)W(%.4lf)particle layout type %s: point %d in cell %d\n",__func__,
               materialPoint->coord[0], materialPoint->coord[1], intParticle->xi[0], intParticle->xi[1], intParticle->weight, 
               materialSwarm->particleLayout->type, i, 0);
            printf("%lf %lf\n",intParticle->xi[0],intParticle->xi[1]); */
         }
      }

      /* Evaluate Integral */
      integral = FeVariable_IntegrateElement( feVariable, integrationSwarm, lElement_I );

      /* Calculate Error */
      error = fabs( ( integral - analyticValue )/( analyticValue ) );
      errorSum += error;
      errorSquaredSum += error*error;
   }

   /* Calculate Mean and Standard Deviation */
   mean = errorSum / (double)count;
   standardDeviation = sqrt( errorSquaredSum / (double)count - mean * mean );
   printf("In %s: Mean %lf SD %lf Sol %lf Count = %d\n", __func__, mean, standardDeviation, analyticValue,   count);
   Stg_Component_Destroy( feVariable, NULL, True );

   compareAgainstReferenceSolution( data->context, data->stream, mean, standardDeviation, "testPCDVC_CircleInterface.expected" );
}

void PCDVCSuite_TestPolynomialFunction( PCDVCSuiteData* data ) {
   Swarm*             integrationSwarm = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"integrationSwarm" );
   Swarm*             materialSwarm    = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"materialPoints" );
   FeMesh*            mesh             = (FeMesh*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"linearMesh" );
   //WeightsCalculator* weights          = (WeightsCalculator*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"weights" );
   FeVariable*        feVariable;
   Element_LocalIndex lElement_I       = 0;
   double             analyticValue    = 0.0;
   double             integral         = 0.0;
   double             error;
   double             errorSquaredSum  = 0.0;
   double             errorSum         = 0.0;
   Index              loop_I;
   Index              count            = Dictionary_GetUnsignedInt_WithDefault( data->context->dictionary, "SampleSize", 5000 );
   void*              generic          = NULL;
   double             mean;
   double             standardDeviation;

   /* Create FeVariable */
   feVariable = FeVariable_New_Full(
      "feVariable",
      (DomainContext*)data->context,
      mesh,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL, 
      1,
      data->context->dim,
      False,
      False,
      MPI_COMM_WORLD,
      data->context->fieldVariable_Register );

   feVariable->_interpolateWithinElement = PolynomialFunction;
   analyticValue = 148.0/3.0;

   for ( loop_I = 0 ; loop_I < count ; loop_I++ ) {
      /* Layout Particles */
      Swarm_Random_Seed( (long)loop_I );
      _Swarm_InitialiseParticles( materialSwarm, generic );

      _IntegrationPointsSwarm_UpdateHook( NULL, integrationSwarm );
      /* The following function should not be called here as it is already called ultimately via the above function */
      /* calling this function again causes a first iteration in Lloyd's algorithm for the Voronoi cells which we don't want here */
      //WeightsCalculator_CalculateCell( weights, integrationSwarm, lElement_I );

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
   printf("In %s: Mean %lf SD %lf Sol %lf\n", __func__, mean, standardDeviation, analyticValue);
   Stg_Component_Destroy( feVariable, NULL, True );

   compareAgainstReferenceSolution( data->context, data->stream, mean, standardDeviation, "testPCDVC_PolynomialFunction.expected" );
}

void PCDVCSuite_TestExponentialInterface( PCDVCSuiteData* data ) {
   Swarm*             integrationSwarm = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"integrationSwarm" );
   Swarm*             materialSwarm    = (Swarm*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"materialPoints" );
   FeMesh*            mesh             = (FeMesh*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"linearMesh" );
   //WeightsCalculator* weights          = (WeightsCalculator*)LiveComponentRegister_Get( data->context->CF->LCRegister, (Name)"weights" );
   FeVariable*        feVariable;
   Element_LocalIndex lElement_I       = 0;
   double             analyticValue    = 0.0;
   double             integral         = 0.0;
   double             error;
   double             errorSquaredSum  = 0.0;
   double             errorSum         = 0.0;
   Index              loop_I;
   Index              count            = Dictionary_GetUnsignedInt_WithDefault( data->context->dictionary, "SampleSize", 5000 );
   void*              generic          = NULL;
   double             mean;
   double             standardDeviation;

   /* Create FeVariable */
   feVariable = FeVariable_New_Full(
      "feVariable",
      (DomainContext*)data->context,
      mesh,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL, 
      1,
      data->context->dim,
      False,
      False,
      MPI_COMM_WORLD,
      data->context->fieldVariable_Register );

   feVariable->_interpolateWithinElement = ExponentialInterface;
   analyticValue = 0.05 * (exp(2) - exp(-2)) + 2.0;

   for ( loop_I = 0 ; loop_I < count ; loop_I++ ) {
      /* Layout Particles */
      Swarm_Random_Seed( (long)loop_I );
      _Swarm_InitialiseParticles( materialSwarm, generic );

      _IntegrationPointsSwarm_UpdateHook( NULL, integrationSwarm );
      /* The following function should not be called here as it is already called ultimately via the above function */
      /* calling this function again causes a first iteration in Lloyd's algorithm for the Voronoi cells which we don't want here */
      //WeightsCalculator_CalculateCell( weights, integrationSwarm, lElement_I );

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
   printf("In %s: Mean %lf SD %lf Sol %lf\n", __func__, mean, standardDeviation, analyticValue);
   Stg_Component_Destroy( feVariable, NULL, True );

   compareAgainstReferenceSolution( data->context, data->stream, mean, standardDeviation, "testPCDVC_ExponentialInterface.expected" );
}

void PCDVCSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, PCDVCSuiteData );
   pcu_suite_setFixtures( suite, PCDVCSuite_Setup, PCDVCSuite_Teardown );
   pcu_suite_addTest( suite, PCDVCSuite_TestCircleInterface );
   pcu_suite_addTest( suite, PCDVCSuite_TestPolynomialFunction );
   pcu_suite_addTest( suite, PCDVCSuite_TestExponentialInterface );
}
