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

#include "types.h"

#include "ParticleLayout.h"
#include "GlobalParticleLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include "SwarmClass.h"
#include "StandardParticle.h"
#include "CellLayout.h"

const Type GlobalParticleLayout_Type = "GlobalParticleLayout";

const Index GlobalParticleLayout_Invalid = (Index) 0;

GlobalParticleLayout* _GlobalParticleLayout_New(  GLOBALPARTICLELAYOUT_DEFARGS  )
{
	GlobalParticleLayout* self;
	
	assert( _sizeOfSelf >= sizeof(GlobalParticleLayout) );

   /* hard-wire class attributes */
   coordSystem = GlobalCoordSystem;
   weightsInitialisedAtStartup = False;

	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (GlobalParticleLayout*)_ParticleLayout_New(  PARTICLELAYOUT_PASSARGS  );
	
   /* attributes */
   self->totalInitialParticles = totalInitialParticles; 
   self->averageInitialParticlesPerCell = averageInitialParticlesPerCell; 
   /* virtual function */
	self->_initialiseParticle = _initialiseParticle;
	
	return self;
}


void _GlobalParticleLayout_Init(
      void*                                               particleLayout,
      Particle_Index                                      totalInitialParticles,
      double                                              averageInitialParticlesPerCell )
{
   GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;

   /* Note the total and average particles per cell need to be set in child
   classes, as they may be worked out differently (eg the ManualParticleLayout
   specifies the particles directly, so the total is implicit) */

   self->totalInitialParticles = totalInitialParticles;
   self->averageInitialParticlesPerCell = averageInitialParticlesPerCell;
}

void _GlobalParticleLayout_Delete( void* particleLayout ) {
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	_ParticleLayout_Delete( self );
}
void _GlobalParticleLayout_Destroy( void* particleLayout, void* data ) {
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	_ParticleLayout_Destroy( self, data );
}

void  _GlobalParticleLayout_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data )  {
   GlobalParticleLayout*	self = (GlobalParticleLayout*)component;
   unsigned int totalInitialParticles;
   double averageInitialParticlesPerCell;

   _ParticleLayout_AssignFromXML( self, cf, data );

   /** Determines whether all particles must be placed within the domain.   If true, if any particles are not loaded, simulation will bail. */
   self->mustUseAllParticles = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"mustUseAllParticles", True );

   totalInitialParticles = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"totalInitialParticles", GlobalParticleLayout_Invalid );

   averageInitialParticlesPerCell = (double )Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"averageInitialParticlesPerCell", GlobalParticleLayout_Invalid  );

   _GlobalParticleLayout_Init( component, totalInitialParticles, averageInitialParticlesPerCell );
}
void _GlobalParticleLayout_Print( void* particleLayout, Stream* stream ) {
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	Journal_Printf( stream, "GlobalParticleLayout (ptr): %p\n", self );
	
	/* Parent class info */
	_ParticleLayout_Print( self, stream );
	
	/* Virtual info */
	Journal_Printf( stream, "\t_initialiseParticle (func ptr): %p\n", self->_initialiseParticle );
	
	/* GlobalParticleLayout info */
	Journal_Printf( stream, "\ttotalInitialParticles: %u\n", self->totalInitialParticles );
	Journal_Printf( stream, "\taverageInitialParticlesPerCell: %g\n", self->averageInitialParticlesPerCell );
}


void* _GlobalParticleLayout_Copy( void* particleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	GlobalParticleLayout*		self = (GlobalParticleLayout*)particleLayout;
	GlobalParticleLayout*		newGlobalParticleLayout;
	
	newGlobalParticleLayout = (GlobalParticleLayout*)_ParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );

	/* Virtual methods */
	newGlobalParticleLayout->_initialiseParticle= self->_initialiseParticle;

	/* data */
	newGlobalParticleLayout->averageInitialParticlesPerCell = self->averageInitialParticlesPerCell;
	newGlobalParticleLayout->totalInitialParticles = self->totalInitialParticles;
	
	return (void*)newGlobalParticleLayout;
}


