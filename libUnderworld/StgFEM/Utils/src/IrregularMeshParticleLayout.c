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

#include "types.h"
#include "IrregularMeshParticleLayout.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const Type IrregularMeshParticleLayout_Type = (const Type)"IrregularMeshParticleLayout";

IrregularMeshParticleLayout* IrregularMeshParticleLayout_New( Name name, Dimension_Index dim, unsigned cellParticleCount ) {
	IrregularMeshParticleLayout* self = (IrregularMeshParticleLayout*) _IrregularMeshParticleLayout_DefaultNew( name );
   _PerCellParticleLayout_Init( self ); 
	_IrregularMeshParticleLayout_Init( self, NULL, dim, cellParticleCount );

	return self;
}

IrregularMeshParticleLayout* _IrregularMeshParticleLayout_New(  IRREGULARMESHPARTICLELAYOUT_DEFARGS  )
{
	IrregularMeshParticleLayout* self;
	
	/* Allocate memory */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	coordSystem                 = GlobalCoordSystem;
	weightsInitialisedAtStartup = False;

	self = (IrregularMeshParticleLayout*)_PerCellParticleLayout_New(  PERCELLPARTICLELAYOUT_PASSARGS  );

	return self;
}

void _IrregularMeshParticleLayout_Init( void* perCellLayout, FeMesh* feMesh, Dimension_Index dim, unsigned cellParticleCount ) {
	IrregularMeshParticleLayout*	self = (IrregularMeshParticleLayout*) perCellLayout;

	self->isConstructed 	= True;
	self->feMesh         = feMesh;
	self->dim           	= dim;
	self->cellParticleCount = cellParticleCount;

	self->sobolGenerator[I_AXIS] = SobolGenerator_NewFromTable( "xSobolGenerator", 0 );
	self->sobolGenerator[J_AXIS] = SobolGenerator_NewFromTable( "ySobolGenerator", 1 );
	if ( dim == 3 )
		self->sobolGenerator[K_AXIS] = SobolGenerator_NewFromTable( "zSobolGenerator", 2 );

	_PerCellParticleLayout_Init( self );
}
	
void _IrregularMeshParticleLayout_Delete( void* perCellLayout ) {
	IrregularMeshParticleLayout* self = (IrregularMeshParticleLayout*)perCellLayout;
	Dimension_Index            dim_I;

	for ( dim_I = 0 ; dim_I < self->dim ; dim_I++ ) 
		Stg_Class_Delete( self->sobolGenerator[ dim_I ] );

	_PerCellParticleLayout_Delete( self );
}

