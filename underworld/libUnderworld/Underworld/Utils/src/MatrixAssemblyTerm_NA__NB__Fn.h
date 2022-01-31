/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_MatrixAssemblyTerm_NA__NB__Fn_h__
#define __Underworld_Utils_MatrixAssemblyTerm_NA__NB__Fn_h__

#ifdef __cplusplus

   extern "C++" {

      #include <Underworld/Function/src/Function.hpp>
      #include <Underworld/Function/src/FEMCoordinate.hpp>

   struct MatrixAssemblyTerm_NA__NB__Fn_cppdata
   {
       Fn::Function* fn;
       Fn::Function::func func;
       std::shared_ptr<FEMCoordinate> input;
   };

   void MatrixAssemblyTerm_NA__NB__Fn_SetFn( void* _self, Fn::Function* fn );

   }

   extern "C" {
#endif

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
	/** Textual name of this class */
	extern const Type MatrixAssemblyTerm_NA__NB__Fn_Type;

	/** MatrixAssemblyTerm_NA__NB__Fn class contents */
	#define __MatrixAssemblyTerm_NA__NB__Fn \
		/* General info */ \
		__StiffnessMatrixTerm \
		/* Virtual info */ \
		/* MatrixAssemblyTerm_NA__NB__Fn info */ \
                void*   cppdata;   \
		FeMesh* geometryMesh; \
		int	max_nElNodes_col; \
                int     max_nElNodes_row; \
		double	*Ni;  \
		double	*Mi;

	struct MatrixAssemblyTerm_NA__NB__Fn { __MatrixAssemblyTerm_NA__NB__Fn };


	#ifndef ZERO
   	#define ZERO 0
	#endif

	#define MATRIXASSEMBLYTERM_NA__NB__FN_DEFARGS \
                STIFFNESSMATRIXTERM_DEFARGS

	#define MATRIXASSEMBLYTERM_NA__NB__FN_PASSARGS \
                STIFFNESSMATRIXTERM_PASSARGS

	MatrixAssemblyTerm_NA__NB__Fn* _MatrixAssemblyTerm_NA__NB__Fn_New(  MATRIXASSEMBLYTERM_NA__NB__FN_DEFARGS  );

	void _MatrixAssemblyTerm_NA__NB__Fn_Delete( void* matrixTerm );

	void _MatrixAssemblyTerm_NA__NB__Fn_Print( void* matrixTerm, Stream* stream );

	void* _MatrixAssemblyTerm_NA__NB__Fn_DefaultNew( Name name );

	void _MatrixAssemblyTerm_NA__NB__Fn_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data );

	void _MatrixAssemblyTerm_NA__NB__Fn_Build( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_NA__NB__Fn_Initialise( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_NA__NB__Fn_Execute( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_NA__NB__Fn_Destroy( void* matrixTerm, void* data );

	void _MatrixAssemblyTerm_NA__NB__Fn_AssembleElement(
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
