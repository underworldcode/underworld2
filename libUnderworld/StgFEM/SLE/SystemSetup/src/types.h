/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_types_h__
#define __StgFEM_SLE_SystemSetup_types_h__
	
	/* FE types/classes */
	typedef struct FeEntryPoint                 FeEntryPoint;
	typedef struct StiffnessMatrix              StiffnessMatrix;
	typedef struct StiffnessMatrixTerm          StiffnessMatrixTerm;
	typedef struct ForceVector                  ForceVector;
	typedef struct ForceTerm                    ForceTerm;
	typedef struct SolutionVector               SolutionVector;
	typedef struct SystemLinearEquations        SystemLinearEquations;
	typedef struct SLE_Solver                   SLE_Solver;
	typedef struct FiniteElementContext         FiniteElementContext;
	typedef struct Assembler		Assembler;
        typedef struct MGOpGenerator			MGOpGenerator;
        typedef struct SROpGenerator			SROpGenerator;
        typedef struct PETScMGSolver			PETScMGSolver;
	
	/* types for lists etc ... for readability */
	typedef Index                       StiffnessMatrix_Index;
	typedef StiffnessMatrix*            StiffnessMatrixPtr;
	typedef Stg_ObjectList              StiffnessMatrixList;
	typedef Index                       ForceVector_Index;
	typedef ForceVector*                ForceVectorPtr;
	typedef Stg_ObjectList              ForceVectorList;
	typedef Index                       SolutionVector_Index;
	typedef SolutionVector*             SolutionVectorPtr;
	typedef Stg_ObjectList              SolutionVectorList;
	typedef Index                       SLE_Solver_Index;
	typedef SolutionVector*             SLE_SolverPtr;
	typedef Stg_ObjectList              SLE_SolverList;
	typedef Index                       SystemLinearEquations_Index;
	typedef SystemLinearEquations*      SystemLinearEquationsPtr;
	typedef Stg_ObjectList              SystemLinearEquationList;

	/* output streams: initialised in StgFEM_SLE_SystemSetup_Init() */
	extern Stream* StgFEM_SLE_Debug;
	extern Stream* StgFEM_SLE_SystemSetup_Debug;

typedef struct {
      void* callback;
      void* object;
} Callback;

typedef enum {
	MGSolver_Status_ConvergedRelative = 2, 
	MGSolver_Status_ConvergedAbsolute = 3, 
	MGSolver_Status_ConvergedIterations = 4, 
	MGSolver_Status_DivergedNull = -2, 
	MGSolver_Status_DivergedIterations = -3, 
	MGSolver_Status_DivergedTolerance = -4, 
	MGSolver_Status_Iterating = 0
} MGSolver_Status;

/** Virtual function types */
typedef void (MGSolver_SetCommFunc)( void* matrixSolver, MPI_Comm comm );
typedef void (MGSolver_SetMatrixFunc)( void* matrixSolver, void* matrix );
typedef void (MGSolver_SetMaxIterationsFunc)( void* matrixSolver, unsigned nIterations );
typedef void (MGSolver_SetRelativeToleranceFunc)( void* matrixSolver, double tolerance );
typedef void (MGSolver_SetAbsoluteToleranceFunc)( void* matrixSolver, double tolerance );
typedef void (MGSolver_SetUseInitialSolutionFunc)( void* matrixSolver, Bool state );

typedef void (MGSolver_SolveFunc)( void* matrixSolver, void* rhs, void* solution );
typedef void (MGSolver_SetupFunc)( void* matrixSolver, void* rhs, void* solution );

typedef MGSolver_Status (MGSolver_GetSolveStatusFunc)( void* matrixSolver );
typedef unsigned (MGSolver_GetIterationsFunc)( void* matrixSolver );
typedef unsigned (MGSolver_GetMaxIterationsFunc)( void* matrixSolver );
typedef double (MGSolver_GetResidualNormFunc)( void* matrixSolver );


/* MatrixSolver class has been depreciated, so this class can no
 * longer inherit from it. all the data previously encapsulated in 
 * the MatrixSolver class is now wrapped up here */
typedef struct {
	MPI_Comm	comm;
	KSP		ksp;
	Mat		matrix;
	Mat		inversion;
	Vec		residual;
	Bool		expiredResidual;
	Bool		matrixChanged;
	Vec		curRHS;
	Vec		curSolution;
	Bool		optionsReady;
} MGSolver_PETScData;

#endif /* __StgFEM_SLE_SystemSetup_types_h__ */
