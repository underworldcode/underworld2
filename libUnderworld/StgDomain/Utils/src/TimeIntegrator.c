/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <StGermain/StGermain.h>

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "types.h"
#include "DomainContext.h"
#include "TimeIntegrator.h"
#include "TimeIntegrand.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type TimeIntegrator_Type = "TimeIntegrator";

TimeIntegrator* TimeIntegrator_New( 
	Name						name,
	unsigned int			order, 
	Bool						simultaneous, 
	EntryPoint_Register*	entryPoint_Register,
	AbstractContext*		context )
{
	TimeIntegrator* self = _TimeIntegrator_DefaultNew( name );

	self->isConstructed = True;
	_TimeIntegrator_Init( self, order, simultaneous, entryPoint_Register, context );

	return self;
}

void* _TimeIntegrator_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(TimeIntegrator);
	Type                                                      type = TimeIntegrator_Type;
	Stg_Class_DeleteFunction*                              _delete = _TimeIntegrator_Delete;
	Stg_Class_PrintFunction*                                _print = _TimeIntegrator_Print;
	Stg_Class_CopyFunction*                                  _copy = _TimeIntegrator_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _TimeIntegrator_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _TimeIntegrator_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _TimeIntegrator_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _TimeIntegrator_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _TimeIntegrator_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _TimeIntegrator_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _TimeIntegrator_New(  TIMEINTEGRATOR_PASSARGS  );
}

TimeIntegrator* _TimeIntegrator_New(  TIMEINTEGRATOR_DEFARGS  )
{
	TimeIntegrator*	self;
	
	assert( _sizeOfSelf >= sizeof(TimeIntegrator) );
	
	/* General info */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (TimeIntegrator*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
    
    self->dt = 0.;
	
	return self;
}

void _TimeIntegrator_Init( 
	void*						timeIntegrator, 
	unsigned int			order, 
	Bool						simultaneous, 
	EntryPoint_Register*	entryPoint_Register,
	AbstractContext*		context )
{
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	self->context = (DomainContext*)context;
	self->debug = Journal_Register( Debug_Type, (Name)self->type  );
	self->info = Journal_Register( Info_Type, (Name)self->type );
		
	self->integrandRegister = NamedObject_Register_New( );
	self->order = order;
	self->simultaneous = simultaneous;

	/* Entry Point Stuff */
	Stg_asprintf( &self->_setupEPName, "%s-Setup", self->name );
	Stg_asprintf( &self->_finishEPName, "%s-Finish", self->name );
	self->setupEP  = EntryPoint_New( self->_setupEPName,  EntryPoint_VoidPtr_CastType );
	self->finishEP = EntryPoint_New( self->_finishEPName, EntryPoint_VoidPtr_CastType );

	if ( entryPoint_Register ) {
		EntryPoint_Register_Add( entryPoint_Register, self->setupEP );
		EntryPoint_Register_Add( entryPoint_Register, self->finishEP );
	}

	self->setupData = Stg_ObjectList_New();
	self->finishData = Stg_ObjectList_New();

	if ( context ) {
		EP_AppendClassHook( Context_GetEntryPoint( context, AbstractContext_EP_UpdateClass ), TimeIntegrator_UpdateClass, self );
	}
}

void _TimeIntegrator_Delete( void* timeIntegrator ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	Memory_Free( self->_setupEPName );
	Memory_Free( self->_finishEPName );
	
	Stg_Class_Delete( self->setupData );
	Stg_Class_Delete( self->finishData );

        // delete register
	Stg_Class_Delete( self->integrandRegister );
	
	/* Stg_Class_Delete parent*/
	_Stg_Component_Delete( self );
}

void _TimeIntegrator_Print( void* timeIntegrator, Stream* stream ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	/* General info */
	Journal_DPrintf( self->debug, "TimeIntegrator - '%s'\n", self->name );
	Journal_PrintPointer( stream, self );
	Stream_Indent( stream );
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* Virtual info */

	/* Regular Info */
	Stg_Class_Print( self->integrandRegister, stream );
	
	Stream_UnIndent( stream );
}

void* _TimeIntegrator_Copy( void* timeIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	TimeIntegrator*	newTimeIntegrator;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	/* TODO */ abort();

	return (void*)newTimeIntegrator;
}


void _TimeIntegrator_AssignFromXML( void* timeIntegrator, Stg_ComponentFactory* cf, void* data ) {
	TimeIntegrator*          self        = (TimeIntegrator*)timeIntegrator;
	unsigned int             order;
	Bool                     simultaneous;
	EntryPoint_Register*     entryPoint_Register=NULL;

	/** Default for order changed to 2nd order (was 1st order) by Pat Sunter, 10 May 2006 */
	order          = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"order", 2  );
	simultaneous   = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"simultaneous", False  );

	self->context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", DomainContext, False, data );
	if( !self->context  )	
		self->context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, False, data );

    if ( self->context ) {
        entryPoint_Register = self->context->entryPoint_Register;
        assert( entryPoint_Register  );
    }

	_TimeIntegrator_Init( self, order, simultaneous, entryPoint_Register, (AbstractContext*)self->context );
}

