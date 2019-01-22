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
#include "FieldVariable.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type TimeIntegrand_Type = "TimeIntegrand";

TimeIntegrand* TimeIntegrand_New( 
		Name                                       name,
		DomainContext*                             context,
		TimeIntegrator*                            timeIntegrator, 
		StgVariable*                                  variable,
		Index                                      dataCount, 
		Stg_Component**                            data,
		Bool                                       allowFallbackToFirstOrder )
{
	TimeIntegrand*	self;

	self = (TimeIntegrand*) _TimeIntegrand_DefaultNew( name );
	_TimeIntegrand_Init( self, context, timeIntegrator, variable, dataCount, data, allowFallbackToFirstOrder );
	return self;
}

TimeIntegrand* _TimeIntegrand_New(  TIMEINTEGRAND_DEFARGS  )
{
	TimeIntegrand*	self;
	
	assert( _sizeOfSelf >= sizeof(TimeIntegrand) );
	
	/* General info */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (TimeIntegrand*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

	/* virtual info */
	self->_calculateTimeDeriv = _calculateTimeDeriv;
	self->_intermediate = _intermediate;
	
	return self;
}

void _TimeIntegrand_Init( 
		void*                                      timeIntegrand,
		DomainContext*                             context,
		TimeIntegrator*                            timeIntegrator, 
		StgVariable*                                  variable, 
		Index                                      dataCount, 
		Stg_Component**                            data,
		Bool                                       allowFallbackToFirstOrder )
{
	TimeIntegrand* self = (TimeIntegrand*)timeIntegrand;

   self->context        = context;
	self->debug          = Journal_Register( Debug_Type, (Name)self->type  );
	self->variable       = variable;
	self->dataCount      = dataCount;
	self->timeIntegrator = timeIntegrator;
	self->data           = Memory_Alloc_Array( Stg_Component*, dataCount, "data" );
	self->allowFallbackToFirstOrder = allowFallbackToFirstOrder;
	memcpy( self->data, data, dataCount * sizeof(Stg_Component*) );

    /* Create empty string. Children classes might add something useful */
	Stg_asprintf(&self->error_msg, "");

	TimeIntegrator_Add( timeIntegrator, self );
}

void _TimeIntegrand_Delete( void* timeIntegrand ) {
	TimeIntegrand* self = (TimeIntegrand*)timeIntegrand;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	_Stg_Component_Delete( self );
}

void _TimeIntegrand_Print( void* timeIntegrand, Stream* stream ) {
	TimeIntegrand* self = (TimeIntegrand*)timeIntegrand;

	/* General info */
	Journal_DPrintf( self->debug, "TimeIntegrand - '%s'\n", self->name );
	Journal_PrintPointer( stream, self );
	Stream_Indent( stream );
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* Virtual info */

	/* Regular Info */
	
	Stream_UnIndent( stream );
}

void* _TimeIntegrand_Copy( void* timeIntegrand, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	TimeIntegrand*	self = (TimeIntegrand*)timeIntegrand;
	TimeIntegrand*	newTimeIntegrand;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
	/* TODO */ abort();

	return (void*)newTimeIntegrand;
}

void* _TimeIntegrand_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                               _sizeOfSelf = sizeof(TimeIntegrand);
	Type                                                       type = TimeIntegrand_Type;
	Stg_Class_DeleteFunction*                               _delete = _TimeIntegrand_Delete;
	Stg_Class_PrintFunction*                                 _print = _TimeIntegrand_Print;
	Stg_Class_CopyFunction*                                   _copy = _TimeIntegrand_Copy;
	Stg_Component_DefaultConstructorFunction*   _defaultConstructor = _TimeIntegrand_DefaultNew;
	Stg_Component_ConstructFunction*                     _construct = _TimeIntegrand_AssignFromXML;
	Stg_Component_BuildFunction*                             _build = _TimeIntegrand_Build;
	Stg_Component_InitialiseFunction*                   _initialise = _TimeIntegrand_Initialise;
	Stg_Component_ExecuteFunction*                         _execute = _TimeIntegrand_Execute;
	Stg_Component_DestroyFunction*                         _destroy = _TimeIntegrand_Destroy;
	TimeIntegrand_CalculateTimeDerivFunction*  _calculateTimeDeriv = _TimeIntegrand_AdvectionTimeDeriv;
	TimeIntegrand_IntermediateFunction*              _intermediate = _TimeIntegrand_Intermediate;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*) _TimeIntegrand_New(  TIMEINTEGRAND_PASSARGS  );
}

