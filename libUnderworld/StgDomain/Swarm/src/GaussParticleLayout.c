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

#include "types.h"

#include "ParticleLayout.h"
#include "PerCellParticleLayout.h"
#include "GaussParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "IntegrationPoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

const Type GaussParticleLayout_Type = "GaussParticleLayout";

GaussParticleLayout* GaussParticleLayout_New( 
   Name name, 
   AbstractContext* context,
   CoordSystem      coordSystem,
   Bool             weightsInitialisedAtStartup,
   Dimension_Index dim, 
   Particle_InCellIndex* particlesPerDim ) {

   GaussParticleLayout* self = _GaussParticleLayout_DefaultNew( name );
   _ParticleLayout_Init( self, context, coordSystem, weightsInitialisedAtStartup );
   _PerCellParticleLayout_Init( self );
   _GaussParticleLayout_Init( self, dim, particlesPerDim );

   return self;
}

GaussParticleLayout* _GaussParticleLayout_New(  GAUSSPARTICLELAYOUT_DEFARGS  )
{
	GaussParticleLayout* self;
	
   /* hard-wire these */
   coordSystem = LocalCoordSystem;
   weightsInitialisedAtStartup = True;
   nameAllocationType = NON_GLOBAL;

	/* Allocate memory */
	self = (GaussParticleLayout*)_PerCellParticleLayout_New(  PERCELLPARTICLELAYOUT_PASSARGS  );

   self->dim = dim;
   if( particlesPerDim )
     memcpy( self->particlesPerDim, particlesPerDim, 3 * sizeof(Particle_InCellIndex) );
	
	return self;
}

void _GaussParticleLayout_Init( void* gaussParticleLayout, Dimension_Index dim, Particle_InCellIndex* particlesPerDim ) {
   GaussParticleLayout* self = (GaussParticleLayout*)gaussParticleLayout;

   self->isConstructed       = True;
   self->coordSystem         = LocalCoordSystem;
   self->weightsInitialisedAtStartup = True;
   self->dim                 = dim;
   memcpy( self->particlesPerDim, particlesPerDim, 3 * sizeof(Particle_InCellIndex) );
}

void _GaussParticleLayout_Delete( void* gaussParticleLayout ) {
	GaussParticleLayout* self = (GaussParticleLayout*)gaussParticleLayout;
	
	_PerCellParticleLayout_Delete( self );
}

void _GaussParticleLayout_Print( void* gaussParticleLayout, Stream* stream ) {
	GaussParticleLayout* self = (GaussParticleLayout*)gaussParticleLayout;
	
	/* General info */
	Journal_Printf( stream, "GaussParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_PerCellParticleLayout_Print( self, stream );
	
	/* Virtual info */
	
	/* GaussParticleLayout */
	Stream_Indent( stream );
	Journal_PrintValue( stream, self->dim );
	Journal_PrintArray( stream, self->particlesPerDim, self->dim );
	Stream_UnIndent( stream );
}


void* _GaussParticleLayout_Copy( void* gaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	GaussParticleLayout*	self = (GaussParticleLayout*)gaussParticleLayout;
	GaussParticleLayout*	newGaussParticleLayout;
	
	newGaussParticleLayout = (GaussParticleLayout*)_PerCellParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newGaussParticleLayout->dim = self->dim;
	memcpy( newGaussParticleLayout->particlesPerDim, self->particlesPerDim, 3 * sizeof(unsigned int) );

	return (void*)newGaussParticleLayout;
}

void* _GaussParticleLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                     _sizeOfSelf = sizeof(GaussParticleLayout);
	Type                                                                             type = GaussParticleLayout_Type;
	Stg_Class_DeleteFunction*                                                     _delete = _GaussParticleLayout_Delete;
	Stg_Class_PrintFunction*                                                       _print = _GaussParticleLayout_Print;
	Stg_Class_CopyFunction*                                                         _copy = _GaussParticleLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                         _defaultConstructor = _GaussParticleLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                           _construct = _GaussParticleLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                                   _build = _GaussParticleLayout_Build;
	Stg_Component_InitialiseFunction*                                         _initialise = _GaussParticleLayout_Initialise;
	Stg_Component_ExecuteFunction*                                               _execute = _GaussParticleLayout_Execute;
	Stg_Component_DestroyFunction*                                               _destroy = _GaussParticleLayout_Destroy;
	AllocationType                                                     nameAllocationType = NON_GLOBAL;
	ParticleLayout_SetInitialCountsFunction*                            _setInitialCounts = _PerCellParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*                      _initialiseParticles = _PerCellParticleLayout_InitialiseParticles;
	CoordSystem                                                               coordSystem = LocalCoordSystem;
	Bool                                                      weightsInitialisedAtStartup = True;
	PerCellParticleLayout_InitialCountFunction*                             _initialCount = _GaussParticleLayout_InitialCount;
	PerCellParticleLayout_InitialiseParticlesOfCellFunction*   _initialiseParticlesOfCell = _GaussParticleLayout_InitialiseParticlesOfCell;
	Dimension_Index                                                                   dim = 0;
	Particle_InCellIndex*                                                 particlesPerDim = NULL;

	return (GaussParticleLayout*)_GaussParticleLayout_New(  GAUSSPARTICLELAYOUT_PASSARGS  );
}

