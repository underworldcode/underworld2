/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <petsc.h>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>
#include <Underworld/Function/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/Function.hpp>

#include "MatrixAssemblyTerm_NA_i__NB_i__Fn.h"

/* Textual name of this class */
const Type MatrixAssemblyTerm_NA_i__NB_i__Fn_Type = "MatrixAssemblyTerm_NA_i__NB_i__Fn";

/* Creation implementation / Virtual constructor */
MatrixAssemblyTerm_NA_i__NB_i__Fn* _MatrixAssemblyTerm_NA_i__NB_i__Fn_New(  MATRIXASSEMBLYTERM_NA_I__NB_I__FN_DEFARGS  )
{
   MatrixAssemblyTerm_NA_i__NB_i__Fn* self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(MatrixAssemblyTerm_NA_i__NB_i__Fn) );
   self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*) _StiffnessMatrixTerm_New(  STIFFNESSMATRIXTERM_PASSARGS  );

   /* Virtual info */
   self->cppdata = (void*) new MatrixAssemblyTerm_NA_i__NB_i__Fn_cppdata;

   return self;
}

void MatrixAssemblyTerm_NA_i__NB_i__Fn_SetFn( void* _self, Fn::Function* fn ){
    MatrixAssemblyTerm_NA_i__NB_i__Fn*  self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)_self;

    // record fn to struct
    MatrixAssemblyTerm_NA_i__NB_i__Fn_cppdata* cppdata = (MatrixAssemblyTerm_NA_i__NB_i__Fn_cppdata*) self->cppdata;
    cppdata->fn = fn;

    // setup fn
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)swarm->mesh, localCoord);
    cppdata->func = fn->getFunction(cppdata->input.get());

    // check output conforms
    const IO_double* iodub = dynamic_cast<const IO_double*>(cppdata->func(cppdata->input.get()));
    if( !iodub )
        throw std::invalid_argument("MatrixAssemblyTerm routine expects functions to return 'double' type values.");
    if( iodub->size() != 1 )
        throw std::invalid_argument("MatrixAssemblyTerm matrix routine expects functions to scalar values.");

}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_Delete( void* matrixTerm ) {
   MatrixAssemblyTerm_NA_i__NB_i__Fn* self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)matrixTerm;

   delete (MatrixAssemblyTerm_NA_i__NB_i__Fn_cppdata*)self->cppdata;
   
   _StiffnessMatrixTerm_Delete( self );
}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_Print( void* matrixTerm, Stream* stream ) {
   MatrixAssemblyTerm_NA_i__NB_i__Fn* self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)matrixTerm;

   _StiffnessMatrixTerm_Print( self, stream );

   /* General info */
}

