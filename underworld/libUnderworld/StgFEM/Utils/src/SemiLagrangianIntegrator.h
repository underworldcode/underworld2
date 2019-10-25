/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Utils_SemiLagrangianIntegrator_h__
#define __StgFEM_Utils_SemiLagrangianIntegrator_h__

   /** Textual name of this class */
   extern const Type SemiLagrangianIntegrator_Type;

   /** SemiLagrangianIntegrator class contents */
   #define __SemiLagrangianIntegrator \
      __Stg_Component \
      \
      /* SemiLagrangianIntegrator info */ \
      FeVariable*                  velocityField;		     \
      Stg_ObjectList*              variableList;       	  \
      Stg_ObjectList*              varStarList;  		     \
      Stg_ObjectList*              varOldList;  	     	  \
      FiniteElementContext*        context;             	  \
      FeVariable*		              advectedField;		     \
      /* for problems with temporally evolving velocity */ \
      FeVariable*		              prevVelField;           \
//      Energy_SLE*                  sle;

   /** Abstract class defining the interface for a SemiLagrangianIntegrator solver - see SemiLagrangianIntegrator.h */
   struct SemiLagrangianIntegrator { __SemiLagrangianIntegrator };

   /* --- Constructor functions --- */
   void* _SemiLagrangianIntegrator_DefaultNew( Name name );

    void SemiLagrangianIntegrator_BuildStaticStencils( FeVariable* stencilField );
    Bool New_BicubicInterpolator( FeVariable* feVariable, FeVariable* stencilField, double* position, unsigned* sizes, double* result );

   /** Creation implementation */

   #ifndef ZERO
   #define ZERO 0
   #endif

   #define SEMILAGRANGIANINTEGRATOR_DEFARGS \
                STG_COMPONENT_DEFARGS

   #define SEMILAGRANGIANINTEGRATOR_PASSARGS \
                STG_COMPONENT_PASSARGS

   SemiLagrangianIntegrator* _SemiLagrangianIntegrator_New(  SEMILAGRANGIANINTEGRATOR_DEFARGS  );

   /* --- Virtual function implementations --- */

   /** Class Virtual Functions Implementations */
   void* _SemiLagrangianIntegrator_Copy( void* slIntegrator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
   void _SemiLagrangianIntegrator_Delete( void* slIntegrator );
   void _SemiLagrangianIntegrator_Print( void* slIntegrator, Stream* stream );

   /** Stg_Component_Build() implementation: does nothing by default as some solvers may not need it. */
   void _SemiLagrangianIntegrator_Build( void* slIntegrator, void* data );

   void _SemiLagrangianIntegrator_AssignFromXML( void* slIntegrator, Stg_ComponentFactory* cf, void* data );

   /** Stg_Component_Initialise() implementation: does nothing by default as some solvers may not neet it. */
   void _SemiLagrangianIntegrator_Initialise( void* slIntegrator, void* data );

   /** Stg_Component_Execute() implementation: Calls SolverSetup() for any per-iteration setup, then
   calls Solve() to calculate the new solutions. */
   void _SemiLagrangianIntegrator_Execute( void* slIntegrator, void* data );

   void _SemiLagrangianIntegrator_Destroy( void* slIntegrator, void* data );

   /* --- Private Functions --- */
   void SemiLagrangianIntegrator_InitSolve( void* slIntegrator, void* data );
   void SemiLagrangianIntegrator_UpdatePreviousVelocityField( void* slIntegrator, void* data );

   /* --- Public functions --- */
   Bool BicubicInterpolator( FeVariable* feVariable, double* coord, double* delta, unsigned* nNodes, double* result );
   Bool PeriodicUpdate( double* pos, double* min, double* max, unsigned dim, Bool isPeriodic );
   void InterpLagrange( double x, double* coords, double (*values)[3], unsigned numdofs, double* result );
   void IntegrateRungeKutta( FeVariable* velocityField, double dt, double* origin, double* position );
   void IntegrateRungeKutta_StgVariableVelocity( FeVariable* velocityField, FeVariable* prevVelField, double dt, double* origin, double* position );
    Bool SemiLagrangianIntegrator_PointsAreClose( double* p1, double* p2, int dim, double rtol, double atol );

   /** Does any required solver setup beyond assembly of the matrices to be solved: e.g. priming the Matrix solvers
   etc. */

   /** Solve:- calculate the new values for all solution vectors in the system. */
   void SemiLagrangianIntegrator_Solve( void* slIntegrator, FeVariable* variableField, FeVariable* variableFieldPrime );
   void SemiLagrangianIntegrator_SolveNew( FeVariable* variableField, double dt, FeVariable* velocityField, FeVariable* varStarField, FeVariable* stencilField  );
#endif