void _TimeIntegrator_Build( void* timeIntegrator, void* data ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

}

void _TimeIntegrator_Initialise( void* timeIntegrator, void* data ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

}

void _TimeIntegrator_Execute( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	double wallTime,tmin,tmax;


	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	wallTime = MPI_Wtime();

	/* Set function pointer */
	switch (self->order) {
		case 1:
			self->_execute = _TimeIntegrator_ExecuteEuler; 
			break;
		case 2:
			if (self->simultaneous) 
				self->_execute = _TimeIntegrator_ExecuteRK2Simultaneous; 
			else
				self->_execute = _TimeIntegrator_ExecuteRK2; 
			break;
		case 4:
			if (self->simultaneous) 
				self->_execute = _TimeIntegrator_ExecuteRK4Simultaneous; 
			else
				self->_execute = _TimeIntegrator_ExecuteRK4; 
			break;
		default:
			Journal_Firewall( False, Journal_Register( Error_Type, (Name)self->type  ),
					"%s '%s' cannot handle order %u\n", self->type, self->name, self->order );
	}

	/* Call real function */
	
	Journal_RPrintf( self->info, "Time Integration\n" );
	self->_execute( self, data );

	wallTime = MPI_Wtime()-wallTime;
        MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
	Journal_RPrintf(self->info,"Time Integration - %.6g [min] / %.6g [max] (secs)\n", tmin, tmax);
	
}

void _TimeIntegrator_Destroy( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
}

void TimeIntegrator_UpdateClass( void* timeIntegrator, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*) timeIntegrator;
	double wallTime,tmin,tmax;
	
	wallTime = MPI_Wtime();
	Journal_RPrintf(self->info,"Time Integration\n");
	self->_execute( self, data );

	wallTime = MPI_Wtime()-wallTime;
        MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
        MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
	Journal_RPrintf(self->info,"Time Integration - %.4g [min] / %.4g [max] (secs)\n", tmin,tmax);	
}

/* +++ Private Functions +++ */
void _TimeIntegrator_ExecuteEuler( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*) timeIntegrator;
	Index					integrand_I;   
	Index					integrandCount = TimeIntegrator_GetCount( self );
	double				dt = self->dt;
	TimeIntegrand*	integrand;
	double wallTime,tmin,tmax;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	
	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );
	
		wallTime = MPI_Wtime();
		TimeIntegrand_FirstOrder( integrand, integrand->variable, dt );

		wallTime = MPI_Wtime() - wallTime;
		MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
		MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
		Journal_RPrintf(self->info,"\t1st order: %35s - %9.4f [min] / %9.4f [max] (secs)\n", integrand->name, tmin,tmax);
	}
    TimeIntegrator_SetTime( self, TimeIntegrator_GetTime( self ) + dt);
	TimeIntegrator_Finalise( self );
}

