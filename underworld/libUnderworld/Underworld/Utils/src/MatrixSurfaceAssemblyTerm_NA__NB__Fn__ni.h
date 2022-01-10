/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_h__
#define __Underworld_Utils_MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_h__

#ifdef __cplusplus

   extern "C++" {

   #include <Underworld/Function/src/Function.hpp>
   #include <Underworld/Function/src/FEMCoordinate.hpp>


   struct MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_cppdata
   {
       Fn::Function* fn;
       Fn::Function::func func;
       std::shared_ptr<FEMCoordinate> input;
   };

   void MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_SetFn( void* _self, Fn::Function* fn );

   }

   extern "C" {
#endif

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
	/** Textual name of this class */
	extern const Type MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Type;

	/** MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni class contents */
	#define __MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni \
    /* General info */ \
    __StiffnessMatrixTerm \
    /* Virtual info */ \
    /* MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni info */ \
    void*   cppdata;   \
    FeMesh* geometryMesh; \
    double	*Ni;  \

	struct MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni { __MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni };


	#ifndef ZERO
   	#define ZERO 0
	#endif

	#define MATRIXASSEMBLYTERM_NA__NB__FN_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define MATRIXASSEMBLYTERM_NA__NB__FN_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni* _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_New(  MATRIXASSEMBLYTERM_NA__NB__FN_DEFARGS  );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Delete( void* matrixTerm );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Print( void* matrixTerm, Stream* stream );

	void* _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_DefaultNew( Name name );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Build( void* matrixTerm, void* data );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Initialise( void* matrixTerm, void* data );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Execute( void* matrixTerm, void* data );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_Destroy( void* matrixTerm, void* data );

	void _MatrixSurfaceAssemblyTerm_NA__NB__Fn__ni_AssembleElement(
		void*                   matrixTerm,
		StiffnessMatrix*        stiffnessMatrix,
		Element_LocalIndex      lElement_I,
		SystemLinearEquations*  sle,
		FiniteElementContext*   context,
		double**                elStiffMat ) ;
    PetscErrorCode AXequalsX( StiffnessMatrix* a, SolutionVector* x, Bool transpose );

#ifdef __cplusplus
   }
   #endif
#endif
