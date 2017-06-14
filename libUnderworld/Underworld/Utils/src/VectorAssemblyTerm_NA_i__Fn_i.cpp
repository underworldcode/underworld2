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
#include <sstream>

#include <mpi.h>
#include <petsc.h>

#include <Underworld/Function/FunctionIO.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>
#include <Underworld/Function/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/Function.hpp>

#include "VectorAssemblyTerm_NA_i__Fn_i.h"

/* Textual name of this class */
const Type VectorAssemblyTerm_NA_i__Fn_i_Type = "VectorAssemblyTerm_NA_i__Fn_i";

/* Creation implementation / Virtual constructor */
VectorAssemblyTerm_NA_i__Fn_i* _VectorAssemblyTerm_NA_i__Fn_i_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  )
{
   VectorAssemblyTerm_NA_i__Fn_i* self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(VectorAssemblyTerm_NA_i__Fn_i) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (VectorAssemblyTerm_NA_i__Fn_i*) _ForceTerm_New(  FORCETERM_PASSARGS  );

   /* Virtual info */
   self->cppdata = (void*) new VectorAssemblyTerm_NA_i__Fn_i_cppdata;
   self->geometryMesh = NULL;

   return self;
}

void _VectorAssemblyTerm_NA_i__Fn_i_SetFn( void* _self, Fn::Function* fn ){
    VectorAssemblyTerm_NA_i__Fn_i*  self = (VectorAssemblyTerm_NA_i__Fn_i*)_self;
    
    // record fn to struct
    VectorAssemblyTerm_NA_i__Fn_i_cppdata* cppdata = (VectorAssemblyTerm_NA_i__Fn_i_cppdata*) self->cppdata;
    cppdata->fn = fn;
    
    FeMesh* mesh = self->forceVector->feVariable->feMesh;
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;

    // setup fn
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)mesh, localCoord);
    cppdata->func = fn->getFunction(cppdata->input.get());
    
    // check output conforms
    const FunctionIO* sampleguy = cppdata->func(cppdata->input.get());
    const FunctionIO* io = dynamic_cast<const FunctionIO*>(sampleguy);
    if( !self->forceVector )
        throw std::invalid_argument( "Assembly term does not appear to have AssembledVector set." );
    if( io->size() != self->forceVector->dim ){
        std::stringstream ss;
        ss << "Assembly term expects function to return array of size " << self->forceVector->dim << ".\n";
        ss << "Provided function returns array of size " << io->size() << ".";
        throw std::invalid_argument( ss.str() );
    }
}



void _VectorAssemblyTerm_NA_i__Fn_i_Delete( void* forceTerm ) {
   VectorAssemblyTerm_NA_i__Fn_i* self = (VectorAssemblyTerm_NA_i__Fn_i*)forceTerm;

    delete (VectorAssemblyTerm_NA_i__Fn_i_cppdata*)self->cppdata;

   _ForceTerm_Delete( self );
}

void _VectorAssemblyTerm_NA_i__Fn_i_Print( void* forceTerm, Stream* stream ) {}

void* _VectorAssemblyTerm_NA_i__Fn_i_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof(VectorAssemblyTerm_NA_i__Fn_i);
   Type                                                      type = VectorAssemblyTerm_NA_i__Fn_i_Type;
   Stg_Class_DeleteFunction*                              _delete = _VectorAssemblyTerm_NA_i__Fn_i_Delete;
   Stg_Class_PrintFunction*                                _print = _VectorAssemblyTerm_NA_i__Fn_i_Print;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _VectorAssemblyTerm_NA_i__Fn_i_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _VectorAssemblyTerm_NA_i__Fn_i_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _VectorAssemblyTerm_NA_i__Fn_i_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _VectorAssemblyTerm_NA_i__Fn_i_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _VectorAssemblyTerm_NA_i__Fn_i_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _VectorAssemblyTerm_NA_i__Fn_i_Destroy;
   ForceTerm_AssembleElementFunction*            _assembleElement = _VectorAssemblyTerm_NA_i__Fn_i_AssembleElement;

   /* Variables that are set to ZERO are variables that will be set eis by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_VectorAssemblyTerm_NA_i__Fn_i_New(  FORCEASSEMBLYTERM_NA__FN_PASSARGS  );
}

void _VectorAssemblyTerm_NA_i__Fn_i_AssignFromXML( void* forceTerm, Stg_ComponentFactory* cf, void* data ) {
   VectorAssemblyTerm_NA_i__Fn_i*  self = (VectorAssemblyTerm_NA_i__Fn_i*)forceTerm;

   /* Construct Parent */
   _ForceTerm_AssignFromXML( self, cf, data );

}

