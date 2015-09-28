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
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h" 
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "TimeIntegrationSuite.h"

#define  CURR_MODULE_NAME "TimeIntegrationSuite.c"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} TimeIntegrationSuiteData;

double TimeIntegrationSuite_GetDt( void* context ) {
   return 0.1;
}

Bool TimeIntegrationSuite_ConstantTimeDeriv( void* timeIntegrand, Index array_I, double* timeDeriv ) {
   timeDeriv[0] = 2.0 * array_I;
   timeDeriv[1] = -1.0 * array_I;

   return True;
}
Bool TimeIntegrationSuite_ConstantTimeDeriv2( void* timeIntegrand, Index array_I, double* timeDeriv ) {
   timeDeriv[0] = -0.5 * array_I;
   timeDeriv[1] = 3.0 * array_I;

   return True;
}
Bool TimeIntegrationSuite_LinearTimeDeriv( void* timeIntegrand, Index array_I, double* timeDeriv ) {
   double time = TimeIntegrand_GetTime( timeIntegrand );

   timeDeriv[0] = 2.0 * array_I * time;
   timeDeriv[1] = -1.0 * array_I * time;

   return True;
}
Bool TimeIntegrationSuite_LinearTimeDeriv2( void* timeIntegrand, Index array_I, double* timeDeriv ) {
   double time = TimeIntegrand_GetTime( timeIntegrand );

   timeDeriv[0] = -0.5 * array_I * time;
   timeDeriv[1] = 3.0 * array_I * time;

   return True;
}
Bool TimeIntegrationSuite_CubicTimeDeriv( void* timeIntegrand, Index array_I, double* timeDeriv ) {
   double time = TimeIntegrand_GetTime( timeIntegrand );

   timeDeriv[0] = 2.0 * array_I * ( time * time * time - time*time );
   timeDeriv[1] = -1.0 * array_I * ( time * time * time - time*time );

   return True;
}
Bool TimeIntegrationSuite_CubicTimeDeriv2( void* timeIntegrand, Index array_I, double* timeDeriv ) {
   double time = TimeIntegrand_GetTime( timeIntegrand );

   timeDeriv[0] = -0.5 * array_I * ( time * time * time - time*time );
   timeDeriv[1] = 3.0 * array_I * ( time * time * time - time*time );

   return True;
}

TimeIntegrand_CalculateTimeDerivFunction* TimeIntegrationSuite_GetFunctionPtr( Name derivName ) {
   if ( strcasecmp( derivName, "Linear" ) == 0 )
      return TimeIntegrationSuite_LinearTimeDeriv;
   else if ( strcasecmp( derivName, "Linear2" ) == 0 )
      return TimeIntegrationSuite_LinearTimeDeriv2;
   else if ( strcasecmp( derivName, "Cubic" ) == 0 )
      return TimeIntegrationSuite_CubicTimeDeriv;
   else if ( strcasecmp( derivName, "Cubic2" ) == 0 )
      return TimeIntegrationSuite_CubicTimeDeriv2;
   else if ( strcasecmp( derivName, "Constant" ) == 0 )
      return TimeIntegrationSuite_ConstantTimeDeriv;
   else if ( strcasecmp( derivName, "Constant2" ) == 0 )
      return TimeIntegrationSuite_ConstantTimeDeriv2;
   else
      Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  ), "Don't understand DerivName '%s'\n", derivName  );

   return NULL;
}

void TimeIntegrationSuite_TestContextType( void* timeIntegrand, Stg_Class* data ) {
   assert( data->type == DomainContext_Type );
}

void TimeIntegrationSuite_TestVariableType( void* timeIntegrand, Stg_Class* data ) {
   assert( data->type == Variable_Type );
}

