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
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>

#include "types.h"
#include "IrregularMeshGaussLayout.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

const Type IrregularMeshGaussLayout_Type = (const Type)"IrregularMeshGaussLayout";

IrregularMeshGaussLayout* IrregularMeshGaussLayout_New( Name name, Dimension_Index dim, Particle_InCellIndex* particlesPerDim, FeMesh* feMesh ) {
	IrregularMeshGaussLayout* self = _IrregularMeshGaussLayout_DefaultNew( name );

	_GaussParticleLayout_Init( (GaussParticleLayout*)self, dim, particlesPerDim );

	self->feMesh = feMesh;

	return self;
}

IrregularMeshGaussLayout* _IrregularMeshGaussLayout_New(  IRREGULARMESHGAUSSLAYOUT_DEFARGS  )
{
	IrregularMeshGaussLayout* self;
	
	/* Allocate memory */
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType          = NON_GLOBAL;
	coordSystem                 = GlobalCoordSystem;
	weightsInitialisedAtStartup = True;

	self = (IrregularMeshGaussLayout*)_GaussParticleLayout_New(  GAUSSPARTICLELAYOUT_PASSARGS  );
	
	return self;
}

void _IrregularMeshGaussLayout_Delete( void* gaussParticleLayout ) {
	IrregularMeshGaussLayout* self = (IrregularMeshGaussLayout*)gaussParticleLayout;
	
	_GaussParticleLayout_Delete( self );
}

void _IrregularMeshGaussLayout_Print( void* gaussParticleLayout, Stream* stream ) {
	IrregularMeshGaussLayout* self = (IrregularMeshGaussLayout*)gaussParticleLayout;
	
	_GaussParticleLayout_Print( self, stream );
}

void* _IrregularMeshGaussLayout_Copy( void* gaussParticleLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	IrregularMeshGaussLayout*	self = (IrregularMeshGaussLayout*)gaussParticleLayout;
	IrregularMeshGaussLayout*	newGaussParticleLayout;
	
	newGaussParticleLayout = (IrregularMeshGaussLayout*)_GaussParticleLayout_Copy( self, dest, deep, nameExt, ptrMap );
	
	return (void*)newGaussParticleLayout;
}

void* _IrregularMeshGaussLayout_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                                    _sizeOfSelf = sizeof(IrregularMeshGaussLayout);
	Type                                                                            type = IrregularMeshGaussLayout_Type;
	Stg_Class_DeleteFunction*                                                    _delete = _IrregularMeshGaussLayout_Delete;
	Stg_Class_PrintFunction*                                                      _print = _IrregularMeshGaussLayout_Print;
	Stg_Class_CopyFunction*                                                        _copy = _IrregularMeshGaussLayout_Copy;
	Stg_Component_DefaultConstructorFunction*                        _defaultConstructor = _IrregularMeshGaussLayout_DefaultNew;
	Stg_Component_ConstructFunction*                                          _construct = _IrregularMeshGaussLayout_AssignFromXML;
	Stg_Component_BuildFunction*                                                  _build = _IrregularMeshGaussLayout_Build;
	Stg_Component_InitialiseFunction*                                        _initialise = _IrregularMeshGaussLayout_Initialise;
	Stg_Component_ExecuteFunction*                                              _execute = _IrregularMeshGaussLayout_Execute;
	Stg_Component_DestroyFunction*                                              _destroy = _IrregularMeshGaussLayout_Destroy;
	ParticleLayout_SetInitialCountsFunction*                           _setInitialCounts = _PerCellParticleLayout_SetInitialCounts;
	ParticleLayout_InitialiseParticlesFunction*                     _initialiseParticles = _PerCellParticleLayout_InitialiseParticles;
	PerCellParticleLayout_InitialCountFunction*                            _initialCount = _GaussParticleLayout_InitialCount;
	PerCellParticleLayout_InitialiseParticlesOfCellFunction*  _initialiseParticlesOfCell = _IrregularMeshGaussLayout_InitialiseParticlesOfCell;
	Dimension_Index                                                                  dim = 0;
	Particle_InCellIndex*                                                particlesPerDim = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType           nameAllocationType = ZERO;
	CoordSystem                     coordSystem = ZERO;
	Bool            weightsInitialisedAtStartup = ZERO;

	return (IrregularMeshGaussLayout*)_IrregularMeshGaussLayout_New(  IRREGULARMESHGAUSSLAYOUT_PASSARGS  );
}

