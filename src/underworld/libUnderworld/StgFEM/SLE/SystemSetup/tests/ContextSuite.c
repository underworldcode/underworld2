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
#include "StgFEM/Discretisation/src/Discretisation.h"
#include "StgFEM/SLE/StgFEM/SLE/SystemSetup/src/SystemSetup.h"

typedef struct {
	FiniteElementContext* context;
} ContextSuiteData;

void ContextSuite_Setup( ContextSuiteData* data ) {
	Journal_Enable_AllTypedStream( False );
}

void ContextSuite_Teardown( ContextSuiteData* data ) {
	Journal_Enable_AllTypedStream( True );
}

double ContextSuite_CalcDtFunc( FiniteElementContext* context) {
	if ( context->timeStep == 0 ) {
		return 1.0;
	}
	else {
		return context->dt * 1.5; 
	}
}

void ContextSuite_TestContext( ContextSuiteData* data ) {
	/** Test Definition: */
	char							expected_file[PCU_PATH_MAX];
	Stg_ComponentFactory*	cf;
	Dictionary*					dictionary;
	Name							outputPath;
	Stream*						stream; 
	char							xml_input[PCU_PATH_MAX];

	/* read in the xml input file */
	pcu_filename_input( "testContext.xml", xml_input );
	
	cf = stgMainInitFromXML( xml_input, MPI_COMM_WORLD, NULL );
	data->context = (FiniteElementContext*)LiveComponentRegister_Get( cf->LCRegister, (Name)"context" ); 
	stgMainBuildAndInitialise(cf );

	dictionary = data->context->dictionary;
	outputPath = Dictionary_GetString( dictionary, (Dictionary_Entry_Key)"outputPath"  );

	/* Run the test  ----------------------------------------------------------------------------------------------------*/
	/* This is where we'd normally construct components if it was real main.
	* instead, we'll just set the dt function so we can test it */
	EP_AppendClassHook( data->context->calcDtEP, ContextSuite_CalcDtFunc, data->context );

	if( data->context->rank == 0 ) 
		Context_PrintConcise( data->context, data->context->verbose );

	if ( True == Dictionary_GetBool_WithDefault( dictionary, (Dictionary_Entry_Key)"showJournalStatus", False ) ) {
		Journal_PrintConcise( );	
	}	

	/* Building phase ---------------------------------------------------------------------------------------------------*/
	Stg_Component_Build( data->context, 0 /* dummy */, False );
    
	/* Initialisaton phase ----------------------------------------------------------------------------------------------*/
	Stg_Component_Initialise( data->context, 0 /* dummy */, False );
    
	/* Run (Solve) phase ------------------------------------------------------------------------------------------------*/
	data->context->maxTimeSteps = 10;
	data->context->dtFactor = 1.0;

	Journal_Enable_TypedStream( InfoStream_Type, True );
	stream = Journal_Register( Info_Type, (Name)"testContext.xml" ); 
	data->context->info = stream;  /* Redirect output to test data stream */
	Stream_RedirectFile_WithPrependedPath( stream, outputPath, "test.dat" );

	Journal_Printf( stream, "Running with no timestep braking, using  " "dt that increases 50%% each step:\n" );
	Stg_Component_Initialise( data->context, 0 /* dummy */, True );

	Stg_Component_Execute( data->context, 0 /* dummy */, False );

	Journal_Printf( stream, "\nTurning on timestep braking, at default " "level, running again:\n" );
	Stg_Component_Initialise( data->context, 0 /* dummy */, True );
	data->context->limitTimeStepIncreaseRate = True;
	Stg_Component_Execute( data->context, 0 /* dummy */, True );

	Journal_Printf( stream, "\nTurning on timestep braking, at 80%% " "level, running again - expect same as original:\n" );
	Stg_Component_Initialise( data->context, 0 /* dummy */, True );
	data->context->maxTimeStepIncreasePercentage = 80;
	Stg_Component_Execute( data->context, 0 /* dummy */, True );

	Stream_CloseAndFreeFile( stream );

	/* Compare results to expected */
	pcu_filename_expected( "testContext.expected", expected_file );
	pcu_check_fileEq( "output/test.dat", expected_file );
	remove("output/test.dat");
}

void ContextSuite( pcu_suite_t* suite ) {
	pcu_suite_setData( suite, ContextSuiteData );
	pcu_suite_setFixtures( suite, ContextSuite_Setup, ContextSuite_Teardown );
	pcu_suite_addTest( suite, ContextSuite_TestContext );
}