/* Note: this function is required to also set the total particle count */
void _GlobalParticleLayout_SetInitialCounts( void* particleLayout, void* _swarm )
{
	GlobalParticleLayout*	self = (GlobalParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_LocalIndex		cell_I;
	Cell_Index		globalCellCount;

	Journal_DPrintf( self->debug, "In %s(): for ParticleLayout \"%s\" (of type %s):\n",
		__func__, self->name, self->type );
	Stream_IndentBranch( Swarm_Debug );

	Journal_DPrintf( self->debug, "Calculating total number of cells in system across all processors "
		"using MPI_Allreduce:\n" );
	(void)MPI_Allreduce( &swarm->cellLocalCount, &globalCellCount, 1, MPI_UNSIGNED, MPI_SUM, swarm->comm );
	Journal_DPrintf( self->debug, "...calculated total global cell count is %u\n", globalCellCount );
	
	Journal_DPrintf( self->debug, "Calculating totalInitialParticles and averageInitialParticlesPerCell:\n" );
	Stream_IndentBranch( Swarm_Debug );
	if ( self->totalInitialParticles ) {
		self->averageInitialParticlesPerCell = self->totalInitialParticles / (double)globalCellCount;
		Journal_DPrintf( self->debug, "totalInitialParticles set as %u, thus calculated "
			"averageInitialParticlesPerCell as %f.\n", self->totalInitialParticles,
			self->averageInitialParticlesPerCell );
	}
	else if ( self->averageInitialParticlesPerCell ) {
		self->totalInitialParticles = globalCellCount * self->averageInitialParticlesPerCell;
		Journal_DPrintf( self->debug, "averageInitialParticlesPerCell set as %f, thus calculated "
			"totalInitialParticles as %u.\n", self->averageInitialParticlesPerCell,
			self->totalInitialParticles );
	}
	else {
		Stream*     errorStream = Journal_Register( Error_Type, (Name)self->type  );
		
		Journal_Firewall( self->totalInitialParticles || self->averageInitialParticlesPerCell,
			errorStream, "Error - in %s(), ParticleLayout \"%s\" (of type %s): neither "
			"totalInitialParticles or averageInitialParticlesPerCell was specified by "
			"child class - one of these needs to be calculated so the "
			"swarm cell memory storage can be set.\n",
			__func__, self->name, self->type );
	}
	Stream_UnIndentBranch( Swarm_Debug );
		
	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		/* Set initial sizes to averageInitialParticlesPerCell, since its our best guess at how many
		 * particles there will be after setup in this cell, and hence how much memory needed */
		swarm->cellParticleSizeTbl[cell_I] += (int)( self->averageInitialParticlesPerCell + 0.5 );
		
	}

	Stream_UnIndentBranch( Swarm_Debug );
}


