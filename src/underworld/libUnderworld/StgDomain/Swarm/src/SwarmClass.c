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

#include "SwarmClass.h"

#include "Swarm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


const Type Swarm_Type = "Swarm";
const Name defaultSwarmParticleCommHandlerName = "defaultSwarmPHandlerName";

/** Default extra factor to malloc for particles array. This is because the number of particles on each process
may increase due to advection, splitting/merging etc, and we want to avoid reallocing if possible. */
const double       DEFAULT_EXTRA_PARTICLES_FACTOR = 0.05;
const unsigned int MINIMUM_PARTICLES_ARRAY_DELTA = 100;
const unsigned int DEFAULT_CELL_PARTICLE_TBL_DELTA = 4;

/* --- Function Definitions --- */

Swarm* Swarm_New( 
   Name                                  name,
   AbstractContext*                      context,
   void*                                 cellLayout,
   void*                                 particleLayout,
   Dimension_Index                       dim,
   SizeT                                 particleSize,
   ExtensionManager_Register*            extensionMgr_Register,
   Variable_Register*                    variable_Register,
   MPI_Comm                              comm,
   void*				                       ics ) 
{
   Swarm* self = _Swarm_DefaultNew( name ); 

   _Swarm_Init( 
      self, context,
      cellLayout,
      particleLayout,
      dim,
      particleSize,
      DEFAULT_CELL_PARTICLE_TBL_DELTA,
      DEFAULT_EXTRA_PARTICLES_FACTOR,
      extensionMgr_Register,
      variable_Register,
      False,
      comm, 
      ics );

   self->isConstructed = True;


   return self;
}

Swarm* _Swarm_New(  SWARM_DEFARGS  )
{
	Swarm* self;
	
	/* Allocate memory */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (Swarm*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

	self->particleSize = particleSize;
    self->preReallocParticleSize = particleSize;
	self->commHandlerList = Stg_ObjectList_New();
	self->nSwarmVars = 0;
	self->swarmVars = NULL;
	self->owningCellVariable = NULL;
	self->globalIdVariable = NULL;
	self->gidExtHandle = (unsigned)-1;
	self->ics = ics;
	self->isAdvecting = False;
	self->allow_parallel_nn = False;

	return self;
}

void _Swarm_Init( 
   Swarm*                                self, 
   AbstractContext*                      context,
   void*                                 cellLayout,
   void*                                 particleLayout,
   Dimension_Index                       dim,
   SizeT                                 particleSize,
   Particle_InCellIndex                  cellParticleTblDelta, 
   double                                extraParticlesFactor,
   ExtensionManager_Register*            extensionMgr_Register,
   Variable_Register*                    variable_Register,
   Bool                                  createGlobalIdVar,
   MPI_Comm                              comm,
   void*                                 ics)
{
	StandardParticle   particle;
	Stream*            errorStream = Journal_Register( Error_Type, (Name)self->type  );
   LiveComponentRegister* lcReg = NULL; 	

	self->debug = Stream_RegisterChild( Swarm_Debug, self->type );
	self->dim   = dim;
	self->comm = comm;
	MPI_Comm_rank( comm, (int *)&self->myRank );
	MPI_Comm_size( comm, (int *)&self->nProc );
	/* Check point and reload by default - only things like integration swarms will turn this off */
	self->isSwarmTypeToCheckPointAndReload = True;

    self->particleSize = particleSize;
    self->preReallocParticleSize = particleSize;
    self->context = (AbstractContext*)context;
	self->cellLayout = (CellLayout*)cellLayout;
	self->particleLayout = (ParticleLayout*)particleLayout;
	
	/* Check that if either the CellLayout or ParticleLayout is inappropriate to be checkpointed,
		we set tthe appropriate flags -- PatrickSunter, 22 August 2006 */
	if ( Stg_Class_IsInstance( self->cellLayout, SingleCellLayout_Type ) ) {
		self->isSwarmTypeToCheckPointAndReload = False;
	}
	
	self->cellLocalCount = 0;
	self->cellDomainCount = 0;
	self->cellShadowCount = 0;
	self->cellPointTbl = NULL;
	self->cellPointCountTbl = NULL;
	
	self->cellParticleTbl = NULL;
	self->cellParticleCountTbl = NULL;
	self->cellParticleSizeTbl = NULL;
	self->shadowCellParticleTbl = NULL;
	self->shadowCellParticleCountTbl = NULL;
	self->shadowParticleCount = 0;
	self->cellParticleTblDelta = cellParticleTblDelta;
	
	self->particles = NULL;
	self->shadowParticles = NULL;
	self->particleLocalCount = 0;
	self->particlesArraySize = 0;
	self->particlesArrayDelta = 0;
	self->extraParticlesFactor = extraParticlesFactor;

	self->shadowTablesBuilt = False;

	Journal_Firewall( extraParticlesFactor > 0.0, errorStream, "Error - in %s: extraParticlesFactor "
		"given as %.3f, but this must be greater than zero to allow swarm to be realloc'ed larger "
		"if necessary\n", extraParticlesFactor );

   self->swarmVariable_Register = SwarmVariable_Register_New( variable_Register );

	self->particleExtensionMgr = ExtensionManager_New_OfStruct( "particle", self->particleSize );
	ExtensionManager_Register_Add( extensionMgr_Register, self->particleExtensionMgr );

	self->owningCellVariable = Swarm_NewScalarVariable(
			self,
			"OwningCell",
			GetOffsetOfMember( particle , owningCell ),
			StgVariable_DataType_Int ); /* Should be unsigned int */

   lcReg = LiveComponentRegister_GetLiveComponentRegister(); /* only needed for tests like GaussLayoutSingleCellSuite which don't have liveComponent_Registers */
   if( lcReg ) {
      LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->owningCellVariable );
      LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->owningCellVariable->variable );
   }

   if( createGlobalIdVar && self->gidExtHandle == (unsigned)-1 ) {

      int *gidExt = NULL;
      // create the integer extension for a particle
      self->gidExtHandle = ExtensionManager_Add( self->particleExtensionMgr, (Name)self->type, sizeof( int ) );

      // create a swarm variable for the globalId extension
      gidExt = ExtensionManager_Get( self->particleExtensionMgr, &particle, self->gidExtHandle );
      self->globalIdVariable = Swarm_NewScalarVariable(
                                 self, "GlobalId",
                                 (ArithPointer) gidExt - (ArithPointer)&particle,
                                 StgVariable_DataType_Int );

      if( lcReg ) {
         LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->globalIdVariable );
         LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), (Stg_Component*)self->globalIdVariable->variable );
      }

   }

	self->swarmReg_I = Swarm_Register_Add( Swarm_Register_GetSwarm_Register(), self );

	if ( ics ) {
		self->ics = Stg_CheckType( ics, VariableCondition );
	}

	self->incArray = IArray_New();

	self->expanding = 0;
    self->mirroredSwarm=NULL;
}


void* _Swarm_ParticleInCellAt( void* swarm, Cell_Index cell_I, Particle_InCellIndex cParticle_I ) {
	Swarm* self =	(Swarm*)swarm;
	
	return (void*)Swarm_ParticleInCellAt( self, cell_I, cParticle_I );
}


void* _Swarm_ParticleAt( void* swarm, Particle_Index dParticle_I ) {
	Swarm* self = (Swarm*)swarm;
	
	return (void*)Swarm_ParticleAt( self, dParticle_I );
}


void _Swarm_Delete( void* swarm ) {
	Swarm*			self = (Swarm*)swarm;

	Stg_Class_Delete( self->commHandlerList ); self->commHandlerList = NULL;
	Stg_Class_Delete( self->incArray ); self->incArray = NULL;

	/* Delete SwarmVariable_Register if it has been created */
	if ( self->swarmVariable_Register ) {
		Stg_Class_Delete( self->swarmVariable_Register ); self->swarmVariable_Register = NULL;
	}

    if( Swarm_Register_Get( Swarm_Register_GetSwarm_Register(), self->name ))
        Swarm_Register_RemoveIndex( Swarm_Register_GetSwarm_Register(), self->swarmReg_I );

	_Stg_Component_Delete( self );
}


