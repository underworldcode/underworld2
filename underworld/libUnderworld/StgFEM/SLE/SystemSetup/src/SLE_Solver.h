/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

	
#ifndef __StgFEM_SLE_SystemSetup_SLE_Solver_h__
#define __StgFEM_SLE_SystemSetup_SLE_Solver_h__
	

	/** Textual name of this class */
	extern const Type SLE_Solver_Type;
	
	/* virtual function interface */
	typedef void (SLE_Solver_SolverSetupFunction) ( void* sleSolver, void* sle );
	typedef void (SLE_Solver_SolveFunction) ( void* sleSolver, void* sle );
	typedef Vec (SLE_Solver_GetResidualFunc) ( void* sleSolver, Index fvIndex );

	typedef void (SLE_Solver_FormResidualFunc) ( void *someSLE, void *someSolver, Vec );
	typedef void (SLE_Solver_GetRhsFunc) ( void *someSLE, void *someSovler, Vec );
	typedef void (SLE_Solver_GetSolutionFunc) ( void *someSLE, void *someSolver, Vec* );

	/** SLE_Solver class contents */
	#define __SLE_Solver \
		__Stg_Component \
		ExtensionManager*						extensionManager; \
		\
		/* Virtual info */ \
		SLE_Solver_SolverSetupFunction*	_solverSetup; \
		SLE_Solver_SolveFunction*			_solve; \
		SLE_Solver_GetResidualFunc*		_getResidual; \
  		SLE_Solver_FormResidualFunc*		_formResidual; \
		SLE_Solver_GetRhsFunc*				_getRhs; \
		SLE_Solver_GetSolutionFunc*		_getSolution; \
		\
		/* SLE_Solver info */ \
		Stream*                            debug; \
		Stream*                            info; \
		Iteration_Index                    maxIterations; \
		Bool                               hasSolved; /* need to know for various approx */ \
   	/* Timing variables for solvers */ \
		double				inneritsinitialtime; \
		double				outeritsinitialtime; \
		double				nonlinearitsinitialtime; \
		double				inneritsendtime; \
		double				outeritsendtime; \
		double				nonlinearitsendtime; \
		double				totalinneritstime; \
		double				totalouteritstime; \
		double				totalnonlinearitstime; \
		int					totalnuminnerits; \
		int				totalnumouterits; \
		int				totalnumnonlinearits; \
		int				avgnuminnerits; \
		int				avgnumouterits; \
		double			avgtimeinnerits; \
		double			avgtimeouterits; \
		double			avgtimenonlinearits; \
		int				currenttimestep; \
		int				previoustimestep; \
        		
	/** Abstract class defining the interface for a SLE_Solver solver - see SLE_Solver.h */
	struct SLE_Solver { __SLE_Solver };

	/* No SLE_Solver_New() or SLE_Solver_Init() as this is an abstract class. */

	/* --- Constructor functions --- */

	/** Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SLE_SOLVER_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                SLE_Solver_SolverSetupFunction*  _solverSetup, \
                SLE_Solver_SolveFunction*              _solve, \
                SLE_Solver_GetResidualFunc*      _getResidual

	#define SLE_SOLVER_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _solverSetup, \
	        _solve,       \
	        _getResidual

	SLE_Solver* _SLE_Solver_New(  SLE_SOLVER_DEFARGS  );

	/** class member initialisation */
	void _SLE_Solver_Init( SLE_Solver* self ) ;

	void SLE_Solver_InitAll( void* sleSolver ) ;

	/* --- Virtual function implementations --- */
	
	/** Class Virtual Functions Implementations */
	void* _SLE_Solver_Copy( void* sleSolver, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _SLE_Solver_Delete( void* sleSolver );

	void _SLE_Solver_Print( void* sleSolver, Stream* stream ) ;

	/** Stg_Component_Build() implementation: does nothing by default as some solvers may not need it. */
	void _SLE_Solver_Build( void* sleSolver, void* data );
	
	void _SLE_Solver_AssignFromXML( void* sleSolver, Stg_ComponentFactory* cf, void* data );

	/** Stg_Component_Initialise() implementation: does nothing by default as some solvers may not neet it. */
	void _SLE_Solver_Initialise( void* sleSolver, void* data );

	/** Stg_Component_Execute() implementation: Calls SolverSetup() for any per-iteration setup, then 
	calls Solve() to calculate the new solutions. */
	void _SLE_Solver_Execute( void* sleSolver, void* data );
	
	void _SLE_Solver_Destroy( void* sleSolver, void* data );

	/* --- Public functions --- */

	/** Does any required solver setup beyond assembly of the matrices to be solved: e.g. priming the Matrix solvers
	etc. */
	void SLE_Solver_SolverSetup( void* sleSolver, void* sle );
	
	/** Solve:- calculate the new values for all solution vectors in the system. */
	void SLE_Solver_Solve( void* sleSolver, void* sle );

	/** Set the maximum number of iterations */
	#define SLE_Solver_SetMaxIterations( self, its ) \
		(self)->maxIterations = its

	/** Get the systems most recent residual */
	#define SLE_Solver_GetResidual( self, fvIndex ) \
		(self)->_getResidual( self, fvIndex )
	

#endif

