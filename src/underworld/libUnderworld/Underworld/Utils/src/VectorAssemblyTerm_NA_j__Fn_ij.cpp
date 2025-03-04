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

#include <Underworld/Function/src/FunctionIO.hpp>
#include <Underworld/Function/src/FEMCoordinate.hpp>
#include <Underworld/Function/src/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/src/Function.hpp>

#include "VectorAssemblyTerm_NA_j__Fn_ij.h"

/* Textual name of this class */
const Type VectorAssemblyTerm_NA_j__Fn_ij_Type = (char*) "VectorAssemblyTerm_NA_j__Fn_ij";

/* Creation implementation / Virtual constructor */
VectorAssemblyTerm_NA_j__Fn_ij* _VectorAssemblyTerm_NA_j__Fn_ij_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  )
{
   VectorAssemblyTerm_NA_j__Fn_ij* self;

   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(VectorAssemblyTerm_NA_j__Fn_ij) );
   /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
   /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
      and so should be set to ZERO in any children of this class. */
   nameAllocationType = NON_GLOBAL;

   self = (VectorAssemblyTerm_NA_j__Fn_ij*) _ForceTerm_New(  FORCETERM_PASSARGS  );

   /* Virtual info */
   self->funeForce = (void*) new VectorAssemblyTerm_NA_j__Fn_ij_cppdata;

   return self;
}

void _VectorAssemblyTerm_NA_j__Fn_ij_SetFn( void* _self, Fn::Function* fn ){
    VectorAssemblyTerm_NA_j__Fn_ij*  self = (VectorAssemblyTerm_NA_j__Fn_ij*)_self;
    double     dim;
    
    // record fn to struct
    VectorAssemblyTerm_NA_j__Fn_ij_cppdata* funeForce = (VectorAssemblyTerm_NA_j__Fn_ij_cppdata*) self->funeForce;
    funeForce->fn = fn;

    if( !self->forceVector )
        throw std::invalid_argument( "Assembly term does not appear to have AssembledVector set." );
    FeMesh* mesh = self->forceVector->feVariable->feMesh;
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;

    dim = Mesh_GetDimSize( mesh );

    // setup fn
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    funeForce->input = std::make_shared<FEMCoordinate>((void*)mesh, localCoord);
    funeForce->func = fn->getFunction(funeForce->input.get());
    
    // check output conforms
    const FunctionIO* sampleguy = funeForce->func(funeForce->input.get());
    const IO_double* iodub = dynamic_cast<const IO_double*>(sampleguy);
    if( !iodub )
        throw std::invalid_argument( "Assembly term expects functions to return 'double' type values." );
    if( dim == 2 ) {
	    if( iodub->size() != 3 ){  //assuming a 2D symmetric tensor
    	    std::stringstream ss;
	        ss << "Assembly term expects function to return array of size " << 3 << ".\n";
    	    ss << "Provided function returns array of size " << iodub->size() << ".";
        	throw std::invalid_argument( ss.str() );
        }
    }
    else {
	    if( iodub->size() != 6 ){  //assuming a DD symmetric tensor
    	    std::stringstream ss;
	        ss << "Assembly term expects function to return array of size " << 6 << ".\n";
    	    ss << "Provided function returns array of size " << iodub->size() << ".";
        	throw std::invalid_argument( ss.str() );
        }
    }
}

void _VectorAssemblyTerm_NA_j__Fn_ij_Delete( void* forceTerm ) {
   VectorAssemblyTerm_NA_j__Fn_ij* self = (VectorAssemblyTerm_NA_j__Fn_ij*)forceTerm;

    delete (VectorAssemblyTerm_NA_j__Fn_ij_cppdata*)self->funeForce;

   _ForceTerm_Delete( self );
}

void _VectorAssemblyTerm_NA_j__Fn_ij_Print( void* forceTerm, Stream* stream ) {}

void* _VectorAssemblyTerm_NA_j__Fn_ij_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                              _sizeOfSelf = sizeof(VectorAssemblyTerm_NA_j__Fn_ij);
   Type                                                      type = VectorAssemblyTerm_NA_j__Fn_ij_Type;
   Stg_Class_DeleteFunction*                              _delete = _VectorAssemblyTerm_NA_j__Fn_ij_Delete;
   Stg_Class_PrintFunction*                                _print = _VectorAssemblyTerm_NA_j__Fn_ij_Print;
   Stg_Class_CopyFunction*                                  _copy = NULL;
   Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _VectorAssemblyTerm_NA_j__Fn_ij_DefaultNew;
   Stg_Component_ConstructFunction*                    _construct = _VectorAssemblyTerm_NA_j__Fn_ij_AssignFromXML;
   Stg_Component_BuildFunction*                            _build = _VectorAssemblyTerm_NA_j__Fn_ij_Build;
   Stg_Component_InitialiseFunction*                  _initialise = _VectorAssemblyTerm_NA_j__Fn_ij_Initialise;
   Stg_Component_ExecuteFunction*                        _execute = _VectorAssemblyTerm_NA_j__Fn_ij_Execute;
   Stg_Component_DestroyFunction*                        _destroy = _VectorAssemblyTerm_NA_j__Fn_ij_Destroy;
   ForceTerm_AssembleElementFunction*            _assembleElement = _VectorAssemblyTerm_NA_j__Fn_ij_AssembleElement;

   /* Variables that are set to ZERO are variables that will be set eis by the current _New function or another parent _New function further up the hierachy */
   AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

   return (void*)_VectorAssemblyTerm_NA_j__Fn_ij_New(  FORCEASSEMBLYTERM_NA__FN_PASSARGS  );
}

