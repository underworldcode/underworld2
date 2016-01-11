/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_AdvectionDiffusion_Multicorrector_h__
#define __Underworld_AdvectionDiffusion_Multicorrector_h__
   
   /** Textual name of this class */
   extern const Type AdvDiffMulticorrector_Type;

   /** AdvDiffMulticorrector class contents */
   #define __AdvDiffMulticorrector \
      /* General info */ \
      __SLE_Solver \
      \
      /* Virtual info */ \
      \
      /* AdvDiffMulticorrector info */ \
      double          gamma; \
      Iteration_Index multiCorrectorIterations; \
      KSP             matrixSolver;

   struct AdvDiffMulticorrector { __AdvDiffMulticorrector };   

   AdvDiffMulticorrector* AdvDiffMulticorrector_New( 
      Name            name,
      double          gamma,
      Iteration_Index multiCorrectorIterations );
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define ADVDIFFMULTICORRECTOR_DEFARGS \
      SLE_SOLVER_DEFARGS

   #define ADVDIFFMULTICORRECTOR_PASSARGS \
      SLE_SOLVER_PASSARGS

   AdvDiffMulticorrector* _AdvDiffMulticorrector_New( ADVDIFFMULTICORRECTOR_DEFARGS );

   void _AdvDiffMulticorrector_Init( 
      AdvDiffMulticorrector* self, 
      double                 gamma,
      Iteration_Index        multiCorrectorIterations );
   
   void AdvDiffMulticorrector_InitAll( 
      void*           solver,
      double          gamma,
      Iteration_Index multiCorrectorIterations );

   void _AdvDiffMulticorrector_Delete( void* solver );
   void _AdvDiffMulticorrector_Print( void* solver, Stream* stream );

   void* _AdvDiffMulticorrector_DefaultNew( Name name ) ;
   void _AdvDiffMulticorrector_AssignFromXML( void* solver, Stg_ComponentFactory* cf, void* data ) ;
   void _AdvDiffMulticorrector_Build( void* solver, void* data ) ;
   void _AdvDiffMulticorrector_Initialise( void* solver, void* data ) ;
   void _AdvDiffMulticorrector_Execute( void* solver, void* data ) ;
   void _AdvDiffMulticorrector_Destroy( void* solver, void* data ) ;

   void _AdvDiffMulticorrector_SolverSetup( void* solver, void* data ) ;
   void _AdvDiffMulticorrector_Solve( void* solver, void* _sle ) ;

   void AdvDiffMulticorrector_Predictors( AdvDiffMulticorrector* self, AdvectionDiffusionSLE* sle, double dt ) ;

   void AdvDiffMulticorrector_Solution( AdvDiffMulticorrector* self, AdvectionDiffusionSLE* sle, Vec deltaPhiDot ) ;
   void AdvDiffMulticorrector_Correctors( AdvDiffMulticorrector* self, AdvectionDiffusionSLE* sle, Vec deltaPhiDot, double dt ) ;

   void AdvDiffMulticorrector_CalculatePhiDot( AdvDiffMulticorrector* self, AdvectionDiffusionSLE* sle, Vec deltaPhiDot ) ;
   void _AdvDiffMulticorrector_CalculatePhiDot_Explicit( AdvDiffMulticorrector* self, AdvectionDiffusionSLE* sle, Vec deltaPhiDot ) ;
   void _AdvDiffMulticorrector_CalculatePhiDot_Implicit( AdvDiffMulticorrector* self, AdvectionDiffusionSLE* sle, Vec deltaPhiDot ) ;

#endif

