/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_MatrixAssemblyTerm_RotationDof_h__
#define __Underworld_Utils_MatrixAssemblyTerm_RotationDof_h__

#ifdef __cplusplus

   extern "C++" {

      #include <Underworld/Function/Function.hpp>
      #include <Underworld/Function/MeshCoordinate.hpp>

   struct MatrixAssemblyTerm_RotationDof_cppdata
   {
       Fn::Function::func eVecFn[3];
       std::shared_ptr<MeshCoordinate> input;
   };

   void MatrixAssemblyTerm_RotationDof_SetEFn( void* _self, int i, Fn::Function* fn );

   }

   extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
	/** Textual name of this class */
	extern const Type MatrixAssemblyTerm_RotationDof_Type;

/** MatrixAssemblyTerm_RotationDof class contents */
#define __MatrixAssemblyTerm_RotationDof \
  __StiffnessMatrixTerm     \
  void*   cppdata;          \
  IArray* inc;              \
  FeMesh* geometryMesh;     \
  int	    max_nElNodes_col; \
  int     max_nElNodes_row; \
  double	*Ni;              \
  double	*Mi;              \

	struct MatrixAssemblyTerm_RotationDof { __MatrixAssemblyTerm_RotationDof };


#ifndef ZERO
 	#define ZERO 0
#endif

	#define MatrixAssemblyTerm_RotationDof_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define MatrixAssemblyTerm_RotationDof_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	MatrixAssemblyTerm_RotationDof* _MatrixAssemblyTerm_RotationDof_New(  MatrixAssemblyTerm_RotationDof_DEFARGS  );

	void _MatrixAssemblyTerm_RotationDof_Delete( void* matrixTerm );

  void AXequalsY( StiffnessMatrix* Amat, SolutionVector* x, SolutionVector* y, Bool transpose );
  PetscErrorCode AXequalsX( StiffnessMatrix* Amat, SolutionVector* x, Bool transpose );

	void* _MatrixAssemblyTerm_RotationDof_DefaultNew( Name name );

	void _MatrixAssemblyTerm_RotationDof_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _MatrixAssemblyTerm_RotationDof_Build( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_RotationDof_Initialise( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_RotationDof_Execute( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_RotationDof_Destroy( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_RotationDof_AssembleElement(
		void*							matrixTerm,
		StiffnessMatrix*			stiffnessMatrix,
		Element_LocalIndex		lElement_I,
		SystemLinearEquations*	sle,
		FiniteElementContext*	context,
		double**						elStiffMat ) ;

#ifdef __cplusplus
   }
   #endif
#endif
