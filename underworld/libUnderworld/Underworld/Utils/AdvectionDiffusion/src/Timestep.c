/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include "mpi.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include "types.h"
#include "Timestep.h"

#include "AdvectionDiffusionSLE.h"
#include "Residual.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <float.h>


double AdvectionDiffusionSLE_CalculateDt( void* advectionDiffusionSLE, FiniteElementContext* context ) {
	AdvectionDiffusionSLE*     self   = (AdvectionDiffusionSLE*) advectionDiffusionSLE;
	double                     advectionTimestep;
	double                     diffusionTimestep;
	double                     advectionTimestepGlobal;
	double                     diffusionTimestepGlobal;
	double                     timestep;
	
	Journal_DPrintf( self->debug, "In func: %s\n", __func__ );
	
	/*  It would be useful to introduce a limit to the change of step in here ... to prevent timesteps
		from becoming arbitrarily large in a single step */ 
	
	/* Calculate Courant Number */
        if(self->pureDiffusion)
            advectionTimestep = DBL_MAX;
        else
            advectionTimestep = AdvectionDiffusionSLE_AdvectiveTimestep( self );
	diffusionTimestep = AdvectionDiffusionSLE_DiffusiveTimestep( self );

	(void)MPI_Allreduce( &advectionTimestep, &advectionTimestepGlobal, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );
	(void)MPI_Allreduce( &diffusionTimestep, &diffusionTimestepGlobal, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD );

	Journal_DPrintf( self->debug, "%s Dominating. - Advective Timestep = %g - Diffusive Timestep = %g\n", 
			advectionTimestepGlobal < diffusionTimestepGlobal ? "Advection" : "Diffusion",
			advectionTimestepGlobal, diffusionTimestepGlobal);
	
	/* Calculate Time Step */
	timestep = MIN( advectionTimestepGlobal, diffusionTimestepGlobal );

	return timestep;
}


double AdvectionDiffusionSLE_DiffusiveTimestep( void* advectionDiffusionSLE ) {
	AdvectionDiffusionSLE*    self              = (AdvectionDiffusionSLE*) advectionDiffusionSLE;
	double                    minSeparation;
	double                    minSeparationEachDim[3];

	Journal_DPrintf( self->debug, "In func: %s\n", __func__ );
	
	FeVariable_GetMinimumSeparation( self->phiField, &minSeparation, minSeparationEachDim );

        self->maxDiffusivity = AdvDiffResidualForceTerm_GetMaxDiffusivity( self->advDiffResidualForceTerm);
	/* This is quite a conservative estimate */

	return self->courantFactor * minSeparation * minSeparation / self->maxDiffusivity;
}


double AdvectionDiffusionSLE_AdvectiveTimestep( void* advectionDiffusionSLE ) {
	AdvectionDiffusionSLE*    self              = (AdvectionDiffusionSLE*) advectionDiffusionSLE;
	AdvDiffResidualForceTerm* residualForceTerm = self->advDiffResidualForceTerm;
	FeVariable*               velocityField     = residualForceTerm->velocityField;
	Node_LocalIndex           nodeLocalCount    = FeMesh_GetNodeLocalSize( self->phiField->feMesh );
	Node_LocalIndex           node_I;
	Dimension_Index           dim               = self->dim;
	Dimension_Index           dim_I;
	double                    timestep          = HUGE_VAL;
	XYZ                       velocity;
	double                    minSeparation;
	double                    minSeparationEachDim[3];
	double*                   meshCoord;
	
	Journal_DPrintf( self->debug, "In func: %s\n", __func__ );

	FeVariable_GetMinimumSeparation( self->phiField, &minSeparation, minSeparationEachDim );

	for( node_I = 0 ; node_I < nodeLocalCount ; node_I++ ){
		if( self->phiField->feMesh == velocityField->feMesh){
		   FeVariable_GetValueAtNode( velocityField, node_I, velocity );
		} else {
         meshCoord = Mesh_GetVertex( self->phiField->feMesh, node_I );
         FieldVariable_InterpolateValueAt( velocityField, meshCoord, velocity );
      }
		
		for ( dim_I = 0 ; dim_I < dim ; dim_I++ ) {
			if( velocity[ dim_I ] == 0.0 ) 
				continue;
			timestep = MIN( timestep, fabs( minSeparationEachDim[ dim_I ]/velocity[ dim_I ] ) );
		}
	}

	return self->courantFactor * timestep;
}


