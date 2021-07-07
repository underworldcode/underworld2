/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_SolutionVector_h__
#define __StgFEM_SLE_SystemSetup_SolutionVector_h__

	/* Textual name of this class */
	extern const Type SolutionVector_Type;

	/* StiffnessMatrix information */
	#define __SolutionVector  \
		/* General info */ \
		__Stg_Component \
		\
		/* StiffnessMatrix info */ \
		Stream*						debug; \
		Vec							vector; \
		MPI_Comm						comm; \
		FeVariable*					feVariable; /** need to get # of global unconstrained dofs */\
    FeEquationNumber*   eqNum; /* eqNum is passed in from python layer */

	struct SolutionVector { __SolutionVector };



	/* Creation implementation / Virtual constructor */
	void* _SolutionVector_DefaultNew( Name name );

	SolutionVector* SolutionVector_New(
		Name							name,
		MPI_Comm						comm,
		FeVariable*					feVariable );


	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SOLUTIONVECTOR_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define SOLUTIONVECTOR_PASSARGS \
                STG_COMPONENT_PASSARGS

	SolutionVector* _SolutionVector_New(  SOLUTIONVECTOR_DEFARGS  );

	/* Initialise implementation */
	void _SolutionVector_Init(
		SolutionVector*			self,
		MPI_Comm						comm,
		FeVariable*					feVariable );

	/* Stg_Class_Delete a ElementType construst */
	void _SolutionVector_Delete( void* solutionVector );

	/* Print the contents of an ElementType construct */
	void _SolutionVector_Print( void* solutionVector, Stream* stream );

	/* Copy */
	#define SolutionVector_Copy( self ) \
		(ForceVector*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define SolutionVector_DeepCopy( self ) \
		(ForceVector*)Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void* _SolutionVector_Copy( void* solutionVector, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

  void _SolutionVector_Build( void* solutionVector, void* data );

	void _SolutionVector_AssignFromXML( void* solutionVector, Stg_ComponentFactory* cf, void* data );

	/* Initialisation implementation */
	void _SolutionVector_Initialise( void* solutionVector, void* data );

	/* Execution implementation */
	void _SolutionVector_Execute( void* solutionVector, void* data );

	void _SolutionVector_Destroy( void* solutionVector, void* data );

	void SolutionVector_ApplyBCsToVariables( void* solutionVector, void* data );

	void SolutionVector_UpdateSolutionOntoNodes( void* solutionVector );

  /** Remove a vector, nVec, from the vector, xVec. */
  double SolutionVector_RemoveVectorSpace( SolutionVector* xVec, SolutionVector* nVec );
	/** Loads the current value at each dof of the feVariable related to this solution vector onto the vector itself */
	void SolutionVector_LoadCurrentFeVariableValuesOntoVector( void* solutionVector );

#endif /* __StgFEM_SLE_SystemSetup_SolutionVector_h__ */