void _Swarm_Print( void* swarm, Stream* stream ) {
	Swarm* self = (Swarm*)swarm;
	
	Cell_Index cell_I;
	
	/* Set the Journal for printing informations */
	Stream* swarmStream = stream;
	
	/* General info */
	Journal_Printf( swarmStream, "Swarm (ptr): %p\n", self );
	
	/* Parent class info */
	_Stg_Component_Print( self, stream );
	
	/* Virtual info */

	/* Swarm info */
	Stg_Class_Print( self->cellLayout, stream );
	Stg_Class_Print( self->particleLayout, stream );
	Journal_Printf( swarmStream, "\tcellLocalCount: %u\n", self->cellLocalCount );
	Journal_Printf( swarmStream, "\tcellDomainCount: %u\n", self->cellDomainCount );
	Journal_Printf( swarmStream, "\tcellShadowCount: %u\n", self->cellShadowCount );
	Journal_Printf( swarmStream, "\tcellPointCountTbl (ptr): %p\n", self->cellPointCountTbl );
	Journal_Printf( swarmStream, "\tcellPointCountTbl[0-%u]: ", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Journal_Printf( swarmStream, "%.3u ", self->cellPointCountTbl[cell_I] );
	}
	Journal_Printf( swarmStream, "\n" );
	
	Journal_Printf( swarmStream, "\tcellPointTbl (ptr): %p\n", self->cellPointTbl );
	Journal_Printf( swarmStream, "\tcellPointTbl[0-%u]: {\n", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Cell_PointIndex point_I;

		Journal_Printf( swarmStream, "\tcellPointTbl[%u][0-%u]: ", cell_I, self->cellPointCountTbl[cell_I] );
		for( point_I = 0; point_I < self->cellPointCountTbl[cell_I]; point_I++ ) {
			Journal_Printf( swarmStream, "{%.3g %.3g %.3g} ",
				(self->cellPointTbl[cell_I][point_I])[0],
				(self->cellPointTbl[cell_I][point_I])[1],
				(self->cellPointTbl[cell_I][point_I])[2] );
		}
		Journal_Printf( swarmStream, "\n" );
	}
	Journal_Printf( swarmStream, "}\n" );
	Journal_Printf( swarmStream, "\n" );
	
	Journal_Printf( swarmStream, "\tcellParticleCountTbl (ptr): %p\n", self->cellParticleCountTbl );
	Journal_Printf( swarmStream, "\tcellParticleCountTbl[0-%u]: ", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Journal_Printf( swarmStream, "%.3u ", self->cellParticleCountTbl[cell_I] );
	}
	Journal_Printf( swarmStream, "\n" );
	Journal_Printf( swarmStream, "\tcellParticleSizeTbl (ptr): %p\n", self->cellParticleSizeTbl );
	Journal_Printf( swarmStream, "\tcellParticleSizeTbl[0-%u]: ", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Journal_Printf( swarmStream, "%.3u ", self->cellParticleSizeTbl[cell_I] );
	}
	Journal_Printf( swarmStream, "\n" );

	/* Print( self->particleExtensionMgr, stream ); */
	Journal_Printf( swarmStream, "\tcellParticleTbl (ptr): %p\n", self->cellParticleTbl );
	Journal_Printf( swarmStream, "\tcellParticleTbl [0-%u]: {\n", self->cellDomainCount );
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		Particle_InCellIndex cParticle_I;
		Particle_Index dParticle_I;

		Journal_Printf( swarmStream, "\t\tcellParticleTbl[%u][0-%u]:\n", cell_I, self->cellParticleCountTbl[cell_I] );
		for( cParticle_I = 0; cParticle_I < self->cellParticleCountTbl[cell_I]; cParticle_I++ ) {
			dParticle_I = self->cellParticleTbl[cell_I][cParticle_I];
			Journal_Printf( swarmStream, "\t\t\t(part. index) %d\n", dParticle_I );
		}
		Journal_Printf( swarmStream, "\n" );
	}
	Journal_Printf( swarmStream, "\t}\n" );
	Journal_Printf( swarmStream, "\tparticlesArraySize: %d\n", self->particlesArraySize );
	Journal_Printf( swarmStream, "\tparticlesArrayDelta: %d\n", self->particlesArrayDelta );
	Journal_Printf( swarmStream, "\textraParticlesFactor: %.3g\n", self->extraParticlesFactor );

	if ( self->ics ) {
		Stg_Class_Print( self->ics, stream );
	}
	else {
		Journal_Printf( stream, "\tics: (null)... not provided (may be Operator type)\n" );
	}
}


