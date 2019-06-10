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
#include "GaussBorderParticleLayout.h"

#include "SwarmClass.h"
#include "StandardParticle.h"
#include "IntegrationPoint.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

const Type GaussBorderParticleLayout_Type = "GaussBorderParticleLayout";

GaussBorderParticleLayout* GaussBorderParticleLayout_New( 
   Name name,
   AbstractContext* context,
   CoordSystem      coordSystem,
   Bool             weightsInitialisedAtStartup,
   Dimension_Index dim, 
   Particle_InCellIndex* particlesPerDim ) {

	GaussBorderParticleLayout* self = _GaussBorderParticleLayout_DefaultNew( name );

   _ParticleLayout_Init( self, context, coordSystem, weightsInitialisedAtStartup );
   _PerCellParticleLayout_Init( self );
   _GaussParticleLayout_Init( self, dim, particlesPerDim );
	_GaussBorderParticleLayout_Init( self );

	return self;
}

GaussBorderParticleLayout* _GaussBorderParticleLayout_New(  GAUSSBORDERPARTICLELAYOUT_DEFARGS  )
{
	GaussBorderParticleLayout* self;
	
	/* Allocate memory */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	dim             = 0;
	particlesPerDim = NULL;

	self = (GaussBorderParticleLayout*)_GaussParticleLayout_New(  GAUSSPARTICLELAYOUT_PASSARGS  );

	
	return self;
}


void _GaussBorderParticleLayout_Init( void* gaussBorderParticleLayout ) {
	GaussBorderParticleLayout* self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;

	self->particlesPerFace = Memory_Alloc_Array_Unnamed( Particle_InCellIndex, 6); // up to 6 faces (3d case)
	_GaussBorderParticleLayout_InitialiseParticlesPerFace( self );
}

void _GaussBorderParticleLayout_Delete( void* gaussBorderParticleLayout ) {
	GaussBorderParticleLayout* self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
	
	_GaussParticleLayout_Delete( self );
}

void _GaussBorderParticleLayout_Print( void* gaussBorderParticleLayout, Stream* stream ) {
	GaussBorderParticleLayout* self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
	
	/* General info */
	Journal_Printf( stream, "GaussBorderParticleLayout (ptr): %p:\n", self );
	
	/* Parent class info */
	_GaussParticleLayout_Print( self, stream );
	
	/* Virtual info */
	
	/* GaussBorderParticleLayout */
	Stream_Indent( stream );
	Journal_PrintValue( stream, self->dim );
	Journal_PrintArray( stream, self->particlesPerDim, self->dim );
	Journal_PrintArray( stream, self->particlesPerFace, ( (self->dim == 3) ? 6 : 4 ) );
	Stream_UnIndent( stream );
}


void* _GaussBorderParticleLayout_Copy( void* gaussBorderParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	GaussBorderParticleLayout*	self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
	GaussBorderParticleLayout*	newGaussBorderParticleLayout;
	
	newGaussBorderParticleLayout = (GaussBorderParticleLayout*)_GaussParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	newGaussBorderParticleLayout->dim = self->dim;
	memcpy( newGaussBorderParticleLayout->particlesPerDim, self->particlesPerDim, 3 * sizeof(unsigned int) );
	
	memcpy( newGaussBorderParticleLayout->particlesPerFace, self->particlesPerFace, 6 * sizeof(Particle_InCellIndex) );

	return (void*)newGaussBorderParticleLayout;
}

void* _GaussBorderParticleLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                     _sizeOfSelf = sizeof(GaussBorderParticleLayout);
	Type                                                                             type = GaussBorderParticleLayout_Type;
	Stg_Class_DeleteFunction*                                                     _delete = _GaussBorderParticleLayout_Delete;
	Stg_Class_PrintFunction*                                                       _print = _GaussBorderParticleLayout_Print;
	Stg_Class_CopyFunction*                                                         _copy = _GaussBorderParticleLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                         _defaultConstructor = _GaussBorderParticleLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                           _construct = _GaussBorderParticleLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                                   _build = _GaussBorderParticleLayout_Build;
	Stg_Component_InitialiseFunction*                                         _initialise = _GaussBorderParticleLayout_Initialise;
	Stg_Component_ExecuteFunction*                                               _execute = _GaussBorderParticleLayout_Execute;
	Stg_Component_DestroyFunction*                                               _destroy = _GaussBorderParticleLayout_Destroy;
	AllocationType                                                     nameAllocationType = NON_GLOBAL;
	ParticleLayout_SetInitialCountsFunction*                            _setInitialCounts = _PerCellParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*                      _initialiseParticles = _PerCellParticleLayout_InitialiseParticles;
	CoordSystem                                                               coordSystem = LocalCoordSystem;
	Bool                                                      weightsInitialisedAtStartup = True;
	PerCellParticleLayout_InitialCountFunction*                             _initialCount = _GaussBorderParticleLayout_InitialCount;
	PerCellParticleLayout_InitialiseParticlesOfCellFunction*   _initialiseParticlesOfCell = _GaussBorderParticleLayout_InitialiseParticlesOfCell;
	Dimension_Index                                                                   dim = 0;
	Particle_InCellIndex*                                                 particlesPerDim = NULL;

	return (GaussBorderParticleLayout*)_GaussBorderParticleLayout_New(  GAUSSBORDERPARTICLELAYOUT_PASSARGS  );
}

