/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_TimeIntegrator_h__
#define __StgDomain_Utils_TimeIntegrator_h__
	
	/* typedefs for virtual functions: */
	extern const Type TimeIntegrator_Type;
	
	/* TimeIntegrator information */
	#define __TimeIntegrator  \
		/* General info */ \
		__Stg_Component \
		\
		DomainContext*				context; \
		Stream*						debug; \
		Stream*						info; \
		NamedObject_Register*	integrandRegister; \
		unsigned int				order; \
		Bool							simultaneous; \
		Name							_setupEPName; \
		Name							_finishEPName; \
		EntryPoint*					setupEP; \
		Stg_ObjectList*			setupData; \
		EntryPoint*					finishEP; \
		Stg_ObjectList*			finishData; \
		double						time; \
		double						dt;

	struct TimeIntegrator { __TimeIntegrator };
	
	/* Creation implementation / Virtual constructor */
	TimeIntegrator* TimeIntegrator_New( 
		Name							name,
		unsigned int				order, 
		Bool							simultaneous, 
		EntryPoint_Register*		entryPoint_Register,
		AbstractContext*			context );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TIMEINTEGRATOR_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define TIMEINTEGRATOR_PASSARGS \
                STG_COMPONENT_PASSARGS

	TimeIntegrator* _TimeIntegrator_New(  TIMEINTEGRATOR_DEFARGS  );
		
	void _TimeIntegrator_Init(	
		void*                                      timeIntegrator, 
		unsigned int                               order, 
		Bool                                       simultaneous, 
		EntryPoint_Register*                       entryPoint_Register,
		AbstractContext*                           context );

	/* 'Class' Virtual Functions */
	void _TimeIntegrator_Delete( void* timeIntegrator );
	void _TimeIntegrator_Print( void* timeIntegrator, Stream* stream );
	#define TimeIntegrator_Copy( self ) \
		(TimeIntegrator*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TimeIntegrator_DeepCopy( self ) \
		(TimeIntegrator*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _TimeIntegrator_Copy( void* timeIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _TimeIntegrator_DefaultNew( Name name ) ;
	void _TimeIntegrator_AssignFromXML( void* timeIntegrator, Stg_ComponentFactory* cf, void* data ) ;
	void _TimeIntegrator_Build( void* timeIntegrator, void* data );
	void _TimeIntegrator_Initialise( void* timeIntegrator, void* data );
	void _TimeIntegrator_Execute( void* timeIntegrator, void* data );
	void _TimeIntegrator_Destroy( void* timeIntegrator, void* data ) ;

	/* +++ Private Functions +++ */
	void TimeIntegrator_UpdateClass( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteEuler( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteRK2( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteRK4( void* timeIntegrator, void* data ) ;

	void _TimeIntegrator_ExecuteRK2Simultaneous( void* timeIntegrator, void* data ) ;
	void _TimeIntegrator_ExecuteRK4Simultaneous( void* timeIntegrator, void* data ) ;

	/* +++ Public Functions +++ */
	void TimeIntegrator_Add( void* timeIntegrator, void* _timeIntegrand ) ;
	#define TimeIntegrator_GetCount( self ) \
		((TimeIntegrator*)self)->integrandRegister->objects->count
	#define TimeIntegrator_GetByIndex( self, index ) \
		( (TimeIntegrand*) NamedObject_Register_GetByIndex( ((TimeIntegrator*)self)->integrandRegister, index ) )

	void TimeIntegrator_Setup( void* timeIntegrator ) ;
	void TimeIntegrator_AppendSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;
	void TimeIntegrator_PrependSetupEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;

	void TimeIntegrator_Finalise( void* timeIntegrator ) ;
	void TimeIntegrator_AppendFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;
	void TimeIntegrator_PrependFinishEP( void* timeIntegrator, Name name, Func_Ptr funcPtr, char* addedBy, void* data ) ;

	void TimeIntegrator_InsertBeforeFinishEP( 
			void* timeIntegrator, 
			Name hookToInsertBefore,
			Name name, 
			Func_Ptr funcPtr, 
			char* addedBy, 
			void* data );

	void TimeIntegrator_InsertAfterFinishEP( 
			void* timeIntegrator, 
			Name hookToInsertAfter, 
			Name name, 
			Func_Ptr funcPtr, 
			char* addedBy, 
			void* data );
	
	void TimeIntegrator_SetTime( void* timeIntegrator, double time ) ;
	double TimeIntegrator_GetTime( void* timeIntegrator ) ;

	StgVariable* StgVariable_NewFromOld( StgVariable* oldVariable, Name name, Bool copyValues ) ;

#endif 