void* _MatrixAssemblyTerm_NA_i__NB_i__Fn_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                                 _sizeOfSelf = sizeof(MatrixAssemblyTerm_NA_i__NB_i__Fn);
   Type                                                         type = MatrixAssemblyTerm_NA_i__NB_i__Fn_Type;
   Stg_Class_DeleteFunction*                                 _delete = _MatrixAssemblyTerm_NA_i__NB_i__Fn_Delete;
   Stg_Class_PrintFunction*                                   _print = _MatrixAssemblyTerm_NA_i__NB_i__Fn_Print;
   Stg_Class_CopyFunction*                                     _copy = NULL;
   Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _MatrixAssemblyTerm_NA_i__NB_i__Fn_DefaultNew;
   Stg_Component_ConstructFunction*                       _construct = _MatrixAssemblyTerm_NA_i__NB_i__Fn_AssignFromXML;
   Stg_Component_BuildFunction*                               _build = _MatrixAssemblyTerm_NA_i__NB_i__Fn_Build;
   Stg_Component_InitialiseFunction*                     _initialise = _MatrixAssemblyTerm_NA_i__NB_i__Fn_Initialise;
   Stg_Component_ExecuteFunction*                           _execute = _MatrixAssemblyTerm_NA_i__NB_i__Fn_Execute;
   Stg_Component_DestroyFunction*                           _destroy = _MatrixAssemblyTerm_NA_i__NB_i__Fn_Destroy;
   StiffnessMatrixTerm_AssembleElementFunction*     _assembleElement = _MatrixAssemblyTerm_NA_i__NB_i__Fn_AssembleElement;

   /* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_MatrixAssemblyTerm_NA_i__NB_i__Fn_New(  MATRIXASSEMBLYTERM_NA_I__NB_I__FN_PASSARGS  );
}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_AssignFromXML( void* matrixTerm, Stg_ComponentFactory* cf, void* data ) {
   MatrixAssemblyTerm_NA_i__NB_i__Fn* self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)matrixTerm;

   /* Construct Parent */
   _StiffnessMatrixTerm_AssignFromXML( self, cf, data );

}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_Build( void* matrixTerm, void* data ) {
   MatrixAssemblyTerm_NA_i__NB_i__Fn*             self             = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)matrixTerm;

   _StiffnessMatrixTerm_Build( self, data );
}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_Initialise( void* matrixTerm, void* data ) {
   MatrixAssemblyTerm_NA_i__NB_i__Fn* self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)matrixTerm;

   _StiffnessMatrixTerm_Initialise( self, data );
}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_Execute( void* matrixTerm, void* data ) {
   _StiffnessMatrixTerm_Execute( matrixTerm, data );
}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_Destroy( void* matrixTerm, void* data ) {
   _StiffnessMatrixTerm_Destroy( matrixTerm, data );
}

void _MatrixAssemblyTerm_NA_i__NB_i__Fn_AssembleElement(
      void*                                              matrixTerm,
      StiffnessMatrix*                                   stiffnessMatrix,
      Element_LocalIndex                                 lElement_I,
      SystemLinearEquations*                             sle,
      FiniteElementContext*                              context,
      double**                                           elStiffMat )
{
   MatrixAssemblyTerm_NA_i__NB_i__Fn*   self = (MatrixAssemblyTerm_NA_i__NB_i__Fn*)matrixTerm;
   Swarm*                              swarm        = self->integrationSwarm;
   FeVariable*                         variable1    = stiffnessMatrix->rowVariable;
   Dimension_Index                     dim          = stiffnessMatrix->dim;
   IntegrationPoint*                   currIntegrationPoint;
   double*                             xi;
   double                              weight;
   Particle_InCellIndex                cParticle_I, cellParticleCount;
   Index                               nodesPerEl;
   Index                               A,B;
   Index                               i;
   double**                            GNx;
   double                              detJac;
   double                              F;
   Cell_Index                          cell_I;
   ElementType*                        elementType;

   /* Set the element type */
   elementType = FeMesh_GetElementType( variable1->feMesh, lElement_I );
   nodesPerEl = elementType->nodeCount;

   if( nodesPerEl > self->max_nElNodes ) {
      /* reallocate */
      if (self->GNx)
         free(self->GNx);
      self->GNx = (double**)AllocArray2D( double, dim, nodesPerEl );
      self->max_nElNodes = nodesPerEl;
   }
   GNx = self->GNx;

   MatrixAssemblyTerm_NA_i__NB_i__Fn_cppdata* cppdata = (MatrixAssemblyTerm_NA_i__NB_i__Fn_cppdata*)self->cppdata;

   debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
   cppdata->input->index() = lElement_I;  // set the elementId for the fem coordinate

   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
      currIntegrationPoint = (IntegrationPoint*)Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );

      xi = currIntegrationPoint->xi;
      weight = currIntegrationPoint->weight;

      ElementType_ShapeFunctionsGlobalDerivs(
         elementType,
         variable1->feMesh, lElement_I,
         xi, dim, &detJac, GNx );

      /* evaluate function */
      const IO_double* funcout = debug_dynamic_cast<const IO_double*>(cppdata->func(cppdata->input.get()));
      F = funcout->at();

      for( A=0; A<nodesPerEl; A++ )
         for( B=0; B<nodesPerEl; B++ )
            for ( i = 0; i < dim ; i++ )
               elStiffMat[A][B] += detJac * weight * GNx[i][A] * GNx[i][B] * F;
   }
}