void _TimeIntegrand_AssignFromXML( void* timeIntegrand, Stg_ComponentFactory* cf, void* data ) {
	TimeIntegrand*         self                    = (TimeIntegrand*)timeIntegrand;
	Index                   dataCount               = 0;
	Stg_Component**         initData                = NULL;
	StgVariable*               variable                = NULL;
	TimeIntegrator*         timeIntegrator          = NULL;
	Bool                    allowFallbackToFirstOrder = False;
	DomainContext*          context;

	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", DomainContext, False, data );
	if( !self->context  )
		context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, False, data  );
	
	variable       =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)StgVariable_Type, StgVariable, False, data  ) ;
	timeIntegrator =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)TimeIntegrator_Type, TimeIntegrator, True, data  ) ;
	initData = Stg_ComponentFactory_ConstructByList( cf, self->name, (Dictionary_Entry_Key)"data", Stg_ComponentFactory_Unlimited, Stg_Component, False, &dataCount, data  );
	allowFallbackToFirstOrder = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"allowFallbackToFirstOrder", False  );	

	_TimeIntegrand_Init( self, context, timeIntegrator, variable, dataCount, initData, allowFallbackToFirstOrder );

	if( initData != NULL )
		Memory_Free( initData );
}

void _TimeIntegrand_Build( void* timeIntegrand, void* data ) {
	TimeIntegrand* self = (TimeIntegrand*)timeIntegrand;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );

	Stg_Component_Build( self->variable, NULL, False );
}

void _TimeIntegrand_Initialise( void* timeIntegrand, void* data ) {
	TimeIntegrand* self = (TimeIntegrand*)timeIntegrand;
	
	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
}

void _TimeIntegrand_Execute( void* timeIntegrand, void* data ) {
	TimeIntegrand*	self = (TimeIntegrand*)timeIntegrand;

	Journal_DPrintf( self->debug, "In %s for %s '%s'\n", __func__, self->type, self->name );
}
	
void _TimeIntegrand_Destroy( void* timeIntegrand, void* data ) {
	TimeIntegrand*	self = (TimeIntegrand*)timeIntegrand;
   	
	Memory_Free( self->data );
    free(self->error_msg);
}

/* +++ Virtual Functions +++ */
void TimeIntegrand_FirstOrder( void* timeIntegrand, StgVariable* startValue, double dt ) {
	TimeIntegrand*	self           = (TimeIntegrand*)timeIntegrand;
	StgVariable*       variable       = self->variable;
	double*         arrayDataPtr;
	double*         startDataPtr;
	double**        timeDeriv;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;
	Bool            successFlag = False;
	Stream*         errorStream = Journal_Register( Error_Type, (Name)self->type  );

	Journal_DPrintf( self->debug, "In func %s for %s '%s'\n", __func__, self->type, self->name );

	/* Update Variables */
	StgVariable_Update( variable );
	StgVariable_Update( startValue );
	arrayCount     = variable->arraySize;

	timeDeriv = Memory_Alloc_2DArray( double, arrayCount, componentCount, (Name)"Time Deriv" );
	for( array_I = 0; array_I < arrayCount; array_I++  ) {
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv[array_I] );
                if(!successFlag) {
                   successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv[array_I] );
                }
		Journal_Firewall( True == successFlag, errorStream,
			"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
			"deriv for item %u in step %u, *failed*.\n\n%s",
			__func__, self->name, self->type, array_I, 1, self->error_msg );
	}

	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = StgVariable_GetPtrDouble( variable, array_I );
		startDataPtr = StgVariable_GetPtrDouble( startValue, array_I );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
			arrayDataPtr[ component_I ] = startDataPtr[ component_I ] + dt * timeDeriv[array_I][ component_I ];
		}
	
		TimeIntegrand_Intermediate( self, array_I );
	}

	Memory_Free( timeDeriv );
}