void _IrregularMeshParticleLayout_Print( void* perCellLayout, Stream* stream ) {
	IrregularMeshParticleLayout* self  = (IrregularMeshParticleLayout*)perCellLayout;
	
	/* General info */
	Journal_Printf( stream, "IrregularMeshParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* IrregularMeshParticleLayout */
	Journal_PrintValue( stream, self->dim );
	
	Stream_UnIndent( stream );
}

void* _IrregularMeshParticleLayout_Copy( void* perCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	IrregularMeshParticleLayout*		self                    = (IrregularMeshParticleLayout*)perCellLayout;
	IrregularMeshParticleLayout*		newIrregularMeshParticleLayout;
	
	newIrregularMeshParticleLayout = (IrregularMeshParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newIrregularMeshParticleLayout->dim = self->dim;

	return (void*)newIrregularMeshParticleLayout;
}

void* _IrregularMeshParticleLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                              _sizeOfSelf = sizeof(IrregularMeshParticleLayout);
	Type                                                                      type = IrregularMeshParticleLayout_Type;
	Stg_Class_DeleteFunction*                                              _delete = _IrregularMeshParticleLayout_Delete;
	Stg_Class_PrintFunction*                                                _print = _IrregularMeshParticleLayout_Print;
	Stg_Class_CopyFunction*                                                  _copy = _IrregularMeshParticleLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                  _defaultConstructor = _IrregularMeshParticleLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                    _construct = _IrregularMeshParticleLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                            _build = _IrregularMeshParticleLayout_Build;
	Stg_Component_InitialiseFunction*                                  _initialise = _IrregularMeshParticleLayout_Initialise;
	Stg_Component_ExecuteFunction*                                        _execute = _IrregularMeshParticleLayout_Execute;
	Stg_Component_DestroyFunction*                                        _destroy = _IrregularMeshParticleLayout_Destroy;
	ParticleLayout_SetInitialCountsFunction*                     _setInitialCounts = _PerCellParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*               _initialiseParticles = _IrregularMeshParticleLayout_InitialiseParticles;
	PerCellParticleLayout_InitialCountFunction*                      _initialCount = _IrregularMeshParticleLayout_InitialCount;
	Dimension_Index                                                            dim = 0;
	unsigned                                                     cellParticleCount = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType                                                     nameAllocationType = ZERO;
	CoordSystem                                                               coordSystem = ZERO;
	Bool                                                      weightsInitialisedAtStartup = ZERO;
	PerCellParticleLayout_InitialiseParticlesOfCellFunction*   _initialiseParticlesOfCell = _IrregularMeshParticleLayout_InitialiseParticlesOfCell;

	return (void*)_IrregularMeshParticleLayout_New(  IRREGULARMESHPARTICLELAYOUT_PASSARGS  );
}


void _IrregularMeshParticleLayout_InitialiseParticles( void* particleLayout, void* _swarm )
{
	IrregularMeshParticleLayout*	self = (IrregularMeshParticleLayout*)particleLayout;
	Swarm*			swarm = (Swarm*)_swarm;
	Cell_Index		cell_I;
	Particle_InCellIndex	cellParticle_I;
	
	/* Allocate a guess at the particle size to prevent a TONNE of unnecessary
	   reallocation. */
	assert( swarm->particleLocalCount >= swarm->cellLocalCount ); // check this is at least one 

	Swarm_Realloc( swarm );
	swarm->particleLocalCount = 0;
	swarm->expanding = 1;

	/* Go through and init particles, cell-by-cell */
	/* Note: don't put particles in the shadow cells... */
	for( cell_I = 0; cell_I < swarm->cellLocalCount; cell_I++ ) {
		Particle_InCellIndex		count = swarm->cellParticleCountTbl[cell_I];
		
		for( cellParticle_I = 0; cellParticle_I < count; cellParticle_I++ ) {
			swarm->cellParticleTbl[cell_I][cellParticle_I] = swarm->particleLocalCount; // add the local particle to cell

			Swarm_AddParticleToCell( swarm, cell_I, swarm->particleLocalCount );
			swarm->particleLocalCount++;
			Swarm_Realloc( swarm ); // realloc for safety even though it was done above
		}	

		_IrregularMeshParticleLayout_InitialiseParticlesOfCell( self, swarm, cell_I );
	}
	swarm->expanding = 0;
	Swarm_Realloc( swarm );
}

void _IrregularMeshParticleLayout_AssignFromXML( void* perCellLayout, Stg_ComponentFactory *cf, void* data ) {
	IrregularMeshParticleLayout* 	self = (IrregularMeshParticleLayout*) perCellLayout;
   FeMesh*                       feMesh = NULL;
	Dimension_Index            	dim;
	unsigned			cellParticleCount;
	
	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
	cellParticleCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"cellParticleCount", 0  );
	feMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FeMesh", FeMesh, True, data  );

	_IrregularMeshParticleLayout_Init( self, feMesh, dim, cellParticleCount );
}
	
void _IrregularMeshParticleLayout_Build( void* perCellLayout, void* data ) {
   IrregularMeshParticleLayout* 	self = (IrregularMeshParticleLayout*) perCellLayout;
   Stg_Component_Build( self->feMesh, data, False );
}
void _IrregularMeshParticleLayout_Initialise( void* perCellLayout, void* data ) {
   IrregularMeshParticleLayout* 	self = (IrregularMeshParticleLayout*) perCellLayout;
   Stg_Component_Initialise( self->feMesh, data, False );
}	
void _IrregularMeshParticleLayout_Execute( void* perCellLayout, void* data ) {	
}
void _IrregularMeshParticleLayout_Destroy( void* perCellLayout, void* data ) {	
}

