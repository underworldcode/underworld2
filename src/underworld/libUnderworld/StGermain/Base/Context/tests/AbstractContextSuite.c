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
#include <math.h>

#include "StGermain/pcu/src/pcu.h"
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"
#include "StGermain/Base/Context/src/Context.h"
#include "AbstractContextSuite.h"

/* Temporarily needed until EP shortcuts are fixed */
#define  CURR_MODULE_NAME "AbstractContextSuite"

/* need to allocate memory for this stream */
Stream* stream;

#define __TestContext \
	__AbstractContext \
	unsigned int	buildHookCalled; \
	unsigned int	icHookCalled; \
	unsigned int	dtHookCalled; \
	unsigned int	solveHookCalled; \
	unsigned int	solve2HookCalled; \
	unsigned int	syncHookCalled; \
	unsigned int	outputHookCalled; \
	unsigned int	dumpHookCalled; \
	unsigned int	checkpointHookCalled; \
	double			computedValue; 
struct TestContext { __TestContext };
typedef struct TestContext TestContext;

double dt = 2.0f;
#define MAX_TIME_STEPS 1000
double GLOBAL_COMP_VALUE[MAX_TIME_STEPS];

void TestSetDt( void* context, double _dt ) {
   dt = _dt;
}

typedef struct {
   TestContext*	ctx;
   Dictionary*		dict;
} AbstractContextSuiteData;

TestContext* TestContext_New(
	Name			name,
	double		startTime,
	double		stopTime,
	MPI_Comm		communicator,
	Dictionary*	dictionary ) 
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(TestContext);
	Type                                                      type = "TestContext";
	Stg_Class_DeleteFunction*                              _delete = _AbstractContext_Delete;
	Stg_Class_PrintFunction*                                _print = _AbstractContext_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = NULL;
	Stg_Component_ConstructFunction*                    _construct = NULL;
	Stg_Component_BuildFunction*                            _build = _AbstractContext_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _AbstractContext_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _AbstractContext_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _AbstractContext_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	AbstractContext_SetDt*                                  _setDt = TestSetDt;

   TestContext* ctx;

   ctx = (TestContext*)_AbstractContext_New(  ABSTRACTCONTEXT_PASSARGS  );

   ctx->buildHookCalled = 0;  
   ctx->icHookCalled = 0;
   ctx->dtHookCalled = 0;
   ctx->solveHookCalled = 0;
   ctx->solve2HookCalled = 0;
   ctx->syncHookCalled = 0;
   ctx->outputHookCalled = 0;
   ctx->dumpHookCalled = 0;
   ctx->checkpointHookCalled = 0;
   ctx->computedValue = 0;
   
   return ctx;
}


void TestBuild( void* context ) {
   TestContext* self = (TestContext*)context;
   self->buildHookCalled++;
}

void TestInitialConditions( void* context ) {
   TestContext* self = (TestContext*)context;
   self->icHookCalled++;
   /* Since the current convention for loading from checkpoint is that there's no special entry point and
    * it should be done by the init() (possibly in data objects themselves), follow that here */   
//   self->computedValue = GLOBAL_COMP_VALUE[self->restartTimestep];
}

double TestDt( void* context ) {
   TestContext* self = (TestContext*)context;
   self->dtHookCalled++;
   return dt;
}

void TestSolve( void* context ) {
   TestContext* self = (TestContext*)context;
   self->solveHookCalled++;
   self->computedValue = pow( 1.1, self->timeStep );
}

void TestSolve2( void* context ) {
   TestContext* self = (TestContext*)context;
   self->solve2HookCalled++;
}

void TestSync( void* context ) {
   TestContext* self = (TestContext*)context;
   self->syncHookCalled++;
}

void TestOutput( void* context ) {
   TestContext* self = (TestContext*)context;
   self->outputHookCalled++;
}

void TestCheckpoint( void* context ) {
   TestContext* self = (TestContext*)context;
   self->checkpointHookCalled++;
   GLOBAL_COMP_VALUE[self->timeStep] = self->computedValue;
}