void* _Swarm_Copy( void* swarm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Swarm*		self = (Swarm*)swarm;
	Swarm*		newSwarm;
	PtrMap*		map = ptrMap;
	Bool		ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newSwarm = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	PtrMap_Append( map, self, newSwarm );
	
	newSwarm->myRank = self->myRank;
	newSwarm->nProc = self->nProc;
	newSwarm->comm = self->comm;
	newSwarm->cellLocalCount = self->cellLocalCount;
	newSwarm->cellDomainCount = self->cellDomainCount;
	newSwarm->cellShadowCount = self->cellShadowCount;
	newSwarm->cellParticleTblDelta = self->cellParticleTblDelta;
	newSwarm->particleLocalCount = self->particleLocalCount;
	newSwarm->particlesArraySize = self->particlesArraySize;
	newSwarm->particlesArrayDelta = self->particlesArrayDelta;
	newSwarm->extraParticlesFactor = self->extraParticlesFactor;
	
	if( self->shadowTablesBuilt ){
		newSwarm->shadowParticleCount = self->shadowParticleCount;
	}

	if( deep ) {
		/* Classes */
		newSwarm->cellLayout = (CellLayout*)Stg_Class_Copy( self->cellLayout, NULL, deep, nameExt, map );
		newSwarm->particleLayout = (ParticleLayout*)Stg_Class_Copy( self->particleLayout, NULL, deep, nameExt, map );
		newSwarm->debug = (Stream*)Stg_Class_Copy( self->debug, NULL, deep, nameExt, map );
		newSwarm->particleExtensionMgr = (ExtensionManager*)Stg_Class_Copy( self->particleExtensionMgr, NULL, deep, nameExt, map );
		newSwarm->commHandlerList = (Stg_ObjectList*)Stg_Class_Copy( self->commHandlerList, NULL, deep, nameExt, map );
		
		/* Arrays */
		if( (newSwarm->cellPointCountTbl = PtrMap_Find( map, self->cellPointCountTbl )) == NULL ) {
			if( self->cellPointCountTbl ) {
				newSwarm->cellPointCountTbl = Memory_Alloc_Array( Cell_PointIndex, newSwarm->cellDomainCount, "Swarm->cellPointCountTbl" );
				memcpy( newSwarm->cellPointCountTbl, self->cellPointCountTbl, newSwarm->cellDomainCount * sizeof( Cell_PointIndex ) );
				PtrMap_Append( map, self->cellPointCountTbl, newSwarm->cellPointCountTbl );
			}
			else {
				newSwarm->cellPointCountTbl = NULL;
			}
		}
		
		if( (newSwarm->cellPointTbl = PtrMap_Find( map, self->cellPointTbl )) == NULL ) {
			if( newSwarm->cellPointCountTbl && self->cellPointTbl ) {
				Index	cell_I;
				
				newSwarm->cellPointTbl = Memory_Alloc_2DComplex( Cell_Point, newSwarm->cellDomainCount, newSwarm->cellPointCountTbl, "Swarm->cellPointTbl" );
				for( cell_I = 0; cell_I < newSwarm->cellDomainCount; cell_I++ ) {
					memcpy( newSwarm->cellPointTbl[cell_I], self->cellPointTbl[cell_I], newSwarm->cellPointCountTbl[cell_I] * sizeof(Cell_Point) );
				}
				PtrMap_Append( map, self->cellPointTbl, newSwarm->cellPointTbl );
			}
			else {
				newSwarm->cellPointTbl = NULL;
			}
		}
		
		if( (newSwarm->cellParticleCountTbl = PtrMap_Find( map, self->cellParticleCountTbl )) == NULL ) {
			if( self->cellParticleCountTbl ) {
				newSwarm->cellParticleCountTbl = Memory_Alloc_Array( Particle_InCellIndex, newSwarm->cellDomainCount, "Swarm->cellParticleCountTbl" );
				memcpy( newSwarm->cellParticleCountTbl, self->cellParticleCountTbl, newSwarm->cellDomainCount * sizeof( Particle_InCellIndex ) );
				PtrMap_Append( map, self->cellParticleCountTbl, newSwarm->cellParticleCountTbl );
			}
			else {
				newSwarm->cellParticleCountTbl = NULL;
			}
		}
		
		if( (newSwarm->cellParticleSizeTbl = PtrMap_Find( map, self->cellParticleSizeTbl )) == NULL ) {
			if( self->cellParticleSizeTbl ) {
				newSwarm->cellParticleSizeTbl = Memory_Alloc_Array( Particle_InCellIndex, newSwarm->cellDomainCount, "Swarm->cellParticleSizeTbl" );
				memcpy( newSwarm->cellParticleSizeTbl, self->cellParticleSizeTbl, newSwarm->cellDomainCount * sizeof( Particle_InCellIndex ) );
				PtrMap_Append( map, self->cellParticleSizeTbl, newSwarm->cellParticleSizeTbl );
			}
			else {
				newSwarm->cellParticleSizeTbl = NULL;
			}
		}
		
		if( (newSwarm->cellParticleTbl = PtrMap_Find( map, self->cellParticleTbl )) == NULL ) {
			if( newSwarm->cellParticleCountTbl && self->cellParticleTbl ) {
				Index	cell_I;
				
				newSwarm->cellParticleTbl = Memory_Alloc_Array( Cell_Particles, newSwarm->cellDomainCount, "Swarm->cellParticleTbl" );
				for( cell_I = 0; cell_I < newSwarm->cellDomainCount; cell_I++ ) {
					if( newSwarm->cellParticleCountTbl[cell_I] ) {
						newSwarm->cellParticleTbl[cell_I] = Memory_Alloc_Array( Particle_Index, newSwarm->cellParticleCountTbl[cell_I], "Swarm->cellParticleTbl[]" );
						memcpy( newSwarm->cellParticleTbl[cell_I], self->cellParticleTbl[cell_I], newSwarm->cellParticleCountTbl[cell_I] * sizeof(Particle_Index) );
					}
					else {
						newSwarm->cellParticleTbl[cell_I] = NULL;
					}
				}
				PtrMap_Append( map, self->cellParticleTbl, newSwarm->cellParticleTbl );
			}
			else {
				newSwarm->cellParticleTbl = NULL;
			}
		}
		
		if( (newSwarm->particles = PtrMap_Find( map, self->particles )) == NULL ) {
			if( self->particles ) {
				newSwarm->particles = (Particle_List)ExtensionManager_Malloc( newSwarm->particleExtensionMgr, newSwarm->particlesArraySize );
				memcpy( newSwarm->particles, self->particles, newSwarm->particlesArraySize * ExtensionManager_GetFinalSize( newSwarm->particleExtensionMgr ) );
				PtrMap_Append( map, self->particles, newSwarm->particles );
			}
			else {
				newSwarm->particles = NULL;
			}
		}

		/*shadow info*/
		if( self->shadowTablesBuilt ){

			if( (newSwarm->shadowCellParticleCountTbl = PtrMap_Find( map, self->shadowCellParticleCountTbl )) == NULL ) {
				if( self->shadowCellParticleCountTbl ) {
					newSwarm->shadowCellParticleCountTbl = Memory_Alloc_Array( Particle_InCellIndex, newSwarm->cellDomainCount, "Swarm->shadowCellParticleCountTbl" );
					memcpy( newSwarm->shadowCellParticleCountTbl, self->shadowCellParticleCountTbl, newSwarm->cellDomainCount * sizeof( Particle_InCellIndex ) );
					PtrMap_Append( map, self->shadowCellParticleCountTbl, newSwarm->shadowCellParticleCountTbl );
				}
				else {
					newSwarm->shadowCellParticleCountTbl = NULL;
				}
			}
		
			if( (newSwarm->shadowCellParticleTbl = PtrMap_Find( map, self->shadowCellParticleTbl )) == NULL ) {
				if( newSwarm->shadowCellParticleCountTbl && self->shadowCellParticleTbl ) {
					Index	cell_I;
				
					newSwarm->shadowCellParticleTbl = Memory_Alloc_Array( Cell_Particles, newSwarm->cellDomainCount, "Swarm->shadowCellParticleTbl" );
					for( cell_I = 0; cell_I < newSwarm->cellDomainCount; cell_I++ ) {
						if( newSwarm->shadowCellParticleCountTbl[cell_I] ) {
							newSwarm->shadowCellParticleTbl[cell_I] = Memory_Alloc_Array( Particle_Index, newSwarm->shadowCellParticleCountTbl[cell_I],
									"Swarm->shadowCellParticleTbl[]" );
							memcpy( newSwarm->shadowCellParticleTbl[cell_I], self->shadowCellParticleTbl[cell_I],
									newSwarm->shadowCellParticleCountTbl[cell_I] * sizeof(Particle_Index) );
						}
						else {
							newSwarm->shadowCellParticleTbl[cell_I] = NULL;
						}
					}
					PtrMap_Append( map, self->shadowCellParticleTbl, newSwarm->shadowCellParticleTbl );
				}
				else {
					newSwarm->shadowCellParticleTbl = NULL;
				}
			}
		
			if( (newSwarm->shadowParticles = PtrMap_Find( map, self->shadowParticles )) == NULL ) {
				if( self->shadowParticles ) {
					newSwarm->shadowParticles = (Particle_List)ExtensionManager_Malloc( newSwarm->particleExtensionMgr, newSwarm->shadowParticleCount);
					memcpy( newSwarm->shadowParticles, self->shadowParticles,
							newSwarm->shadowParticleCount* ExtensionManager_GetFinalSize( newSwarm->particleExtensionMgr ) );
					PtrMap_Append( map, self->shadowParticles, newSwarm->shadowParticles );
				}
				else {
					newSwarm->shadowParticles = NULL;
				}
			}		
		}

		newSwarm->ics = self->ics ? (VariableCondition*)Stg_Class_Copy( self->ics, NULL, deep, nameExt, map ) : NULL;
	}
	else {
		newSwarm->cellLayout = self->cellLayout;
		newSwarm->particleLayout = self->particleLayout;
		newSwarm->debug = self->debug;
		newSwarm->cellPointTbl = self->cellPointTbl;
		newSwarm->cellPointCountTbl = self->cellPointCountTbl;
		newSwarm->cellParticleTbl = self->cellParticleTbl;
		newSwarm->cellParticleCountTbl = self->cellParticleCountTbl;
		newSwarm->cellParticleSizeTbl = self->cellParticleSizeTbl;
		newSwarm->particles = self->particles;

		newSwarm->shadowCellParticleTbl = self->shadowCellParticleTbl;
		newSwarm->shadowCellParticleCountTbl = self->shadowCellParticleCountTbl;
		newSwarm->shadowParticles = self->shadowParticles;

		newSwarm->particleExtensionMgr = self->particleExtensionMgr;
		newSwarm->commHandlerList = self->commHandlerList;

		newSwarm->ics = self->ics;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newSwarm;
}

void* _Swarm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                _sizeOfSelf = sizeof(Swarm);
	Type                                                        type = Swarm_Type;
	Stg_Class_DeleteFunction*                                _delete = _Swarm_Delete;
	Stg_Class_PrintFunction*                                  _print = _Swarm_Print;
	Stg_Class_CopyFunction*                                    _copy = _Swarm_Copy;
	Stg_Component_DefaultConstructorFunction*    _defaultConstructor = _Swarm_DefaultNew;
	Stg_Component_ConstructFunction*                      _construct = _Swarm_AssignFromXML;
	Stg_Component_BuildFunction*                              _build = _Swarm_Build;
	Stg_Component_InitialiseFunction*                    _initialise = _Swarm_Initialise;
	Stg_Component_ExecuteFunction*                          _execute = _Swarm_Execute;
	Stg_Component_DestroyFunction*                          _destroy = _Swarm_Destroy;
	SizeT                                               particleSize = sizeof(IntegrationPoint);
	void*                                                        ics = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return _Swarm_New(  SWARM_PASSARGS  );			    /* ics_renamed */
}