void _VectorAssemblyTerm_NA_j__Fn_ij_AssignFromXML( void* forceTerm, Stg_ComponentFactory* cf, void* data ) {
   VectorAssemblyTerm_NA_j__Fn_ij*  self = (VectorAssemblyTerm_NA_j__Fn_ij*)forceTerm;

   /* Construct Parent */
   _ForceTerm_AssignFromXML( self, cf, data );

}

void _VectorAssemblyTerm_NA_j__Fn_ij_Build( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_j__Fn_ij* self = (VectorAssemblyTerm_NA_j__Fn_ij*)forceTerm;

   _ForceTerm_Build( self, data );
}

void _VectorAssemblyTerm_NA_j__Fn_ij_Initialise( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_j__Fn_ij* self = (VectorAssemblyTerm_NA_j__Fn_ij*)forceTerm;
   ForceVector* vector = self->forceVector;
   ElementType* elementType;

   _ForceTerm_Initialise( self, data );

   // allocate GNx before assembly
   elementType = FeMesh_GetElementType( vector->feVariable->feMesh, 0 );
   self->maxNodesPerEl = elementType->nodeCount;
   self->GNx   = Memory_Alloc_2DArray( double, vector->dim, self->maxNodesPerEl, (Name)(char*)"GNx" );
}

void _VectorAssemblyTerm_NA_j__Fn_ij_Execute( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_j__Fn_ij* self = (VectorAssemblyTerm_NA_j__Fn_ij*)forceTerm;

   _ForceTerm_Execute( self, data );
}

void _VectorAssemblyTerm_NA_j__Fn_ij_Destroy( void* forceTerm, void* data ) {
   VectorAssemblyTerm_NA_j__Fn_ij* self = (VectorAssemblyTerm_NA_j__Fn_ij*)forceTerm;

   _ForceTerm_Destroy( self, data );
}


void _VectorAssemblyTerm_NA_j__Fn_ij_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec ) {
   VectorAssemblyTerm_NA_j__Fn_ij* self = Stg_CheckType( forceTerm, VectorAssemblyTerm_NA_j__Fn_ij );
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
   double                     **GNx;
   const double *             eForce;
 
   /* Since we are integrating over the velocity mesh - we want the velocity mesh here and not the temperature mesh */
   mesh = forceVector->feVariable->feMesh;

   VectorAssemblyTerm_NA_j__Fn_ij_cppdata* funeForce      = (VectorAssemblyTerm_NA_j__Fn_ij_cppdata*)self->funeForce;
    
   debug_dynamic_cast<ParticleInCellCoordinate*>(funeForce->input->localCoord())->index()      = lElement_I;  // set the elementId as the owning cell for the particleCoord

   funeForce->input->index()        = lElement_I;  // set the elementId for the fem coordinate

   /* Set the element type */
   elementType = FeMesh_GetElementType( mesh, lElement_I );
   nodesPerEl  = elementType->nodeCount;
   if( nodesPerEl > self->maxNodesPerEl ) { // test if reallocation is required
       Memory_Free( self->GNx );
       self->GNx = Memory_Alloc_2DArray_Unnamed( double, dim, nodesPerEl );
       self->maxNodesPerEl = nodesPerEl;
   }
   GNx = self->GNx;
   
   /* assumes constant number of dofs per element */
   dofsPerNode = forceVector->feVariable->fieldComponentCount;

   cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
   cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

   for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
      debug_dynamic_cast<ParticleInCellCoordinate*>(funeForce->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
      particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
      xi       = particle->xi;
     
     
      /* Calculate Determinant of Jacobian and Shape Functions */
      ElementType_ShapeFunctionsGlobalDerivs( elementType, mesh, lElement_I, xi, dim, &detJac, GNx );

      /* evaluate function */
      const IO_double* funeForceout      = debug_dynamic_cast<const IO_double*>(funeForce->func(funeForce->input.get()));
      
      eForce = funeForceout->data();
        	  	
      factor = detJac * particle->weight;
 
      if( dim == 2 ) {
            for( A = 0 ; A < nodesPerEl ; A++ ) {
			      elForceVec[A * dofsPerNode + I_AXIS ] -= factor * (GNx[0][A] * eForce[0] + GNx[1][A] * eForce[2] ) ;
			      elForceVec[A * dofsPerNode + J_AXIS ] -= factor * (GNx[1][A] * eForce[1] + GNx[0][A] * eForce[2] ) ;
            }           
      } 
      else {
            for( A = 0 ; A < nodesPerEl ; A++ ) {
		      	  elForceVec[A * dofsPerNode + I_AXIS ] -= factor * (GNx[0][A]*eForce[0] + GNx[1][A]*eForce[3] + GNx[2][A]*eForce[4]);
			      elForceVec[A * dofsPerNode + J_AXIS ] -= factor * (GNx[1][A]*eForce[1] + GNx[0][A]*eForce[3] + GNx[2][A]*eForce[5]);
			      elForceVec[A * dofsPerNode + K_AXIS ] -= factor * (GNx[2][A]*eForce[2] + GNx[0][A]*eForce[4] + GNx[1][A]*eForce[5]);
		    }
        
      }
   }
}
