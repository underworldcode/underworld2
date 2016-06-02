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
#include<cstdlib>

#include <mpi.h>
#include <petsc.h>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>
#include <Underworld/Function/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/Function.hpp>

#include "MatrixAssemblyTerm_NA__NB__Fn.h"


/* Textual name of this class */
const Type MatrixAssemblyTerm_NA__NB__Fn_Type = "MatrixAssemblyTerm_NA__NB__Fn";

/* Creation implementation / Virtual constructor */
MatrixAssemblyTerm_NA__NB__Fn* _MatrixAssemblyTerm_NA__NB__Fn_New(  MATRIXASSEMBLYTERM_NA__NB__FN_DEFARGS  ) {
    MatrixAssemblyTerm_NA__NB__Fn* self;
    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(MatrixAssemblyTerm_NA__NB__Fn) );
    self = (MatrixAssemblyTerm_NA__NB__Fn*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

/* Virtual info */
    self->cppdata = (void*) new MatrixAssemblyTerm_NA__NB__Fn_cppdata;
    self->max_nElNodes_col = 0;
    self->max_nElNodes_row = 0;
    self->Ni = NULL;
    self->Mi = NULL;
    self->geometryMesh = NULL;

    return self;
}

void MatrixAssemblyTerm_NA__NB__Fn_SetFn( void* _self, Fn::Function* fn ){
    MatrixAssemblyTerm_NA__NB__Fn*  self = (MatrixAssemblyTerm_NA__NB__Fn*)_self;

    // record fn to struct
    MatrixAssemblyTerm_NA__NB__Fn_cppdata* cppdata = (MatrixAssemblyTerm_NA__NB__Fn_cppdata*) self->cppdata;
    cppdata->fn = fn;

    // setup fn
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)swarm->mesh, localCoord);
    cppdata->func = fn->getFunction(cppdata->input);

    // check output conforms
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(cppdata->func(cppdata->input));
    if( !iodub )
        throw std::invalid_argument("MatrixAssemblyTerm routine expects functions to return 'double' type values.");
    if( iodub->size() != 1 )
        throw std::invalid_argument("MatrixAssemblyTerm matrix routine expects functions to scalar values.");

}

void _MatrixAssemblyTerm_NA__NB__Fn_Delete( void* matrixTerm ) {
    MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;

    _StiffnessMatrixTerm_Delete( self );
}

void _MatrixAssemblyTerm_NA__NB__Fn_Print( void* matrixTerm, Stream* stream ) {
    MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;
    _StiffnessMatrixTerm_Print( self, stream );
    /* General info */
}

void* _MatrixAssemblyTerm_NA__NB__Fn_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                                 _sizeOfSelf = sizeof(MatrixAssemblyTerm_NA__NB__Fn);
    Type                                                         type = MatrixAssemblyTerm_NA__NB__Fn_Type;
    Stg_Class_DeleteFunction*                                 _delete = _MatrixAssemblyTerm_NA__NB__Fn_Delete;
    Stg_Class_PrintFunction*                                   _print = _MatrixAssemblyTerm_NA__NB__Fn_Print;
    Stg_Class_CopyFunction*                                     _copy = NULL;
    Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _MatrixAssemblyTerm_NA__NB__Fn_DefaultNew;
    Stg_Component_ConstructFunction*                       _construct = _MatrixAssemblyTerm_NA__NB__Fn_AssignFromXML;
    Stg_Component_BuildFunction*                               _build = _MatrixAssemblyTerm_NA__NB__Fn_Build;
    Stg_Component_InitialiseFunction*                     _initialise = _MatrixAssemblyTerm_NA__NB__Fn_Initialise;
    Stg_Component_ExecuteFunction*                           _execute = _MatrixAssemblyTerm_NA__NB__Fn_Execute;
    Stg_Component_DestroyFunction*                           _destroy = _MatrixAssemblyTerm_NA__NB__Fn_Destroy;
    StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _MatrixAssemblyTerm_NA__NB__Fn_AssembleElement;
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_MatrixAssemblyTerm_NA__NB__Fn_New(  MATRIXASSEMBLYTERM_NA__NB__FN_PASSARGS  );
}

void _MatrixAssemblyTerm_NA__NB__Fn_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
    MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;
    /* Construct Parent */
    _StiffnessMatrixTerm_AssignFromXML( self, cf, data );
   //  self->geometryMesh = Stg_ComponentFactory_ConstructByKey( cf,
   //                                                      self->name,
   //                                                      (Dictionary_Entry_Key)"GeometryMesh",
   //                                                      FeMesh,
   //                                                      False,
   //                                                      data );
}

void _MatrixAssemblyTerm_NA__NB__Fn_Build( void* matrixTerm, void* data ) {
    MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;
    _StiffnessMatrixTerm_Build( self, data );

    self->Ni = (double*)malloc(sizeof(double)*4);
    self->Mi = (double*)malloc(sizeof(double)*4);
}

void _MatrixAssemblyTerm_NA__NB__Fn_Initialise( void* matrixTerm, void* data ) {
    MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;
    _StiffnessMatrixTerm_Initialise( self, data );
}

