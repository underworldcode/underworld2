/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_FeEntryPoint_h__
#define __StgFEM_SLE_SystemSetup_FeEntryPoint_h__
	
	/* Templates for feEntry point type */
	typedef void		(FeEntryPoint_AssembleStiffnessMatrix_Function)	( 
					void* stiffnessMatrix, 
					Bool bcRemoveQuery,
					void* _sle,
					void* _context
					);
	typedef void		(FeEntryPoint_AssembleStiffnessMatrix_CallFunction)	( 
					void* feEntryPoint, 
					void* stiffnessMatrix, 
					Bool bcRemoveQuery,
					void* _sle,
					void* _context
					);

	typedef void		(FeEntryPoint_AssembleForceVector_Function)	( 
					void* forceVector );
	typedef void		(FeEntryPoint_AssembleForceVector_CallFunction)	( 
					void* feEntryPoint, 
					void* forceVector ); 
		
	#define 			FeEntryPoint_AssembleStiffnessMatrix_CastType	(EntryPoint_CastType_MAX+1)
	#define 			FeEntryPoint_AssembleForceVector_CastType	(FeEntryPoint_AssembleStiffnessMatrix_CastType+1)
	#define 			FeEntryPoint_CastType_MAX			(FeEntryPoint_AssembleForceVector_CastType+1)
	
	/** Textual name of this class */
	extern const Type FeEntryPoint_Type;
	
	/** FeEntryPoint info */
	#define __FeEntryPoint \
		/* General info */ \
		__EntryPoint \
		\
		/* Virtual info */ \
		\
		/* FeEntryPoint info */
	struct FeEntryPoint { __FeEntryPoint };
	
	/* Create a new FeEntryPoint */
	FeEntryPoint* FeEntryPoint_New( Name name, unsigned int castType );
	
	/* Initialise an FeEntryPoint */
	void FeEntryPoint_Init( void* feEntryPoint, Name name, unsigned int castType );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define FEENTRYPOINT_DEFARGS \
                ENTRYPOINT_DEFARGS

	#define FEENTRYPOINT_PASSARGS \
                ENTRYPOINT_PASSARGS

	FeEntryPoint* _FeEntryPoint_New(  FEENTRYPOINT_DEFARGS  );
	
	/* Initialisation implementation */
	void _FeEntryPoint_Init( FeEntryPoint* self );
	
	/* Copy */
	#define FeEntryPoint_Copy( self ) \
		(FeEntryPoint*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define FeEntryPoint_DeepCopy( self ) \
		(FeEntryPoint*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	/* modified GetRun implementation */
	Func_Ptr _FeEntryPoint_GetRun( void* feEntryPoint );
	
	void _FeEntryPoint_Run_AssembleStiffnessMatrix( 
		void* feEntryPoint, 
		void* stiffnessMatrix, 
		Bool bcRemoveQuery,
		void* _sle,
		void* _context
		);

	void _FeEntryPoint_Run_AssembleForceVector( 
		void* feEntryPoint, 
		void* forceVector );
	
#endif /* __Fe_FeEntryPoint_h__ */

