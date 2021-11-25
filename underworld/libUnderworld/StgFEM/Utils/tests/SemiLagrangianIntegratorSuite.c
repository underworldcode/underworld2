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
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include "SemiLagrangianIntegratorSuite.h"

#define CURR_MODULE_NAME "SemiLagrangianIntegratorSuite"
#define TOLERANCE 0.035

typedef struct {
} SemiLagrangianIntegratorSuiteData;

double Dt( void* _context ) {
   FiniteElementContext*	context		= (FiniteElementContext*) _context;
   double			dt;
   FeVariable*		velocityField	= (FeVariable*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"VelocityField"  );
   double			velMax;
   double			delta[3], minDelta;

   velMax = FieldVariable_GetMaxGlobalFieldMagnitude( velocityField );
   FeVariable_GetMinimumSeparation( velocityField, &minDelta, delta );

   dt = 0.5 * minDelta / velMax;

   return dt;
}

void SemiLagrangianIntegratorSuite_Line( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) {
   FiniteElementContext*	context		= (FiniteElementContext*)_context;
   Dictionary*		dictionary	= context->dictionary;
   FeVariable*		feVariable	= (FeVariable*) FieldVariable_Register_GetByName( context->fieldVariable_Register, "PhiField" );
   FeMesh*			mesh		= feVariable->feMesh;
   double*			coord		= Mesh_GetVertex( mesh, node_lI );
   double*			result		= (double*)_result;
   double			width        	= Dictionary_GetDouble_WithDefault( dictionary, (Dictionary_Entry_Key)"lineWidth", 1.0  );
   double			lineRadius	= Dictionary_GetDouble_WithDefault( dictionary, (Dictionary_Entry_Key)"lineRadius", 10.0 );
   double			radius		= sqrt( (coord[0] - 0.5)*(coord[0] - 0.5) + (coord[1] - 0.5)*(coord[1] - 0.5) );

   if( fabs( coord[0] - coord[1] ) < width && radius < lineRadius  )
      *result = 2.0;
   else
      *result = 1.0;
}

void SemiLagrangianIntegratorSuite_ShearCellX( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) {
   FiniteElementContext*	context		= (FiniteElementContext*)_context;
   FeVariable*		feVariable	= (FeVariable*) FieldVariable_Register_GetByName( context->fieldVariable_Register, "VelocityField" );
   FeMesh*			mesh		= feVariable->feMesh;
   double*			coord		= Mesh_GetVertex( mesh, node_lI );
   double*			result		= (double*)_result;

   *result = M_PI * sin( M_PI * coord[0] ) * cos( M_PI * coord[1] );
}

void SemiLagrangianIntegratorSuite_ShearCellY( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) {
   FiniteElementContext*	context		= (FiniteElementContext*)_context;
   FeVariable*		feVariable	= (FeVariable*) FieldVariable_Register_GetByName( context->fieldVariable_Register, "VelocityField" );
   FeMesh*			mesh		= feVariable->feMesh;
   double*			coord		= Mesh_GetVertex( mesh, node_lI );
   double*			result		= (double*)_result;

   *result = -M_PI * cos( M_PI * coord[0] ) * sin( M_PI * coord[1] );
}

void SemiLagrangianIntegratorSuite_UpdatePositions( void* data, FiniteElementContext* context ) {
   Index                   reverseTimeStep = Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "reverseTimeStep", 100 );
   FeVariable*		velocityField	= (FeVariable*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"VelocityField" );
   FeMesh*			mesh		= velocityField->feMesh;
   unsigned		node_I;
   Index			dim_I;
   unsigned		nDims		= Mesh_GetDimSize( mesh );
   double			velocity[3];
   double			phi;
   SemiLagrangianIntegrator*	slIntegrator;
   FeVariable*		phiField;
   FeVariable*		phiStarField;

   _FeVariable_SyncShadowValues( velocityField );

   /* reverse the numerically advected particles (& the semi lagrangian field also) */
   if( context->timeStep == reverseTimeStep + 1  ) {
      for( node_I = 0; node_I < Mesh_GetLocalSize( mesh, MT_VERTEX ); node_I++ ) {
         _FeVariable_GetValueAtNode( velocityField, node_I, velocity );

         for( dim_I = 0; dim_I < nDims; dim_I++ ) {
            velocity[dim_I] *= -1;
         }

         FeVariable_SetValueAtNode( velocityField, node_I, velocity );
      }
   }

   slIntegrator = (SemiLagrangianIntegrator*)LiveComponentRegister_Get( context->CF->LCRegister, (Name)"integrator" );
   phiField     = (FeVariable* )LiveComponentRegister_Get( context->CF->LCRegister, (Name)"PhiField" );
   phiStarField = (FeVariable* )LiveComponentRegister_Get( context->CF->LCRegister, (Name)"PhiStarField"  );
   SemiLagrangianIntegrator_Solve( slIntegrator, phiField, phiStarField );

   for( node_I = 0; node_I < Mesh_GetLocalSize( mesh, MT_VERTEX ); node_I++ ) {
      FeVariable_GetValueAtNode( phiStarField, node_I, &phi );
      FeVariable_SetValueAtNode( phiField, node_I, &phi );
   }
}

