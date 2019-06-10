/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_ContextEntryPoint_h__
#define __StGermain_Base_Context_ContextEntryPoint_h__
	
	/* Templates for default entry point type */
	typedef double			(ContextEntryPoint_Dt_Cast)		( void* context );
	typedef double			(ContextEntryPoint_Dt_CallCast)		( void* entryPoint, void* context );
	typedef void			(ContextEntryPoint_Step_Cast)		( void* context, double dt );
	typedef void			(ContextEntryPoint_Step_CallCast)	( void* entryPoint, void* context, double dt );
	#define 			ContextEntryPoint_Dt_CastType		(EntryPoint_CastType_MAX+1)
	#define 			ContextEntryPoint_Step_CastType		(ContextEntryPoint_Dt_CastType+1)
	#define 			ContextEntryPoint_CastType_MAX		(ContextEntryPoint_Step_CastType+1)
	
	/** Textual name of this class */
	extern const Type ContextEntryPoint_Type;
	
	/** ContextEntryPoint info */
	#define __ContextEntryPoint \
		/* General info */ \
		__EntryPoint \
		\
		/* Virtual info */ \
		\
		/* ContextEntryPoint info */
	struct ContextEntryPoint { __ContextEntryPoint };
	
	/* Create a new ContextEntryPoint */
	ContextEntryPoint* ContextEntryPoint_New( Name name, unsigned int castType );
	
	/* Initialise an ContextEntryPoint */
	void ContextEntryPoint_Init( void* contextEntryPoint, Name name, unsigned int castType );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CONTEXTENTRYPOINT_DEFARGS \
                ENTRYPOINT_DEFARGS

	#define CONTEXTENTRYPOINT_PASSARGS \
                ENTRYPOINT_PASSARGS

	ContextEntryPoint* _ContextEntryPoint_New(  CONTEXTENTRYPOINT_DEFARGS  );
	
	/* Initialisation implementation */
	void _ContextEntryPoint_Init( ContextEntryPoint* self );
	
	
	/* Default GetRun implementation */
	Func_Ptr _ContextEntryPoint_GetRun( void* contextEntryPoint );
	
	/* Context entry point run... one void* arguement passed, double returned. */
	double _ContextEntryPoint_Run_Dt( void* contextEntryPoint, void* data0 );
	
	/* Context entry point run... one void* arguement passed, one double arguement passed */
	void _ContextEntryPoint_Run_Step( void* contextEntryPoint, void* data0, double data1 );
	
#endif /* __StGermain_Base_Context_ContextEntryPoint_h__ */