void _GaussParticleLayout_AssignFromXML( void* gaussParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	GaussParticleLayout*   self = (GaussParticleLayout*)gaussParticleLayout;
	Particle_InCellIndex   particlesPerDim[3];
	Particle_InCellIndex   defaultVal;
	Dimension_Index        dim;

   _PerCellParticleLayout_AssignFromXML( self, cf, data );

	dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0  );
	if (dim == 0)
        dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", defaultVal  );

	defaultVal = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"gaussParticles", 2  );

	particlesPerDim[ I_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"gaussParticlesX", defaultVal  );
	particlesPerDim[ J_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"gaussParticlesY", defaultVal );
	if ( dim == 3  )
		particlesPerDim[ K_AXIS ] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"gaussParticlesZ", defaultVal  );
	else
		particlesPerDim[ K_AXIS ] = 1;	

	_GaussParticleLayout_Init( self, dim, particlesPerDim );
}
	
void _GaussParticleLayout_Build( void* gaussParticleLayout, void* data ) {
}
	
void _GaussParticleLayout_Initialise( void* gaussParticleLayout, void* data ) {
}
	
void _GaussParticleLayout_Execute( void* gaussParticleLayout, void* data ) {
}

void _GaussParticleLayout_Destroy( void* gaussParticleLayout, void* data ) {
}

Particle_InCellIndex _GaussParticleLayout_InitialCount( void* gaussParticleLayout, void* celllayout, Cell_Index cell_I )
{
	GaussParticleLayout* self   = (GaussParticleLayout*)gaussParticleLayout;
	Particle_InCellIndex count;
	Dimension_Index      dim;	
	Dimension_Index      dim_I;
	
	dim = self->dim;
	count = 1;
	for( dim_I = 0; dim_I < dim; dim_I++ ) {
		count = count * (Particle_InCellIndex)( self->particlesPerDim[ dim_I ] );
	}
	
	return count;
	
}

#define TRIPLE_MAX( A, B, C )  MAX( MAX( (A), (B) ), (C) )