void TimeIntegrand_SecondOrder( void* timeIntegrand, StgVariable* startValue, double dt ) {
	TimeIntegrand*	self           = (TimeIntegrand*)timeIntegrand;
	StgVariable*       variable       = self->variable;
	double*         arrayDataPtr;
	double*         startDataPtr;
	double*         timeDeriv;
	double*         startData;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;
	double          startTime      = TimeIntegrator_GetTime( self->timeIntegrator );
	Bool            successFlag = False;
	Stream*         errorStream = Journal_Register( Error_Type, (Name)self->type  );

	timeDeriv = Memory_Alloc_Array( double, componentCount, "Time Deriv" );
	startData = Memory_Alloc_Array( double, componentCount, "StartData" );
	memset( timeDeriv, 0, componentCount * sizeof( double ) );
	memset( startData, 0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	StgVariable_Update( variable );
	StgVariable_Update( startValue );
	arrayCount     = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = StgVariable_GetPtrDouble( variable, array_I );
		startDataPtr = StgVariable_GetPtrDouble( startValue, array_I );

		TimeIntegrator_SetTime( self->timeIntegrator, startTime );

		/* Store Original Value in case startValue == self->variable */
		memcpy( startData, startDataPtr, sizeof( double ) * componentCount );

		/* Do Predictor Step */
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );
		Journal_Firewall( True == successFlag, errorStream,
			"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
			"deriv for item %u in step %u, *failed*.\n\n%s",
			__func__, self->name, self->type, array_I, 1, self->error_msg );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
			arrayDataPtr[ component_I ] = startData[ component_I ] + 0.5 * dt * timeDeriv[ component_I ];
		TimeIntegrand_Intermediate( self, array_I );

		TimeIntegrator_SetTime( self->timeIntegrator, startTime + 0.5 * dt );

		/* Do Corrector Step */
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );

		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
				arrayDataPtr[ component_I ] = startData[ component_I ] + dt * timeDeriv[ component_I ];
			
			TimeIntegrand_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n\n%s", __func__, self->name, self->type, array_I, 2, self->error_msg );
				
			_TimeIntegrand_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}
	}

	Memory_Free( timeDeriv );
	Memory_Free( startData );
}

