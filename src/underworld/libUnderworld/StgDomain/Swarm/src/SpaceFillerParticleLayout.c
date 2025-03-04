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
#include "SpaceFillerParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "ShadowInfo.h"
#include "CellLayout.h"
#include "ElementCellLayout.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const Type SpaceFillerParticleLayout_Type = "SpaceFillerParticleLayout";

const Index SpaceFillerParticleLayout_Invalid = (Index) 0;

SpaceFillerParticleLayout* SpaceFillerParticleLayout_New( 
		Name             name,
      AbstractContext* context, 
      CoordSystem      coordSystem,
      Bool             weightsInitialisedAtStartup,
      unsigned int     totalInitialParticles, 
      double           averageInitialParticlesPerCell,
		Dimension_Index  dim )
{
	SpaceFillerParticleLayout* self = (SpaceFillerParticleLayout*) _SpaceFillerParticleLayout_DefaultNew( name );

   _ParticleLayout_Init( self, context, coordSystem, weightsInitialisedAtStartup );
   _GlobalParticleLayout_Init( self, totalInitialParticles, averageInitialParticlesPerCell );
	_SpaceFillerParticleLayout_Init( self, dim );
	return self;
}

SpaceFillerParticleLayout* _SpaceFillerParticleLayout_New(  SPACEFILLERPARTICLELAYOUT_DEFARGS  )
{
   SpaceFillerParticleLayout* self;

   /* Allocate memory */
   self = (SpaceFillerParticleLayout*)_GlobalParticleLayout_New(  GLOBALPARTICLELAYOUT_PASSARGS  );

   self->dim = dim;

	return self;
}

void _SpaceFillerParticleLayout_Init( 
		void*                   spaceFillerParticleLayout,
		Dimension_Index         dim )
{
	SpaceFillerParticleLayout*	self = (SpaceFillerParticleLayout*) spaceFillerParticleLayout;

	self->isConstructed = True;
	self->dim           = dim;

	self->sobolGenerator[I_AXIS] = SobolGenerator_NewFromTable( "xSobolGenerator", 0 );
	self->sobolGenerator[J_AXIS] = SobolGenerator_NewFromTable( "ySobolGenerator", 1 );
	if ( dim == 3 )
		self->sobolGenerator[K_AXIS] = SobolGenerator_NewFromTable( "zSobolGenerator", 2 );

	/* Must set one or the other. Fail if both set, or none */
	Journal_Firewall( 
		(self->totalInitialParticles == SpaceFillerParticleLayout_Invalid)
			^ ((Index)self->averageInitialParticlesPerCell == SpaceFillerParticleLayout_Invalid),
		Journal_MyStream( Error_Type, self ),
		"Error in func %s for %s '%s' - Both averageInitialParticlesPerCell and totalInitialParticles%sspecified.\n",
		__func__, 
		self->type, 
		self->name,
		(self->totalInitialParticles == SpaceFillerParticleLayout_Invalid) ? " not " : " " );
	
}


	
void _SpaceFillerParticleLayout_Delete( void* spaceFillerParticleLayout ) {
	SpaceFillerParticleLayout* self = (SpaceFillerParticleLayout*)spaceFillerParticleLayout;

	_GlobalParticleLayout_Delete( self );
}

void _SpaceFillerParticleLayout_Print( void* spaceFillerParticleLayout, Stream* stream ) {
	SpaceFillerParticleLayout* self  = (SpaceFillerParticleLayout*)spaceFillerParticleLayout;
	
	/* General info */
	Journal_Printf( stream, "SpaceFillerParticleLayout (ptr): %p:\n", self );
	Stream_Indent( stream );
	
	/* Parent class info */
	_GlobalParticleLayout_Print( self, stream );
	
	/* SpaceFillerParticleLayout */
	Journal_PrintValue( stream, self->dim );
	
	Stream_UnIndent( stream );
}

void* _SpaceFillerParticleLayout_Copy( void* spaceFillerParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	SpaceFillerParticleLayout*		self                    = (SpaceFillerParticleLayout*)spaceFillerParticleLayout;
	SpaceFillerParticleLayout*		newSpaceFillerParticleLayout;
	
	newSpaceFillerParticleLayout = (SpaceFillerParticleLayout*)_GlobalParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newSpaceFillerParticleLayout->dim = self->dim;

	return (void*)newSpaceFillerParticleLayout;
}