#if 0
void _IrregularMeshParticleLayout_InitialiseParticles( void* perCellLayout, void* _swarm ) {
	IrregularMeshParticleLayout*        self     		= (IrregularMeshParticleLayout*)perCellLayout;
	Swarm*				    swarm 		= (Swarm*)_swarm;
	GlobalParticle*         	    particle 		= NULL;
	Particle_Index			    lParticle_I		= 0;
	Particle_Index			    cParticle_I		= 0;
	Particle_Index			    newParticle_I 	= 0;
	Cell_Index			    cell_I;
	Particle_Index          	    globalParticlesInitialisedCount = 0;
	Progress*			    prog;
	int                     	    nRanks;
	Stream*                 	    errorStream 	= Journal_Register( Error_Type, (Name)self->type );
	FeMesh*			    	    mesh 	 	= (FeMesh*)((ElementCellLayout*)swarm->cellLayout )->mesh;

	Journal_DPrintf( self->debug, "In %s(): for ParticleLayout \"%s\" (of type %s):\n", __func__, self->name, self->type );
	Stream_IndentBranch( Swarm_Debug );

	Stream_IndentBranch( Swarm_Debug );

	/* Use a progress meter. */
	prog = Progress_New();
	Progress_SetStream( prog, self->debug );
	Progress_SetTitle( prog, "Generating global particles" );
	Progress_SetPrefix( prog, "\t" );
	Progress_SetRange( prog, 0, self->totalInitialParticles );
	Progress_Update( prog );

	/* Allocate a guess at the particle size to prevent a TONNE of unnecessary reallocation. */
	MPI_Comm_size( MPI_COMM_WORLD, &nRanks );
	assert( swarm->particleLocalCount == 0 );
	swarm->particleLocalCount = self->averageInitialParticlesPerCell * Mesh_GetLocalSize( mesh, Mesh_GetDimSize( mesh ) );

	Swarm_Realloc( swarm );

	swarm->particleLocalCount = 0;
	swarm->expanding = 1;

	lParticle_I = 0;
	for( cell_I = 0; cell_I < Mesh_GetLocalSize( mesh, Mesh_GetDimSize( mesh ) ); cell_I++ ) {
		for( cParticle_I = 0; cParticle_I < self->averageInitialParticlesPerCell; cParticle_I++ ) {
			particle = (GlobalParticle*)Swarm_ParticleAt( swarm, lParticle_I );
			_IrregularMeshParticleLayout_InitialiseParticle_ForCell( self, swarm, cell_I, particle );
			Swarm_AddParticleToCell( swarm, cell_I, lParticle_I );
			swarm->particleLocalCount++;
			lParticle_I++;
			Progress_Increment( prog );
		}
	}

	/* Removing the lock on the swarm and realloc once more to remove any unused space. */
	swarm->expanding = 0;
	Swarm_Realloc( swarm );

	/* Delete progress meter. */
	Stg_Class_Delete( prog );

	Stream_UnIndentBranch( Swarm_Debug );

	/* Do a test to make sure that the total particles assigned across all processors == totalInitialParticles count */
	MPI_Allreduce( &swarm->particleLocalCount, &globalParticlesInitialisedCount, 1, MPI_UNSIGNED, MPI_SUM, swarm->comm );
	Journal_Firewall( globalParticlesInitialisedCount == self->totalInitialParticles, errorStream,
		"Error - in %s() - for GlobalParticleLayout \"%s\", of type %s: after initialising particles, "
		"actual global count of particles initialised was %u, whereas requested global total "
		"totalInitialParticles was %u. If actual is < requested, it means some particles were not "
		"identified by any processor as inside their domain. If actual > requested, it means that "
		"some particles were identified by _multiple_ processors as belonging to their domain. Both "
		"these states are erroneous.\n",
		__func__, self->name, self->type, globalParticlesInitialisedCount, self->totalInitialParticles );

	Stream_UnIndentBranch( Swarm_Debug );
}
#endif

Particle_InCellIndex _IrregularMeshParticleLayout_InitialCount( void* particleLayout, void* cellLayout, Cell_Index cell_I ) {
	IrregularMeshParticleLayout*  	self     = (IrregularMeshParticleLayout*)particleLayout;
	return self->cellParticleCount;
}

void _IrregularMeshParticleLayout_InitialiseParticlesOfCell( void* particleLayout, void* _swarm, Cell_Index cell_I ) {
	IrregularMeshParticleLayout*	self 			= (IrregularMeshParticleLayout*)particleLayout;
	Swarm*                      	swarm   		= (Swarm*)_swarm;
	FeMesh*				feMesh			= self->feMesh;
	Particle_InCellIndex		particlesThisCell 	= swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex		cParticle_I 		= 0;
	GlobalParticle*	        	particle 		= NULL;
	double				lCoord[3];
	unsigned			dim_i;

	for( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {	
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;

		for( dim_i = 0; dim_i < self->dim; dim_i++ ) {
			lCoord[dim_i] = SobolGenerator_GetNextNumber_WithMinMax( self->sobolGenerator[dim_i], -1.0, +1.0 );
		}

		FeMesh_CoordLocalToGlobal( feMesh, cell_I, lCoord, particle->coord );
	}
}