void TimeIntegrand_FourthOrder( void* timeIntegrand, StgVariable* startValue, double dt ) {
	TimeIntegrand*	self           = (TimeIntegrand*)timeIntegrand;
	StgVariable*       variable       = self->variable;
	double*         arrayDataPtr;
	double*         startDataPtr;
	double*         timeDeriv;
	double*         finalTimeDeriv;
	double*         startData;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;
	double          startTime      = TimeIntegrator_GetTime( self->timeIntegrator );
	Bool            successFlag = False;
	Stream*         errorStream = Journal_Register( Error_Type, (Name)self->type  );

	timeDeriv      = Memory_Alloc_Array( double, componentCount, "Time Deriv" );
	startData      = Memory_Alloc_Array( double, componentCount, "StartData" );
	finalTimeDeriv = Memory_Alloc_Array( double, componentCount, "StartData" );
	memset( timeDeriv,      0, componentCount * sizeof( double ) );
	memset( startData,      0, componentCount * sizeof( double ) );
	memset( finalTimeDeriv, 0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	StgVariable_Update( variable );
	StgVariable_Update( startValue );
	arrayCount     = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = StgVariable_GetPtrDouble( variable, array_I );
		startDataPtr = StgVariable_GetPtrDouble( startValue, array_I );
		
		TimeIntegrator_SetTime( self->timeIntegrator, startTime );

		/* Store Original Value in case startValue == self->variable */
		memcpy( startData, startDataPtr, sizeof( double ) * componentCount );

		/* Do first Step - store K1 in finalTimeDeriv and update for next step */
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, finalTimeDeriv );
		Journal_Firewall( True == successFlag, errorStream,
			"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
			"deriv for item %u in step %u, *failed*.\n\n%s",
			__func__, self->name, self->type, array_I, 1, self->error_msg );

		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
			arrayDataPtr[ component_I ] = startData[ component_I ] + 0.5 * dt * finalTimeDeriv[ component_I ];
		TimeIntegrand_Intermediate( self, array_I );

		TimeIntegrator_SetTime( self->timeIntegrator, startTime + 0.5 * dt );

		/* Do Second Step - add 2xK2 value to finalTimeDeriv and update for next step */
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );
		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
				arrayDataPtr[ component_I ] = startData[ component_I ] + 0.5 * dt * timeDeriv[ component_I ];
				finalTimeDeriv[ component_I ] += 2.0 * timeDeriv[ component_I ];
			}
			TimeIntegrand_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n\n%s", __func__, self->name, self->type, array_I, 2, self->error_msg );
				
			_TimeIntegrand_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}

		/* Do Third Step - add 2xK3 value to finalTimeDeriv and update for next step */
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );
		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
				arrayDataPtr[ component_I ] = startData[ component_I ] + dt * timeDeriv[ component_I ];
				finalTimeDeriv[ component_I ] += 2.0 * timeDeriv[ component_I ];
			}
			TimeIntegrand_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n\n%s", __func__, self->name, self->type, array_I, 3, self->error_msg );
				
			_TimeIntegrand_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}

		TimeIntegrator_SetTime( self->timeIntegrator, startTime + dt );

		/* Do Fourth Step - 'K1 + 2K2 + 2K3' and K4 finalTimeDeriv to find final value */
		successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );
		if ( True == successFlag ) {
			for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
				arrayDataPtr[ component_I ] = startData[ component_I ] + 
					dt/6.0 * (timeDeriv[ component_I ] + finalTimeDeriv[ component_I ] );
			}		
			TimeIntegrand_Intermediate( self, array_I );
		}
		else {
			Journal_Firewall( True == self->allowFallbackToFirstOrder, errorStream,
				"Error - in %s(), for TimeIntegrand \"%s\" of type %s: When trying to find time "
				"deriv for item %u in step %u, *failed*, and self->allowFallbackToFirstOrder "
				"not enabled.\n\n%s", __func__, self->name, self->type, array_I, 4, self->error_msg );
				
			_TimeIntegrand_RewindToStartAndApplyFirstOrderUpdate( self,
				arrayDataPtr, startData, startTime, dt,
				timeDeriv, array_I );
		}
	}

	Memory_Free( timeDeriv );
	Memory_Free( startData );
	Memory_Free( finalTimeDeriv );
}


/* Note : this function is used to apply to just one item/particle - see the array_I parameter */
void _TimeIntegrand_RewindToStartAndApplyFirstOrderUpdate( 
		TimeIntegrand* self,
		double*         arrayDataPtr,
		double*         startData,
		double          startTime,
		double          dt,
		double*         timeDeriv,
		Index           array_I )
{
	StgVariable*       variable       = self->variable;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Bool            successFlag = False;

	/* First, go back to initial positions, so we can re-calculate the time derivative there */
	for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
		arrayDataPtr[ component_I ] = startData[ component_I ];
	}	
	TimeIntegrand_Intermediate( self, array_I );

	/* Now recalculate time deriv at start positions, then do a full dt first order update from
	 * there */
	TimeIntegrator_SetTime( self->timeIntegrator, startTime );
	successFlag = TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );
	for ( component_I = 0 ; component_I < componentCount ; component_I++ ) 
		arrayDataPtr[ component_I ] = startData[ component_I ] + dt * timeDeriv[ component_I ];
	TimeIntegrand_Intermediate( self, array_I );
}


/** +++ Sample Time Deriv Functions +++ **/


/** This function assumes that
 *            the ODE that we are solving is \dot \phi = u(x,y) 
 *            the velocity Field Variable is stored in data[0]
 *            the variable being updated is the global coordinate of the object */
