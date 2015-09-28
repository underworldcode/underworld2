/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Solvers_Assembly_VelocityMassMatrixTerm_h__
#define __Solvers_Assembly_VelocityMassMatrixTerm_h__

   /** Textual name of this class */
   extern const Type VelocityMassMatrixTerm_Type;

   /** VelocityMassMatrixTerm class contents */
   #define __VelocityMassMatrixTerm \
      /* General info */ \
      __StiffnessMatrixTerm \
      \
      /* Virtual info */ \
      \
      /* VelocityMassMatrixTerm info */ \
      Stream*     errorStream;

   struct VelocityMassMatrixTerm { __VelocityMassMatrixTerm };

   #ifndef ZERO
   #define ZERO 0
   #endif

   #define VELOCITYMASSMATRIXTERM_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

   #define VELOCITYMASSMATRIXTERM_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

   VelocityMassMatrixTerm* _VelocityMassMatrixTerm_New(  VELOCITYMASSMATRIXTERM_DEFARGS  );

   void _VelocityMassMatrixTerm_Init( void* matrixTerm );

   void _VelocityMassMatrixTerm_Delete( void* matrixTerm );

   void _VelocityMassMatrixTerm_Print( void* matrixTerm, Stream* stream );

   void* _VelocityMassMatrixTerm_DefaultNew( Name name );

   void _VelocityMassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

   void _VelocityMassMatrixTerm_Build( void* matrixTerm, void* data );

   void _VelocityMassMatrixTerm_Initialise( void* matrixTerm, void* data );

   void _VelocityMassMatrixTerm_Execute( void* matrixTerm, void* data );

   void _VelocityMassMatrixTerm_Destroy( void* matrixTerm, void* data );

   void _VelocityMassMatrixTerm_AssembleElement(
      void*							matrixTerm,
      StiffnessMatrix*			stiffnessMatrix,
      Element_LocalIndex		lElement_I,
      SystemLinearEquations*	sle,
      FiniteElementContext*	context,
      double**						elStiffMat );

#endif