void _Swarm_AssignFromXML( void* swarm, Stg_ComponentFactory* cf, void* data ) {
	Swarm*                  self                     = (Swarm*)swarm;
   AbstractContext*        context                  = NULL;
	CellLayout*             cellLayout               = NULL;
	ParticleLayout*         particleLayout           = NULL;
	void*                   extensionManagerRegister = NULL;
	double                  extraParticlesFactor     = 0.0;
	Particle_InCellIndex    cellParticleTblDelta     = 0;
	Dimension_Index         dim;
	/** Type                    particleType; */
	Variable_Register*      variable_Register        = NULL;
	VariableCondition* 	ic            		 = NULL;
    Bool                    createGlobalIdVar        = False;
    MPI_Comm comm = NULL;

	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
	if( !context  )
		context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, False, data  );

    if( context )
        comm = context->communicator;
    else
        comm = MPI_COMM_WORLD;
    
	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
    dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim );

	cellLayout =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)CellLayout_Type, CellLayout, True, data  ) ;
	particleLayout =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)ParticleLayout_Type, ParticleLayout, False, data );
	
	extensionManagerRegister = extensionMgr_Register; 
	assert( extensionManagerRegister );
	variable_Register = context ? context->variable_Register : NULL;
	
	cellParticleTblDelta = 
		Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"cellParticleTblDelta", DEFAULT_CELL_PARTICLE_TBL_DELTA );
	extraParticlesFactor = 
		Stg_ComponentFactory_GetDouble( cf, self->name, (Dictionary_Entry_Key)"extraParticlesFactor", DEFAULT_EXTRA_PARTICLES_FACTOR );
	
	{
		unsigned int count = 0;
		int i = 0;
		Stg_Component **components = NULL;

		components = (Stg_Component** )Stg_ComponentFactory_ConstructByList( cf, self->name, (Dictionary_Entry_Key)"ParticleCommHandlers", Stg_ComponentFactory_Unlimited, ParticleCommHandler, False, &count, data );

		if( count == 0  ){
 			Journal_Printf( self->debug, "Warning: Swarm has 0 Communication handlers..!\n" );
 		}
 		else{
 			for( i=0; i<count; i++ ){
 				Stg_ObjectList_Append( self->commHandlerList, components[i] );
 			}
 			Memory_Free( components );
		}
	}

	/* construct the variable condition IC */
	ic = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"IC", VariableCondition, False, data  );

    createGlobalIdVar = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"createGlobalId", False );
	
	_Swarm_Init( 
			self, context,
			cellLayout,
			particleLayout, 
			dim,
         sizeof(IntegrationPoint),
			cellParticleTblDelta,
			extraParticlesFactor,
			extensionManagerRegister,
			variable_Register,
            createGlobalIdVar,
			comm,
			ic );
}


void _Swarm_Build( void* swarm, void* data ) {
	Swarm* self = (Swarm*)swarm;

	Stg_Component_Build( self->cellLayout, data, False );
	Stg_Component_Build( self->particleLayout, data, False );

	_Swarm_BuildCells( self, data );
	_Swarm_BuildParticles( self, data );
	Stg_Component_Build( self->owningCellVariable, data, False );
    Stg_Component_Build( self->globalIdVariable, data, False );

}

			
void _Swarm_Initialise( void* swarm, void* data ) {
	Swarm* self = (Swarm*)swarm;

	Journal_DPrintf( self->debug, "In %s(): for swarm \"%s\" (of type %s)\n", __func__, self->name, self->type ); 
	Stream_IndentBranch( Swarm_Debug );
    Stg_Component_Initialise( self->owningCellVariable, data, False );
    Stg_Component_Build( self->globalIdVariable, data, False );

	Stg_Component_Initialise( self->cellLayout, data, False );
	Stg_Component_Initialise( self->particleLayout, data, False );

	_Swarm_InitialiseCells( self, data );

	_Swarm_InitialiseParticles( self, data );

	if( self->ics ) {
		Journal_DPrintf( self->debug, "applying the ICs for this swarm.\n" );
		Stream_Indent( self->debug );
		Stg_Component_Initialise( self->ics, data, False );
		/* call the initial conditions plugin here */
		VariableCondition_Apply( self->ics, data );
	}

	Stream_UnIndentBranch( Swarm_Debug );
   
   Journal_DPrintf( self->debug, "...done in %s().\n", __func__ );

}


void _Swarm_Execute( void* swarm, void* data ) {
}

void _Swarm_Destroy( void* swarm, void* data ) {
	Swarm* self = (Swarm*)swarm;
   Cell_LocalIndex cell_I;

	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		if( self->cellParticleTbl[cell_I] ){
			Memory_Free( self->cellParticleTbl[cell_I] ); self->cellParticleTbl[cell_I] = NULL;
		}

		if(self->shadowTablesBuilt){
			if( self->shadowCellParticleTbl[cell_I] ){
				Memory_Free( self->shadowCellParticleTbl[cell_I] ); self->shadowCellParticleTbl[cell_I] = NULL;
			}
		}
	}

	if( self->shadowTablesBuilt ){
		Memory_Free( self->shadowCellParticleTbl ); self->shadowCellParticleTbl = NULL;
		Memory_Free( self->shadowCellParticleCountTbl ); self->shadowCellParticleCountTbl = NULL;
		if ( self->shadowParticles ) {
			ExtensionManager_Free( self->particleExtensionMgr, self->shadowParticles ); self->shadowParticles = NULL;
		}
	}

	Memory_Free( self->cellParticleTbl ); self->cellParticleTbl = NULL;
	Memory_Free( self->cellParticleCountTbl ); self->cellParticleCountTbl = NULL;
	Memory_Free( self->cellParticleSizeTbl ); self->cellParticleSizeTbl = NULL;
	if ( self->particles ) {
		ExtensionManager_Free( self->particleExtensionMgr, self->particles ); self->particles = NULL;
	}
	if(self->owningCellVariable)
		Stg_Component_Destroy(self->owningCellVariable, data, False );


	FreeArray( self->swarmVars ); self->swarmVars = NULL;

	Memory_Free( self->cellPointTbl ); self->cellPointTbl = NULL;
	Memory_Free( self->cellPointCountTbl ); self->cellPointCountTbl = NULL;

}

void _Swarm_BuildCells( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
	Cell_Index		cell_I;
	Cell_PointIndex		pointCount;
	
	/* Need to do this first - as the cellLayout may be dependent on a mesh etc */
	Stg_Component_Build( self->cellLayout, data, False );

	Journal_DPrintf( self->debug, "In %s():\n", __func__ ); 
	Stream_IndentBranch( Swarm_Debug );

	self->cellLocalCount = CellLayout_CellLocalCount( self->cellLayout );
	self->cellShadowCount = CellLayout_CellShadowCount( self->cellLayout );
	self->cellDomainCount = self->cellLocalCount + self->cellShadowCount;

	Journal_DPrintf( self->debug, "CellLayout \"%s\" (of type %s) returned cell counts of:\n"
		"cellLocalCount: %u, cellShadowCount: %u, cellDomainCount: %u",
		self->cellLayout->name, self->cellLayout->type,
		self->cellLocalCount, self->cellShadowCount, self->cellDomainCount );

	self->cellPointCountTbl = Memory_Alloc_Array( Cell_PointIndex, self->cellDomainCount, "Swarm->cellPointCountTbl" );
	pointCount = 0;

	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		self->cellPointCountTbl[cell_I] = CellLayout_PointCount( self->cellLayout, cell_I );
		pointCount += self->cellPointCountTbl[cell_I];
	}

	self->cellPointTbl = Memory_Alloc_2DComplex( Cell_Point, self->cellDomainCount, self->cellPointCountTbl, "Swarm->cellPointTbl" );

	self->cellParticleCountTbl = Memory_Alloc_Array( Particle_InCellIndex, self->cellDomainCount,
		"Swarm->cellParticleCountTbl" );
	self->cellParticleSizeTbl = Memory_Alloc_Array( Particle_InCellIndex, self->cellDomainCount,
		"Swarm->cellParticleSizeTbl" );
	self->cellParticleTbl = Memory_Alloc_Array( Cell_Particles, self->cellDomainCount, "Swarm->cellParticleTbl" ); 

   for( cell_I = 0; cell_I < self->cellLocalCount; cell_I++ ) {
      /* Set initial counts to empty, till we add the particles */
      self->cellParticleCountTbl[cell_I] = 0;
      /* lets default to size of 1  */
      self->cellParticleSizeTbl[cell_I] = 1;
      self->cellParticleTbl[cell_I] = NULL;
   }
   /* Now initialise the shadow cell particle counts */
   for (; cell_I < self->cellDomainCount; cell_I++ ) {
      self->cellParticleCountTbl[cell_I] = 0;
      self->cellParticleSizeTbl[cell_I] = 0;
      self->cellParticleTbl[cell_I] = NULL;
   }

	Stream_UnIndentBranch( Swarm_Debug );
   
}