double SemiLagrangianIntegratorSuite_EvaluateError( FeVariable* phiField, FeVariable* phiOldField, Swarm* gaussSwarm ) {
   FeMesh*			feMesh		= phiField->feMesh;
   GaussParticleLayout*	particleLayout 	= (GaussParticleLayout*)gaussSwarm->particleLayout;
   Index			lElement_I, lCell_I;
   unsigned		nDims		= Mesh_GetDimSize( feMesh );
   unsigned		numMeshElements	= Mesh_GetLocalSize( feMesh, nDims );
   double			elementError;
   double			lErrorSq	= 0.0;
   double			lAnalyticSq 	= 0.0;
   double			gErrorSq, gAnalyticSq, gErrorNorm;
   IntegrationPoint*	gaussPoint;
   unsigned		gaussPoint_I, numGaussPoints;
   double			initialValue, finalValue;
   double			elErrorSq, elAnalyticSq;
   ElementType*		elementType;
   double			detJac;

   for( lElement_I = 0; lElement_I < numMeshElements; lElement_I++ ) {
      lCell_I = CellLayout_MapElementIdToCellId( gaussSwarm->cellLayout, lElement_I );
      numGaussPoints = _GaussParticleLayout_InitialCount( particleLayout, NULL, lCell_I );

      elementType = FeMesh_GetElementType( feMesh, lElement_I );

      elErrorSq = 0.0;
      elAnalyticSq = 0.0;

      for( gaussPoint_I = 0; gaussPoint_I < numGaussPoints; gaussPoint_I++ ) {
         gaussPoint = (IntegrationPoint*) Swarm_ParticleInCellAt( gaussSwarm, lCell_I, gaussPoint_I );
         FeVariable_InterpolateWithinElement( phiOldField, lElement_I, gaussPoint->xi, &initialValue );
         FeVariable_InterpolateWithinElement( phiField, lElement_I, gaussPoint->xi, &finalValue );

         detJac = ElementType_JacobianDeterminant( elementType, feMesh, lElement_I, gaussPoint->xi, nDims );

         elErrorSq += ( finalValue - initialValue ) * ( finalValue - initialValue ) * gaussPoint->weight * detJac;
         elAnalyticSq += ( initialValue * initialValue ) * gaussPoint->weight * detJac;
      }

      elementError = sqrt( elErrorSq ) / sqrt( elAnalyticSq );

      lErrorSq += elErrorSq;
      lAnalyticSq += elAnalyticSq;
   }

   MPI_Allreduce( &lErrorSq, &gErrorSq, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
   MPI_Allreduce( &lAnalyticSq, &gAnalyticSq, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );

   gErrorNorm = sqrt( gErrorSq ) / sqrt( gAnalyticSq );

   return gErrorNorm;
}

void SemiLagrangianIntegratorSuite_Setup( SemiLagrangianIntegratorSuiteData* data ) {
}

void SemiLagrangianIntegratorSuite_Teardown( SemiLagrangianIntegratorSuiteData* data ) {
}

void SemiLagrangianIntegratorSuite_Test( SemiLagrangianIntegratorSuiteData* data ) {
   Stg_ComponentFactory*	cf;
   ConditionFunction*      condFunc;
   //char			xml_input[PCU_PATH_MAX];
   double			l2Error;
   FeVariable*		phiField;
   FeVariable*		phiOldField;
   Swarm*			gaussSwarm;
   double			phi[3];
   unsigned		node_i;
   AbstractContext*	context;

   //pcu_filename_input( "testSemiLagrangianIntegrator.xml", xml_input );
   cf = stgMainInitFromXML( "StgFEM/Utils/input/testSemiLagrangianIntegrator.xml", MPI_COMM_WORLD, NULL );
   context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, True, NULL  );

   condFunc = ConditionFunction_New( SemiLagrangianIntegratorSuite_Line, (Name)"Line", NULL  );
   ConditionFunction_Register_Add( condFunc_Register, condFunc );
   condFunc = ConditionFunction_New( SemiLagrangianIntegratorSuite_ShearCellX, (Name)"ShearCellX", NULL  );
   ConditionFunction_Register_Add( condFunc_Register, condFunc );
   condFunc = ConditionFunction_New( SemiLagrangianIntegratorSuite_ShearCellY, (Name)"ShearCellY", NULL  );
   ConditionFunction_Register_Add( condFunc_Register, condFunc );

   /* manually set the timestep */
   ContextEP_ReplaceAll( context, AbstractContext_EP_Dt, Dt );
   ContextEP_Append( context, AbstractContext_EP_UpdateClass, SemiLagrangianIntegratorSuite_UpdatePositions );

   stgMainBuildAndInitialise( cf );

   phiField = (FeVariable*)LiveComponentRegister_Get( cf->LCRegister, (Name)"PhiField" );
   phiOldField = (FeVariable* )LiveComponentRegister_Get( cf->LCRegister, (Name)"PhiFieldInitial" );
   gaussSwarm = (Swarm* )LiveComponentRegister_Get( cf->LCRegister, (Name)"gaussSwarm"  );
   for( node_i = 0; node_i < Mesh_GetLocalSize( phiField->feMesh, MT_VERTEX ); node_i++ ) {
      FeVariable_GetValueAtNode( phiField, node_i, phi );
      FeVariable_SetValueAtNode( phiOldField, node_i, phi );
   }

   stgMainLoop( cf );

   l2Error = SemiLagrangianIntegratorSuite_EvaluateError( phiField, phiOldField, gaussSwarm );

   pcu_check_true( l2Error < TOLERANCE );

   stgMainDestroy( cf );
}


void SemiLagrangianIntegratorSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, SemiLagrangianIntegratorSuiteData );
   pcu_suite_setFixtures( suite, SemiLagrangianIntegratorSuite_Setup, SemiLagrangianIntegratorSuite_Teardown );
   pcu_suite_addTest( suite, SemiLagrangianIntegratorSuite_Test );
}