void _TimeIntegrator_ExecuteRK2( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*) timeIntegrator;
	Index					integrand_I;   
	Index					integrandCount = TimeIntegrator_GetCount( self );
	double				dt = self->dt;
	TimeIntegrand*	integrand;
	double wallTime,tmin,tmax;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	
	wallTime = MPI_Wtime();

	/* Set Time */
    double startTime = TimeIntegrator_GetTime( self );
	
	TimeIntegrator_Setup( self );
	
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );
				
		TimeIntegrator_SetTime( self, startTime );
		
		wallTime = MPI_Wtime();
		TimeIntegrand_SecondOrder( integrand, integrand->variable, dt );

        	wallTime = MPI_Wtime()-wallTime;
        	MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
        	MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
		Journal_RPrintf(self->info,"\t2nd order: %35s - %9.4f [min] / %9.4f [max] (secs)\n", integrand->name, tmin, tmax);
		
	}
	
    TimeIntegrator_SetTime( self, TimeIntegrator_GetTime( self ) + 0.5*dt);

	TimeIntegrator_Finalise( self );
	
}


void _TimeIntegrator_ExecuteRK4( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*) timeIntegrator;
	Index					integrand_I;
	Index					integrandCount = TimeIntegrator_GetCount( self );
	double				dt = self->dt;
	TimeIntegrand*	integrand;
	double wallTime,tmin,tmax;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	/* Set Time */
    double startTime = TimeIntegrator_GetTime( self );

	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );

		TimeIntegrator_SetTime( self, startTime );
		wallTime = MPI_Wtime();
		TimeIntegrand_FourthOrder( integrand, integrand->variable, dt );
       	
		wallTime = MPI_Wtime()-wallTime;
        	MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
        	MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
		Journal_RPrintf(self->info,"\t4th order: %35s - %9.4f [min] / %9.4f [max] (secs)\n", integrand->name, tmin, tmax);
	}
    
	TimeIntegrator_Finalise( self );
}


void _TimeIntegrator_ExecuteRK2Simultaneous( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	AbstractContext*	context = (AbstractContext*) data;
	Index					integrand_I;   
	Index					integrandCount = TimeIntegrator_GetCount( self );
	double				dt = self->dt;
	TimeIntegrand*	integrand;
	Variable**			originalVariableList;

    assert(0); /* this shit be broken */
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	Journal_Firewall( 
		False,
		Journal_MyStream( Error_Type, self ),
		"Error in %s '%s' - This function is temporarily unavailable \n"
		"Please only use non-simultaneous update or only first order update.\n", 
		self->type, self->name );

	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime );

	originalVariableList = Memory_Alloc_Array( Variable*, integrandCount, "originalVariableList" );
	
	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );
		Journal_RPrintf(self->info,"\t2nd order (simultaneous): %s\n", integrand->name);
		
		/* Store Original */
		originalVariableList[ integrand_I ] = Variable_NewFromOld( integrand->variable, "Original", True );

		/* Predictor Step */
		TimeIntegrand_FirstOrder( integrand, integrand->variable, 0.5 * dt );
	}
	TimeIntegrator_Finalise( self );
	
	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime + 0.5 * dt );

	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );

		/* Corrector Step */
		TimeIntegrand_FirstOrder( integrand, originalVariableList[ integrand_I ], dt );

		/* Free Original */
		Stg_Class_Delete( originalVariableList[ integrand_I ] );
	}
	TimeIntegrator_Finalise( self );
	Memory_Free( originalVariableList );
}