void TestDump( void* context ) {
   TestContext* self = (TestContext*)context;
   self->dumpHookCalled++;
}

void AbstractContextSuite_Setup( AbstractContextSuiteData* data ) {
   Stg_ComponentFactory*	cf;
   MPI_Comm						CommWorld;
   Index							ii;

   data->dict = Dictionary_New();

   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"outputPath", Dictionary_Entry_Value_FromString( "output" )  );
   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"checkpointEvery", Dictionary_Entry_Value_FromUnsignedInt( 5 )  );
   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"dumpEvery", Dictionary_Entry_Value_FromUnsignedInt( 2 )  );
   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"maxTimeSteps", Dictionary_Entry_Value_FromUnsignedInt( 10 )  );
   
   cf = Stg_ComponentFactory_New( data->dict, NULL );

   for (ii=0; ii < MAX_TIME_STEPS; ii++) {
      GLOBAL_COMP_VALUE[ii] = 0.0;
   }

   MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
   /* Build the context */
   data->ctx = TestContext_New( "context", 0, 0, CommWorld, data->dict );

	_AbstractContext_Init( (AbstractContext*)data->ctx );
   _AbstractContext_AssignFromXML( data->ctx, cf, NULL );

   Stream_Enable( data->ctx->info, False );
}

void AbstractContextSuite_Teardown( AbstractContextSuiteData* data ) {
	_Stg_Component_Delete( data->ctx );
   LiveComponentRegister_DestroyAll( LiveComponentRegister_GetLiveComponentRegister() );
   LiveComponentRegister_Delete();
}

void AbstractContextSuite_TestDefaultEPs( AbstractContextSuiteData* data ) {
   ContextEntryPoint*      contextEP=NULL;

   /* Assert that default EPs are set up correctly, eg for saving */
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Construct" );
   pcu_check_true( contextEP->hooks->count == 1 );
   pcu_check_streq( ((Hook*)contextEP->hooks->data[0])->name, "_AbstractContext_Construct_Hook" );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_ConstructExtensions" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Build" );
   pcu_check_true( contextEP->hooks->count == 0 );
   //pcu_check_streq( ((Hook*)contextEP->hooks->data[0])->name, "BuildAllLiveComponents" );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Initialise" );
   pcu_check_true( contextEP->hooks->count == 0 );
   //pcu_check_streq( ((Hook*)contextEP->hooks->data[0])->name, "InitialiseAllLiveComponents" );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Execute" );
   pcu_check_true( contextEP->hooks->count == 1 );
   pcu_check_streq( ((Hook*)contextEP->hooks->data[0])->name, "_AbstractContext_Execute_Hook" );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Destroy" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_DestroyExtensions" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Dt" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Step" );
   pcu_check_true( contextEP->hooks->count == 1 );
   pcu_check_streq( ((Hook*)contextEP->hooks->data[0])->name, "_AbstractContext_Step" );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Solve" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_UpdateClass" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Sync" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_FrequentOutput" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Dump" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_DumpClass" );
   pcu_check_true( contextEP->hooks->count == 0 );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_Save" );
   pcu_check_true( contextEP->hooks->count == 2 );
   pcu_check_streq( ((Hook*)contextEP->hooks->data[0])->name, "_AbstractContext_CreateCheckpointDirectory" );
   pcu_check_streq( ((Hook*)contextEP->hooks->data[1])->name, "_AbstractContext_SaveTimeInfo" );
   contextEP = (ContextEntryPoint*)AbstractContext_GetEntryPoint( data->ctx, "Context_SaveClass" );
   pcu_check_true( contextEP->hooks->count == 0 );
}