void _Swarm_BuildParticles( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
        unsigned                v_i;	
	/* Need to do this first - as the particleLayout may be dependent on a mesh etc */
	Stg_Component_Build( self->particleLayout, data, False );
	
	ParticleLayout_SetInitialCounts( self->particleLayout, self );
	
	/* Now allocate the particles array */
	/* We allocate extra space to try & avoid avoid reallocing later as particles are advected etc. */
	self->particlesArrayDelta = (Particle_Index)( (double) self->particleLocalCount * self->extraParticlesFactor);
	if ( 0 == self->particlesArrayDelta ) {
		self->particlesArrayDelta = MINIMUM_PARTICLES_ARRAY_DELTA;		
	}

	self->particlesArraySize = self->particleLocalCount + self->particlesArrayDelta;

	self->particles = (Particle_List)ExtensionManager_Malloc( self->particleExtensionMgr, self->particlesArraySize );
	/*
	** NEED TO UPDATE THINGS IF ARRAYS ARE REALLOC'D
	*/
	for( v_i = 0; v_i < self->nSwarmVars; v_i++ ) {
		if( self->swarmVars[v_i]->variable )
			StgVariable_Update( self->swarmVars[v_i]->variable );
        }
   /* lets finish allocing cells */
   char tempStr[100];
   Cell_LocalIndex cell_I;
   for( cell_I = 0; cell_I < self->cellLocalCount; cell_I++ ) {
      sprintf( tempStr, "self->cellParticleTbl[%d]", cell_I );
      self->cellParticleTbl[cell_I] = Memory_Alloc_Array( Particle_Index, self->cellParticleSizeTbl[cell_I], tempStr );
   }

}

void _Swarm_BuildShadowParticles( void* swarm ) {
	Swarm*			self = (Swarm*)swarm;
	int i = 0;
	
	self->shadowTablesBuilt = True;

	self->shadowCellParticleCountTbl = Memory_Alloc_Array( Particle_InCellIndex, self->cellDomainCount,
		"Swarm->shadowCellParticleCountTbl" );
	self->shadowCellParticleTbl = Memory_Alloc_Array( Cell_Particles, self->cellDomainCount, "Swarm->shadowCellParticleTbl" );
	for( i=0; i<self->cellDomainCount; i++ ){
		self->shadowCellParticleTbl[i] = NULL;
	}
	
	self->shadowParticles = NULL;
}

void _Swarm_InitialiseCells( void* swarm, void* data ) {
	Swarm*			self = (Swarm*)swarm;
	Cell_Index		cell_I;
	
	for( cell_I = 0; cell_I < self->cellDomainCount; cell_I++ ) {
		CellLayout_InitialiseCellPoints( self->cellLayout, cell_I, self->cellPointCountTbl[cell_I], 
			self->cellPointTbl[cell_I] );
	}
}


void _Swarm_InitialiseParticles( void* swarm, void* data ) {
	Swarm* self = (Swarm*)swarm;
    int v_i;
	
	ParticleLayout_InitialiseParticles( self->particleLayout, self );

	/* Need to re-do the delta calculation here, since if using a global particle layouts, we didn't know the 
	 * localParticleCount until the above call, and had just used a default delta. We can now calculate the
	 * correct one. */
	self->particlesArrayDelta = (Particle_Index)( (double) self->particleLocalCount * self->extraParticlesFactor );
	if ( 0 == self->particlesArrayDelta ) {
		self->particlesArrayDelta = MINIMUM_PARTICLES_ARRAY_DELTA;		
	}
    
	for( v_i = 0; v_i < self->nSwarmVars; v_i++ ) {
		if( self->swarmVars[v_i]->variable )
			StgVariable_Update( self->swarmVars[v_i]->variable );
        }


	Swarm_UpdateAllParticleOwners( self );
}


/** This function assumes particle advection and hence global coords are used */
void Swarm_UpdateAllParticleOwners( void* swarm ) {
	Swarm*			self = (Swarm*)swarm;
	Particle_Index		lParticle_I;
	Progress*		prog;
    int v_i;

   /* if not advecting, nothing to do */
//	if ( !self->isAdvecting ) {
//		return;
//	}

	prog = Progress_New();
	Progress_SetTitle( prog, "Updating particle owners" );
	Progress_SetPrefix( prog, "\t" );
	Progress_SetRange( prog, 0, self->particleLocalCount );
	Progress_Update( prog );
	
	Journal_DPrintfL( self->debug, 1, "In %s() for Swarm \"%s\"\n", __func__, self->name );
	Stream_IndentBranch( Swarm_Debug );	
	for ( lParticle_I=0; lParticle_I < self->particleLocalCount; lParticle_I++ ) {
		Swarm_UpdateParticleOwner( self, lParticle_I );
		Progress_Increment( prog );
	}

	Stg_Class_Delete( prog );

    {
        int ii;
        for( ii=0; ii<self->commHandlerList->count; ii++ ){
            ParticleCommHandler *pComm = NULL;

            pComm = (ParticleCommHandler*)(Stg_ObjectList_At(self->commHandlerList, ii));
            Stg_Component_Execute( pComm, self, True);
        }
    }

	for( v_i = 0; v_i < self->nSwarmVars; v_i++ )
    {
		if( self->swarmVars[v_i]->variable )
			StgVariable_Update( self->swarmVars[v_i]->variable );
    }

	Stream_UnIndentBranch( Swarm_Debug );
}


void Swarm_UpdateParticleOwner( void* swarm, Particle_Index particle_I ) {
	Swarm* 			self           = (Swarm*)swarm;
	GlobalParticle*	        particle       = (GlobalParticle*) Swarm_ParticleAt( self, particle_I );
	Cell_DomainIndex	newOwningCell;
	Particle_InCellIndex	cParticle_I;

	newOwningCell = CellLayout_CellOf( self->cellLayout, particle );

	if ( newOwningCell != particle->owningCell ) { /* if not still in same cell */
        if (particle->owningCell < self->cellDomainCount) {  /* if currently owned, remove */
            Cell_LocalIndex		oldOwningCell = particle->owningCell;
            cParticle_I = Swarm_GetParticleIndexWithinCell( self, particle->owningCell, particle_I );
            Swarm_RemoveParticleFromCell( self, oldOwningCell, cParticle_I );
        }
	
		if ( newOwningCell == self->cellDomainCount ) {
			/* "New cell == domain count -> Particle has moved outside domain */
			particle->owningCell = self->cellDomainCount;
		}	
		else {
            /* if new cell is in my domain, add entry to new cell's table */
			Swarm_AddParticleToCell( self, newOwningCell, particle_I );
		}
	}
	Stream_UnIndent( self->debug );	
}


void Swarm_RemoveParticleFromCell( void* swarm, Cell_DomainIndex dCell_I, Particle_InCellIndex cParticle_I ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex*	sizePtr = &self->cellParticleSizeTbl[dCell_I];
	Particle_InCellIndex*	countPtr = &self->cellParticleCountTbl[dCell_I];

	self->cellParticleTbl[dCell_I][cParticle_I] = self->cellParticleTbl[dCell_I][*countPtr-1];
	(*countPtr)--;
	if ( *countPtr == (*sizePtr - self->cellParticleTblDelta) ) {
		(*sizePtr) = *countPtr;
		self->cellParticleTbl[dCell_I] = Memory_Realloc_Array( self->cellParticleTbl[dCell_I],
			Particle_Index, *sizePtr );
	}
}


void Swarm_DeleteParticle( void* swarm, Particle_Index particleToDelete_lI ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex    cParticle_I = 0;
	Particle_Index          lastParticle_I = 0;
	GlobalParticle*         lastParticle = NULL;	
	GlobalParticle*         particleToDelete = NULL;	
	SizeT                   particleSize = self->particleExtensionMgr->finalSize;
	Stream*                 errorStr = Journal_Register( Error_Type, (Name)self->type  );

	Journal_Firewall( particleToDelete_lI < self->particleLocalCount, errorStr,
		"Error- in %s(): particleToDelete_lI passed in (%u) is >= swarm's local particle count %u.\n",
		__func__, particleToDelete_lI, self->particleLocalCount );

	particleToDelete = (GlobalParticle*)Swarm_ParticleAt( self, particleToDelete_lI );
	cParticle_I = Swarm_GetParticleIndexWithinCell( self, particleToDelete->owningCell, particleToDelete_lI );

	Swarm_RemoveParticleFromCell( self, particleToDelete->owningCell, cParticle_I );

	lastParticle_I = self->particleLocalCount - 1;
	lastParticle   = (GlobalParticle*)Swarm_ParticleAt( self, lastParticle_I );

	/* In the current data structure for particles (a regular array), if we delete a particle we need to "swap"
	 * the 'last' particle into the hole we just created, and update it's cell's reference to it.
	 * The only special case is if the particle we are deleting happens to be the last particle, in which case
	 * no swap is necessary. */ 
	if ( particleToDelete_lI != lastParticle_I ) {
		/* Get last Particle information */
		Cell_Index            lastParticle_CellIndex       = lastParticle->owningCell;
		Particle_InCellIndex  lastParticle_IndexWithinCell = 0;
		
		lastParticle_IndexWithinCell = Swarm_GetParticleIndexWithinCell( self, lastParticle_CellIndex, lastParticle_I);

		/* Copy over particle */
		memcpy( particleToDelete, lastParticle, particleSize );
			
		/* Change value in cell particle table to point to new index in array */
		self->cellParticleTbl[lastParticle_CellIndex][ lastParticle_IndexWithinCell ] = particleToDelete_lI;
	}

	/* re-set memory at location of last particle to zero so it is clear that it's been deleted */
	memset( lastParticle, 0, particleSize );

	self->particleLocalCount--;
	/* Call the memory management function in case we need to re-allocate the swarm size smaller now */
	Swarm_Realloc( swarm );
}