void _GlobalParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm )
{
	GlobalParticleLayout*	self = (GlobalParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	GlobalParticle*         particle = NULL;
	Particle_Index		newParticle_I=0;
	Cell_Index		cell_I;
	Particle_Index          globalParticlesInitialisedCount=0;
	Progress*		prog;
	int                     nRanks;
	Stream*                 errorStream = Journal_Register( Error_Type, (Name)self->type  );
    unsigned                ii;
    double*                 minCoord;
    double*                 maxCoord;
    double*                 minCoordGlobal;
    double*                 maxCoordGlobal;
    
	/** allocate memory for min/max arrays */
	minCoord       = Memory_Alloc_Array( double, 3, "GlobalParticleLayout minCoord"       );
	maxCoord       = Memory_Alloc_Array( double, 3, "GlobalParticleLayout maxCoord"       );
	minCoordGlobal = Memory_Alloc_Array( double, 3, "GlobalParticleLayout minCoordGlobal" );
	maxCoordGlobal = Memory_Alloc_Array( double, 3, "GlobalParticleLayout maxCoordGlobal" );
    
	maxCoord[0] = maxCoord[1] = maxCoord[2] = -DBL_MAX;
	minCoord[0] = minCoord[1] = minCoord[2] =  DBL_MAX;
    
	if(swarm->dim==2){
        maxCoord[2] = 0;
        minCoord[2] = 0;
	}
	
	Journal_DPrintf( self->debug, "In %s(): for ParticleLayout \"%s\" (of type %s):\n",
		__func__, self->name, self->type );
	Stream_IndentBranch( Swarm_Debug );

	Journal_DPrintf( self->debug, "For each of the %u total global requested particles, "
		"generating a particle, and checking if it's in this processor's domain. If so, "
		"adding it to the appropriate local cell.\n", self->totalInitialParticles );
	Stream_IndentBranch( Swarm_Debug );

	/* Use a progress meter. */
	prog = Progress_New();
	Progress_SetStream( prog, self->debug );
	Progress_SetTitle( prog, "Generating global particles" );
	Progress_SetPrefix( prog, "\t" );
	Progress_SetRange( prog, 0, self->totalInitialParticles );
	Progress_Update( prog );

	/* Allocate a guess at the particle size to prevent a TONNE of unnecessary
	   reallocation. */
	MPI_Comm_size( MPI_COMM_WORLD, &nRanks );
	assert( swarm->particleLocalCount == 0 );
	swarm->particleLocalCount = self->totalInitialParticles/nRanks + 1;
	Swarm_Realloc( swarm );
	swarm->particleLocalCount = 0;
	swarm->expanding = 1;

	Particle_Index	lParticle_I=0;
	while( newParticle_I < self->totalInitialParticles ) {
		
		particle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );
		/* Work out which cell the new particle is in */
		/* First specify the particle doesn't have an owning cell yet, so as
		not to confuse the search algorithm if its an irregular cell/mesh layout */
		particle->owningCell = -1;
        /** run initialise particle routine, which may stomp on owningcell value */
		GlobalParticleLayout_InitialiseParticle( self, swarm, newParticle_I, particle );
        /** if stomped on, retain, else determine */
        if(particle->owningCell != -1)
           cell_I = particle->owningCell;
        else
           cell_I = CellLayout_CellOf( swarm->cellLayout, particle );

		/** record mins / max */
		for(ii = 0; ii< swarm->dim; ii++){
            if( particle->coord[ii] < minCoord[ii] ) minCoord[ii] = particle->coord[ii];
            if( particle->coord[ii] > maxCoord[ii] ) maxCoord[ii] = particle->coord[ii];
		}

		/* If we found a further particle inside our domain, add it to a cell */
		if ( cell_I < swarm->cellLocalCount ) {
			Journal_DPrintfL( self->debug, 3, "global particle %u at (%.2f,%.2f,%.2f) inside local cell %u\n"
				"adding it to cell and saving it as local particle %u.\n",
				newParticle_I, particle->coord[0], particle->coord[1], particle->coord[2],
				cell_I, lParticle_I );
				
			Stream_IndentBranch( Swarm_Debug );
			/* Add it to that cell */
			Swarm_AddParticleToCell( swarm, cell_I, lParticle_I );
			lParticle_I++;
			swarm->particleLocalCount++;
			Swarm_Realloc( swarm );
			Stream_UnIndentBranch( Swarm_Debug );
		}
		else {
			Journal_DPrintfL( self->debug, 4, "global particle %u at (%.2f,%.2f,%.2f) outside this proc's domain:\n"
				"ignoring.\n", newParticle_I, particle->coord[0], particle->coord[1], particle->coord[2] );
		}		
				
		newParticle_I++;

		Progress_Increment( prog );
	}

	/* Removing the lock on the swarm and realloc once more to remove any
	   unused space. */
	swarm->expanding = 0;
	Swarm_Realloc( swarm );

	/* Delete progress meter. */
	Stg_Class_Delete( prog );

	Stream_UnIndentBranch( Swarm_Debug );

	if( self->mustUseAllParticles == True ){
        /* Do a test to make sure that the total particles assigned across all processors ==
         totalInitialParticles count */
        (void)MPI_Allreduce( &swarm->particleLocalCount, &globalParticlesInitialisedCount, 1, MPI_UNSIGNED, MPI_SUM, swarm->comm );
        (void)MPI_Allreduce( minCoord, minCoordGlobal, 3, MPI_DOUBLE, MPI_MIN, swarm->comm );
        (void)MPI_Allreduce( maxCoord, maxCoordGlobal, 3, MPI_DOUBLE, MPI_MAX, swarm->comm );
        Journal_Firewall( globalParticlesInitialisedCount == self->totalInitialParticles, errorStream,
                         "Error - in %s() - for GlobalParticleLayout \"%s\", of type %s: after initialising particles, "
                         "actual global count of particles initialised was %u, whereas the user requested global total "
                         "totalInitialParticles was %u.\n"
                         "If actual is < requested, it means some particles were not identified by any processor as "
                         "inside their domain.\n"
                         "This is usually caused because some particles are outside the mesh domain.\n"
                         "If actual > requested, it means that some particles were identified by _multiple_ processors "
                         "as belonging to their domain.\n"
                         "If you are happy to allow some particles to not be used, set the component flag 'mustUseAllParticles' to 'False'.\n\n"
                         "Note that particles in this layout takes min values of (%g, %g, %g)\n"
                         "                                         max values of (%g, %g, %g).\nCompare this with your domain min/max.",
                         __func__, self->name, self->type, globalParticlesInitialisedCount, self->totalInitialParticles,
                         minCoord[0],minCoord[1],minCoord[2],maxCoord[0],maxCoord[1],maxCoord[2] );
    }

    Memory_Free( minCoord );
    Memory_Free( maxCoord );
    Memory_Free( minCoordGlobal );
    Memory_Free( maxCoordGlobal );

	Stream_UnIndentBranch( Swarm_Debug );
}


void GlobalParticleLayout_InitialiseParticle( void* particleLayout, void* _swarm, Particle_Index newParticle_I, void* particle )
{
	GlobalParticleLayout* self = (GlobalParticleLayout*)particleLayout;
	
	self->_initialiseParticle( self, _swarm, newParticle_I, particle );
}




