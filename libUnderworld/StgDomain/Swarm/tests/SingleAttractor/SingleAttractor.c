/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>
#include <StgDomain/Utils/Utils.h>
#include <StgDomain/Swarm/Swarm.h>

#include <assert.h>
#include <string.h>

const Type StGermain_SingleAttractor_Type = "StGermain_SingleAttractor";

typedef struct {
	__Codelet
} StGermain_SingleAttractor;

struct _Particle {
	__GlobalParticle
	double  velocity[3];
	double  randomColour;
};

void StGermain_SingleAttractor_UpdatePositions( DomainContext* context ) {
	Cell_LocalIndex			lCell_I;
	Particle_InCellIndex		cParticle_I;
	Particle* 	        	currParticle;
	Index				dim_I;
	Swarm*                          swarm = (Swarm*) LiveComponentRegister_Get( context->CF->LCRegister, (Name)"swarm"  );
	Coord                           attractorPoint;
	Mesh*				mesh;
	Stream*                         stream = Journal_Register( Info_Type, (Name)"particleUpdate"  );
	unsigned int                    movementSpeedDivisor = 0;
	int                             movementSign = 1;
	unsigned int                    explosionPeriod = 20;
	double				minCrd[3], maxCrd[3];

	Stream_SetPrintingRank( stream, Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "procToWatch", 0 ) );
	movementSpeedDivisor = Dictionary_GetDouble_WithDefault( context->dictionary, (Dictionary_Entry_Key)"movementSpeedDivisor", 10 );
	
	mesh = (Mesh* )LiveComponentRegister_Get( context->CF->LCRegister, (Name)"mesh-linear"  );
	Mesh_GetGlobalCoordRange( mesh, minCrd, maxCrd );
	for ( dim_I=0; dim_I < 3; dim_I++ ) {
		attractorPoint[dim_I] = (maxCrd[dim_I] - minCrd[dim_I]) / 3;
	}
	Journal_Printf( stream, "Calculated attractor point is at (%f,%f,%f):\n", attractorPoint[0], attractorPoint[1], attractorPoint[2] );
	
	/* Now decide if we are attracting or repelling */
	if ( ( ( (context->timeStep - 1) / explosionPeriod ) % 2 ) == 0 ) {
		Journal_Printf( stream, "Timestep %d - Implosive mode\n", context->timeStep );
		movementSign = 1;
	}
	else {
		Journal_Printf( stream, "Timestep %d - Explosive mode\n", context->timeStep );
		movementSign = -1;
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


void _StGermain_SingleAttractor_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	DomainContext*   context;

	context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, True, data  );
	Stream_SetPrintingRank( 
		Journal_Register( Info_Type, (Name)"Context"  ),
		Dictionary_GetUnsignedInt_WithDefault( context->dictionary, "procToWatch", 0 ) );
	
	ContextEP_ReplaceAll( context, AbstractContext_EP_Solve, StGermain_SingleAttractor_UpdatePositions );
}


void* _StGermain_SingleAttractor_DefaultNew( Name name ) {
	return Codelet_New(
			StGermain_SingleAttractor_Type,
			_StGermain_SingleAttractor_DefaultNew,
			_StGermain_SingleAttractor_AssignFromXML,
			_Codelet_Build,
			_Codelet_Initialise,
			_Codelet_Execute,
			_Codelet_Destroy,
			name );
	}

Index StGermain_SingleAttractor_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, StGermain_SingleAttractor_Type, (Name)"0", _StGermain_SingleAttractor_DefaultNew  );
}


