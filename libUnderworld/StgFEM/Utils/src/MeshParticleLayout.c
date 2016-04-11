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
#include <StgFEM/Discretisation/Discretisation.h>

#include "types.h"
#include "MeshParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const Type MeshParticleLayout_Type = "MeshParticleLayout";


MeshParticleLayout* MeshParticleLayout_New( 
   Name                 name, 
   AbstractContext* context,
   CoordSystem      coordSystem,
   Bool             weightsInitialisedAtStartup,
   Mesh*            mesh,
   Particle_InCellIndex cellParticleCount, 
   unsigned int         seed,
   unsigned int         filltype )
{
	MeshParticleLayout* self = (MeshParticleLayout*) _MeshParticleLayout_DefaultNew( name );

   _ParticleLayout_Init( self, context, coordSystem, weightsInitialisedAtStartup );
   _PerCellParticleLayout_Init( self );
	_MeshParticleLayout_Init( self, mesh, cellParticleCount, seed, filltype );

	return self;
}

MeshParticleLayout* _MeshParticleLayout_New(  MESHPARTICLELAYOUT_DEFARGS  )
{
	MeshParticleLayout* self;

   coordSystem = GlobalCoordSystem;   
   weightsInitialisedAtStartup = False;
	/* Allocate memory */
	self = (MeshParticleLayout*)_PerCellParticleLayout_New(  PERCELLPARTICLELAYOUT_PASSARGS  );

   self->mesh = mesh;
   self->cellParticleCount = cellParticleCount;
   self->seed = seed;

   self->sobolGenerator[I_AXIS] = SobolGenerator_NewFromTable( "xSobolGenerator", 0 );
   self->sobolGenerator[J_AXIS] = SobolGenerator_NewFromTable( "ySobolGenerator", 1 );
   self->sobolGenerator[K_AXIS] = SobolGenerator_NewFromTable( "zSobolGenerator", 2 );

	return self;
}

void _MeshParticleLayout_Init( void* meshParticleLayout, Mesh* mesh, Particle_InCellIndex cellParticleCount, unsigned int seed, unsigned int filltype ) {
	MeshParticleLayout* self = (MeshParticleLayout*)meshParticleLayout;

	self->mesh = mesh;
	self->isConstructed     = True;
	self->cellParticleCount = cellParticleCount;
	self->seed              = seed;
    self->filltype          = filltype;
	
	Swarm_Random_Seed( self->seed );
}


void _MeshParticleLayout_Delete( void* meshParticleLayout ) {
	MeshParticleLayout* self = (MeshParticleLayout*)meshParticleLayout;
	
	/* Stg_Class_Delete parent class */
	_PerCellParticleLayout_Delete( self );
}