void _MatrixAssemblyTerm_NA__NB__Fn_Execute( void* matrixTerm, void* data ) {
    _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _MatrixAssemblyTerm_NA__NB__Fn_Destroy( void* matrixTerm, void* data ) {
    MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;
    /*
    if( self->Ni ) {
      Memory_Free( self->Ni );
      self->Ni=NULL;
    }
    if( self->Mi ) {
      Memory_Free( self->Mi );
      self->Mi=NULL;
    }
    */

   delete (MatrixAssemblyTerm_NA__NB__Fn_cppdata*)self->cppdata;

    _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _MatrixAssemblyTerm_NA__NB__Fn_AssembleElement(
   void*                                              matrixTerm,
   StiffnessMatrix*                                   stiffnessMatrix,
   Element_LocalIndex                                 lElement_I,
   SystemLinearEquations*                             sle,
   FiniteElementContext*                              context,
   double**                                           elStiffMat )
{
   MatrixAssemblyTerm_NA__NB__Fn* self = (MatrixAssemblyTerm_NA__NB__Fn*)matrixTerm;
   Swarm*                              swarm        = self->integrationSwarm;
   FeVariable*                         variable_row = stiffnessMatrix->rowVariable;
   FeVariable*                         variable_col = stiffnessMatrix->columnVariable;
   Dimension_Index                     dim          = stiffnessMatrix->dim;
   Particle_InCellIndex                cParticle_I, cellParticleCount;
   Node_ElementLocalIndex              nodesPerEl_row, nodesPerEl_col;

   Dof_Index                           dofPerNode_col;
   Index                               row, col; /* Indices into the stiffness matrix */
   Node_ElementLocalIndex              rowNode_I, colNode_I;
   Dof_Index                           colDof_I;
   double                              *xi, *Ni, *Mi;
   double                              detJac, weight, F, factor;
   IntegrationPoint*                   intPoint;
   Cell_Index                          cell_I;
   ElementType*                        elementType_row;
   ElementType*                        elementType_col;


   MatrixAssemblyTerm_NA__NB__Fn_cppdata* cppdata = (MatrixAssemblyTerm_NA__NB__Fn_cppdata*)self->cppdata;
   debug_dynamic_cast<ParticleInCellCoordinate>(cppdata->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
   cppdata->input->index() = lElement_I;

   FeMesh*                 geometryMesh = ( self->geometryMesh ? self->geometryMesh : variable_row->feMesh );
   ElementType*            geometryElementType;

   /* Set the element type */
   geometryElementType = FeMesh_GetElementType( geometryMesh, lElement_I );

   elementType_row = FeMesh_GetElementType( variable_row->feMesh, lElement_I );
   nodesPerEl_row = elementType_row->nodeCount;
   elementType_col = FeMesh_GetElementType( variable_col->feMesh, lElement_I );
   nodesPerEl_col = elementType_col->nodeCount;

   dofPerNode_col = variable_col->fieldComponentCount;

   // allocate shape function array, Mi and Ni
   if( nodesPerEl_row > self->max_nElNodes_row ) {
      /* reallocate */
      if (self->Mi) free(self->Mi);
      self->Mi = (double*)AllocArray( double, nodesPerEl_row );
      self->max_nElNodes_row = nodesPerEl_row;
   }

   if( nodesPerEl_col > self->max_nElNodes_col ) {
      if( self->Ni) free(self->Ni);
      self->Ni = (double*)AllocArray(double, nodesPerEl_col );
      self->max_nElNodes_col = nodesPerEl_col;
   }
   Ni = self->Ni;
   if (elementType_row == elementType_col)
      Mi = Ni;

   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      debug_dynamic_cast<ParticleInCellCoordinate>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
      /* get integration point information */
      intPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
      xi = intPoint->xi;
      weight = intPoint->weight;

      /* evaluate shape function and jacobian determinant */
      detJac = ElementType_JacobianDeterminant( geometryElementType, geometryMesh, lElement_I, xi, dim );
      ElementType_EvaluateShapeFunctionsAt( elementType_col, xi, Ni );
      if (elementType_row != elementType_col)
         ElementType_EvaluateShapeFunctionsAt( elementType_row, xi, Mi );

      // /* evaluate function */
      std::shared_ptr<const IO_double> funcout = debug_dynamic_cast<const IO_double>(cppdata->func(cppdata->input));
      F = funcout->at();

      factor = weight*detJac*F;
      /* build stiffness matrix */
      for ( rowNode_I = 0; rowNode_I < nodesPerEl_row ; rowNode_I++) {
         for (colNode_I = 0; colNode_I < nodesPerEl_col; colNode_I++ ) {
            for( colDof_I=0; colDof_I<dofPerNode_col; colDof_I++) {
                  /* note that we use the row dof count here too */
                  row = rowNode_I*dofPerNode_col + colDof_I;
                  col = colNode_I*dofPerNode_col + colDof_I;
                  elStiffMat[row][col] +=  factor * ( Ni[rowNode_I] * Mi[colNode_I] );
            }
         }
      }
   }
}
