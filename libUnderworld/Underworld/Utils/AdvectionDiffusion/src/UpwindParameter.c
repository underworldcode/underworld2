/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <math.h>
#include <assert.h>

#include "mpi.h"
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SystemSetup/SystemSetup.h"

#include "types.h"
#include "AdvectionDiffusionSLE.h"
#include "UpwindParameter.h"
#include "Residual.h"

#define ISQRT15 0.25819888974716112567

/** AdvectionDiffusion_UpwindDiffusivity - See Brooks, Hughes 1982 Section 3.3 
 * All equations refer to this paper if not otherwise indicated */
double AdvDiffResidualForceTerm_UpwindDiffusivity( 
		AdvDiffResidualForceTerm* self, 
		AdvectionDiffusionSLE* sle, 
		Swarm* swarm, 
		FeMesh* mesh, 
		Element_LocalIndex lElement_I, 
		Dimension_Index dim )
{
	FeVariable*                velocityField   = self->velocityField;
	Coord                      xiElementCentre = {0.0,0.0,0.0};
	double                     xiUpwind;
	double                     velocityCentre[3];
	double                     pecletNumber;
	double                     lengthScale;
	double                     upwindDiffusivity;
	Dimension_Index            dim_I;
	double*                    leastCoord;
	double*                    greatestCoord;
	Node_LocalIndex            nodeIndex_LeastValues, nodeIndex_GreatestValues;
	unsigned                   nInc, *inc;
	IArray*		 incArray;
	
	Cell_Index                 cell_I;
	IntegrationPoint*          particle;
	Variable*                  diffusivityVariable = self->diffusivityVariable;
	Particle_Index             lParticle_I;
	double                     averageDiffusivity;
	Particle_InCellIndex       cParticle_I;
	Particle_InCellIndex       particleCount;

	
	/* Compute the average diffusivity */
	/* Find Number of Particles in Element */
	cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
	particleCount = swarm->cellParticleCountTbl[ cell_I ];

	/* Average diffusivity for element */
	if ( diffusivityVariable ) {
		averageDiffusivity = 0.0;
		for ( cParticle_I = 0 ; cParticle_I < particleCount ; cParticle_I++ ) {
			lParticle_I = swarm->cellParticleTbl[lElement_I][cParticle_I];
			particle    = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
			averageDiffusivity += self->_getDiffusivityFromIntPoint( self, particle );
		}
		averageDiffusivity /= (double)particleCount;
	}
	else {
		averageDiffusivity = self->defaultDiffusivity;
	}
	
	if (sle->maxDiffusivity < averageDiffusivity)
		sle->maxDiffusivity = averageDiffusivity;
	
	
	
	
	
	
	/* Change Diffusivity if it is too small */
	if ( averageDiffusivity < SUPG_MIN_DIFFUSIVITY ) 
		averageDiffusivity = SUPG_MIN_DIFFUSIVITY;
	
	/* Calculate Velocity At Middle of Element - See Eq. 3.3.6 */
	FeVariable_InterpolateFromMeshLocalCoord( velocityField, mesh, lElement_I, xiElementCentre, velocityCentre );
	
	/* Calculate Length Scales - See Fig 3.4 - ASSUMES BOX MESH TODO - fix */
	incArray = self->incarray;
	FeMesh_GetElementNodes( mesh, lElement_I, incArray );
	nInc = IArray_GetSize( incArray );
	inc = IArray_GetPtr( incArray );
	
	
	nodeIndex_LeastValues = inc[0];
	nodeIndex_GreatestValues = (dim == 2) ? inc[3] : (dim == 3) ? inc[7] : inc[1];
	leastCoord    = Mesh_GetVertex( mesh, nodeIndex_LeastValues );
	greatestCoord = Mesh_GetVertex( mesh, nodeIndex_GreatestValues );
	
	upwindDiffusivity = 0.0;
	for ( dim_I = 0 ; dim_I < dim ; dim_I++ ) {
		lengthScale = fabs(greatestCoord[ dim_I ] - leastCoord[ dim_I ]);
		
		/* Calculate Peclet Number (alpha) - See Eq. 3.3.5 */
		pecletNumber = velocityCentre[ dim_I ] * lengthScale / (2.0 * averageDiffusivity);
		
		/* Calculate Upwind Local Coordinate - See Eq. 3.3.4 and (2.4.2, 3.3.1 and 3.3.2) */
		xiUpwind = AdvDiffResidualForceTerm_UpwindParam( self, pecletNumber );
		
		/* Calculate Upwind Thermal Diffusivity - See Eq. 3.3.3  */
		upwindDiffusivity += xiUpwind * velocityCentre[ dim_I ] * lengthScale;
	}
	upwindDiffusivity *= ISQRT15;         /* See Eq. 3.3.11 */
	
	
	return upwindDiffusivity;
}


/** AdvectionDiffusion_UpwindXiExact - Brooks, Hughes 1982 equation 2.4.2
 *\f$ \bar \xi = coth( \alpha ) - \frac{1}{\alpha} \f$ */
double AdvDiffResidualForceTerm_UpwindXiExact( void* residual, double pecletNumber ) {
	if (fabs(pecletNumber) < 1.0e-8 )
		return 0.33333333333333 * pecletNumber;
	else if (pecletNumber < -20.0)
		return -1.0 - 1.0/pecletNumber;
	else if (pecletNumber > 20.0)
		return +1.0 - 1.0/pecletNumber;
		
	return cosh( pecletNumber )/sinh( pecletNumber ) - 1.0/pecletNumber;
}

/** AdvectionDiffusion_UpwindXiDoublyAsymptoticAssumption - Brooks, Hughes 1982 equation 3.3.1
 * Simplification of \f$ \bar \xi = coth( \alpha ) - \frac{1}{\alpha} \f$ from Brooks, Hughes 1982 equation 2.4.2
 * \f[
\bar \xi \sim \left\{ \begin{array}{rl} 
             -1                 &for \quad \alpha <= -3 \\
             \frac{\alpha}{3}   &for \quad -3 < \alpha <= 3 \\
             +1                 &for \quad \alpha > +3
             \end{array} \right.  
           
\f]*/
double AdvDiffResidualForceTerm_UpwindXiDoublyAsymptoticAssumption( void* residual, double pecletNumber ) {
	if (pecletNumber <= -3.0)
		return -1;
	else if (pecletNumber <= 3.0)
		return 0.33333333333333 * pecletNumber;
	else
		return 1.0;
}
	
/** AdvectionDiffusion_UpwindXiCriticalAssumption - Brooks, Hughes 1982 equation 3.3.2
 * Simplification of \f$ \bar \xi = coth( \alpha ) - \frac{1}{\alpha} \f$ from Brooks, Hughes 1982 equation 2.4.2
 * \f[
  \bar \xi \sim \left\{ \begin{array}{rl}
              -1 - \frac{1}{\alpha}   &for \quad \alpha <= -1 \\
               0                      &for \quad -1 < \alpha <= +1 \\
              +1 - \frac{1}{\alpha}   &for \quad \alpha > +1          
              \end{array} \right.              
\f]    */

double AdvDiffResidualForceTerm_UpwindXiCriticalAssumption( void* residual, double pecletNumber ) {
	if (pecletNumber <= -1.0)
		return -1.0 - 1.0/pecletNumber;
	else if (pecletNumber <= 1.0)
		return 0.0;
	else
		return 1.0 - 1.0/pecletNumber;
}
	


