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

const Type StGermain_Bouncer_Type = "StGermain_Bouncer";

typedef struct {
	__Codelet
} StGermain_Bouncer;

struct _Particle {
	__GlobalParticle
	double  velocity[3];
	double  randomColour;
};

void StGermain_Bouncer_UpdatePositions( DomainContext* context ) {
	Cell_LocalIndex			lCell_I;
	Particle_InCellIndex		cParticle_I;
	Particle* 	        	currParticle;
	Index				dim_I;
	Swarm*                          swarm = (Swarm*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm"  );
	BlockGeometry*                  blockGeom;
	Stream*                         stream = Journal_Register( Debug_Type, (Name)"particleUpdate"  );
	unsigned int                    movementSpeedDivisor = 5;
	Particle_Index                  lParticle_I = 0;

	Stream_SetPrintingRank( stream, Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "procToWatch", 0 ) );
	
	blockGeom = (BlockGeometry*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"geometry" );

	if ( context->timeStep == 1  ) {
		/* for each particle, set a random velocity */
		for ( lParticle_I=0; lParticle_I < swarm->particleLocalCount; lParticle_I++ ) {
			currParticle = (Particle*)Swarm_ParticleAt( swarm, lParticle_I );
			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				currParticle->velocity[dim_I] = ((double) ( rand() - (double)(RAND_MAX)/2 )) / RAND_MAX * 0.1;
			}	
		}
	}
	
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
				movementVector[dim_I] = currParticle->velocity[dim_I] / movementSpeedDivisor;
				if ( ( currParticle->velocity[dim_I] < 0 ) 
					&& ( fabs(currParticle->velocity[dim_I] ) > ((*oldCoord)[dim_I] - blockGeom->min[dim_I]) ) )
				{
					Journal_Printf( stream, "\t\tFlipping vel in %d dir\n", dim_I );
					movementVector[dim_I] *= -1;
					currParticle->velocity[dim_I] *= -1;
				}	
				if ( ( currParticle->velocity[dim_I] > 0 ) 
					&& ( fabs(currParticle->velocity[dim_I] ) > ( blockGeom->max[dim_I] - (*oldCoord)[dim_I] ) ) )
				{
					Journal_Printf( stream, "\t\tFlipping vel in %d dir\n", dim_I );
					movementVector[dim_I] *= -1;
					currParticle->velocity[dim_I] *= -1;
				}	
				
				newParticleCoord[dim_I] = (*oldCoord)[dim_I] + movementVector[dim_I];
			}

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


void _StGermain_Bouncer_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	DomainContext*   context;

	context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, True, data  );
	
	ContextEP_ReplaceAll( context, AbstractContext_EP_Solve, StGermain_Bouncer_UpdatePositions );
}


void* _StGermain_Bouncer_DefaultNew( Name name ) {
	return Codelet_New(
			StGermain_Bouncer_Type,
			_StGermain_Bouncer_DefaultNew,
			_StGermain_Bouncer_AssignFromXML,
			_Codelet_Build,
			_Codelet_Initialise,
			_Codelet_Execute,
			_Codelet_Destroy,
			name );
	}

Index StGermain_Bouncer_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, StGermain_Bouncer_Type, (Name)"0", _StGermain_Bouncer_DefaultNew  );
}