void _MeshParticleLayout_Print( void* meshParticleLayout, Stream* stream ) {
	MeshParticleLayout* self = (MeshParticleLayout*)meshParticleLayout;
	
	/* Set the Journal for printing informations */
	Stream* meshParticleLayoutStream = stream;
	
	/* General info */
	Journal_Printf( meshParticleLayoutStream, "MeshParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* MeshParticleLayout */
	Journal_Printf( meshParticleLayoutStream, "\tcellParticleCount: %u\n", self->cellParticleCount );
	Journal_Printf( meshParticleLayoutStream, "\tseed: %u\n", self->seed );
}


void* _MeshParticleLayout_Copy( void* meshParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	MeshParticleLayout*		self = (MeshParticleLayout*)meshParticleLayout;
	MeshParticleLayout*		newMeshParticleLayout;
	
	newMeshParticleLayout = (MeshParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newMeshParticleLayout->cellParticleCount = self->cellParticleCount;
	newMeshParticleLayout->seed = self->seed;
	
	return (void*)newMeshParticleLayout;
}


void* _MeshParticleLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                     _sizeOfSelf = sizeof(MeshParticleLayout);
	Type                                                                             type = MeshParticleLayout_Type;
	Stg_Class_DeleteFunction*                                                     _delete = _MeshParticleLayout_Delete;
	Stg_Class_PrintFunction*                                                       _print = _MeshParticleLayout_Print;
	Stg_Class_CopyFunction*                                                         _copy = _MeshParticleLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                         _defaultConstructor = _MeshParticleLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                           _construct = _MeshParticleLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                                   _build = _MeshParticleLayout_Build;
	Stg_Component_InitialiseFunction*                                         _initialise = _MeshParticleLayout_Initialise;
	Stg_Component_ExecuteFunction*                                               _execute = _MeshParticleLayout_Execute;
	Stg_Component_DestroyFunction*                                               _destroy = _MeshParticleLayout_Destroy;
	AllocationType                                                     nameAllocationType = NON_GLOBAL;
	ParticleLayout_SetInitialCountsFunction*                            _setInitialCounts = _PerCellParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*                      _initialiseParticles = _PerCellParticleLayout_InitialiseParticles;
	CoordSystem                                                               coordSystem = LocalCoordSystem;
	Bool                                                      weightsInitialisedAtStartup = True;
	PerCellParticleLayout_InitialCountFunction*                             _initialCount = _MeshParticleLayout_InitialCount;
	PerCellParticleLayout_InitialiseParticlesOfCellFunction*   _initialiseParticlesOfCell = _MeshParticleLayout_InitialiseParticlesOfCell;
	Mesh*                                                                            mesh = NULL;
	Particle_InCellIndex                                                cellParticleCount = 0;
	unsigned int                                                                     seed = 0;

	return (void*)_MeshParticleLayout_New(  MESHPARTICLELAYOUT_PASSARGS  );
}

void _MeshParticleLayout_AssignFromXML( void* meshParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	MeshParticleLayout*       self = (MeshParticleLayout*)meshParticleLayout;
	Particle_InCellIndex        cellParticleCount;
	unsigned int                seed;
	unsigned int                filltype;
    Mesh* mesh = NULL;

    _PerCellParticleLayout_AssignFromXML( self, cf, data );

	cellParticleCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"cellParticleCount", 0  );
	seed = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"seed", 13  );
	mesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FeMesh", Mesh, True, data  );
    // filltype, 0=sobol, 1=random
	filltype = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"filltype", 0  );

	_MeshParticleLayout_Init( self, mesh, cellParticleCount, seed, filltype );

}
	
void _MeshParticleLayout_Build( void* meshParticleLayout, void* data ) {
	MeshParticleLayout*       self = (MeshParticleLayout*)meshParticleLayout;

	assert( self );

	Stg_Component_Build( self->mesh, NULL, False );
}
	
void _MeshParticleLayout_Initialise( void* meshParticleLayout, void* data ) {
	MeshParticleLayout*       self = (MeshParticleLayout*)meshParticleLayout;

	assert( self );

	Stg_Component_Initialise( self->mesh, NULL, False );
}
	
void _MeshParticleLayout_Execute( void* meshParticleLayout, void* data ) {
	
}
	
void _MeshParticleLayout_Destroy( void* meshParticleLayout, void* data ) {
   MeshParticleLayout* self = (MeshParticleLayout*)meshParticleLayout;

   _PerCellParticleLayout_Destroy( self, data );
}


Particle_InCellIndex _MeshParticleLayout_InitialCount( void* meshParticleLayout, void* celllayout, Cell_Index cell_I ) {
	MeshParticleLayout* self = (MeshParticleLayout*)meshParticleLayout;
	return self->cellParticleCount;
}

void _MeshParticleLayout_InitialiseParticlesOfCell( void* meshParticleLayout, void* _swarm, Cell_Index cell_I ) {
	MeshParticleLayout*	self = (MeshParticleLayout*)meshParticleLayout;
	Swarm*                      	swarm   		= (Swarm*)_swarm;
	Particle_InCellIndex		particlesThisCell 	= swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex		cParticle_I 		= 0;
	GlobalParticle*	        	particle 		= NULL;
	double				        lCoord[3];
	unsigned		         	dim_i;
    unsigned	         	    nDims = Mesh_GetDimSize( self->mesh );


	for( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {	
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;

        if (self->filltype == 0 ) {
            for( dim_i = 0; dim_i < nDims; dim_i++ )
                lCoord[dim_i] = SobolGenerator_GetNextNumber_WithMinMax( self->sobolGenerator[dim_i], -1.0, +1.0 );
        } else if (self->filltype == 1){
            for( dim_i = 0; dim_i < nDims; dim_i++ )
                lCoord[dim_i] = Swarm_Random_Random_WithMinMax( -1.0, +1.0 );
        } else
            Journal_Firewall(
				NULL,
				NULL,
				"In func %s: Invalid fill type setting.  Must be 0 (space filler), or 1 (random).",
				__func__ );


		FeMesh_CoordLocalToGlobal( self->mesh, cell_I, lCoord, particle->coord );
	}
}


