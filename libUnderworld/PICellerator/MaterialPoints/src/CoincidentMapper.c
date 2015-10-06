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
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include <PICellerator/PopulationControl/PopulationControl.h>
#include <PICellerator/Weights/Weights.h>

#include "MaterialPoints.h"

#include <assert.h>
#include <string.h>
#include <math.h>

const Type CoincidentMapper_Type = "CoincidentMapper";

void* _CoincidentMapper_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                 _sizeOfSelf = sizeof(CoincidentMapper);
	Type                                                                         type = CoincidentMapper_Type;
	Stg_Class_DeleteFunction*                                                 _delete = _CoincidentMapper_Delete;
	Stg_Class_PrintFunction*                                                   _print = NULL;
	Stg_Class_CopyFunction*                                                     _copy = NULL;
	Stg_Component_DefaultConstructorFunction*                     _defaultConstructor = _CoincidentMapper_DefaultNew;
	Stg_Component_ConstructFunction*                                       _construct = _CoincidentMapper_AssignFromXML;
	Stg_Component_BuildFunction*                                               _build = _CoincidentMapper_Build;
	Stg_Component_InitialiseFunction*                                     _initialise = _CoincidentMapper_Initialise;
	Stg_Component_ExecuteFunction*                                           _execute = NULL;
	Stg_Component_DestroyFunction*                                           _destroy = NULL;
	AllocationType                                                 nameAllocationType = NON_GLOBAL;

	return _CoincidentMapper_New(  COINCIDENTMAPPER_PASSARGS  );
}

CoincidentMapper* _CoincidentMapper_New(  COINCIDENTMAPPER_DEFARGS  ) {
	return (CoincidentMapper*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
}

void _CoincidentMapper_AssignFromXML( void* mapper, Stg_ComponentFactory* cf, void* data ) {
	CoincidentMapper* self = (CoincidentMapper*)mapper;

	self->integrationSwarm = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)IntegrationPointsSwarm_Type, IntegrationPointsSwarm, True, data  );
    self->materialSwarm    = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)GeneralSwarm_Type, GeneralSwarm, True, data  );
    
    self->integrationSwarm->mirroredSwarm = self->materialSwarm;
    self->materialSwarm->mirroredSwarm    = self->integrationSwarm;
}

void _CoincidentMapper_Delete( void* mapper ) {
	CoincidentMapper* self = (CoincidentMapper*)mapper;

	_Stg_Component_Delete( self );
}

void _CoincidentMapper_Build( void* mapper, void* cf ) {
	CoincidentMapper* self = (CoincidentMapper*)mapper;
	Stg_Component_Build( self->materialSwarm,    NULL, False );
	Stg_Component_Build( self->integrationSwarm, NULL, False );
}

void _CoincidentMapper_Initialise( void* mapper, void* cf ) {
	CoincidentMapper* self = (CoincidentMapper*)mapper;
	Stg_Component_Initialise( self->materialSwarm,    NULL, False );
	Stg_Component_Initialise( self->integrationSwarm, NULL, False );
}

void _CoincidentMapper_Map( void* mapper ) {
	CoincidentMapper*			self = (CoincidentMapper*)mapper;
	IntegrationPointsSwarm*	    integrationSwarm = self->integrationSwarm;
	GeneralSwarm*		        materialSwarm = self->materialSwarm;
	IntegrationPoint*			integrationPoint;
	GlobalParticle*				materialPoint;
	FeMesh*						mesh = integrationSwarm->mesh;
	Particle_Index				particle_lI;
	Cell_Index					cell_dI;

	integrationSwarm->particleLocalCount = materialSwarm->particleLocalCount;
	Swarm_Realloc( integrationSwarm );

	for( cell_dI = 0; cell_dI < integrationSwarm->cellDomainCount; cell_dI++ ) {
		integrationSwarm->cellParticleCountTbl[cell_dI] = 0;
		integrationSwarm->cellParticleSizeTbl[cell_dI] = 0;

		if ( integrationSwarm->cellParticleTbl[cell_dI] ) {
			Memory_Free( integrationSwarm->cellParticleTbl[cell_dI] );
		}
		integrationSwarm->cellParticleTbl[cell_dI] = NULL;
	}

	/* Map each point */
	for ( particle_lI = 0; particle_lI < materialSwarm->particleLocalCount; particle_lI++ ) {
		integrationPoint = (IntegrationPoint*)Swarm_ParticleAt( integrationSwarm, particle_lI );
		materialPoint = (GlobalParticle*)Swarm_ParticleAt( materialSwarm, particle_lI );

		cell_dI = materialPoint->owningCell;

        Journal_Firewall( cell_dI<materialSwarm->cellDomainCount, NULL,
            "Error - in %s(): particle %u appears to be outside the domain.",
            __func__, particle_lI );

		Swarm_AddParticleToCell( integrationSwarm, cell_dI, particle_lI );

		/* Convert global to local coordinates */
		ElementType_ConvertGlobalCoordToElLocal(
			FeMesh_GetElementType( mesh, cell_dI ),
			mesh, 
			cell_dI, 
			materialPoint->coord,
			integrationPoint->xi );

#ifdef DEBUG
		/* Check the result is between -1 to 1 in all dimensions : if not, something is stuffed */
        Index dim_I;
		for ( dim_I= 0; dim_I < materialSwarm->dim; dim_I++ ) {
			Journal_Firewall(
				(integrationPoint->xi[dim_I] >= -1.001) && (integrationPoint->xi[dim_I] <= 1.001 ),
				NULL,
				"Error - in %s(): unable to map swarm particle %d in cell %d of swarm \"%s\" (type %s) "
				"coord to a valid element local coordinate.\n"
                "Particle coord was (%.3f,%.3f,%.3f).\n"
				"Conversion to local coord was (%.4f,%.4f,%.4f).\n"
                "Note that this error can occur when the mesh is overly deformed.",
				__func__, particle_lI, cell_dI, materialSwarm->name, materialSwarm->type,
				materialPoint->coord[0], materialPoint->coord[1], materialPoint->coord[2],
                integrationPoint->xi[0], integrationPoint->xi[1], integrationPoint->xi[2] );
		}
#endif

    }
}