void AbstractContextSuite_TestRunBasic( AbstractContextSuiteData* data ) {
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Build, TestBuild );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Initialise, TestInitialConditions );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Solve, TestSolve ); 
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Sync, TestSync ); 
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Dt, TestDt ); 

   Stg_Component_Build( data->ctx, 0 /* dummy */, False );
   Stg_Component_Initialise( data->ctx, 0 /* dummy */, False );
   Stg_Component_Execute( data->ctx, 0 /* dummy */, False );

   pcu_check_true( data->ctx->buildHookCalled == 1 ); 
   pcu_check_true( data->ctx->icHookCalled == 1 );
   pcu_check_true( data->ctx->dtHookCalled == 10 );
   pcu_check_true( data->ctx->solveHookCalled == 11 );
   pcu_check_true( data->ctx->solve2HookCalled == 11 );
   pcu_check_true( data->ctx->syncHookCalled == 10 );
   pcu_check_true( data->ctx->outputHookCalled == 11 );
   pcu_check_true( data->ctx->dumpHookCalled == (10/Dictionary_GetUnsignedInt(data->dict, "dumpEvery" ) + 1));
   pcu_check_true( data->ctx->checkpointHookCalled == (10/Dictionary_GetUnsignedInt(data->dict, "checkpointEvery" ) + 1) );
}

void AbstractContextSuite_TestRestartFromCheckpoint( AbstractContextSuiteData* data ) {
   Stg_ComponentFactory* cf;
   MPI_Comm CommWorld;

   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Build, TestBuild );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Initialise, TestInitialConditions );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Solve, TestSolve ); 
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Dt, TestDt ); 

   Stg_Component_Build( data->ctx, 0 /* dummy */, False );
   Stg_Component_Initialise( data->ctx, 0 /* dummy */, False );
   Stg_Component_Execute( data->ctx, 0 /* dummy */, False );
   Stg_Component_Destroy( data->ctx, 0 /* dummy */, False );

   data->dict = Dictionary_New();

   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"outputPath", Dictionary_Entry_Value_FromString( "output" )  );
   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"checkpointEvery", Dictionary_Entry_Value_FromUnsignedInt( 5 )  );
   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"dumpEvery", Dictionary_Entry_Value_FromUnsignedInt( 2 )  );
   Dictionary_Add( data->dict, (Dictionary_Entry_Key)"maxTimeSteps", Dictionary_Entry_Value_FromUnsignedInt( 10 )  );

   /* ReBuild the context */
   Dictionary_Set( data->dict, (Dictionary_Entry_Key)"maxTimeSteps", Dictionary_Entry_Value_FromUnsignedInt( 20 )  );
   Dictionary_Set( data->dict, (Dictionary_Entry_Key)"restartTimestep", Dictionary_Entry_Value_FromUnsignedInt( 5 )  );
   MPI_Comm_dup( MPI_COMM_WORLD, &CommWorld );
   cf = Stg_ComponentFactory_New( data->dict, NULL );

   data->ctx = TestContext_New( "context", 0, 0, CommWorld, data->dict );

	_AbstractContext_Init( (AbstractContext*)data->ctx );
   _AbstractContext_AssignFromXML( data->ctx, cf, NULL );

   Stream_Enable( data->ctx->info, False );

   /* add hooks to existing entry points */
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Build, TestBuild );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Initialise, TestInitialConditions );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Solve, TestSolve );
   ContextEP_ReplaceAll( data->ctx, AbstractContext_EP_Dt, TestDt );

   /* Run the context for the second time */
   Stg_Component_Build( data->ctx, 0 /* dummy */, False );
   Stg_Component_Initialise( data->ctx, 0 /* dummy */, False );
   Stg_Component_Execute( data->ctx, 0 /* dummy */, False );

   /* As a fairly simple test of basic CP infrastructure, we know that the timesteps should equal
    *  run1_ts + run2_ts, and computed value should equal 1.1 to power (run1_ts + run2_ts) */  
   pcu_check_true( data->ctx->timeStep == (5 + 20) );
   pcu_check_true( abs(data->ctx->computedValue - pow( 1.1, (5 + 20) )) < 1e-8 );
}

void AbstractContextSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, AbstractContextSuiteData );
   pcu_suite_setFixtures( suite, AbstractContextSuite_Setup, AbstractContextSuite_Teardown );
   pcu_suite_addTest( suite, AbstractContextSuite_TestDefaultEPs );
   pcu_suite_addTest( suite, AbstractContextSuite_TestRunBasic );
   pcu_suite_addTest( suite, AbstractContextSuite_TestRestartFromCheckpoint );
}


