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
#include <StgFEM/SLE/SLE.h>

#include "types.h"
#include "LaplacianStiffnessMatrixTerm.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type LaplacianStiffnessMatrixTerm_Type = "LaplacianStiffnessMatrixTerm";

LaplacianStiffnessMatrixTerm* LaplacianStiffnessMatrixTerm_New( 
	Name							name,
	FiniteElementContext*	context,
	StiffnessMatrix*			stiffnessMatrix,
	Swarm*						integrationSwarm )
{
	LaplacianStiffnessMatrixTerm* self = (LaplacianStiffnessMatrixTerm*) _LaplacianStiffnessMatrixTerm_DefaultNew( name );

	self->isConstructed = False;	
	_StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, integrationSwarm, NULL );
	_LaplacianStiffnessMatrixTerm_Init( self );

	return self;
}

/* Creation implementation / Virtual constructor */
LaplacianStiffnessMatrixTerm* _LaplacianStiffnessMatrixTerm_New(  LAPLACIANSTIFFNESSMATRIXTERM_DEFARGS  )
{
	LaplacianStiffnessMatrixTerm* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(LaplacianStiffnessMatrixTerm) );
	self = (LaplacianStiffnessMatrixTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
	
	/* Virtual info */
	
	return self;
}

void _LaplacianStiffnessMatrixTerm_Init( void* matrixTerm ) {
}

void _LaplacianStiffnessMatrixTerm_Delete( void* matrixTerm ) {
	LaplacianStiffnessMatrixTerm* self = (LaplacianStiffnessMatrixTerm*)matrixTerm;

	_StiffnessMatrixTerm_Delete( self );
}

void _LaplacianStiffnessMatrixTerm_Print( void* matrixTerm, Stream* stream ) {
	LaplacianStiffnessMatrixTerm* self = (LaplacianStiffnessMatrixTerm*)matrixTerm;
	
	_StiffnessMatrixTerm_Print( self, stream );

	/* General info */
}

void* _LaplacianStiffnessMatrixTerm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(LaplacianStiffnessMatrixTerm);
	Type                                                         type = LaplacianStiffnessMatrixTerm_Type;
	Stg_Class_DeleteFunction*                                 _delete = _LaplacianStiffnessMatrixTerm_Delete;
	Stg_Class_PrintFunction*                                   _print = _LaplacianStiffnessMatrixTerm_Print;
	Stg_Class_CopyFunction*                                     _copy = NULL;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _LaplacianStiffnessMatrixTerm_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _LaplacianStiffnessMatrixTerm_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _LaplacianStiffnessMatrixTerm_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _LaplacianStiffnessMatrixTerm_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _LaplacianStiffnessMatrixTerm_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _LaplacianStiffnessMatrixTerm_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _LaplacianStiffnessMatrixTerm_AssembleElement;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	return (void*)_LaplacianStiffnessMatrixTerm_New(  LAPLACIANSTIFFNESSMATRIXTERM_PASSARGS  );
}

void _LaplacianStiffnessMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
	LaplacianStiffnessMatrixTerm*            self             = (LaplacianStiffnessMatrixTerm*)matrixTerm;

	/* Construct Parent */
	_StiffnessMatrixTerm_AssignFromXML( self, cf, data );

	_LaplacianStiffnessMatrixTerm_Init( self );
}

void _LaplacianStiffnessMatrixTerm_Build( void* matrixTerm, void* data ) {
	LaplacianStiffnessMatrixTerm*             self             = (LaplacianStiffnessMatrixTerm*)matrixTerm;

	_StiffnessMatrixTerm_Build( self, data );
}

void _LaplacianStiffnessMatrixTerm_Initialise( void* matrixTerm, void* data ) {
	LaplacianStiffnessMatrixTerm*             self             = (LaplacianStiffnessMatrixTerm*)matrixTerm;

	_StiffnessMatrixTerm_Initialise( self, data );
}

void _LaplacianStiffnessMatrixTerm_Execute( void* matrixTerm, void* data ) {
	_StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _LaplacianStiffnessMatrixTerm_Destroy( void* matrixTerm, void* data ) {
	_StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _LaplacianStiffnessMatrixTerm_AssembleElement( 
		void*                                              matrixTerm,
		StiffnessMatrix*                                   stiffnessMatrix, 
		Element_LocalIndex                                 lElement_I, 
		SystemLinearEquations*                             sle,
		FiniteElementContext*                              context,
		double**                                           elStiffMat ) 
{
	LaplacianStiffnessMatrixTerm*       self         = Stg_CheckType( matrixTerm, LaplacianStiffnessMatrixTerm );
	Swarm*                              swarm        = self->integrationSwarm;
	FeVariable*                         variable1    = stiffnessMatrix->rowVariable;
	Dimension_Index                     dim          = stiffnessMatrix->dim;
	IntegrationPoint*                   currIntegrationPoint;
	double*                             xi;
	double                              weight;
	Particle_InCellIndex                cParticle_I, cellParticleCount;
	Index                               nodesPerEl;
	Index                               i,j;
	Dimension_Index                     dim_I;
	double**                            GNx;
	double                              detJac;
	
	Cell_Index                          cell_I;
	ElementType*                        elementType;
	
	/* Set the element type */
	elementType = FeMesh_GetElementType( variable1->feMesh, lElement_I );
	nodesPerEl = elementType->nodeCount;
	
	if( nodesPerEl > self->max_nElNodes ) {
		/* reallocate */
		self->GNx = ReallocArray2D( self->GNx, double, dim, nodesPerEl );
		self->max_nElNodes = nodesPerEl;
	}
	GNx = self->GNx;
	
	cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
	cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

	/* Slap the laplacian together */
	for( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
		
		currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );

		xi = currIntegrationPoint->xi;
		weight = currIntegrationPoint->weight;
		
		ElementType_ShapeFunctionsGlobalDerivs( 
			elementType,
			variable1->feMesh, lElement_I,
			xi, dim, &detJac, GNx );
			
		for( i=0; i<nodesPerEl; i++ ) {
			for( j=0; j<nodesPerEl; j++ ) {
				for ( dim_I = 0; dim_I < dim ; dim_I++ ) { 
					elStiffMat[i][j] += detJac * weight * GNx[dim_I][i] * GNx[dim_I][j];
				}
			}
		}
	}
}


