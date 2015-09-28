/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_types_h__
#define __StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_types_h__

	/* Classes */
	typedef	struct AdvectionDiffusionSLE         AdvectionDiffusionSLE; 
	typedef	struct AdvDiffResidualForceTerm      AdvDiffResidualForceTerm; 
	typedef	struct LumpedMassMatrixForceTerm     LumpedMassMatrixForceTerm; 
	typedef	struct AdvDiffMulticorrector         AdvDiffMulticorrector; 

	typedef enum { Exact, DoublyAsymptoticAssumption, CriticalAssumption } AdvDiffResidualForceTerm_UpwindParamFuncType;

#endif