Bool _TimeIntegrand_AdvectionTimeDeriv( void* timeIntegrand, Index array_I, double* timeDeriv ) {
	TimeIntegrand*	     self          = (TimeIntegrand*) timeIntegrand;
	FieldVariable*       velocityField = (FieldVariable*)  self->data[0];
	double*              coord;
	InterpolationResult  result;

	/* Get Coordinate of Object using Variable */
	coord = StgVariable_GetPtrDouble( self->variable, array_I );

	result = FieldVariable_InterpolateValueAt( velocityField, coord, timeDeriv );

	if ( result == OTHER_PROC || result == OUTSIDE_GLOBAL || isinf(timeDeriv[0]) || isinf(timeDeriv[1]) || 
			( velocityField->dim == 3 && isinf(timeDeriv[2]) ) ) 
	{
		#if 0
		Journal_Printf( Journal_Register( Error_Type, (Name)self->type  ),
			"Error in func '%s' for particle with index %u.\n\tPosition (%g, %g, %g)\n\tVelocity here is (%g, %g, %g)."
			"\n\tInterpolation result is %s.\n",
			__func__, array_I, coord[0], coord[1], coord[2], 
			timeDeriv[0], timeDeriv[1], ( velocityField->dim == 3 ? timeDeriv[2] : 0.0 ),
			InterpolationResultToStringMap[result]  );
		return False;	
		#endif
	}

	return True;
}


void _TimeIntegrand_Intermediate( void* timeIntegrand, Index array_I ) {}

/* +++ Public Functions +++ */
void TimeIntegrand_StoreTimeDeriv( void* timeIntegrand, StgVariable* timeDeriv ) {
	TimeIntegrand*	self           = (TimeIntegrand*)timeIntegrand;
	double*         arrayDataPtr;
	Index           array_I; 
	Index           arrayCount;

	/* Update Variable */
	StgVariable_Update( timeDeriv );
	arrayCount = timeDeriv->arraySize;

	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		arrayDataPtr = StgVariable_GetPtrDouble( timeDeriv, array_I );
		TimeIntegrand_CalculateTimeDeriv( self, array_I, arrayDataPtr );
	}
}

void TimeIntegrand_Add2TimesTimeDeriv( void* timeIntegrand, StgVariable* timeDerivVariable ) {
	TimeIntegrand*	self           = (TimeIntegrand*)timeIntegrand;
	StgVariable*       variable       = self->variable;
	double*         timeDerivPtr;
	double*         timeDeriv;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;

	timeDeriv = Memory_Alloc_Array( double, componentCount, "Time Deriv" );
	memset( timeDeriv,      0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	StgVariable_Update( variable );
	StgVariable_Update( timeDerivVariable );
	arrayCount = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		TimeIntegrand_CalculateTimeDeriv( self, array_I, timeDeriv );
		timeDerivPtr = StgVariable_GetPtrDouble( timeDerivVariable, array_I );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
			timeDerivPtr[ component_I ] += 2.0 * timeDeriv[ component_I ];
		}
	}

	Memory_Free( timeDeriv );
}
	

void TimeIntegrand_FourthOrderFinalStep( void* timeIntegrand, StgVariable* startData, StgVariable* timeDerivVariable, double dt ) {
	TimeIntegrand*	self           = (TimeIntegrand*)timeIntegrand;
	StgVariable*       variable       = self->variable;
	double*         k4;
	double*         k1_2k2_2k3;
	double*         startPtr;
	double*         arrayPtr;
	Index           component_I; 
	Index           componentCount = *variable->dataTypeCounts;
	Index           array_I; 
	Index           arrayCount;

	k4 = Memory_Alloc_Array( double, componentCount, "k4 Time Deriv" );
	memset( k4, 0, componentCount * sizeof( double ) );
	
	/* Update Variables */
	StgVariable_Update( variable );
	StgVariable_Update( startData );
	StgVariable_Update( timeDerivVariable );
	arrayCount     = variable->arraySize;
	
	for ( array_I = 0 ; array_I < arrayCount ; array_I++ ) {
		TimeIntegrand_CalculateTimeDeriv( self, array_I, k4 );

		k1_2k2_2k3 = StgVariable_GetPtrDouble( timeDerivVariable, array_I );
		arrayPtr = StgVariable_GetPtrDouble( variable, array_I );
		startPtr = StgVariable_GetPtrDouble( startData, array_I );
		
		for ( component_I = 0 ; component_I < componentCount ; component_I++ ) {
			arrayPtr[ component_I ] = startPtr[ component_I ] + dt/6.0 * ( k4[ component_I ] + k1_2k2_2k3[ component_I ] );
		}
		TimeIntegrand_Intermediate( self, array_I );
	}

	Memory_Free( k4 );
}