void _VectorAssemblyTerm_NA_i__Fn_i_Build( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_i__Fn_i* self = (VectorAssemblyTerm_NA_i__Fn_i*)forceTerm;

   _ForceTerm_Build( self, data );
}

void _VectorAssemblyTerm_NA_i__Fn_i_Initialise( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_i__Fn_i* self = (VectorAssemblyTerm_NA_i__Fn_i*)forceTerm;

   _ForceTerm_Initialise( self, data );
}

void _VectorAssemblyTerm_NA_i__Fn_i_Execute( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_i__Fn_i* self = (VectorAssemblyTerm_NA_i__Fn_i*)forceTerm;

   _ForceTerm_Execute( self, data );
}

void _VectorAssemblyTerm_NA_i__Fn_i_Destroy( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_i__Fn_i* self = (VectorAssemblyTerm_NA_i__Fn_i*)forceTerm;

   _ForceTerm_Destroy( self, data );
}


void _VectorAssemblyTerm_NA_i__Fn_i_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec ) {
   VectorAssemblyTerm_NA_i__Fn_i* self = Stg_CheckType( forceTerm, VectorAssemblyTerm_NA_i__Fn_i );
   IntegrationPointsSwarm*    swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
   Dimension_Index            dim = forceVector->dim;
   IntegrationPoint*          particle;
   FeMesh*                    mesh;
   double*                    xi;
   Particle_InCellIndex       cParticle_I;
   Particle_InCellIndex       cellParticleCount;
   Element_NodeIndex          nodesPerEl;
   Node_ElementLocalIndex     A;
   ElementType*               elementType;
   Dof_Index                  dofsPerNode, i;
   Cell_Index                 cell_I;
   double                     detJac;
   double                     factor;

   /* Since we are integrating over the velocity mesh - we want the velocity mesh here and not the temperature mesh */
   mesh = forceVector->feVariable->feMesh;

   VectorAssemblyTerm_NA_i__Fn_i_cppdata* cppdata = (VectorAssemblyTerm_NA_i__Fn_i_cppdata*)self->cppdata;
    
   debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
   cppdata->input->index()   = lElement_I;  // set the elementId for the fem coordinate

   /* Set the element type */
   elementType = FeMesh_GetElementType( mesh, lElement_I );
   nodesPerEl  = elementType->nodeCount;
   if( nodesPerEl > self->maxNodesPerEl ) { // test if reallocation is required
      Memory_Free( self->GNx );
      self->GNx = Memory_Alloc_2DArray_Unnamed( double, dim, nodesPerEl );
      self->maxNodesPerEl = nodesPerEl;
   }

   /* assumes constant number of dofs per element */
   dofsPerNode = forceVector->feVariable->fieldComponentCount;

   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
      particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
      xi       = particle->xi;

      /* Calculate Determinant of Jacobian and Shape Functions */
      ElementType_ShapeFunctionsGlobalDerivs( elementType, mesh, lElement_I, xi, dim, &detJac, self->GNx );

      /* evaluate function */
      const FunctionIO* funcout = debug_dynamic_cast<const FunctionIO*>(cppdata->func(cppdata->input.get()));

      factor = detJac * particle->weight;
      for( i = 0; i < dofsPerNode; i++ )
      {
         for( A = 0; A < nodesPerEl; A++ )
         {
            unsigned row = A * dofsPerNode + i;
            for( unsigned dim_i = 0; dim_i < dim; dim_i++ )
               elForceVec[row] += factor * self->GNx[dim_i][A] * funcout->at<double>(dim_i);
         }
      }
   }
}



