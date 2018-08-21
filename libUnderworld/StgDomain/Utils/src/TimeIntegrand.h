/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_TimeIntegrand_h__
#define __StgDomain_Utils_TimeIntegrand_h__
	
	typedef Bool (TimeIntegrand_CalculateTimeDerivFunction) ( void* timeIntegrator, Index array_I, double* timeDeriv );
	typedef void (TimeIntegrand_IntermediateFunction) ( void* timeIntegrator, Index array_I );

	extern const Type TimeIntegrand_Type;
	
	/* TimeIntegrand information */
	#define __TimeIntegrand  \
		/* General info */ \
		__Stg_Component \
		\
		DomainContext*				   context;		 \
		/* Virtual info */ \
		TimeIntegrand_CalculateTimeDerivFunction* _calculateTimeDeriv;  \
		TimeIntegrand_IntermediateFunction*       _intermediate;  \
		/* Other info */ \
		TimeIntegrator*                            timeIntegrator;       \
		StgVariable*                                  variable;             \
		Index                                      dataCount;            \
		Stg_Component**                            data;                 \
		Bool                                       allowFallbackToFirstOrder; \
		Stream*                                    debug;                \
		
	struct TimeIntegrand { __TimeIntegrand };
	
	/* Creation implementation / Virtual constructor */
	TimeIntegrand* TimeIntegrand_New( 
		Name                                   name,
		DomainContext*                         context,
		TimeIntegrator*                        timeIntegrator, 
		StgVariable*                              variable, 
		Index                                  dataCount, 
		Stg_Component**                        data,
		Bool                                   allowFallbackToFirstOrder );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TIMEINTEGRAND_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                TimeIntegrand_CalculateTimeDerivFunction*  _calculateTimeDeriv, \
                TimeIntegrand_IntermediateFunction*              _intermediate

	#define TIMEINTEGRAND_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _calculateTimeDeriv, \
	        _intermediate      

	TimeIntegrand* _TimeIntegrand_New(  TIMEINTEGRAND_DEFARGS  );

void _TimeIntegrand_Init( 
		void*                                      timeIntegrand,
		DomainContext*                             context,
		TimeIntegrator*                            timeIntegrator, 
		StgVariable*                                  variable, 
		Index                                      dataCount, 
		Stg_Component**                            data,
		Bool                                       allowFallbackToFirstOrder );
		
	/* 'Class' Virtual Functions */
	void _TimeIntegrand_Delete( void* timeIntegrator );
	void _TimeIntegrand_Print( void* timeIntegrator, Stream* stream );
	#define TimeIntegrand_Copy( self ) \
		(TimeIntegrand*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TimeIntegrand_DeepCopy( self ) \
		(TimeIntegrand*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _TimeIntegrand_Copy( void* timeIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _TimeIntegrand_DefaultNew( Name name ) ;
	void _TimeIntegrand_AssignFromXML( void* timeIntegrand, Stg_ComponentFactory* cf, void* data ) ;
	void _TimeIntegrand_Build( void* timeIntegrator, void* data );
	void _TimeIntegrand_Initialise( void* timeIntegrator, void* data );
	void _TimeIntegrand_Execute( void* timeIntegrator, void* data );
	void _TimeIntegrand_Destroy( void* timeIntegrand, void* data );

	/* +++ Virtual Functions +++ */
	#define TimeIntegrand_CalculateTimeDeriv( timeIntegrand, array_I, timeDeriv ) \
		( ((TimeIntegrand*) timeIntegrand )->_calculateTimeDeriv( timeIntegrand, array_I, timeDeriv ) )
	#define TimeIntegrand_Intermediate( timeIntegrand, array_I ) \
		( ((TimeIntegrand*) timeIntegrand )->_intermediate( timeIntegrand, array_I ) )

	/* +++ Private Functions +++ */
	Bool _TimeIntegrand_AdvectionTimeDeriv( void* timeIntegrand, Index array_I, double* timeDeriv ) ;
	void _TimeIntegrand_Intermediate( void* timeIntegrand, Index array_I );
	void _TimeIntegrand_RewindToStartAndApplyFirstOrderUpdate( 
		TimeIntegrand* self,
		double*         arrayDataPtr,
		double*         startData,
		double          startTime,
		double          dt,
		double*         timeDeriv,
		Index           array_I );

	/* +++ Public Functions +++ */
	void TimeIntegrand_FirstOrder( void* timeIntegrator, StgVariable* startValue, double dt );
	void TimeIntegrand_SecondOrder( void* timeIntegrator, StgVariable* startValue, double dt );
	void TimeIntegrand_FourthOrder( void* timeIntegrator, StgVariable* startValue, double dt );

	void TimeIntegrand_StoreTimeDeriv( void* timeIntegrand, StgVariable* timeDeriv ) ;
	void TimeIntegrand_Add2TimesTimeDeriv( void* timeIntegrand, StgVariable* timeDerivVariable ) ;
	void TimeIntegrand_FourthOrderFinalStep( void* timeIntegrand, StgVariable* startData, StgVariable* timeDerivVariable, double dt ) ;

	#define TimeIntegrand_GetTime( timeIntegrand ) \
		TimeIntegrator_GetTime( ((TimeIntegrand*) timeIntegrand)->timeIntegrator ) 

#endif 

