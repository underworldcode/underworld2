/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/



#ifndef __StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_Timestep_h__
#define __StgFEM_SLE_ProvidedSystems_AdvectionDiffusion_Timestep_h__

double AdvectionDiffusionSLE_CalculateDt( void* advectionDiffusionSLE, FiniteElementContext* context ) ;
double AdvectionDiffusionSLE_DiffusiveTimestep( void* _advectionDiffusionSolver ) ;
double AdvectionDiffusionSLE_AdvectiveTimestep( void* _advectionDiffusionSolver ) ;
		
#endif