void _IrregularMeshGaussLayout_AssignFromXML( void* gaussParticleLayout, Stg_ComponentFactory* cf, void* data ) {
	IrregularMeshGaussLayout*   self = (IrregularMeshGaussLayout*)gaussParticleLayout;

	_GaussParticleLayout_AssignFromXML( self, cf, data );

	self->feMesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"FeMesh", FeMesh, True, data  );
	self->coordSystem = GlobalCoordSystem;
}
	
#define TRIPLE_MAX( A, B, C )  MAX( MAX( (A), (B) ), (C) )

void _IrregularMeshGaussLayout_Build( void* gaussParticleLayout, void* data ) {
	IrregularMeshGaussLayout*   	self 			= (IrregularMeshGaussLayout*)gaussParticleLayout;
	unsigned			maxParticlesPerDim;

	maxParticlesPerDim = TRIPLE_MAX( self->particlesPerDim[ I_AXIS ], self->particlesPerDim[ J_AXIS ], self->particlesPerDim[ K_AXIS ] );
	self->abscissa  = Memory_Alloc_Array( double, maxParticlesPerDim, "gauss abscissa" );

   Stg_Component_Build( self->feMesh, data, False );
   _GaussParticleLayout_Build( self, data );
}
	
void _IrregularMeshGaussLayout_Initialise( void* gaussParticleLayout, void* data ) {
   IrregularMeshGaussLayout*   	self 			= (IrregularMeshGaussLayout*)gaussParticleLayout;
   Stg_Component_Initialise( self->feMesh, data, False);
   _GaussParticleLayout_Initialise( self, data );
}
	
void _IrregularMeshGaussLayout_Execute( void* gaussParticleLayout, void* data ) {
}

void _IrregularMeshGaussLayout_Destroy( void* gaussParticleLayout, void* data ) {
	IrregularMeshGaussLayout*   	self 			= (IrregularMeshGaussLayout*)gaussParticleLayout;
	
	Memory_Free( self->abscissa );
	/* disable destruction of feMesh, as it is still required by things sometimes, eg during checkpointRestarts */
   /* Stg_Component_Destroy( self->feMesh, data, False); */
   _GaussParticleLayout_Destroy( self, data );
}

/* remember this only has to initialise one particle at a time */
void _IrregularMeshGaussLayout_InitialiseParticlesOfCell( void* gaussParticleLayout, void* _swarm, Cell_Index cell_I ) {
	IrregularMeshGaussLayout*      self			= (IrregularMeshGaussLayout*)gaussParticleLayout;
	Swarm*                    swarm 		= (Swarm*)_swarm;
	GlobalParticle*           particle		= NULL;
	Index                     index2D;
	IJK                       ijkIndex;
	Index                     index;
	Dimension_Index           dim_I;
	div_t                     divide;
	Particle_InCellIndex      particlesThisCell 	= swarm->cellParticleCountTbl[cell_I];
	Particle_InCellIndex      cParticle_I 		= 0;
	double			  xi[3]			= {0.0, 0.0, 0.0};
	double			  weights[27];
	
	for ( cParticle_I = 0; cParticle_I < particlesThisCell; cParticle_I++ ) {
		particle = (GlobalParticle*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
		particle->owningCell = cell_I;
		
		/* Find the i, j, k index of this particular particle */
		divide = div( cParticle_I, self->particlesPerDim[ I_AXIS ] * self->particlesPerDim[ J_AXIS ] );
		ijkIndex[ K_AXIS ] = divide.quot;
		index2D = divide.rem;

		divide = div( index2D, self->particlesPerDim[ I_AXIS ] );
		ijkIndex[ J_AXIS ] = divide.quot;
		ijkIndex[ I_AXIS ] = divide.rem;

		for( dim_I = 0 ; dim_I < self->dim ; dim_I++ ) {
			index = ijkIndex[ dim_I ];
			GaussParticleLayout_GetAbscissaAndWeights1D( weights, self->abscissa, self->particlesPerDim[ dim_I ] );
			xi[dim_I] = self->abscissa[index];
		}

		FeMesh_CoordLocalToGlobal( self->feMesh, cell_I, xi, particle->coord );
	}	
}