void _GaussBorderParticleLayout_AssignFromXML( void* gaussBorderParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	GaussBorderParticleLayout*   self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;

   _GaussParticleLayout_AssignFromXML( self, cf, data );

	_GaussBorderParticleLayout_Init( self );
}
	
void _GaussBorderParticleLayout_Build( void* gaussBorderParticleLayout, void* data ) {
   GaussBorderParticleLayout* self   = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
   _GaussParticleLayout_Build( self, data );
}
	
void _GaussBorderParticleLayout_Initialise( void* gaussBorderParticleLayout, void* data ) {
   GaussBorderParticleLayout* self   = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
   _GaussParticleLayout_Initialise( self, data );
}
	
void _GaussBorderParticleLayout_Execute( void* gaussBorderParticleLayout, void* data ) {
}

void _GaussBorderParticleLayout_Destroy( void* gaussBorderParticleLayout, void* data ) {
   GaussBorderParticleLayout* self   = (GaussBorderParticleLayout*)gaussBorderParticleLayout;

   Memory_Free( self->particlesPerFace );
   _GaussParticleLayout_Destroy( self, data );
}

Particle_InCellIndex _GaussBorderParticleLayout_InitialCount( void* gaussBorderParticleLayout, void* celllayout, Cell_Index cell_I )
{
	GaussBorderParticleLayout* self   = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
	Particle_InCellIndex count = 0;
	Index numFaces = (self->dim == 3) ? 6 : 4;
	Index face_I;

	for( face_I = 0; face_I < numFaces; face_I++ ){
		count += self->particlesPerFace[ face_I ];
	}

	return count;
}


#define TRIPLE_MAX( A, B, C )  MAX( MAX( (A), (B) ), (C) )


/* remember this only has to initialise one particle at a time */
void _GaussBorderParticleLayout_InitialiseParticlesOfCell( void* gaussBorderParticleLayout, void* _swarm, Cell_Index cell_I )
{
	GaussBorderParticleLayout*      self                = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
	Swarm*                    swarm               = (Swarm*)_swarm;
	IntegrationPoint*         particle            = NULL;
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
	Index			  face_I;
	Particle_InCellIndex	  particle_I_ThisFace;

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

	face_I = 0;
	particle_I_ThisFace = 0;
	for ( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {
		particle = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;

		/* Find which face (side) this particle lives on */
		/* easier to keep track of the face thruout the loop than go and recalculate it each time, I think*/
		if( particle_I_ThisFace == self->particlesPerFace[ face_I ] ) {
			face_I++;
			particle_I_ThisFace = 0;
		}
		
		/* Find the i, j, k index of this particular particle */
		// we're now looking at something with one less dimension than the cell, so we'll just ignore the K_AXIS
		divide = div( particle_I_ThisFace, self->particlesPerDim[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, I_AXIS) ] );
		
		if( self->dim == 3 ) {
			ijkIndex[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, J_AXIS) ] = divide.quot;
		}

		ijkIndex[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, I_AXIS) ] = divide.rem;

		particle->weight = 1.0;
			
		for( dim_I = 0 ; dim_I <  (self->dim - 1) ; dim_I++ ) {
			index = ijkIndex[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ];
			GaussParticleLayout_GetAbscissaAndWeights1D( weights, abscissa, self->particlesPerDim[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ] );

			/* Assign particle stuff */
			/* TODO: this is a hack, since this class doesn't officially know that the MaterialPointsSwarm
			 * exists yet. However, we need some way for material swarms to use this layout, for testing
			 * purposes. In the simple system of only 1 swarm type, this component always initialised
			 * both local and global co-ords.
			 * -- PatrickSunter - 1 May 2006
			 */
			// this is a slightly different version of the Gauss layout, so I'm assuming this bit still applies
			// (and I've modified it to work with this layout)
			if ( 0 == strcmp( swarm->type, "MaterialPointsSwarm" ) ) {
				((GlobalParticle*)particle)->coord[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ] = 
					min[ GaussBorderParticleLayout_GetFaceAxis(self, self->dim, dim_I) ] +
					0.5 * ( max[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ] 
					- min[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ] ) 
					* ( abscissa[ index ] + 1.0 );
			}
			else {
				particle->xi[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ] = abscissa[ index ];
				particle->weight *= weights[ index ];
			}	
		}
		// assign the coordinate that's constant on this face. the %2 thing is just a convention I picked for labelling faces:
		// even faces have a constant coord of -1, odd faces have +1 (so left is even with x=-1, right is odd with x=1, etc)
		particle->xi[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, K_AXIS) ] = (face_I % 2 == 0) ? -1 : 1;
		// currently assuming a cube, -1,-1,-1 to 1,1,1, just as the Gauss layout does.

		particle_I_ThisFace++;
	}	

	Memory_Free( weights );
	Memory_Free( abscissa );
}