void Swarm_DeleteParticleAndReplaceWithNew( void* swarm, Particle_Index particleToDelete_lI,
		void* replacementParticle, Cell_Index replacementParticle_cellIndex )
{
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex    cParticle_I = 0;
	GlobalParticle*         particleToDelete = NULL;	
	Stream*                 errorStr = Journal_Register( Error_Type, (Name)self->type  );
	
	Journal_Firewall( particleToDelete_lI < self->particleLocalCount, errorStr,
		"Error- in %s(): particleToDelete_lI passed in (%u) is >= swarm's local particle count %u.\n",
		__func__, particleToDelete_lI, self->particleLocalCount );

	particleToDelete = (GlobalParticle*)Swarm_ParticleAt( self, particleToDelete_lI );
	cParticle_I = Swarm_GetParticleIndexWithinCell( self, particleToDelete->owningCell, particleToDelete_lI );

	Swarm_RemoveParticleFromCell( self, particleToDelete->owningCell, cParticle_I );

	Journal_DPrintfL( self->debug, 2, 
		"Copying over particle %u using replacement particle, and adding it to cell %u\n", 
		particleToDelete_lI, replacementParticle_cellIndex );

	/* Copy over particle to delete with it's replacement */
	memcpy( particleToDelete, replacementParticle, self->particleExtensionMgr->finalSize );
			
	/* Add a reference to replacement particle in appropriate cell entry */
	Swarm_AddParticleToCell( self, replacementParticle_cellIndex, particleToDelete_lI );
}


void Swarm_AddParticleToCell( void* swarm, Cell_DomainIndex dCell_I, Particle_Index particle_I ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex*	newCountPtr = &self->cellParticleCountTbl[dCell_I];
	Particle_InCellIndex*	newSizePtr = &self->cellParticleSizeTbl[dCell_I];

	Swarm_ParticleAt( self, particle_I )->owningCell = dCell_I;

	if ( *newCountPtr == *newSizePtr ) {
		(*newSizePtr) += self->cellParticleTblDelta;
		self->cellParticleTbl[dCell_I] = Memory_Realloc_Array( self->cellParticleTbl[dCell_I],
			Particle_Index, (*newSizePtr) );
	}	
	self->cellParticleTbl[dCell_I][*newCountPtr] = particle_I;
	(*newCountPtr)++;
}

void Swarm_AddShadowParticleToShadowCell( void* swarm, Cell_DomainIndex dCell_I, Particle_Index shadowParticle_I ) {
	Swarm* 			self = (Swarm*)swarm;
	
	if( self->shadowTablesBuilt ){
		Particle_InCellIndex*	newCountPtr = &self->shadowCellParticleCountTbl[dCell_I-self->cellLocalCount];

		Journal_DPrintfL( self->debug, 3, "Adding shadow particle %d to shadow cell %d: shadow cell's particle count now %d",
			shadowParticle_I, dCell_I, (*newCountPtr)+1 );

		Swarm_ShadowParticleAt( self, shadowParticle_I)->owningCell = dCell_I;

		self->shadowCellParticleTbl[dCell_I][*newCountPtr] = shadowParticle_I;
		(*newCountPtr)++;
		Journal_DPrintfL( self->debug, 3, "\n" );
	}
}

Particle_InCellIndex Swarm_GetParticleIndexWithinCell( void* swarm, Cell_DomainIndex owningCell, Particle_Index particle_I) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_InCellIndex	cParticle_I;
	Particle_InCellIndex	particleCount = self->cellParticleCountTbl[owningCell];

	/* find the PIC index in the cell */
	for ( cParticle_I=0; cParticle_I < particleCount; cParticle_I++ ) {
		if ( particle_I == self->cellParticleTbl[owningCell][cParticle_I] ) break;
	}	
	#if DEBUG
		Journal_Firewall( cParticle_I < self->cellParticleCountTbl[owningCell], Swarm_Error,
			"Error- In func %s: Particle %u not found among cell %u's %u particles.\n",
			__func__, particle_I, owningCell, particleCount );
	#endif

	return cParticle_I;
}

Particle_Index Swarm_FindClosestParticle( void* _swarm, Dimension_Index dim, double* coord, double *distance) {
	Swarm*               swarm       = (Swarm*) _swarm;
	Particle_InCellIndex cParticle_I;
	Cell_LocalIndex      lCell_I;
	GlobalParticle       testParticle;
	double               minDistance;
	double               distanceToParticle;
	NeighbourIndex       neighbourCount;
	NeighbourIndex       neighbour_I;
	NeighbourIndex*      neighbourList;
	Particle_Index       closestParticle_I;

	/* Find cell this coordinate is in */
	memcpy( testParticle.coord, coord, sizeof(Coord) );
	/* First specify the particle doesn't have an owning cell yet, so as
	not to confuse the search algorithm */
	testParticle.owningCell = swarm->cellDomainCount;
	lCell_I = CellLayout_CellOf( swarm->cellLayout, &testParticle );

	/* Test if this cell is on this processor - if not then bail */
	if (lCell_I >= swarm->cellLocalCount)
		return (Particle_Index) -1;

	/* Find Closest Particle in this Cell */
	cParticle_I = Swarm_FindClosestParticleInCell( swarm, lCell_I, dim, coord, &minDistance );

	/* Convert to Local Particle Index */
	closestParticle_I = swarm->cellParticleTbl[ lCell_I ][ cParticle_I ];

	/* Find neighbours to this cell - TODO This Assumes ElementCellLayout */
	Mesh_GetIncidence( ((ElementCellLayout*)swarm->cellLayout)->mesh, dim, lCell_I, dim, swarm->incArray );
	neighbourCount = IArray_GetSize( swarm->incArray );
	neighbourList = IArray_GetPtr( swarm->incArray );

	/* Loop over neighbours */
	for ( neighbour_I = 0 ; neighbour_I < neighbourCount ; neighbour_I++ ) {
		lCell_I = neighbourList[ neighbour_I ];

		if( lCell_I < swarm->cellDomainCount ) {
			cParticle_I = Swarm_FindClosestParticleInCell( swarm, lCell_I, dim, coord, &distanceToParticle );

			/* Check to see if closest particle in this cell is closest to this coord */
			if (minDistance > distanceToParticle) {
				minDistance = distanceToParticle;
				closestParticle_I = swarm->cellParticleTbl[ lCell_I ][ cParticle_I ];
			}
		}
	}

	/* Return Distance to this particle */
	if (distance != NULL)
		*distance = minDistance;
	return closestParticle_I;
}


Particle_InCellIndex Swarm_FindClosestParticleInCell( void* swarm, Cell_DomainIndex dCell_I, Dimension_Index dim, double* coord, double* distance ) {
	Swarm*               self                = (Swarm*) swarm;
	Particle_InCellIndex cParticle_I         = 0;
	Particle_InCellIndex particle_I          = 0;
	GlobalParticle*      particle            = NULL;
	double               minDistance         = HUGE_VAL;
	double               distanceToParticle;

	/* TODO: need to reconsider - gauss particle layout should be allowed, but not swarms that have no local
	 * co-ordinates */
	/*
	Journal_Firewall(
		self->particleLayout->coordSystem == GlobalCoordSystem,
		Journal_MyStream( Error_Type, self ),
		"Error in %s(), swarm %s:%s is not using a global coord system\n",
		__func__,
		self->type,
		self->name );
	*/	
	
	Journal_Firewall( dCell_I < self->cellDomainCount, Swarm_Error, "Bad Cell_DomainIndex %u.\n", dCell_I );

	/* Loop over particles find closest to vertex */
	for( cParticle_I = 0 ; cParticle_I < self->cellParticleCountTbl[dCell_I] ; cParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( self, dCell_I, cParticle_I );

		/* Calculate distance to particle */
		distanceToParticle = 
			(particle->coord[ I_AXIS ] - coord[ I_AXIS ]) * 
			(particle->coord[ I_AXIS ] - coord[ I_AXIS ]) +
			(particle->coord[ J_AXIS ] - coord[ J_AXIS ]) * 
			(particle->coord[ J_AXIS ] - coord[ J_AXIS ]) ;

		if (dim == 3) {
			distanceToParticle += 
				(particle->coord[ K_AXIS ] - coord[ K_AXIS ]) * 
				(particle->coord[ K_AXIS ] - coord[ K_AXIS ]) ;
		}
		/* Don't do square root here because it is unnessesary: i.e. a < b <=> sqrt(a) < sqrt(b) */
			
		/* Check if this is the closest particle */
		if (minDistance > distanceToParticle) {
			particle_I = cParticle_I;
			minDistance = distanceToParticle;
		}
	}

	/* Return Distance to this particle */
	if (distance != NULL)
		/* Do square root here in case someone wants to actually use this distance */
		*distance = sqrt(minDistance);

	return particle_I;
}

