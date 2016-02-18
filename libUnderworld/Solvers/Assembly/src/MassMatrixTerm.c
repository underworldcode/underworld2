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
#include "MassMatrixTerm.h"

#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type MassMatrixTerm_Type = "MassMatrixTerm";

/* Creation implementation / Virtual constructor */
MassMatrixTerm* _MassMatrixTerm_New(  MASSMATRIXTERM_DEFARGS  ) {
    MassMatrixTerm* self;
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(MassMatrixTerm) );
    self = (MassMatrixTerm*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );
    /* Virtual info */
    self->max_nElNodes_col = 0;
    self->max_nElNodes_row = 0;
    self->Ni = NULL;
    self->Mi = NULL;
    self->geometryMesh = NULL;

    return self;
}

void _MassMatrixTerm_Init( void* matrixTerm, FeMesh* geometryMesh ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
}

void _MassMatrixTerm_Delete( void* matrixTerm ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Delete( self );
}

void _MassMatrixTerm_Print( void* matrixTerm, Stream* stream ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Print( self, stream );
    /* General info */
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
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_MassMatrixTerm_New(  MASSMATRIXTERM_PASSARGS  );
}

void _MassMatrixTerm_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
    /* Construct Parent */
    _StiffnessMatrixTerm_AssignFromXML( self, cf, data );
    self->geometryMesh = Stg_ComponentFactory_ConstructByKey( cf,
                                                        self->name,
                                                        (Dictionary_Entry_Key)"GeometryMesh",
                                                        FeMesh,
                                                        False,
                                                        data );
}

void _MassMatrixTerm_Build( void* matrixTerm, void* data ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Build( self, data );
}

void _MassMatrixTerm_Initialise( void* matrixTerm, void* data ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
    _StiffnessMatrixTerm_Initialise( self, data );
}

void _MassMatrixTerm_Execute( void* matrixTerm, void* data ) {
    _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _MassMatrixTerm_Destroy( void* matrixTerm, void* data ) {
    MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
    if( self->Ni ) Memory_Free( self->Ni ); 
    if( self->Mi ) Memory_Free( self->Mi );
    _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _MassMatrixTerm_AssembleElement(
   void*                                              matrixTerm,
   StiffnessMatrix*                                   stiffnessMatrix, 
   Element_LocalIndex                                 lElement_I, 
   SystemLinearEquations*                             sle,
   FiniteElementContext*                              context,
   double**                                           elStiffMat ) 
{
   MassMatrixTerm* self = (MassMatrixTerm*)matrixTerm;
   Swarm*                              swarm        = self->integrationSwarm;
   FeVariable*                         variable_row = stiffnessMatrix->rowVariable;
   FeVariable*                         variable_col = stiffnessMatrix->columnVariable;
   Dimension_Index                     dim          = stiffnessMatrix->dim;
   double*                             xi;
   double                              weight;
   Particle_InCellIndex                cParticle_I, cellParticleCount;
   Node_ElementLocalIndex              nodesPerEl_row;
   Node_ElementLocalIndex              nodesPerEl_col;	

   Dof_Index                           dofPerNode_col;
   Index                               row, col; /* Indices into the stiffness matrix */
   Node_ElementLocalIndex              rowNode_I;
   Node_ElementLocalIndex              colNode_I;
   Dof_Index                           colDof_I;
   double*                             Ni;
   double*                             Mi;
   double                              detJac;
   IntegrationPoint*                   currIntegrationPoint;

   Cell_Index                          cell_I;
   ElementType*                        elementType_row;
   ElementType*                        elementType_col;

   FeMesh*                 geometryMesh = ( self->geometryMesh ? self->geometryMesh : variable_row->feMesh );
   ElementType*            geometryElementType;

   /* Set the element type */
   geometryElementType = FeMesh_GetElementType( geometryMesh, lElement_I );

   elementType_row = FeMesh_GetElementType( variable_row->feMesh, lElement_I );
   nodesPerEl_row = elementType_row->nodeCount;
   elementType_col = FeMesh_GetElementType( variable_col->feMesh, lElement_I );
   nodesPerEl_col = elementType_col->nodeCount;

   dofPerNode_col = variable_col->fieldComponentCount;

   if( nodesPerEl_row > self->max_nElNodes_row ) {
      self->Mi = ReallocArray( self->Mi, double, nodesPerEl_row );
      self->max_nElNodes_row = nodesPerEl_row;
   }

   if( nodesPerEl_col > self->max_nElNodes_col ) {
      self->Ni = ReallocArray( self->Ni, double, nodesPerEl_col );
      self->max_nElNodes_col = nodesPerEl_col;
   }
   Ni = self->Ni;
   if (elementType_row == elementType_col)
      Mi = Ni;
   
   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
      xi = currIntegrationPoint->xi;
      weight = currIntegrationPoint->weight;		
      detJac = ElementType_JacobianDeterminant( geometryElementType, geometryMesh, lElement_I, xi, dim );
      double weight_detJac = weight*detJac;
      ElementType_EvaluateShapeFunctionsAt( elementType_col, xi, Ni );
      if (elementType_row != elementType_col)
         ElementType_EvaluateShapeFunctionsAt( elementType_row, xi, Mi );
      /* build stiffness matrix */
      for ( rowNode_I = 0; rowNode_I < nodesPerEl_row ; rowNode_I++) {  	
         for (colNode_I = 0; colNode_I < nodesPerEl_col; colNode_I++ ) {
            for( colDof_I=0; colDof_I<dofPerNode_col; colDof_I++) {
                  row = rowNode_I*dofPerNode_col + colDof_I;   /* note that we use the row dof count here too */
                  col = colNode_I*dofPerNode_col + colDof_I;
                  elStiffMat[row][col] +=  weight_detJac * ( Ni[rowNode_I] * Mi[colNode_I] );
            }
         }
      }
   }/*for cParticle_I*/
}
