/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>

#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>
#include <StgDomain/Mesh/src/Mesh.h>
#include <StgDomain/Utils/src/Utils.h>
#include <StgDomain/Swarm/src/Swarm.h>

#include <assert.h>
#include <string.h>

const Type StGermain_VaryingCornerAttractors_Type = "StGermain_VaryingCornerAttractors";

typedef struct {
	__Codelet
} StGermain_VaryingCornerAttractors;

struct _Particle {
	__GlobalParticle
	double  velocity[3];
	double  randomColour;
};

void StGermain_VaryingCornerAttractors_UpdatePositions( DomainContext* context ) {
	Cell_LocalIndex			lCell_I;
	Particle_InCellIndex		cParticle_I;
	Particle* 	        	currParticle;
	Index				dim_I;
	Swarm*                          swarm = (Swarm*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm"  );
	Coord                           attractorPoint;
	BlockGeometry*                  blockGeometry;
	Stream*                         stream = Journal_Register( Debug_Type, (Name)"particleUpdate" );
	unsigned int                    movementSpeedDivisor = 10;
	int                             movementSign = 1;
	unsigned int                    cornerPeriod = 10;
	unsigned int                    numCorners = (swarm->dim-1 )*4;
	unsigned int                    explosionPeriod = numCorners*cornerPeriod;
	Coord                           cornerCoords[8];
	int                             modValue = 0;
	int                             cornerIndex = 0;

	Stream_SetPrintingRank( stream, Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "procToWatch", 0 ) );
	
	blockGeometry = (BlockGeometry*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"geometry" );

		/* Bottom left corner */
	cornerCoords[0][I_AXIS] = blockGeometry->min[I_AXIS];
	cornerCoords[0][J_AXIS] = blockGeometry->min[J_AXIS];
	cornerCoords[0][K_AXIS] = blockGeometry->min[K_AXIS];
	/* Bottom right corner */
	cornerCoords[1][I_AXIS] = blockGeometry->max[I_AXIS];
	cornerCoords[1][J_AXIS] = blockGeometry->min[J_AXIS];
	cornerCoords[1][K_AXIS] = blockGeometry->min[K_AXIS];
	/* Top right corner */
	cornerCoords[2][I_AXIS] = blockGeometry->max[I_AXIS];
	cornerCoords[2][J_AXIS] = blockGeometry->max[J_AXIS];
	cornerCoords[2][K_AXIS] = blockGeometry->min[K_AXIS];
	/* Top left corner */
	cornerCoords[3][I_AXIS] = blockGeometry->min[I_AXIS];
	cornerCoords[3][J_AXIS] = blockGeometry->max[J_AXIS];
	cornerCoords[3][K_AXIS] = blockGeometry->min[K_AXIS];
	/* Bottom left corner */
	cornerCoords[4][I_AXIS] = blockGeometry->min[I_AXIS];
	cornerCoords[4][J_AXIS] = blockGeometry->max[J_AXIS];
	cornerCoords[4][K_AXIS] = blockGeometry->max[K_AXIS];
	/* Bottom right corner */
	cornerCoords[5][I_AXIS] = blockGeometry->min[I_AXIS];
	cornerCoords[5][J_AXIS] = blockGeometry->min[J_AXIS];
	cornerCoords[5][K_AXIS] = blockGeometry->max[K_AXIS];
	/* Top right corner */
	cornerCoords[6][I_AXIS] = blockGeometry->max[I_AXIS];
	cornerCoords[6][J_AXIS] = blockGeometry->min[J_AXIS];
	cornerCoords[6][K_AXIS] = blockGeometry->max[K_AXIS];
	/* Top left corner */
	cornerCoords[7][I_AXIS] = blockGeometry->max[I_AXIS];
	cornerCoords[7][J_AXIS] = blockGeometry->max[J_AXIS];
	cornerCoords[7][K_AXIS] = blockGeometry->max[K_AXIS];

	/* calculate which corner */
	modValue = (context->timeStep - 1) % (numCorners * cornerPeriod );
	cornerIndex = modValue / cornerPeriod;
	memcpy( attractorPoint, cornerCoords[cornerIndex], 3 * sizeof(double) );
	Journal_Printf( stream, "Calculated attractor point is at (%f,%f,%f):\n", attractorPoint[0], attractorPoint[1], attractorPoint[2] );
	
	/* Can't really explode in this test as particles go out of box */
	#if 0
	/* Now decide if we are attracting or repelling */
	if ( ( ( (context->timeStep - 1) / explosionPeriod ) % 2 ) == 0 ) {
		Journal_Printf( stream, "Timestep %d - Implosive mode\n", context->timeStep );
		movementSign = 1;
	}
	else {
		Journal_Printf( stream, "Timestep %d - Explosive mode\n", context->timeStep );
		movementSign = -1;
	}	
	#endif

	
	for ( lCell_I=0; lCell_I < swarm->cellLocalCount; lCell_I++ ) {
		Journal_Printf( stream, "\tUpdating Particles positions in local cell %d:\n", lCell_I );
		for ( cParticle_I=0; cParticle_I < swarm->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
			Coord movementVector = {0,0,0};
			Coord newParticleCoord = {0,0,0};
			Coord* oldCoord;

			currParticle = (Particle*)Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );
			oldCoord = &currParticle->coord;
			Journal_Printf( stream, "\t\tUpdating particleInCell %d:\n", cParticle_I );

			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				movementVector[dim_I] = ( attractorPoint[dim_I] - (*oldCoord)[dim_I] ) /
					movementSpeedDivisor;
				movementVector[dim_I] *= movementSign;	
				if ( movementSign == -1 ) {
					movementVector[dim_I] *= (float)movementSpeedDivisor / (movementSpeedDivisor-1); 
				}
				newParticleCoord[dim_I] = (*oldCoord)[dim_I] + movementVector[dim_I];
			}
			memcpy( currParticle->velocity, movementVector, 3*sizeof(double) ); 

			Journal_Printf( stream, "\t\tChanging its coords from (%f,%f,%f) to (%f,%f,%f):\n",
				(*oldCoord)[0], (*oldCoord)[1], (*oldCoord)[2],
				newParticleCoord[0], newParticleCoord[1], newParticleCoord[2] );

			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				currParticle->coord[dim_I] = newParticleCoord[dim_I];
			}
		}
	}

	Swarm_UpdateAllParticleOwners( swarm );
}


void _StGermain_VaryingCornerAttractors_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	DomainContext*   context;

	context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, True, data  );
	
	ContextEP_ReplaceAll( context, AbstractContext_EP_Solve, StGermain_VaryingCornerAttractors_UpdatePositions );
}


void* _StGermain_VaryingCornerAttractors_DefaultNew( Name name ) {
	return Codelet_New(
			StGermain_VaryingCornerAttractors_Type,
			_StGermain_VaryingCornerAttractors_DefaultNew,
			_StGermain_VaryingCornerAttractors_AssignFromXML,
			_Codelet_Build,
			_Codelet_Initialise,
			_Codelet_Execute,
			_Codelet_Destroy,
			name );
	}

Index StGermain_VaryingCornerAttractors_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, StGermain_VaryingCornerAttractors_Type, (Name)"0", _StGermain_VaryingCornerAttractors_DefaultNew  );
}


