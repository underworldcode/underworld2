/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_ForceTerm_h__
#define __StgFEM_SLE_SystemSetup_ForceTerm_h__

	typedef void (ForceTerm_AssembleElementFunction)	(
		void*						forceTerm, 
		ForceVector*			forceVec, 
		Element_LocalIndex	lElement_I,
		double*					elForceVecToAdd );
	
	
	/* Textual name of this class */
	extern const Type ForceTerm_Type;
	
	/* ForceTerm information */
   #define __ForceTerm  \
      /* General info */ \
      __Stg_Component \
      \
      FiniteElementContext*					context; \
      /* Virtual info */ \
      ForceTerm_AssembleElementFunction*	_assembleElement; \
      \
      /* General info */ \
      Stream*										debug; \
      ForceVector*                        forceVector; \
      Swarm*										integrationSwarm; \
      Stg_Component*								extraInfo;
	
	struct ForceTerm { __ForceTerm };


	
	/* Creation implementation / Virtual constructor */
	ForceTerm* ForceTerm_New(
		Name							name,
		FiniteElementContext*	context,
		ForceVector*				forceVector,
		Swarm*						integrationSwarm,
		Stg_Component*				extraInfo );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define FORCETERM_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                ForceTerm_AssembleElementFunction*  _assembleElement

	#define FORCETERM_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _assembleElement

	ForceTerm* _ForceTerm_New(  FORCETERM_DEFARGS  );
	
	void _ForceTerm_Init(
		void*							forceTerm,
		FiniteElementContext*	context,
		ForceVector*				forceVector,
		Swarm*						integrationSwarm,
		Stg_Component*				extraInfo );

	/* 'Stg_Class' Virtual Functions */
	void _ForceTerm_Delete( void* forceTerm );

	void _ForceTerm_Print( void* forceTerm, Stream* stream );

	#define ForceTerm_Copy( self ) \
		(ForceTerm*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ForceTerm_DeepCopy( self ) \
		(ForceTerm*)Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void* _ForceTerm_Copy( void* forceTerm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _ForceTerm_DefaultNew( Name name );

	void _ForceTerm_AssignFromXML( void* forceTerm, Stg_ComponentFactory* cf, void* data );

	void _ForceTerm_Build( void* forceTerm, void* data );

	void _ForceTerm_Initialise( void* forceTerm, void* data );

	void _ForceTerm_Execute( void* forceTerm, void* data );

	void _ForceTerm_Destroy( void* forceTerm, void* data );
	
	void ForceTerm_AssembleElement( 
		void*						forceTerm, 
		ForceVector*			forceVector, 
		Element_LocalIndex	lElement_I,
		double*					elForceVecToAdd );

	void _ForceTerm_AssembleElement( 
		void*						forceTerm, 
		ForceVector*			forceVec, 
		Element_LocalIndex	lElement_I,
		double*					elForceVecToAdd ) ;

	void ForceTerm_SetAssembleElementFunction( void* forceTerm, ForceTerm_AssembleElementFunction* assembleElementFunction ) ;

#endif /* __StgFEM_SLE_SystemSetup_ForceTerm_h__ */

