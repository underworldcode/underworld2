/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_AdvectionDiffusion_AdvectionDiffusionSLE_h__
#define __Underworld_AdvectionDiffusion_AdvectionDiffusionSLE_h__

#ifdef __cplusplus

extern "C++" {


#include <Underworld/Function/src/Function.hpp>
#include <Underworld/Function/src/FEMCoordinate.hpp>

    struct SUPGVectorTerm_NA__Fn_cppdata
    {
        Fn::Function* fn;
        Fn::Function::func func;
        std::shared_ptr<FEMCoordinate> input;
    };

}

extern "C" {
#endif

#include "mpi.h"
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>

/*
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"
*/

#include "types.h"
#include "Residual.h"
#include "Multicorrector.h"
#include "Timestep.h"

	extern const Type AdvectionDiffusionSLE_Type;

	#define __AdvectionDiffusionSLE \
		__SystemLinearEquations \
		/* Items passed into constructor */ \
		FeVariable*						phiField; \
		ForceVector*					residual; \
		Stg_Component*					massMatrix; \
		Dimension_Index				dim; \
		/* Items Created By solver */ \
		SolutionVector*				phiVector; /* should be passed in */ \
		FeVariable*						phiDotField; \
		DofLayout*						phiDotDofLayout; \
		SolutionVector*				phiDotVector; \
		/* pointer to force term which must always be given */ \
		AdvDiffResidualForceTerm*	advDiffResidualForceTerm; \
		/* Timestep Stuff */ \
		double							currentDt; \
		double							courantFactor; \
		double							maxDiffusivity; \
		\
		Variable_Register*			variableReg; \
		FieldVariable_Register*		fieldVariableReg;\
                                                                 \
                Bool pureDiffusion; \
                void *cppdata;

	struct AdvectionDiffusionSLE { __AdvectionDiffusionSLE };

	AdvectionDiffusionSLE* AdvectionDiffusionSLE_New(
		Name							name,
		FiniteElementContext*	context,
		SLE_Solver*					solver,
		Bool							isNonLinear,
		double						nonLinearTolerance,
		Iteration_Index			nonLinearMaxIterations,
		Bool							killNonConvergent,
		EntryPoint_Register*		entryPoint_Register,
		MPI_Comm						comm,
		FeVariable*					phiField,
		ForceVector*				residual,
		Stg_Component*				massMatrix,
		Dimension_Index			dim,
		double						courantFactor,
		Variable_Register*		variable_Register,
		FieldVariable_Register*	fieldVariable_Register ) ;


	#define ADVECTIONDIFFUSIONSLE_DEFARGS \
                SYSTEMLINEAREQUATIONS_DEFARGS

	#define ADVECTIONDIFFUSIONSLE_PASSARGS \
                SYSTEMLINEAREQUATIONS_PASSARGS

	AdvectionDiffusionSLE* _AdvectionDiffusionSLE_New(  ADVECTIONDIFFUSIONSLE_DEFARGS  );

	void _AdvectionDiffusionSLE_Init(
		void*							sle,
		FeVariable*					phiField,
		ForceVector*				residual,
		Stg_Component*				massMatrix,
		Dimension_Index			dim,
		double						courantFactor,
		Variable_Register*		variable_Register,
		FieldVariable_Register*	fieldVariable_Register );

	void __AdvDiffResidualForceTerm_UpdateLocalMemory( AdvectionDiffusionSLE* sle );

	/** Virtual Functions from "Class" Class */
	void _AdvectionDiffusionSLE_Delete( void* sle );

	void _AdvectionDiffusionSLE_Print( void* sle, Stream* stream );

	/** Virtual Functions from "Stg_Component" Class */
	void* _AdvectionDiffusionSLE_DefaultNew( Name name );

	void _AdvectionDiffusionSLE_AssignFromXML( void* sle, Stg_ComponentFactory* cf, void* data );

	void _AdvectionDiffusionSLE_Build( void* sle, void* data );

	void _AdvectionDiffusionSLE_Initialise( void* sle, void* data );

	void _AdvectionDiffusionSLE_Execute( void* sle, void* _context );

	void _AdvectionDiffusionSLE_Destroy( void* sle, void* _context );

	//Vector* _AdvectionDiffusionSLE_GetResidual( void* sle, Index fv_I );
	Vec _AdvectionDiffusionSLE_GetResidual( void* sle, Index fv_I );

	void AdvectionDiffusionSLE_ResetStoredValues( void* sle );

#ifdef __cplusplus
}
#endif

#endif
