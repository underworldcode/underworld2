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
#include <StgFEM/Discretisation/src/Discretisation.h>
#include <StgFEM/SLE/src/SLE.h>

#include "types.h"
#include "DivergenceMatrixTerm.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type DivergenceMatrixTerm_Type = "DivergenceMatrixTerm";

DivergenceMatrixTerm* DivergenceMatrixTerm_New( 
	Name					     name,
	FiniteElementContext*  context,
	StiffnessMatrix*	     stiffnessMatrix,
	Swarm*				     integrationSwarm )
{
	DivergenceMatrixTerm* self = (DivergenceMatrixTerm*) _DivergenceMatrixTerm_DefaultNew( name );
	
	self->isConstructed = True;
	_StiffnessMatrixTerm_Init( self, context, stiffnessMatrix, integrationSwarm, NULL );
   _DivergenceMatrixTerm_Init( self );

   return self;
}

/* Creation implementation / Virtual constructor */
DivergenceMatrixTerm* _DivergenceMatrixTerm_New(  DIVERGENCEMATRIXTERM_DEFARGS  )
{
   DivergenceMatrixTerm* self;
	
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(DivergenceMatrixTerm) );
   self = (DivergenceMatrixTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
	
   /* Virtual info */
	
   return self;
}

void _DivergenceMatrixTerm_Init( void* matrixTerm ) {
   DivergenceMatrixTerm* self = (DivergenceMatrixTerm*)matrixTerm;

	self->max_nElNodes_row = 0;
	self->Ni_row = NULL;
}

void _DivergenceMatrixTerm_Delete( void* matrixTerm ) {
   DivergenceMatrixTerm* self = (DivergenceMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Delete( self );
}

void _DivergenceMatrixTerm_Print( void* matrixTerm, Stream* stream ) {
   DivergenceMatrixTerm* self = (DivergenceMatrixTerm*)matrixTerm;
	
   _StiffnessMatrixTerm_Print( self, stream );

   /* General info */
}

void* _DivergenceMatrixTerm_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(DivergenceMatrixTerm);
	Type                                                         type = DivergenceMatrixTerm_Type;
	Stg_Class_DeleteFunction*                                 _delete = _DivergenceMatrixTerm_Delete;
	Stg_Class_PrintFunction*                                   _print = _DivergenceMatrixTerm_Print;
	Stg_Class_CopyFunction*                                     _copy = NULL;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _DivergenceMatrixTerm_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _DivergenceMatrixTerm_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _DivergenceMatrixTerm_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _DivergenceMatrixTerm_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _DivergenceMatrixTerm_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _DivergenceMatrixTerm_Destroy;
	StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _DivergenceMatrixTerm_AssembleElement;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_DivergenceMatrixTerm_New(  DIVERGENCEMATRIXTERM_PASSARGS  );
}

void _DivergenceMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
   DivergenceMatrixTerm* self = (DivergenceMatrixTerm*)matrixTerm;

   /* Construct Parent */
   _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

   _DivergenceMatrixTerm_Init( self );
}

void _DivergenceMatrixTerm_Build( void* matrixTerm, void* data ) {
   DivergenceMatrixTerm* self = (DivergenceMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Build( self, data );
}

void _DivergenceMatrixTerm_Initialise( void* matrixTerm, void* data ) {
   DivergenceMatrixTerm* self = (DivergenceMatrixTerm*)matrixTerm;

   _StiffnessMatrixTerm_Initialise( self, data );
}

void _DivergenceMatrixTerm_Execute( void* matrixTerm, void* data ) {
   _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _DivergenceMatrixTerm_Destroy( void* matrixTerm, void* data ) {
	DivergenceMatrixTerm* self = (DivergenceMatrixTerm*) matrixTerm;

   if( self->Ni_row) Memory_Free( self->Ni_row );
	_StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _DivergenceMatrixTerm_AssembleElement( 
   void*							matrixTerm,
   StiffnessMatrix*			stiffnessMatrix, 
   Element_LocalIndex		lElement_I, 
   SystemLinearEquations*	sle,
   FiniteElementContext*	context,
   double**						elStiffMat ) 
{
   DivergenceMatrixTerm*	self         = Stg_CheckType( matrixTerm, DivergenceMatrixTerm );
   Swarm*						swarm        = self->integrationSwarm;
   FeVariable*					variable_row = stiffnessMatrix->rowVariable;
   FeVariable*					variable_col = stiffnessMatrix->columnVariable;
   Dimension_Index			dim          = stiffnessMatrix->dim;
   double*						xi;
   double						weight;
   Particle_InCellIndex		cParticle_I, cellParticleCount;
   Node_ElementLocalIndex	nodesPerEl_row;
   Node_ElementLocalIndex	nodesPerEl_col;	
	
   Dof_Index					dofPerNode_row, dofPerNode_col;
   Index							row, col; /* Indices into the stiffness matrix */
   Node_ElementLocalIndex	rowNode_I;
   Node_ElementLocalIndex	colNode_I;
   Dof_Index					rowDof_I, colDof_I;
   double**						GNx_col;
   double*						Ni_row;
   double						detJac;
   IntegrationPoint*			currIntegrationPoint;
	
   Cell_Index					cell_I;
   ElementType*				elementType_row;
   ElementType*				elementType_col;
	
   /* Set the element type */
   elementType_row = FeMesh_GetElementType( variable_row->feMesh, lElement_I );
   nodesPerEl_row = elementType_row->nodeCount;
	
   elementType_col = FeMesh_GetElementType( variable_col->feMesh, lElement_I );
   nodesPerEl_col = elementType_col->nodeCount;
		
   dofPerNode_row = 1;	/* pressure */
   dofPerNode_col = dim;	/* velocity */
	
	if( nodesPerEl_col > self->max_nElNodes ) {
		/* reallocate */
		self->GNx = ReallocArray2D( self->GNx, double, dim, nodesPerEl_col );
		self->max_nElNodes = nodesPerEl_col;
	}
	GNx_col = self->GNx;

	if( nodesPerEl_row > self->max_nElNodes_row ) {
   /* allocate */
		self->Ni_row = ReallocArray( self->Ni_row, double, nodesPerEl_row );
		self->max_nElNodes_row = nodesPerEl_row;
	}
	Ni_row = self->Ni_row;
	
   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];
	
   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
      xi = currIntegrationPoint->xi;
      weight = currIntegrationPoint->weight;
		
      /* get shape function derivs for the row (ie. velocity) */
      ElementType_ShapeFunctionsGlobalDerivs( elementType_col, variable_col->feMesh, lElement_I, xi, dim, &detJac, GNx_col );
		
      /* get the shape functions for the col. (ie. pressure) */
      ElementType_EvaluateShapeFunctionsAt( elementType_row, xi, Ni_row );
		
      /* build stiffness matrix */
      for ( rowNode_I = 0; rowNode_I < nodesPerEl_row ; rowNode_I++) {  	
         for( rowDof_I=0; rowDof_I<dofPerNode_row; rowDof_I++) {	
            row = rowNode_I*dofPerNode_row + rowDof_I;

            for (colNode_I = 0; colNode_I < nodesPerEl_col; colNode_I++ ) { 
               for( colDof_I=0; colDof_I<dofPerNode_col; colDof_I++) {		
                  col = colNode_I*dofPerNode_col + colDof_I;
						
                  elStiffMat[row][col] += weight * ( -detJac ) * ( GNx_col[colDof_I][colNode_I] * Ni_row[rowNode_I] );
               }
            }
         }
      }
   }
}


