/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_ForceVector_h__
#define __StgFEM_SLE_SystemSetup_ForceVector_h__
	
	
	/* Textual name of this class */
	extern const Type ForceVector_Type;
	
	/* StiffnessMatrix information */
	#define __ForceVector  \
		/* General info */ \
		__SolutionVector \
		\
		/* Virtual info */ \
		\
		/* StiffnessMatrix info */ \
		Index							localSize;  \
		Dimension_Index			dim;  \
		EntryPoint_Register*		entryPoint_Register;  \
		FeEntryPoint*				assembleForceVector;  \
		Name							_assembleForceVectorEPName;  \
		Stg_ObjectList*			forceTermList;  \
		Stg_Component*				applicationDepExtraInfo; /**< Default is NULL: passed to elForceVec during assembly */\
		Assembler*					bcAsm;  \
		IArray*						inc;  \
		int							nModifyCBs; \
		Callback*					modifyCBs;
	
	struct ForceVector { __ForceVector };


	
	/* Creation implementation / Virtual constructor */
	
	ForceVector* ForceVector_New(
		Name							name,
		FeVariable*					feVariable,
		Dimension_Index			dim,
		void*							entryPoint_Register,
		MPI_Comm						comm );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define FORCEVECTOR_DEFARGS \
                SOLUTIONVECTOR_DEFARGS

	#define FORCEVECTOR_PASSARGS \
                SOLUTIONVECTOR_PASSARGS

	ForceVector* _ForceVector_New(  FORCEVECTOR_DEFARGS  ); 

	void _ForceVector_Init(
		void*						forceVector,
		Dimension_Index		dim,
		EntryPoint_Register*	entryPoint_Register );
	
	/* 'Stg_Class' Virtual Functions */
	void _ForceVector_Delete( void* forceVector );

	void _ForceVector_Print( void* forceVector, Stream* stream );

	#define ForceVector_Copy( self ) \
		(ForceVector*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ForceVector_DeepCopy( self ) \
		(ForceVector*)Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void* _ForceVector_Copy( void* forceVector, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Virtual Functions */
	void* _ForceVector_DefaultNew( Name name );

	void _ForceVector_AssignFromXML( void* forceVector, Stg_ComponentFactory* cf, void* data );

	void _ForceVector_Build( void* forceVector, void* data );

	void _ForceVector_Initialise( void* forceVector, void* data );

	void _ForceVector_Execute( void* forceVector, void* data );

	void _ForceVector_Destroy( void* forceVector, void* data );
	
	/** Interface to assemble this Force Vector. Calls an entry point, meaning the user can specify if, and then how,
	it should be assembled. */
	void ForceVector_Assemble( void* forceVector );
	
	/** Prints the contents of a single element's force vector */
	void ForceVector_PrintElementForceVector(
		ForceVector* self,
		Element_LocalIndex element_lI,
		Dof_EquationNumber** elementLM,
		double* elForceVecToAdd );
	
	void ForceVector_GlobalAssembly_General( void* forceVector ) ;

	void ForceVector_AssembleElement( void* forceVector, Element_LocalIndex element_lI, double* elForceVecToAdd ) ;

	void ForceVector_AddForceTerm( void* forceVector, void* forceTerm ) ;

    void ForceVector_Zero( void* forceVector ) ;

#endif /* __StgFEM_SLE_SystemSetup_ForceVector_h__ */

