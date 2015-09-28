/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_StiffnessMatrixTerm_h__
#define __StgFEM_SLE_SystemSetup_StiffnessMatrixTerm_h__

	typedef void (StiffnessMatrixTerm_AssembleElementFunction)	(
			void*                             stiffnessMatrixTerm, 
			StiffnessMatrix*                  stiffnessMatrix, 
			Element_LocalIndex                lElement_I,
			SystemLinearEquations*            sle,
			FiniteElementContext*             context,
			double**                          elStiffMatToAdd );
	
	
	/* Textual name of this class */
	extern const Type StiffnessMatrixTerm_Type;
	
	/* StiffnessMatrix information */
	#define __StiffnessMatrixTerm  \
		/* General info */ \
		__Stg_Component \
		\
		FiniteElementContext*				     context;                  \
		/* Virtual info */ \
		StiffnessMatrixTerm_AssembleElementFunction*         _assembleElement;         \
		\
		/* General info */ \
		Stream*                                              debug;                    \
		Swarm*                                               integrationSwarm;         \
		Stg_Component*                                       extraInfo;                \
		StiffnessMatrix*                                     stiffnessMatrix;          \
		/* Data for GNx storage */ \
	  double                   **GNx; /* store globalDerivative ptr here */ \
	  double                   *N; /* store array for shape functions here */ \
		int                      max_nElNodes;  /* holds the maxNumNodes per element */ 
	
	struct StiffnessMatrixTerm { __StiffnessMatrixTerm };
	
	/* Creation implementation / Virtual constructor */
	StiffnessMatrixTerm* StiffnessMatrixTerm_New(
		Name                                                 name,
		FiniteElementContext*				                    context,
		StiffnessMatrix*                                     stiffnessMatrix,
		Swarm*                                               integrationSwarm,
		Stg_Component*                                       extraInfo );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STIFFNESSMATRIXTERM_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                StiffnessMatrixTerm_AssembleElementFunction*  _assembleElement

	#define STIFFNESSMATRIXTERM_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _assembleElement

	StiffnessMatrixTerm* _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_DEFARGS  );
	
	/* 'Stg_Class' Virtual Functions */
	void _StiffnessMatrixTerm_Delete( void* stiffnessMatrixTerm );
	void _StiffnessMatrixTerm_Print( void* stiffnessMatrixTerm, Stream* stream );
	#define StiffnessMatrixTerm_Copy( self ) \
		(StiffnessMatrixTerm*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define StiffnessMatrixTerm_DeepCopy( self ) \
		(StiffnessMatrixTerm*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _StiffnessMatrixTerm_Copy( void* stiffnessMatrixTerm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _StiffnessMatrixTerm_DefaultNew( Name name );
	void _StiffnessMatrixTerm_AssignFromXML( void* stiffnessMatrixTerm, Stg_ComponentFactory* cf, void* data );
	void _StiffnessMatrixTerm_Build( void* stiffnessMatrixTerm, void* data );
	void _StiffnessMatrixTerm_Initialise( void* stiffnessMatrixTerm, void* data );
	void _StiffnessMatrixTerm_Execute( void* stiffnessMatrixTerm, void* data );
	void _StiffnessMatrixTerm_Destroy( void* stiffnessMatrixTerm, void* data );

	void _StiffnessMatrixTerm_Init(
		void*                                                stiffnessMatrixTerm,
		FiniteElementContext*				                    context,
		StiffnessMatrix*                                     stiffnessMatrix,
		Swarm*                                               integrationSwarm,
		Stg_Component*                                       extraInfo );
	
	void StiffnessMatrixTerm_AssembleElement( 
			void*                             stiffnessMatrixTerm, 
			StiffnessMatrix*                  stiffnessMatrix, 
			Element_LocalIndex                lElement_I,
			SystemLinearEquations*            sle,
			FiniteElementContext*             context,
			double**                          elStiffMatToAdd );

	void _StiffnessMatrixTerm_AssembleElement( 
			void*                             stiffnessMatrixTerm, 
			StiffnessMatrix*                  stiffnessMatrix, 
			Element_LocalIndex                lElement_I,
			SystemLinearEquations*            sle,
			FiniteElementContext*             context,
			double**                          elStiffMatToAdd ) ;

	void StiffnessMatrixTerm_SetAssembleElementFunction( void* stiffnessMatrixTerm, StiffnessMatrixTerm_AssembleElementFunction* assembleElementFunction ) ;

#endif /* __StgFEM_SLE_SystemSetup_StiffnessMatrixTerm_h__ */

