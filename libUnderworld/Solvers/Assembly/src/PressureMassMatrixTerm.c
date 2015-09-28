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
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#include "types.h"
#include "PressureMassMatrixTerm.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type PressMassMatrixTerm_Type = "PressMassMatrixTerm";

/* Creation implementation / Virtual constructor */
PressMassMatrixTerm* _PressMassMatrixTerm_New(  PRESSMASSMATRIXTERM_DEFARGS  ) {
    PressMassMatrixTerm* self;
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(PressMassMatrixTerm) );
    self = (PressMassMatrixTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
    /* Virtual info */
    return self;
}

void _PressMassMatrixTerm_Init( void* matrixTerm, FeMesh* geometryMesh ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    self->max_nElNodes_col = 0;
    self->Ni = NULL;
    self->geometryMesh = geometryMesh;
}

void _PressMassMatrixTerm_Delete( void* matrixTerm ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Delete( self );
}

void _PressMassMatrixTerm_Print( void* matrixTerm, Stream* stream ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Print( self, stream );
    /* General info */
}

void* _PressMassMatrixTerm_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                                 _sizeOfSelf = sizeof(PressMassMatrixTerm);
    Type                                                         type = PressMassMatrixTerm_Type;
    Stg_Class_DeleteFunction*                                 _delete = _PressMassMatrixTerm_Delete;
    Stg_Class_PrintFunction*                                   _print = _PressMassMatrixTerm_Print;
    Stg_Class_CopyFunction*                                     _copy = NULL;
    Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _PressMassMatrixTerm_DefaultNew;
    Stg_Component_ConstructFunction*                       _construct = _PressMassMatrixTerm_AssignFromXML;
    Stg_Component_BuildFunction*                               _build = _PressMassMatrixTerm_Build;
    Stg_Component_InitialiseFunction*                     _initialise = _PressMassMatrixTerm_Initialise;
    Stg_Component_ExecuteFunction*                           _execute = _PressMassMatrixTerm_Execute;
    Stg_Component_DestroyFunction*                           _destroy = _PressMassMatrixTerm_Destroy;
    StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _PressMassMatrixTerm_AssembleElement;
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_PressMassMatrixTerm_New(  PRESSMASSMATRIXTERM_PASSARGS  );
}

void _PressMassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    FeMesh*              geometryMesh;
    /* Construct Parent */
    _StiffnessMatrixTerm_AssignFromXML( self, cf, data );
    geometryMesh = Stg_ComponentFactory_ConstructByKey(
	cf,
	self->name,
	(Dictionary_Entry_Key)"GeometryMesh",
	FeMesh,
	True,
	data );
    _PressMassMatrixTerm_Init( self, geometryMesh );
}

void _PressMassMatrixTerm_Build( void* matrixTerm, void* data ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Build( self, data );
}

void _PressMassMatrixTerm_Initialise( void* matrixTerm, void* data ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Initialise( self, data );
}

void _PressMassMatrixTerm_Execute( void* matrixTerm, void* data ) {
    _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _PressMassMatrixTerm_Destroy( void* matrixTerm, void* data ) {
    PressMassMatrixTerm* self = (PressMassMatrixTerm*)matrixTerm;
    if( self->Ni ) Memory_Free( self->Ni ); 
    _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _PressMassMatrixTerm_AssembleElement( 
    void*                                              matrixTerm,
    StiffnessMatrix*                                   stiffnessMatrix, 
    Element_LocalIndex                                 lElement_I, 
    SystemLinearEquations*                             sle,
    FiniteElementContext*                              context,
    double**                                           elStiffMat ) 
{
    PressMassMatrixTerm*             self         = Stg_CheckType( matrixTerm, PressMassMatrixTerm );
    Swarm*                              swarm        = self->integrationSwarm;
    FeVariable*                         variable_row = stiffnessMatrix->rowVariable;
    FeVariable*                         variable_col = stiffnessMatrix->columnVariable;
    Dimension_Index                     dim          = stiffnessMatrix->dim;
    double*                             xi;
    double                              weight;
    Particle_InCellIndex                cParticle_I, cellParticleCount;
    Node_ElementLocalIndex              nodesPerEl_row;
    Node_ElementLocalIndex              nodesPerEl_col;	
    Dof_Index                           totalDofsThisElement_row, totalDofsThisElement_col;
	
    Dof_Index                           dofPerNode_row, dofPerNode_col;
    Index                               row, col; /* Indices into the stiffness matrix */
    Node_ElementLocalIndex              rowNode_I;
    Node_ElementLocalIndex              colNode_I;
    Dof_Index                           rowDof_I, colDof_I;
    double**                            GNx_row;
    double*                             Ni;
    double                              detJac;
    IntegrationPoint*                   currIntegrationPoint;
	
    Cell_Index                          cell_I;
    ElementType*                        elementType_row;
    ElementType*                        elementType_col;

    FeMesh*                 geometryMesh = self->geometryMesh;
    ElementType*            geometryElementType;
	
    /* Set the element type */
    geometryElementType = FeMesh_GetElementType( geometryMesh, lElement_I );/* need this so can get detJac for constant mesh apparently */

    elementType_row = FeMesh_GetElementType( variable_row->feMesh, lElement_I );
    nodesPerEl_row = elementType_row->nodeCount;
    elementType_col = FeMesh_GetElementType( variable_col->feMesh, lElement_I );
    nodesPerEl_col = elementType_col->nodeCount;


    dofPerNode_row = 1;	/* P0 pressure */
    dofPerNode_col = 1;	/* P0 pressure */
	
    totalDofsThisElement_row = nodesPerEl_row * dofPerNode_row;
    totalDofsThisElement_col = nodesPerEl_col * dofPerNode_col;	

    if( nodesPerEl_row > self->max_nElNodes ) {
	/* reallocate */
	self->GNx = ReallocArray2D( self->GNx, double, dim, nodesPerEl_row );
	self->max_nElNodes = nodesPerEl_row; }

    GNx_row = self->GNx;

    if( nodesPerEl_col > self->max_nElNodes_col ) {
	/* allocate */
	self->Ni = ReallocArray( self->Ni, double, nodesPerEl_col );
	self->max_nElNodes_col = nodesPerEl_col; }
    Ni = self->Ni;
    cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
    cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

    for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
	currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
	xi = currIntegrationPoint->xi;
	weight = currIntegrationPoint->weight;		
	detJac = ElementType_JacobianDeterminant( geometryElementType, geometryMesh, lElement_I, xi, dim );
	/* get the shape functions for the col. (ie. pressure) */
	ElementType_EvaluateShapeFunctionsAt( elementType_col, xi, Ni );
	/* build stiffness matrix */
	for ( rowNode_I = 0; rowNode_I < nodesPerEl_row ; rowNode_I++) {  	
	    for( rowDof_I=0; rowDof_I<dofPerNode_row; rowDof_I++) {	
		row = rowNode_I*dofPerNode_row + rowDof_I;
		for (colNode_I = 0; colNode_I < nodesPerEl_col; colNode_I++ ) { 
		    for( colDof_I=0; colDof_I<dofPerNode_col; colDof_I++) {		
			col = colNode_I*dofPerNode_col + colDof_I;
			elStiffMat[row][col] +=  weight * ( detJac ) * ( Ni[rowNode_I] * Ni[colNode_I] );}}}}
    }/*for cParticle_I*/
}


