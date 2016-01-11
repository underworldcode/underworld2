/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_AdvectionDiffusion_UpwindParameter_h__
#define __Underworld_AdvectionDiffusion_UpwindParameter_h__

#define SUPG_MIN_DIFFUSIVITY 1.0e-20


        double AdvDiffResidualForceTerm_UpwindDiffusivity( 
		AdvDiffResidualForceTerm* self, 
		AdvectionDiffusionSLE* sle, 
		Swarm* swarm, 
		FeMesh* mesh, 
		Element_LocalIndex lElement_I, 
		Dimension_Index dim );

	double AdvDiffResidualForceTerm_UpwindXiExact( void* residual, double pecletNumber ) ;
	double AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption( void* residual, double pecletNumber ) ;
	double AdvDiffResidualForceTerm_UpwindXiCriticalAssumption( void* residual, double pecletNumber ) ;

#endif
