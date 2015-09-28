/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_SystemLinearEquations_h__
#define __StgFEM_SLE_SystemSetup_SystemLinearEquations_h__

	/** Textual name of this class */
	extern const Type SystemLinearEquations_Type;
	
	/* virtual function interface */
	typedef void (SystemLinearEquations_LM_SetupFunction) ( void* sle, void* data );
	typedef void (SystemLinearEquations_MatrixSetupFunction) ( void* sle, void* data );
	typedef void (SystemLinearEquations_VectorSetupFunction) ( void* sle, void* data );
	typedef void (SystemLinearEquations_UpdateSolutionOntoNodesFunc) ( void* sle, void* data );
	typedef void (SystemLinearEquations_MG_SelectStiffMatsFunc) ( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms );

	/* for solving non linear systems using Newton's method */
	typedef int (SystemLinearEquations_BuildFFunc) ( SNES nls, Vec x, Vec f, void* context );
	typedef int (SystemLinearEquations_BuildJFunc) ( SNES nls, Vec x, Mat* A, Mat* B, MatStructure* matStruct, void* context );	
	typedef void (SystemLinearEquations_SetFFunc) ( Vec* F, void* context );
	typedef void (SystemLinearEquations_ConfigureNonlinearSolver) ( void* nls, void* data );
	typedef void (SLE_FormFunctionFunc)( void *someSLE, Vec X, Vec F, void *ctx );
	
	/*
	** SystemLinearEquations class contents.
	*/
	
	#define __SystemLinearEquations \
		/* General info */ \
		__Stg_Component \
		ExtensionManager*												extensionManager; \
		\
		/* Virtual info */ \
		SystemLinearEquations_LM_SetupFunction*				_LM_Setup; \
		SystemLinearEquations_MatrixSetupFunction*			_matrixSetup; \
		SystemLinearEquations_VectorSetupFunction*			_vectorSetup; \
		SystemLinearEquations_UpdateSolutionOntoNodesFunc*	_updateSolutionOntoNodes; \
		SystemLinearEquations_MG_SelectStiffMatsFunc*		_mgSelectStiffMats; \
		\
		/* SystemLinearEquations info */ \
		Stream*															debug; \
		Stream*															info; \
		Stream*															convergenceStream; \
		Bool																makeConvergenceFile; \
		MPI_Comm															comm; \
		StiffnessMatrixList*											stiffnessMatrices; \
		ForceVectorList*												forceVectors; \
		SolutionVectorList*											solutionVectors; \
		SLE_Solver*														solver; \
		FiniteElementContext*										context; \
		Name																executeEPName; \
		EntryPoint*														executeEP; \
		Name																integrationSetupEPName; \
		EntryPoint*														integrationSetupEP; \
		EntryPoint_Register*											entryPoint_Register; \
		\
		Bool																removeBCs; \
		\
		/* Non-linear info */ \
		Bool																isNonLinear; \
		Stg_Component_ExecuteFunction*							linearExecute; \
		double															nonLinearTolerance; \
		Iteration_Index												nonLinearMaxIterations; \
		Iteration_Index												nonLinearIteration_I; \
		Bool																killNonConvergent; \
		Iteration_Index												nonLinearMinIterations; \
		double															curResidual; \
		double															curSolveTime; \
		/* BEGIN LUKE'S FRICTIONAL BCS BIT */ \
		char*																nlSetupEPName; \
		EntryPoint*														nlSetupEP; \
		char*																nlEPName; \
		EntryPoint*														nlEP; \
		char*																postNlEPName; \
		EntryPoint*														postNlEP; \
		char*																nlConvergedEPName; \
		EntryPoint*														nlConvergedEP; \
		Bool																nlFormJacobian; \
 		Vec																nlCurIterate; \
		/* END LUKE'S FRICTIONAL BCS BIT */ \
		/* Multi-grid data. */ \
		Bool																mgEnabled; \
		Bool																mgUpdate; \
		unsigned															nMGHandles; \
		unsigned*														mgHandles; /* one per MG'd 'StiffnessMatrix' */ \
		/* for solving non linear systems using Newton's method */\
		Name																nonLinearSolutionType; \
		SystemLinearEquations_BuildFFunc*						_buildF; \
		SystemLinearEquations_BuildJFunc*						_buildJ; \
		void*																buildFContext; \
		void*																buildJContext; \
		SNES																nlSolver; \
		Bool																linearSolveInitGuess; \
		Vec																F; \
		Vec																X; \
		Mat																A; \
		Mat																J; \
		Mat																P; \
		SystemLinearEquations_SetFFunc*							_setFFunc; \
		SystemLinearEquations_ConfigureNonlinearSolver*		_configureNLSolverFunc; \
		SystemLinearEquations_SetFFunc*							_updateOldFields; \
		Name																optionsPrefix; \
		/* parameter and methods relevant to PICARD */ \
		Name																picard_form_function_type; \
		double															alpha; \
		double															rtol,ttol,stol,abstol; \
		Bool																picard_monitor; \
      SLE_FormFunctionFunc*										_sleFormFunction; \
      Bool                                               runatExecutePhase;
		
		
	/** Abstract class to manage the set up, building, initialisation etc of a System of
	Linear Equations - see SystemLinearEquations.h */
	struct SystemLinearEquations { __SystemLinearEquations };

	/** Constructor */
	SystemLinearEquations* SystemLinearEquations_New(
		Name							name,
		FiniteElementContext*	context,
		SLE_Solver*					solver,
		Bool                    removeBCs,
		void*							nlSolver,
		Bool							isNonLinear,
		double						nonLinearTolerance,
		Iteration_Index			nonLinearMaxIterations,
		Bool							killNonConvergent,		
		EntryPoint_Register*		entryPoint_Register,
		MPI_Comm						comm );

	/** Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SYSTEMLINEAREQUATIONS_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                SystemLinearEquations_LM_SetupFunction*                            _LM_Setup, \
                SystemLinearEquations_MatrixSetupFunction*                      _matrixSetup, \
                SystemLinearEquations_VectorSetupFunction*                      _vectorSetup, \
                SystemLinearEquations_UpdateSolutionOntoNodesFunc*  _updateSolutionOntoNodes, \
                SystemLinearEquations_MG_SelectStiffMatsFunc*             _mgSelectStiffMats

	#define SYSTEMLINEAREQUATIONS_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _LM_Setup,                \
	        _matrixSetup,             \
	        _vectorSetup,             \
	        _updateSolutionOntoNodes, \
	        _mgSelectStiffMats      

	SystemLinearEquations* _SystemLinearEquations_New(  SYSTEMLINEAREQUATIONS_DEFARGS  );

	void _SystemLinearEquations_Init(
		void*                   sle,
		SLE_Solver*             solver,
		Bool                    removeBCs,
		void*                   nlSolver,
		FiniteElementContext*   context,
		Bool                    makeConvergenceFile,
		Bool                    isNonLinear,
		double                  nonLinearTolerance,
		Iteration_Index         nonLinearMaxIterations,
		Bool                    killNonConvergent,
		Iteration_Index         nonLinearMinIterations,
		Name                    nonLinearSolutionType,
		Name                    optionsPrefix,
		EntryPoint_Register*    entryPoint_Register,
		MPI_Comm                comm );

	/* Stg_Class_Delete() implementations */
	void _SystemLinearEquations_Delete( void* sle );

	/* Stg_Class_Print() implementation */
	void _SystemLinearEquations_Print( void* sle, Stream* stream );
	
	/* Copy implementation */
	#define SystemLinearEquations_Copy( self ) \
		(SystemLinearEquations*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define SystemLinearEquations_DeepCopy( self ) \
		(SystemLinearEquations*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
		
	void* _SystemLinearEquations_Copy( void* sle, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* +++ Virtual Functions +++ */
	void* _SystemLinearEquations_DefaultNew( Name name ) ;

	void _SystemLinearEquations_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data );

	void _SystemLinearEquations_Build( void* sle, void* data );

	void _SystemLinearEquations_Initialise( void* sle, void* data );

	/** Stg_Component_Execute() implementation: Assembles the correct values into all matrices of the
	system, then executes the system's solver. */
	void _SystemLinearEquations_Execute( void* sle, void* data );
	
	void _SystemLinearEquations_Destroy( void* sle, void* data );

	void SystemLinearEquations_ExecuteSolver( void* sle, void* data ) ;

	void SystemLinearEquations_BC_Setup( void* sle, void* data );

	/* LM Setup */
	void SystemLinearEquations_LM_Setup( void* sle, void* data );

	void _SystemLinearEquations_LM_Setup( void* sle, void* data );

	void SystemLinearEquations_IntegrationSetup( void* sle, void* data );
	
	/* Matrix Setup */
	void SystemLinearEquations_MatrixSetup( void* sle, void* data );

	void _SystemLinearEquations_MatrixSetup( void* sle, void* data );

	/* Vector Setup */
	void SystemLinearEquations_VectorSetup( void* sle, void* data );

	void _SystemLinearEquations_VectorSetup( void* sle, void* data );

	/* +++ Public Functions / Macros +++ */
	
	/* AddStiffnessMatrix macro */
	#define SystemLinearEquations_AddStiffnessMatrix( sle, stiffnessMatrix ) \
		Stg_ObjectList_Append( ((sle)->stiffnessMatrices), stiffnessMatrix )
	Index _SystemLinearEquations_AddStiffnessMatrix( void* sle, StiffnessMatrix* stiffnessMatrix );

	/* GetStiffnessMatrix macro */
	#define SystemLinearEquations_GetStiffnessMatrix( sle, stiffnessMatrixName ) \
		((StiffnessMatrix*)Stg_ObjectList_Get( ((sle)->stiffnessMatrices), stiffnessMatrixName ))
	StiffnessMatrix* _SystemLinearEquations_GetStiffnessMatrix( void* sle, Name stiffnessMatrixName );

	#define SystemLinearEquations_GetStiffnessMatrixAt( sle, stiffnessMatrixIndex ) \
		((StiffnessMatrix*)Stg_ObjectList_At( ((sle)->stiffnessMatrices), stiffnessMatrixIndex ))

	/* AddForceVector macro */
	#define SystemLinearEquations_AddForceVector( sle, forceVector ) \
		Stg_ObjectList_Append( ((sle)->forceVectors), forceVector )
	Index _SystemLinearEquations_AddForceVector( void* sle, ForceVector* forceVector );

	/* GetForceVector macro */
	#define SystemLinearEquations_GetForceVector( sle, forceVectorName ) \
		((ForceVector*)Stg_ObjectList_Get( ((sle)->forceVectors), forceVectorName ))
	ForceVector* _SystemLinearEquations_GetForceVector( void* sle, Name forceVectorName );

	#define SystemLinearEquations_GetForceVectorAt( sle, forceVectorIndex ) \
		((ForceVector*)Stg_ObjectList_At( ((sle)->forceVectors), forceVectorIndex ))

	/* AddSolutionVector macro */
	#define SystemLinearEquations_AddSolutionVector( sle, solutionVector ) \
		Stg_ObjectList_Append( ((sle)->solutionVectors), solutionVector )
	Index _SystemLinearEquations_AddSolutionVector( void* sle, SolutionVector* solutionVector );

	/* GetSolutionVector macro */
	#define SystemLinearEquations_GetSolutionVector( sle, solutionVectorName ) \
		((SolutionVector*)Stg_ObjectList_Get( ((sle)->solutionVectors), solutionVectorName ))
	SolutionVector* _SystemLinearEquations_GetSolutionVector( void* sle, Name solutionVectorName );

	#define SystemLinearEquations_GetSolutionVectorAt( sle, solutionVectorIndex ) \
		((SolutionVector*)Stg_ObjectList_At( ((sle)->solutionVectors), solutionVectorIndex ))

	/* Update all solution vectors back onto mesh nodes */
	void SystemLinearEquations_UpdateSolutionOntoNodes( void* sle, void* data );

	void _SystemLinearEquations_UpdateSolutionOntoNodes( void* sle, void* data );

	void SystemLinearEquations_ZeroAllVectors( void* sle, void* data );

	/* Non-linear stuff */
	/* matrix free finite difference newton's method non linear solve */
	void SystemLinearEquations_NewtonMFFDExecute( void* sle, void* data );

	/* solitary waves model with hand rolled J */
	void SystemLinearEquations_NewtonInitialise( void* sle, void* data );

	void SystemLinearEquations_NewtonExecute( void* sle, void* data );

	void SystemLinearEquations_NewtonFinalise( void* sle, void* data );

	void SystemLinearEquations_NonLinearExecute( void* sle, void* data );

	void SystemLinearEquations_AddNonLinearSetupEP( void* sle, const char* name, EntryPoint_2VoidPtr_Cast func );

	void SystemLinearEquations_AddNonLinearEP( void* sle, const char* name, EntryPoint_2VoidPtr_Cast func );

	void SystemLinearEquations_AddPostNonLinearEP( void* sle, const char* name, EntryPoint_2VoidPtr_Cast func );

	void SystemLinearEquations_SetToNonLinear( void* sle, Bool isNonLinear );

	void SystemLinearEquations_CheckIfNonLinear( void* sle );
	
	/*
	** All the multi-grid virtual functions and their general implementations.
	*/
	
	void SystemLinearEquations_MG_Enable( void* _sle );
	
	void SystemLinearEquations_MG_SelectStiffMats( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms );

	void _SystemLinearEquations_MG_SelectStiffMats( void* _sle, unsigned* nSMs, StiffnessMatrix*** sms );

   void _SystemLinearEquations_RunEP( void* sle, void* _context )	;

   void SystemLinearEquations_SetCustomRunPoint( void* sle, void* _context, const Name entryPointName );

   typedef void (SystemLinearEquations_RunEPFunction) ( void* sle, void* context );
   SystemLinearEquations_RunEPFunction* SystemLinearEquations_GetRunEPFunction();
   void SystemLinearEquations_SetRunDuringExecutePhase( void* sle, Bool setRunDuringExectutePhase );

#endif