/* remember this only has to initialise one particle at a time */
void _GaussParticleLayout_InitialiseParticlesOfCell( void* gaussParticleLayout, void* _swarm, Cell_Index cell_I )
{
	GaussParticleLayout*      self                = (GaussParticleLayout*)gaussParticleLayout;
	Swarm*                    swarm               = (Swarm*)_swarm;
	IntegrationPoint*         particle            = NULL;
	Index                     index2D;
	Particle_InCellIndex      maxParticlesPerDim;
	IJK                       ijkIndex;
	Index                     index;
	Dimension_Index           dim_I;
	div_t                     divide;
	double*                   weights;
	double*                   abscissa;
	Coord                     min;
	Coord                     max;
	Particle_InCellIndex      particlesThisCell = swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex      cParticle_I = 0;
	

	if ( 0 == strcmp( swarm->type, "MaterialPointsSwarm" ) ) {
		/* TODO: This is a special rule allowing a Gauss particle layout to be used to initialise
		global co-ords if you want to use it in a material swarm */
		self->coordSystem = GlobalCoordSystem;
	}

	Swarm_GetCellMinMaxCoords( _swarm, cell_I, min, max );

	/* Allocate Memory */
	maxParticlesPerDim = TRIPLE_MAX( self->particlesPerDim[ I_AXIS ],
		self->particlesPerDim[ J_AXIS ], self->particlesPerDim[ K_AXIS ] );

	weights   = Memory_Alloc_Array( double, maxParticlesPerDim, "gauss weights" );
	abscissa  = Memory_Alloc_Array( double, maxParticlesPerDim, "gauss abscissa" );

	for ( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {
		particle = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;
		
		/* Find the i, j, k index of this particular particle */
		divide = div( cParticle_I, self->particlesPerDim[ I_AXIS ] * self->particlesPerDim[ J_AXIS ] );
		ijkIndex[ K_AXIS ] = divide.quot;
		index2D = divide.rem;

		divide = div( index2D, self->particlesPerDim[ I_AXIS ] );
		ijkIndex[ J_AXIS ] = divide.quot;
		ijkIndex[ I_AXIS ] = divide.rem;

		particle->weight = 1.0;
		for( dim_I = 0 ; dim_I < self->dim ; dim_I++ ) {
			index = ijkIndex[ dim_I ];
			GaussParticleLayout_GetAbscissaAndWeights1D( weights, abscissa, self->particlesPerDim[ dim_I ] );

			/* Assign particle stuff */
			/* TODO: this is a hack, since this class doesn't officially know that the MaterialPointsSwarm
			 * exists yet. However, we need some way for material swarms to use this layout, for testing
			 * purposes. In the simple system of only 1 swarm type, this component always initialised
			 * both local and global co-ords.
			 * -- PatrickSunter - 1 May 2006
			 */
			if ( 0 == strcmp( swarm->type, "MaterialPointsSwarm" ) ) {
				((GlobalParticle*)particle)->coord[ dim_I ] = 
					min[ dim_I ] +
						0.5 * ( max[ dim_I ] - min[ dim_I ] ) 
						* ( abscissa[ index ] + 1.0 );
			}
			else {
				particle->xi[ dim_I ] = abscissa[ index ];
				particle->weight *= weights[ index ];
			}	
		}
		
	}	

	Memory_Free( weights );
	Memory_Free( abscissa );
}


/* Values taken from table from:
 * Eric W. Weisstein. "Legendre-Gauss Quadrature." From MathWorld--A Wolfram Web Resource. http://mathworld.wolfram.com/Legendre-GaussQuadrature.html */
void GaussParticleLayout_GetAbscissaAndWeights1D( double* weight, double* abscissa, Index pointCount ) {
	switch ( pointCount ) {
		case 1:
			abscissa[0]  = 0.0;
			weight[0] = 2.0;
			break;
		case 2:
			abscissa[0]  = - 1.0/sqrt(3.0);
			abscissa[1]  = - abscissa[0];
		
			weight[0] = 1.0;
			weight[1] = weight[0];
			break;
		case 3:
			abscissa[0]  = - sqrt(15.0)/5.0;
			abscissa[1]  = 0.0;
			abscissa[2]  = - abscissa[0];
		
			weight[0] = 5.0/9.0;
			weight[1] = 8.0/9.0;
			weight[2] = weight[0];
			break;
		case 4:
			abscissa[0]  = - sqrt( 525.0 + 70.0 * sqrt(30.0) )/35.0;
			abscissa[1]  = - sqrt( 525.0 - 70.0 * sqrt(30.0) )/35.0;
			abscissa[2]  = - abscissa[1];
			abscissa[3]  = - abscissa[0];
			
			weight[0] = (18.0 - sqrt(30.0))/36.0;
			weight[1] = (18.0 + sqrt(30.0))/36.0;
			weight[2] = weight[1];
			weight[3] = weight[0];
			break;
		case 5:
			abscissa[0]  = - sqrt( 245.0 + 14.0 * sqrt( 70.0 ) )/21.0;
			abscissa[1]  = - sqrt( 245.0 - 14.0 * sqrt( 70.0 ) )/21.0;
			abscissa[2]  = 0.0;
			abscissa[3]  = - abscissa[1];
			abscissa[4]  = - abscissa[0];

			weight[0] = ( 322.0 - 13.0 * sqrt( 70.0 ) )/900.0;
			weight[1] = ( 322.0 + 13.0 * sqrt( 70.0 ) )/900.0;
			weight[2] = 128.0/225.0;
			weight[3] = weight[1];
			weight[4] = weight[0];
			break;
		default:
			Journal_Firewall( 
				pointCount <= 5, 
				Journal_Register( Error_Type, (Name)GaussParticleLayout_Type  ),
				"In func %s: Cannot give values for '%u' gauss points\n.", 
				__func__, 
				pointCount );
	}
}


