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
#include <math.h>

#define PI 3.1415926535897931

const Type StGermain_Spinner_Type = "StGermain_Spinner";

typedef struct {
	__Codelet
} StGermain_Spinner;

struct _Particle {
	__GlobalParticle
	double  velocity[3];
	double  randomColour;
};

void StGermain_Spinner_UpdatePositions( DomainContext* context ) {
	Cell_LocalIndex			lCell_I;
	Particle_InCellIndex		cParticle_I;
	Particle* 	        	currParticle;
	Index				dim_I;
	Swarm*                          swarm = (Swarm*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm"  );
	BlockGeometry*                  blockGeom;
	Stream*                         stream = Journal_Register( Debug_Type, (Name)"particleUpdate"  );
	unsigned int                    movementSpeedDivisor = 100;
	double                          x, y;
	Bool                            reverseMode = False;
	Index                           reverseTimeStep;
	
	reverseTimeStep = Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "reverseTimeStep", 100 );
	if ( context->timeStep >= reverseTimeStep ) {
		Journal_Printf( stream, "Current timestep %d >= reverse step -> reversing flow field.\n",
			context->timeStep );
		reverseMode = True;	
	}	

	Stream_SetPrintingRank( stream, Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "procToWatch", 0 ) );
	
	blockGeom = (BlockGeometry*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"geometry" );

	for ( lCell_I=0; lCell_I < swarm->cellLocalCount; lCell_I++  ) {
		Journal_Printf( stream, "\tUpdating Particles positions in local cell %d:\n", lCell_I );
		for ( cParticle_I=0; cParticle_I < swarm->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
			Coord movementVector = {0,0,0};
			Coord newParticleCoord = {0,0,0};
			Coord* oldCoord;

			currParticle = (Particle*)Swarm_ParticleInCellAt( swarm, lCell_I, cParticle_I );
			oldCoord = &currParticle->coord;
			Journal_Printf( stream, "\t\tUpdating particleInCell %d:\n", cParticle_I );

			x = currParticle->coord[0];
			y = currParticle->coord[1];
			movementVector[0] = 2 * -1 * pow( sin( PI * x ), 2 ) * sin( PI * y ) * cos( PI * y );
			movementVector[1] = 2 *  1 * pow( sin( PI * y ), 2 ) * sin( PI * x ) * cos( PI * x );
			movementVector[2] = 0;

			if ( reverseMode ) {
				movementVector[0] *= -1;
				movementVector[1] *= -1;
			}
			
			for ( dim_I=0; dim_I < 3; dim_I++ ) {
				movementVector[dim_I] /= movementSpeedDivisor;
				currParticle->velocity[dim_I] = movementVector[dim_I];
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


void _StGermain_Spinner_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	DomainContext*   context;

	context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, True, data  );
	
	ContextEP_ReplaceAll( context, AbstractContext_EP_Solve, StGermain_Spinner_UpdatePositions );
}


void* _StGermain_Spinner_DefaultNew( Name name ) {
	return Codelet_New(
			StGermain_Spinner_Type,
			_StGermain_Spinner_DefaultNew,
			_StGermain_Spinner_AssignFromXML,
			_Codelet_Build,
			_Codelet_Initialise,
			_Codelet_Execute,
			_Codelet_Destroy,
			name );
	}

Index StGermain_Spinner_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, StGermain_Spinner_Type, (Name)"0", _StGermain_Spinner_DefaultNew  );
}