void Swarm_PrintParticleCoords( void* swarm, Stream* stream ) {
	Swarm* 			self = (Swarm*)swarm;
	Particle_Index		lParticle_I=0;
	GlobalParticle*   	currParticle = NULL;
	double*			coord = NULL;
	
	Journal_Printf( stream, "Printing coords of all local processor particles:\n" );
	
	Stream_Indent( stream );
	for ( lParticle_I = 0; lParticle_I < self->particleLocalCount; lParticle_I++ ) {
		currParticle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );
		coord = currParticle->coord;

		Journal_Printf( stream, "local particle %6d: (%7.5g, %7.5g, %7.5g)\n",
			lParticle_I, coord[0], coord[1], coord[2] );
	}
	Stream_UnIndent( stream );
}


void Swarm_PrintParticleCoords_ByCell( void* swarm, Stream* stream ) {
	Swarm* 			self = (Swarm*)swarm;
	Cell_Index		lCell_I=0;
	Particle_InCellIndex	cParticle_I=0;
	Particle_Index		lParticle_I=0;
	GlobalParticle*         currParticle = NULL;
	double*			coord = NULL;
	
	Journal_Printf( stream, "Printing coords of all local particles, cell-by-cell:\n" );
	
	Stream_Indent( stream );
	for ( lCell_I = 0; lCell_I < self->cellLocalCount; lCell_I++ ) {
		Journal_Printf( stream, "Local Cell %3d:\n", lCell_I );
		
		for ( cParticle_I = 0; cParticle_I < self->cellParticleCountTbl[lCell_I]; cParticle_I++ ) {
			lParticle_I = self->cellParticleTbl[lCell_I][cParticle_I];
			currParticle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );
			coord = currParticle->coord;

			Journal_Printf( stream, "\tpart. InCell %3d (local index %6d): (%7.5g, %7.5g, %7.5g)\n",
				cParticle_I, lParticle_I, coord[0], coord[1], coord[2] );
		}		
	}
	Stream_UnIndent( stream );
}


void Swarm_GetCellMinMaxCoords( void* swarm, Cell_DomainIndex cell_I, Coord min, Coord max ) {
	Swarm*              self     =  (Swarm*) swarm;
	Dimension_Index		  dim_I;
	int                 dim      = self->dim;
	Cell_PointIndex		  cPoint_I;
	double*             currCoord;

	for ( dim_I = 0; dim_I < dim; dim_I++ ) {
		min[dim_I] = (self->cellPointTbl[cell_I][0])[dim_I];
		max[dim_I] = (self->cellPointTbl[cell_I][0])[dim_I];
	}

	for ( cPoint_I = 1; cPoint_I < self->cellPointCountTbl[cell_I]; cPoint_I++ ) {
		for ( dim_I = 0; dim_I < dim; dim_I++ ) {
			currCoord = (self->cellPointTbl[cell_I][cPoint_I]);
			if ( currCoord[dim_I] < min[dim_I] ) {
				min[dim_I] = currCoord[dim_I];
			}
			else if ( currCoord[dim_I] > max[dim_I] ) {
				max[dim_I] = currCoord[dim_I];
			}
		}
	}
}



SwarmVariable* Swarm_NewScalarVariable( 
		void*                           swarm,
		Name                            nameExt,
		int                             dataOffset,
		StgVariable_DataType               dataType )
{
	Swarm*                   self              = (Swarm*) swarm;
	Name                     name;
	StgVariable*                variable;
	SizeT                    dataOffsets[]     = { 0 };		/* Init value later */
	StgVariable_DataType        dataTypes[]       = { 0 };		/* Init value later */
	Index                    dataTypeCounts[]  = { 1 };
	SwarmVariable*           swarmVariable;
	Variable_Register*       variable_Register      = NULL;
	SwarmVariable_Register*  swarmVariable_Register = NULL;

    if( dataOffset == -1 ){
        dataOffset = ExtensionManager_GetFinalSize( self->particleExtensionMgr );
        ExtensionManager_Add( self->particleExtensionMgr, (Name)self->type, StgVariable_SizeOfDataType(dataType) ) ;
    }
	Journal_Firewall(
		dataOffset < ExtensionManager_GetFinalSize( self->particleExtensionMgr ),
		Journal_MyStream( Error_Type, self ),
		"Error in func %s - Failed to create Variable from extension: data offset within particle structure provided was invalid.\n"
		"\t%s %s\n"
		"\tdataOffset = %d, particle size = %d\n",
		__func__,
		self->type,
		self->name,
		dataOffset,
		ExtensionManager_GetFinalSize( self->particleExtensionMgr ) );
		
	dataOffsets[0] = dataOffset;
	dataTypes[0]   = dataType;
	
	/* Get Pointers To Registers */
	swarmVariable_Register = self->swarmVariable_Register;
	if ( swarmVariable_Register ) 
		variable_Register = swarmVariable_Register->variable_Register;
	
	name = Stg_Object_AppendSuffix( self, (Name)nameExt  );
	variable = StgVariable_New( 
		name,
		self->context,
		1, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		0, /* no component names */
		&self->particleExtensionMgr->finalSize,
		&self->particleLocalCount,
		NULL,
		(void**)&self->particles,
		variable_Register );

	swarmVariable = SwarmVariable_New( name, self->context, self, variable, 1, False );

	Memory_Free( name );

	return swarmVariable;
}

SwarmVariable* Swarm_NewVectorVariable( 
		void*                           _swarm,
		Name                            nameExt,
		int                             dataOffset,
		StgVariable_DataType               dataType,
		Index                           dataTypeCount,
		...                         /* vector component names */ )
{
	Swarm*                   self             = (Swarm*) _swarm;
	StgVariable*                variable;
	SizeT                    dataOffsets[]    = { 0 };	/* Init later... */
	StgVariable_DataType	     dataTypes[]      = { 0 };	/* Init later... */	
	Index                    dataTypeCounts[] = { 0 };	/* Init later... */	
	Name*                    dataNames;
	Index                    vector_I;
	Name                     name;
	SwarmVariable*           swarmVariable;
	Variable_Register*       variable_Register      = NULL;
	SwarmVariable_Register*  swarmVariable_Register = NULL;
	va_list                  ap;

    if( dataOffset == -1 ){
        dataOffset = ExtensionManager_GetFinalSize( self->particleExtensionMgr );
        ExtensionManager_Add( self->particleExtensionMgr, (Name)self->type, dataTypeCount*StgVariable_SizeOfDataType(dataType) ) ;
    }
	Journal_Firewall(
		dataOffset < ExtensionManager_GetFinalSize( self->particleExtensionMgr ),
		Journal_MyStream( Error_Type, self ),
		"Error in func %s - Failed to create Variable from extension: data offset within particle structure provided was invalid.\n"
		"\t%s %s\n"
		"\tdataOffset = %d, particle size = %d\n",
		__func__,
		self->type,
		self->name,
		dataOffset,
		ExtensionManager_GetFinalSize( self->particleExtensionMgr ) );


	/* Initialise arrays */
	dataOffsets[0]    = dataOffset;
	dataTypes[0]      = dataType;
	dataTypeCounts[0] = dataTypeCount;
	
	/* Create name for normal variable */
	Stg_asprintf( &name, "%s-%s", self->name, nameExt );

	/* Get names of extra variables */
	dataNames = Memory_Alloc_Array( Name, dataTypeCount, "dataNames" );
    va_start(ap, dataTypeCount );
    Name first = (Name)va_arg(ap, Name);
    if( !first ) {  // if NULL, continue add names programatically
        char suffix[50] = "";
        for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
            if( dataTypeCount > 1 ) sprintf((char*)suffix, "%u", vector_I);
                dataNames[vector_I] = Stg_Object_AppendSuffix( self, (Name) suffix );
        }
    } else {
        va_start( ap, dataTypeCount );
        for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
        dataNames[vector_I] = Stg_Object_AppendSuffix( self, (Name ) va_arg( ap, Name ) );
        }
        va_end( ap );
    }
	
	/* Get Pointers To Registers */
	swarmVariable_Register = self->swarmVariable_Register;
	if ( swarmVariable_Register ) 
		variable_Register = swarmVariable_Register->variable_Register;
	
	/* Construct */
	variable = StgVariable_New( 
		name,
		self->context,
		1, 
		dataOffsets, 
		dataTypes, 
		dataTypeCounts, 
		dataNames,
		&self->particleExtensionMgr->finalSize,
		&self->particleLocalCount,
		NULL,
		(void**)&self->particles,
		variable_Register );

	/* Need to free these guys individually */
    if ( swarmVariable_Register && variable_Register ) {
        for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
			swarmVariable = SwarmVariable_New(
					dataNames[ vector_I ], self->context, 
					self, 
					Variable_Register_GetByName( variable_Register, dataNames[ vector_I ] ),
					1, False );
		}
		Memory_Free( dataNames[ vector_I ] );
	}
	swarmVariable = SwarmVariable_New( name, self->context, self, variable, dataTypeCount, False );

	Memory_Free( dataNames );
	Memory_Free( name );
	
	return swarmVariable;
}