void _TimeIntegrator_ExecuteRK4Simultaneous( void* timeIntegrator, void* data ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	AbstractContext*	context = (AbstractContext*) data;
	Index					integrand_I;   
	Index					integrandCount = TimeIntegrator_GetCount( self );
	double				dt = self->dt;
	TimeIntegrand*	integrand;
	Variable**			originalVariableList;
	Variable**			timeDerivVariableList;
    assert(0); /* this shit be broken */

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime );
	
	originalVariableList  = Memory_Alloc_Array( Variable*, integrandCount, "originalVariableList" );
	timeDerivVariableList = Memory_Alloc_Array( Variable*, integrandCount, "timeDerivVariableList" );

	/* First Step */
	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );
		Journal_RPrintf(self->info,"\t4nd order (simultaneous): %s\n", integrand->name);

		/* Store Original Position Variable */
		originalVariableList[ integrand_I ]  = Variable_NewFromOld( integrand->variable, "Original", True );
		timeDerivVariableList[ integrand_I ] = Variable_NewFromOld( integrand->variable, "k1+2k2+2k3", False );

		/* Store k1 */
		TimeIntegrand_StoreTimeDeriv( integrand, timeDerivVariableList[ integrand_I ] );

		/* 1st Step */
		TimeIntegrand_FirstOrder( integrand, integrand->variable, 0.5 * dt );
	}
	TimeIntegrator_Finalise( self );
	
	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime + 0.5 * dt );
	
	/* Second Step */
	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );

		/* Add k2 */
		TimeIntegrand_Add2TimesTimeDeriv( integrand, timeDerivVariableList[ integrand_I ] );

		TimeIntegrand_FirstOrder( integrand, originalVariableList[ integrand_I ], 0.5 * dt );
	}
	TimeIntegrator_Finalise( self );

	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );
		
		/* Add k3 */
		TimeIntegrand_Add2TimesTimeDeriv( integrand, timeDerivVariableList[ integrand_I ] );

		/* 3rd Step */
		TimeIntegrand_FirstOrder( integrand, originalVariableList[ integrand_I ], dt );
	}
	TimeIntegrator_Finalise( self );
	
	/* Set Time */
	TimeIntegrator_SetTime( self, context->currentTime + dt );
	
	TimeIntegrator_Setup( self );
	for ( integrand_I = 0 ; integrand_I < integrandCount ; integrand_I++ ) {
		integrand = TimeIntegrator_GetByIndex( self, integrand_I );

		TimeIntegrand_FourthOrderFinalStep( integrand, originalVariableList[ integrand_I ], timeDerivVariableList[ integrand_I ], dt );

		/* Free Original */
		Stg_Class_Delete( timeDerivVariableList[ integrand_I ] );
		Stg_Class_Delete( originalVariableList[ integrand_I ] );
	}
	TimeIntegrator_Finalise( self );

	Memory_Free( originalVariableList );
	Memory_Free( timeDerivVariableList );
}



/* +++ Public Functions +++ */

void TimeIntegrator_Add( void* timeIntegrator, void* _timeIntegrand ) {
	TimeIntegrator*	self           = (TimeIntegrator*) timeIntegrator;
	TimeIntegrand*	timeIntegrand = (TimeIntegrand*)_timeIntegrand;

	NamedObject_Register_Add( self->integrandRegister, timeIntegrand );
}

void TimeIntegrator_Setup( void* timeIntegrator ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	EntryPoint*            entryPoint      = self->setupEP;
	Hook_Index             hookIndex;
	double wallTime,tmin,tmax;
	
	/* Shouldn't this be using a call to a run function of the entry point class ? */ 
	for( hookIndex = 0; hookIndex < entryPoint->hooks->count; hookIndex++ ) {
		wallTime = MPI_Wtime();
		
		((EntryPoint_2VoidPtr_Cast*)((Hook*)entryPoint->hooks->data[hookIndex])->funcPtr)(
			self, Stg_ObjectList_At( self->setupData, hookIndex ) );
	
			wallTime = MPI_Wtime()-wallTime;
        	MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
        	MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
		Journal_RPrintf(self->info,"\t       EP: %35s - %9.4f [min] / %9.4f [max] (secs)\n",(entryPoint->hooks->data[hookIndex])->name,tmin,tmax);	
	}		
}

void TimeIntegrator_AppendSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Append( self->setupEP, name, funcPtr, addedBy );
	Stg_ObjectList_Append( self->setupData, data );
}

void TimeIntegrator_PrependSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator* self = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Prepend( self->setupEP, name, funcPtr, addedBy );
	Stg_ObjectList_Prepend( self->setupData, data );
}
	
