/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <assert.h>
#include <string.h>
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/Discretisation/Discretisation.h>
#include <StgFEM/SLE/SLE.h>

#include "types.h"
#include "MassMatrixTerm.h"

/* Textual name of this class */
const Type MassMatrixTerm_Type = "MassMatrixTerm";

MassMatrixTerm* MassMatrixTerm_New( 
	Name							name,
	FiniteElementContext*	context,
	StiffnessMatrix*			stiffnessMatrix,
	Swarm*						integrationSwarm,
	FeVariable*					field )
{
    MassMatrixTerm* self = (MassMatrixTerm*) _MassMatrixTerm_DefaultNew( name );

	self->isConstructed = True;
	_StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, integrationSwarm, NULL );
	_MassMatrixTerm_Init( self, field );

    return self;
}

/* Creation implementation / Virtual constructor */
MassMatrixTerm* _MassMatrixTerm_New(  MASSMATRIXTERM_DEFARGS  )
{
    MassMatrixTerm* self;
	
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(MassMatrixTerm) );
    self = (MassMatrixTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
	
    /* Virtual info */
	
    return self;
}

void _MassMatrixTerm_Init( void* matrixTerm, FeVariable* field ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;

    self->field = field;
}

void _MassMatrixTerm_Delete( void* matrixTerm ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;

    _StiffnessMatrixTerm_Delete( self );
}

void _MassMatrixTerm_Print( void* matrixTerm, Stream* stream ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
	
    _StiffnessMatrixTerm_Print( self, stream );

    /* General info */
    Journal_PrintPointer( stream, self->field );
}

void* _MassMatrixTerm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(MassMatrixTerm);
	Type                                                         type = MassMatrixTerm_Type;
	Stg_Class_DeleteFunction*                                 _delete = _MassMatrixTerm_Delete;
	Stg_Class_PrintFunction*                                   _print = _MassMatrixTerm_Print;
	Stg_Class_CopyFunction*                                     _copy = NULL;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _MassMatrixTerm_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _MassMatrixTerm_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _MassMatrixTerm_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _MassMatrixTerm_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _MassMatrixTerm_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _MassMatrixTerm_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _MassMatrixTerm_AssembleElement;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_MassMatrixTerm_New(  MASSMATRIXTERM_PASSARGS  );
}

void _MassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
    MassMatrixTerm*            self             = (MassMatrixTerm*)matrixTerm;
    FeVariable*                field;

    /* Construct Parent */
    _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

    field = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"field", FeVariable, True, data  ) ;

    _MassMatrixTerm_Init( self, field );
}

void _MassMatrixTerm_Build( void* matrixTerm, void* data ) {
    MassMatrixTerm*             self             = (MassMatrixTerm*)matrixTerm;

    _StiffnessMatrixTerm_Build( self, data );
    Stg_Component_Build( self->field, data, False );
}

void _MassMatrixTerm_Initialise( void* matrixTerm, void* data ) {
    MassMatrixTerm*             self             = (MassMatrixTerm*)matrixTerm;

    _StiffnessMatrixTerm_Initialise( self, data );
    Stg_Component_Initialise( self->field, data, False );
}

void _MassMatrixTerm_Execute( void* matrixTerm, void* data ) {
    _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _MassMatrixTerm_Destroy( void* matrixTerm, void* data ) {
   MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;

   Stg_Component_Destroy( self->field, data, False );
   _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}


void _MassMatrixTerm_AssembleElement(
	void*							matrixTerm,
	StiffnessMatrix*			stiffMat, 
	Element_LocalIndex		lElement_I, 
	SystemLinearEquations*	sle, 
	FiniteElementContext*	context, 
	double**						elStiffMat )
{
	MassMatrixTerm*       self         = Stg_CheckType( matrixTerm, MassMatrixTerm );
	StiffnessMatrix			*stiffnessMatrix = self->stiffnessMatrix;
	Swarm*                              swarm        = self->integrationSwarm;
	FeVariable*                         variable1    = stiffnessMatrix->rowVariable;
	Dimension_Index                     dim          = stiffnessMatrix->dim;
	int num_node_dofs = self->field->fieldComponentCount;
	IntegrationPoint*                   currIntegrationPoint;
	double*                             xi;
	double                              weight;
    Particle_InCellIndex                cParticle_I, cellParticleCount;
    Index                               nodesPerEl;
    Index                               i,j, k;
    double**                            GNx, *N;
    double                              detJac;
	
    Cell_Index                          cell_I;
    ElementType*                        elementType;
	
    /* Set the element type */
    elementType = FeMesh_GetElementType( variable1->feMesh, lElement_I );
    nodesPerEl = elementType->nodeCount;
	
    if( nodesPerEl > self->max_nElNodes ) {
	/* reallocate */
	self->GNx = ReallocArray2D( self->GNx, double, dim, nodesPerEl );
	self->N = ReallocArray( self->N, double, nodesPerEl );
	self->max_nElNodes = nodesPerEl;
    }
    GNx = self->GNx;
    N = self->N;

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

	ElementType_EvaluateShapeFunctionsAt( elementType, xi, N );

	for( i=0; i<nodesPerEl; i++ ) {
	    for( j=0; j<nodesPerEl; j++ ) {
		for( k = 0; k < num_node_dofs; k++)
		    elStiffMat[num_node_dofs*i + k][num_node_dofs*j + k] += detJac * weight * N[i] * N[j];
	    }
	}
    }
}