StgVariable* Swarm_GetShadowVariable( void* _swarm, StgVariable* variable )
{
	Swarm* self = (Swarm*) _swarm;
	/* Construct */
	return StgVariable_New(
		NULL,
		NULL,
		1, 
		variable->offsets,
		variable->dataTypes,
		variable->dataTypeCounts,
		NULL,
		&self->particleExtensionMgr->finalSize,
		&self->shadowParticleCount,
		NULL,
		(void**)&self->shadowParticles,
		NULL );

}

void Swarm_Realloc( void* swarm ) {
	Swarm*         self               = (Swarm*) swarm;
	Particle_Index particleLocalCount = self->particleLocalCount;
	Particle_Index delta              = self->particlesArrayDelta;
	int v_i;
    int p_i;
    Bool reallocSwarm;


	if ( !self->expanding && particleLocalCount <= self->particlesArraySize - delta ) {
		/* Decrease size of array if necessary */
		self->particlesArraySize = particleLocalCount;
        reallocSwarm = True;
	}
	else if ( particleLocalCount >= self->particlesArraySize ) {
	    /* Increase size of array if necessary */
	    self->particlesArraySize = (particleLocalCount/delta + 1)*delta;
        reallocSwarm = True;
	}
	else if ( self->particleExtensionMgr->finalSize != self->preReallocParticleSize) {
		/* If change in the size of the particle.. */
        reallocSwarm = True;
	}
    else {
        reallocSwarm = False;     
    }

	/* Do realloc */
    if ( reallocSwarm )
        self->particles = Memory_Realloc_Array_Bytes(
                self->particles,
                self->particleExtensionMgr->finalSize,
                self->particlesArraySize );

    
    /* ok, now if particle size has changed, we need to restride the data */
    if ( self->particleExtensionMgr->finalSize > self->preReallocParticleSize ) {
        /* if particles expanding, work in reverse */
        for (p_i=particleLocalCount-1; p_i>0; p_i--) {
            size_t oldLocation = (size_t)self->particles + p_i*self->preReallocParticleSize;
            size_t newLocation = (size_t)self->particles + p_i*self->particleExtensionMgr->finalSize;
            // note that we use memmove incase of overlapping source/destination
            memmove(newLocation, oldLocation, self->preReallocParticleSize);
        }
    } else if ( self->particleExtensionMgr->finalSize < self->preReallocParticleSize) {
        Journal_Firewall( NULL, NULL, "Error in func %s - Particle contractions not currently supported.", __func__);
//        /* if particles contracting, work forward */
//        for (p_i=1; p_i<particleLocalCount; p_i++) {
//            size_t oldLocation = (size_t)self->particles + p_i*self->preReallocParticleSize;
//            size_t newLocation = (size_t)self->particles + p_i*self->particleExtensionMgr->finalSize;
//            memcpy(newLocation, oldLocation, self->preReallocParticleSize);
//        }
    }

    if(reallocSwarm){
      /*
      ** NEED TO UPDATE THINGS IF ARRAYS ARE REALLOC'D
      */
      for( v_i = 0; v_i < self->nSwarmVars; v_i++ ) {
		if( self->swarmVars[v_i]->variable )
          StgVariable_Update( self->swarmVars[v_i]->variable );
      }
    
      self->preReallocParticleSize = self->particleExtensionMgr->finalSize;


#ifdef DEBUG
      /* Set extra memory at end of array to zero */
      if ( self->particlesArraySize > particleLocalCount ) {
		Particle_Index newParticleCount = self->particlesArraySize - particleLocalCount;
		void*          startNewParticlePtr = Swarm_ParticleAt( self, particleLocalCount );
		memset( startNewParticlePtr, 0, self->particleExtensionMgr->finalSize * newParticleCount );
      }
#endif	
    }
}


void Swarm_CheckCoordsAreFinite( void* swarm ) {
	Swarm*              self               = (Swarm*) swarm;
	GlobalParticle*     particle;
	double*             coord;
	Stream*             errorStream        = Journal_Register( Error_Type, (Name)self->type );
	Dimension_Index     dim                = self->dim;
	Particle_Index      particleLocalCount = self->particleLocalCount;
	Particle_Index      lParticle_I;

	for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++ ) {
		particle = (GlobalParticle* )Swarm_ParticleAt( self, lParticle_I );
		coord    = particle->coord;
		
		Journal_Firewall( 
				! isinf( coord[0] ) && ! isinf( coord[1] ) && ( dim == 2 || ! isinf(coord[2]) ), 
				errorStream,
				"Error in func %s - Coord for particle with %u is not finite (%g, %g, %g).\n", 
				__func__, lParticle_I, coord[0], coord[1], (dim == 3 ? coord[2] : 0.0) );
	}
}


void Swarm_AssignIndexWithinShape( void* swarm, void* _shape, StgVariable* variableToAssign, Index indexToAssign ) {
	Swarm*            self              = Stg_CheckType( swarm, Swarm );
	Stg_Shape*        shape             = Stg_CheckType( _shape, Stg_Shape );
	GlobalParticle*   particle;
	Particle_Index    lParticle_I;

	Journal_Firewall( 
		self->particleLocalCount == variableToAssign->arraySize,
		Journal_Register( Error_Type, (Name)self->type  ),
		"In func %s: Trying to assign to variable '%s' with a different number of values \
		than the number of particles in swarm '%s'.\n",
		__func__, variableToAssign->name, self->name );
		


	for ( lParticle_I = 0 ; lParticle_I < self->particleLocalCount ; lParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleAt( self, lParticle_I );

		if ( Stg_Shape_IsCoordInside( shape, particle->coord ) ) 
			StgVariable_SetValueInt( variableToAssign, lParticle_I, indexToAssign );
	}
}

StandardParticle* Swarm_CreateNewParticle( void* swarm, Particle_Index* newParticle_I ) {
	Swarm*              self               = (Swarm*) swarm;

	*newParticle_I = self->particleLocalCount;
	self->particleLocalCount++;
	Swarm_Realloc( self );
	
	return Swarm_ParticleAt( self, *newParticle_I );
}


Bool Swarm_AddCommHandler( Swarm *self, void *commHandler )
{
	assert( commHandler );

	if( IsChild( ((Stg_Component*)commHandler)->type, ParticleCommHandler_Type ) ){
		Stg_ObjectList_Append( self->commHandlerList, commHandler );
		return True;
	}
	else{
		Journal_Firewall( 0, self->debug, "Trying to add a class which is not of the type ParticleCommHandler..!\n " );
		return False;
	}
}

void Swarm_AddVariable( Swarm* self, SwarmVariable* swarmVar ) {
	assert( self );
	assert( swarmVar );

	self->swarmVars = MemRearray( self->swarmVars, SwarmVariable*, self->nSwarmVars + 1, 
				      SwarmClass_Type );
	self->swarmVars[self->nSwarmVars] = swarmVar;
	self->nSwarmVars++;
}