void TimeIntegrator_Finalise( void* timeIntegrator ) {
	TimeIntegrator*	self = (TimeIntegrator*)timeIntegrator;
	EntryPoint*			entryPoint = self->finishEP;
	Hook_Index			hookIndex;
	double				wallTime,tmin,tmax;
	
	for( hookIndex = 0; hookIndex < entryPoint->hooks->count; hookIndex++ ) {
		wallTime = MPI_Wtime();
		
		((EntryPoint_2VoidPtr_Cast*)((Hook*)entryPoint->hooks->data[hookIndex])->funcPtr)( self, Stg_ObjectList_At( self->finishData, hookIndex ) );

   	wallTime = MPI_Wtime()-wallTime;
		MPI_Reduce( &wallTime, &tmin, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD );
		MPI_Reduce( &wallTime, &tmax, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD );
		Journal_RPrintf(self->info,"\t       EP: %35s - %9.4f (secs)\n",(entryPoint->hooks->data[hookIndex])->name,tmin,tmax);	
	}
}

void TimeIntegrator_AppendFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Append( self->finishEP, name, funcPtr, addedBy );
	Stg_ObjectList_Append( self->finishData, data );
}

void TimeIntegrator_PrependFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	EntryPoint_Prepend( self->finishEP, name, funcPtr, addedBy );
	Stg_ObjectList_Prepend( self->finishData, data );
}

void TimeIntegrator_InsertBeforeFinishEP( 
	void* timeIntegrator, 
	Name hookToInsertBefore,
	Name name, 
	Func_Ptr funcPtr, 
	char* addedBy, 
	void* data ) 
{
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	if ( Stg_ObjectList_GetIndex( self->finishEP->hooks, hookToInsertBefore ) != (unsigned int)-1 ) {
		EntryPoint_InsertBefore( self->finishEP, hookToInsertBefore, name, funcPtr, addedBy );
		Stg_ObjectList_InsertAtIndex( 
				self->finishData,
				Stg_ObjectList_GetIndex( self->finishEP->hooks, name ),
				data );
	}
	else {
		TimeIntegrator_AppendFinishEP( self, name, funcPtr, addedBy, data );
	}
}

void TimeIntegrator_InsertAfterFinishEP( 
	void* timeIntegrator, 
	Name hookToInsertAfter, 
	Name name, 
	Func_Ptr funcPtr, 
	char* addedBy, 
	void* data ) 
{
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	if ( Stg_ObjectList_GetIndex( self->finishEP->hooks, hookToInsertAfter ) != (unsigned int)-1 ) {
		EntryPoint_InsertAfter( self->finishEP, hookToInsertAfter, name, funcPtr, addedBy );
		Stg_ObjectList_InsertAtIndex(
				self->finishData,
				Stg_ObjectList_GetIndex( self->finishEP->hooks, name ),
				data );
	}
	else {
		TimeIntegrator_AppendFinishEP( self, name, funcPtr, addedBy, data );
	}
}

void TimeIntegrator_SetTime( void* timeIntegrator, double time ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;

	self->time = time;
}

double TimeIntegrator_GetTime( void* timeIntegrator ) {
	TimeIntegrator*        self            = (TimeIntegrator*)timeIntegrator;
	return self->time;
}

Variable* Variable_NewFromOld( Variable* oldVariable, Name name, Bool copyValues ) {
	Variable*         self;
	Index             array_I;
	SizeT             dataOffsets[] = { 0 };
	void*             myPtr;
	void*             oldPtr;

	Variable_Update( oldVariable );

	self = Variable_New(  
		name,
		self->context,
		1,
		dataOffsets,
		oldVariable->dataTypes,
		oldVariable->dataTypeCounts,
		NULL,
		0,
		oldVariable->arraySizePtr,
		oldVariable->arraySizeFunc,
		NULL,
		NULL );

	self->allocateSelf = True;
	self->arrayPtrPtr = &self->arrayPtr;
	if ( oldVariable->isBuilt )
		Stg_Component_Build( self, NULL, True );
	if ( oldVariable->isInitialised )
		Stg_Component_Initialise( self, NULL, True );

	assert(self->offsetCount == 1);

	if ( copyValues ) {
		for ( array_I = 0 ; array_I < self->arraySize ; array_I++ ) {
			myPtr = Variable_GetStructPtr( self, array_I );
			oldPtr = Variable_GetStructPtr( oldVariable, array_I );
			memcpy( myPtr, oldPtr, self->dataSizes[0] ); 
		}
	}

	return self;
}