void TimeIntegrationSuite_Setup( TimeIntegrationSuiteData* data ) {
   Journal_Enable_AllTypedStream( False );

   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void TimeIntegrationSuite_Teardown( TimeIntegrationSuiteData* data ) {
   Journal_Enable_AllTypedStream( True );
}

void TimeIntegrationSuite_TestDriver( TimeIntegrationSuiteData* data, char *_name, char *_DerivName0, char *_DerivName1, int _order ) {
   Stg_ComponentFactory* cf;
   Stream*               stream;
   Dictionary*           dictionary;
   TimeIntegrator*       timeIntegrator;
   TimeIntegrand*        timeIntegrand;
   TimeIntegrand*        timeIntegrandList[2];
   DomainContext*        context;
   Variable*             variable;
   Variable*             variableList[2];
   double*               array;
   double*               array2;
   Index                 size0 = 11;
   Index                 size1 = 7;
   Index                 array_I;
   Index                 timestep = 0;
   Index                 maxTimesteps = 10;
   Bool                  simultaneous;
   unsigned              order;
   double                error = 0.0;
   Name                  derivName;
   double                tolerance = 0.001;
   Index                 integrand_I;
   Index                 integrandCount = 2;
   char                  expected_file[PCU_PATH_MAX];

   dictionary = Dictionary_New();
   Dictionary_Add( dictionary, (Dictionary_Entry_Key)"outputPath", Dictionary_Entry_Value_FromString("./output") );
   Dictionary_Add( dictionary, (Dictionary_Entry_Key)"DerivName0", Dictionary_Entry_Value_FromString(_DerivName0) );
   Dictionary_Add( dictionary, (Dictionary_Entry_Key)"DerivName1", Dictionary_Entry_Value_FromString(_DerivName1) );

   context = DomainContext_New( "context", 0, 0, MPI_COMM_WORLD, NULL );
   cf = stgMainConstruct( dictionary, NULL, data->comm, context );
   stgMainBuildAndInitialise( cf );
      
   ContextEP_Append( context, AbstractContext_EP_Dt, TimeIntegrationSuite_GetDt );

   /* Create Stuff */
   order = _order;
   simultaneous = False;
   variableList[0] = Variable_NewVector( "testVariable", (AbstractContext*)context, Variable_DataType_Double, 2, &size0, NULL, (void**)&array, NULL );
   variableList[1] = Variable_NewVector( "testVariable2", (AbstractContext*)context, Variable_DataType_Double, 2, &size1, NULL, (void**)&array2, NULL );
   timeIntegrator = TimeIntegrator_New( "testTimeIntegrator", order, simultaneous, NULL, NULL );
   timeIntegrator->context = context;
   timeIntegrandList[0] = TimeIntegrand_New( "testTimeIntegrand0", context, timeIntegrator, variableList[0], 0, NULL, True );
   timeIntegrandList[1] = TimeIntegrand_New( "testTimeIntegrand1", context, timeIntegrator, variableList[1], 0, NULL, True );

   Journal_Enable_AllTypedStream( True );
   stream = Journal_Register( Info_Type, (Name)"EulerStream"  );
   Stream_RedirectFile( stream, _name );

   Stream_Enable( timeIntegrator->info, False );
   derivName = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"DerivName0" );
   timeIntegrandList[0]->_calculateTimeDeriv = TimeIntegrationSuite_GetFunctionPtr( derivName  );
   Journal_Printf( stream, "DerivName0 - %s\n", derivName );
   derivName = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"DerivName1" );
   timeIntegrandList[1]->_calculateTimeDeriv = TimeIntegrationSuite_GetFunctionPtr( derivName  );
   Journal_Printf( stream, "DerivName1 - %s\n", derivName );

   /* Print Stuff to file */
   Journal_PrintValue( stream, order );
   Journal_PrintBool( stream, simultaneous );

   /* Add stuff to EPs */
   TimeIntegrator_AppendSetupEP( timeIntegrator, "start1", TimeIntegrationSuite_TestContextType, CURR_MODULE_NAME, context );
   TimeIntegrator_AppendFinishEP( timeIntegrator, "finish1", TimeIntegrationSuite_TestVariableType, CURR_MODULE_NAME, variableList[0] );
   TimeIntegrator_PrependSetupEP( timeIntegrator, "start0", TimeIntegrationSuite_TestVariableType, CURR_MODULE_NAME, variableList[0] );
   TimeIntegrator_PrependFinishEP( timeIntegrator, "finish0", TimeIntegrationSuite_TestContextType, CURR_MODULE_NAME, context );

   /* Build */
   Stg_Component_Build( variableList[0], context, False );
   Stg_Component_Build( variableList[1], context, False );
   Stg_Component_Build( timeIntegrator, context, False );
   Stg_Component_Build( timeIntegrandList[0], context, False );
   Stg_Component_Build( timeIntegrandList[1], context, False );
   array = Memory_Alloc_Array( double, 2 * size0, "name" );
   array2 = Memory_Alloc_Array( double, 2 * size1, "name" );

   /* Initialise */
   memset( array, 0, sizeof(double) * 2 * size0 );
   memset( array2, 0, sizeof(double) * 2 * size1 );
   Stg_Component_Initialise( timeIntegrator, context, False );
   Stg_Component_Initialise( variableList[0], context, False );
   Stg_Component_Initialise( variableList[1], context, False );
   Stg_Component_Initialise( timeIntegrandList[0], context, False );
   Stg_Component_Initialise( timeIntegrandList[1], context, False );

   for ( timestep = 0.0 ; timestep < maxTimesteps ; timestep ++ ) {
      Journal_Printf( stream, "Step %u - Time = %.3g\n", timestep, context->currentTime );

      Stg_Component_Execute( timeIntegrator, context, True );
      context->currentTime += AbstractContext_Dt( context );

      for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
         timeIntegrand   = timeIntegrandList[ integrand_I ];
         variable         = variableList[ integrand_I ];
         for ( array_I = 0 ; array_I < variable->arraySize ; array_I++ ) {
            if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_ConstantTimeDeriv ) {
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) - 2.0 * array_I * context->currentTime );
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) + array_I * context->currentTime );
            }
            else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_ConstantTimeDeriv2 ) {
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) + 0.5 * array_I * context->currentTime );
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) - 3 * array_I * context->currentTime );
            }
            else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_LinearTimeDeriv ) {
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) - array_I * context->currentTime * context->currentTime );
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) + 0.5 * array_I * context->currentTime * context->currentTime );
            }
            else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_LinearTimeDeriv2 ) {
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) + 0.25 * array_I * context->currentTime * context->currentTime );
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) - 1.5 * array_I * context->currentTime * context->currentTime );
            }
            else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_CubicTimeDeriv ) {
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) - 2.0 * array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0)/3.0));
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) + array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0 )/3.0));
            }
            else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_CubicTimeDeriv2 ) {
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 0 ) + 0.5 * array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0)/3.0));
               error += fabs( Variable_GetValueAtDouble( variable, array_I, 1 ) - 3.0 * array_I * ( 0.25 * pow( context->currentTime, 4.0 ) - pow( context->currentTime, 3.0 )/3.0));
            }
            else
               Journal_Firewall( 0 , Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  ), "Don't understand _calculateTimeDeriv = %p\n", timeIntegrand->_calculateTimeDeriv );
         }
      }
   }
   pcu_check_lt( error, tolerance );

   if ( error < tolerance )
      Journal_Printf( stream, "Passed\n" );
   else
      Journal_Printf( stream, "Failed - Error = %lf\n", error );
   
   Journal_Enable_AllTypedStream( False );

   if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_ConstantTimeDeriv
      || timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_ConstantTimeDeriv2 ) {
      pcu_filename_expected( "testTimeIntegrationEulerOutput.expected", expected_file );
   }
   else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_LinearTimeDeriv
      || timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_LinearTimeDeriv2 ) {
      pcu_filename_expected( "testTimeIntegrationRK2Output.expected", expected_file );
   }
   else if ( timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_CubicTimeDeriv
      || timeIntegrand->_calculateTimeDeriv == TimeIntegrationSuite_CubicTimeDeriv2 ) {
      pcu_filename_expected( "testTimeIntegrationRK4Output.expected", expected_file );
   }

   pcu_check_fileEq( _name, expected_file );

   /* Destroy stuff */
   Stream_CloseAndFreeFile( stream );
   Memory_Free( array );
   Memory_Free( array2 );
   Stg_Class_Delete( variable );
   _Stg_Component_Delete( timeIntegrator );
   _Stg_Component_Delete( timeIntegrandList[0] );
   _Stg_Component_Delete( timeIntegrandList[1] );
   remove( _name );
}
   
void TimeIntegrationSuite_TestEuler( TimeIntegrationSuiteData* data ) {
   unsigned procToWatch;

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {   
      TimeIntegrationSuite_TestDriver( data, "testIntegrationEuler", "Constant", "Constant2", 1 );
   }
}

void TimeIntegrationSuite_TestRK2( TimeIntegrationSuiteData* data ) {
   unsigned procToWatch;

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {   
      TimeIntegrationSuite_TestDriver( data, "testIntegrationRK2", "Linear", "Linear2", 2 );
   }
}

void TimeIntegrationSuite_TestRK4( TimeIntegrationSuiteData* data ) {
   unsigned procToWatch;

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch ) {   
      TimeIntegrationSuite_TestDriver( data, "testIntegrationRK4", "Cubic", "Cubic2", 4 );
   }
}

void TimeIntegrationSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, TimeIntegrationSuiteData );
   pcu_suite_setFixtures( suite, TimeIntegrationSuite_Setup, TimeIntegrationSuite_Teardown );
   pcu_suite_addTest( suite, TimeIntegrationSuite_TestEuler );
   pcu_suite_addTest( suite, TimeIntegrationSuite_TestRK2 );
   pcu_suite_addTest( suite, TimeIntegrationSuite_TestRK4 );
}