void* _SpaceFillerParticleLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                _sizeOfSelf = sizeof(SpaceFillerParticleLayout);
	Type                                                                        type = SpaceFillerParticleLayout_Type;
	Stg_Class_DeleteFunction*                                                _delete = _SpaceFillerParticleLayout_Delete;
	Stg_Class_PrintFunction*                                                  _print = _SpaceFillerParticleLayout_Print;
	Stg_Class_CopyFunction*                                                    _copy = _SpaceFillerParticleLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                    _defaultConstructor = _SpaceFillerParticleLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                      _construct = _SpaceFillerParticleLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                              _build = _SpaceFillerParticleLayout_Build;
	Stg_Component_InitialiseFunction*                                    _initialise = _SpaceFillerParticleLayout_Initialise;
	Stg_Component_ExecuteFunction*                                          _execute = _SpaceFillerParticleLayout_Execute;
	Stg_Component_DestroyFunction*                                          _destroy = _SpaceFillerParticleLayout_Destroy;
	AllocationType                                                nameAllocationType = NON_GLOBAL;
	ParticleLayout_SetInitialCountsFunction*                       _setInitialCounts = _GlobalParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*                 _initialiseParticles = _SpaceFillerParticleLayout_InitialiseParticles;
	CoordSystem                                                          coordSystem = GlobalCoordSystem;
	Bool                                                 weightsInitialisedAtStartup = False;
	GlobalParticleLayout_InitialiseParticleFunction*             _initialiseParticle = _SpaceFillerParticleLayout_InitialiseParticle;
	Particle_Index                                             totalInitialParticles = 0;
	double                                            averageInitialParticlesPerCell = 0.0;
	Dimension_Index                                                              dim = 0;

	return (void*)_SpaceFillerParticleLayout_New(  SPACEFILLERPARTICLELAYOUT_PASSARGS  );
}


void _SpaceFillerParticleLayout_AssignFromXML( void* spaceFillerParticleLayout, Stg_ComponentFactory *cf, void* data ) {
   SpaceFillerParticleLayout* self = (SpaceFillerParticleLayout*) spaceFillerParticleLayout;
   Dimension_Index            dim;

   _GlobalParticleLayout_AssignFromXML( self, cf, data );

   dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
   dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim );

   _SpaceFillerParticleLayout_Init( self, dim );
}
	
void _SpaceFillerParticleLayout_Build( void* spaceFillerParticleLayout, void* data ) {
}
void _SpaceFillerParticleLayout_Initialise( void* spaceFillerParticleLayout, void* data ) {
}	
void _SpaceFillerParticleLayout_Execute( void* spaceFillerParticleLayout, void* data ) {	
}
void _SpaceFillerParticleLayout_Destroy( void* spaceFillerParticleLayout, void* data ) {	
   SpaceFillerParticleLayout* self = (SpaceFillerParticleLayout*)spaceFillerParticleLayout;
   int dim_I;
   
   for ( dim_I = 0 ; dim_I < self->dim ; dim_I++ ) 
      Stg_Class_Delete( self->sobolGenerator[ dim_I ] );

   _GlobalParticleLayout_Destroy( self, data );
}

void _SpaceFillerParticleLayout_InitialiseParticles( void* spaceFillerParticleLayout, void* swarm ) {
	SpaceFillerParticleLayout*        self     = (SpaceFillerParticleLayout*)spaceFillerParticleLayout;

	/* Initialise random number generator */
	_GlobalParticleLayout_InitialiseParticles( self, swarm );
}

void _SpaceFillerParticleLayout_InitialiseParticle( 
		void*             spaceFillerParticleLayout, 
		void*             _swarm, 
		Particle_Index    newParticle_I,
		void*             _particle )
{
	SpaceFillerParticleLayout*  self     = (SpaceFillerParticleLayout*)spaceFillerParticleLayout;
	Swarm*                      swarm    = (Swarm*)_swarm;
	double				minCrd[3];
	double				maxCrd[3];
	Dimension_Index             dim_I;
	double*                     coord;
	GlobalParticle*             particle = (GlobalParticle*)_particle;
	/* Note: we have to assume element cell layout to get the full box coords - maybe these
	should be available as part of all cell layouts */
	ElementCellLayout*          eCellLayout = Stg_CheckType( swarm->cellLayout, ElementCellLayout );
	Mesh*                       mesh = eCellLayout->mesh;

	Mesh_GetGlobalCoordRange( mesh, minCrd, maxCrd );

	coord = particle->coord;
	memset( coord, 0, sizeof(Coord) );

	for ( dim_I = 0; dim_I < self->dim; dim_I++ ) {
		coord[ dim_I ] = SobolGenerator_GetNextNumber_WithMinMax(
			self->sobolGenerator[ dim_I ], 
			minCrd[ dim_I ], 
			maxCrd[ dim_I ]);
	}
}