/*
  for a cuboids/square element, each face (or side) is essentially a 2d square (or 1d straight line); but which plane it's in varies from face to face.
  the next function is just a means of encoding that: so face 1 (in a 3d cell) is in the i-k plane
  thus we can treat it as a 2d face with 'i-axis' in the i direction and 'j-axis' in the *k* direction
  for the sake of generality, we treat all faces as having an I_AXIS and J_AXIS; this function maps the axis and face no to
  the 'real' axis that it corresponds to
  the 'K-AXIS' is the axis that is 'constant' for this face. It's also the direction of the normal vector for the face (side)
  the 'J-AXIS' in the 2d case isn't used

  in other words, the function allows me to map from a general boundary section to a specific one ( and vice-versa ), which allows the code
  above to be generalised and not consist of a largish switch-case statement.
*/
Dimension_Index GaussBorderParticleLayout_GetFaceAxis( void* gaussBorderParticleLayout, Index face_I, Dimension_Index axis) {
	GaussBorderParticleLayout*	self;
	Dimension_Index				faceAxes3D[6][3];

	self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;

	faceAxes3D[0][ I_AXIS ] = I_AXIS; faceAxes3D[0][ J_AXIS ] = K_AXIS; faceAxes3D[0][ K_AXIS ] = J_AXIS;
	faceAxes3D[1][ I_AXIS ] = I_AXIS; faceAxes3D[1][ J_AXIS ] = K_AXIS; faceAxes3D[1][ K_AXIS ] = J_AXIS;
	faceAxes3D[2][ I_AXIS ] = J_AXIS; faceAxes3D[2][ J_AXIS ] = K_AXIS; faceAxes3D[2][ K_AXIS ] = I_AXIS;
	faceAxes3D[3][ I_AXIS ] = J_AXIS; faceAxes3D[3][ J_AXIS ] = K_AXIS; faceAxes3D[3][ K_AXIS ] = I_AXIS;
	faceAxes3D[4][ I_AXIS ] = I_AXIS; faceAxes3D[4][ J_AXIS ] = J_AXIS; faceAxes3D[4][ K_AXIS ] = K_AXIS;
	faceAxes3D[5][ I_AXIS ] = I_AXIS; faceAxes3D[5][ J_AXIS ] = J_AXIS; faceAxes3D[5][ K_AXIS ] = K_AXIS;

	return faceAxes3D[face_I][axis];
}

Index GaussBorderParticleLayout_ParticleInCellIndexToFaceIndex( void* gaussBorderParticleLayout, Particle_InCellIndex cParticle_I) {
	GaussBorderParticleLayout*	self = (GaussBorderParticleLayout*)gaussBorderParticleLayout;
	Index								face_I;
	Index								numfaces = (self->dim == 3) ? 6 : 4;
	int								cParticle_I_signed = (int) cParticle_I;

	for( face_I = 0; face_I < numfaces; face_I++ ) {
		cParticle_I_signed -= self->particlesPerFace[ face_I ];
		if(cParticle_I_signed < 0)	/* ParticleInCellIndex is unsigned int - which generally isn't <0...hence the signed int. */
			break;
	}

	return face_I;
}

void _GaussBorderParticleLayout_InitialiseParticlesPerFace( GaussBorderParticleLayout* self )
{
	Index face_I;
	Index numFaces = (self->dim == 3) ? 6 : 4;
	Dimension_Index dim_I;

	/* find no of particles for each face. */
	for( face_I = 0; face_I < numFaces; face_I++ ) {
		self->particlesPerFace[ face_I ] = 1;
		for( dim_I = 0; dim_I < self->dim - 1; dim_I++ ){
			self->particlesPerFace[ face_I ] *= self->particlesPerDim[ GaussBorderParticleLayout_GetFaceAxis(self, face_I, dim_I) ];
		}
	}

}


