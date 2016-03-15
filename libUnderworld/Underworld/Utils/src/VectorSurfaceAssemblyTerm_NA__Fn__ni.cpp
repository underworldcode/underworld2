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

#include "VectorAssemblyTerm_NA__Fn.h"
#include "VectorSurfaceAssemblyTerm_NA__Fn__ni.h"

/* Textual name of this class */
const Type VectorSurfaceAssemblyTerm_NA__Fn__ni_Type = "VectorSurfaceAssemblyTerm_NA__Fn__ni";

/* Creation implementation / Virtual constructor */
VectorSurfaceAssemblyTerm_NA__Fn__ni* _VectorSurfaceAssemblyTerm_NA__Fn__ni_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self;

    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(VectorSurfaceAssemblyTerm_NA__Fn__ni) );
    /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
    /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
       and so should be set to ZERO in any children of this class. */
    nameAllocationType = NON_GLOBAL;

    self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*) _ForceTerm_New(  FORCETERM_PASSARGS  );

    /* Virtual info */
    self->cppdata = (void*) new VectorAssemblyTerm_NA__Fn_cppdata;

    return self;
}

void _VectorSurfaceAssemblyTerm_SetBNodes( void* _self, void* bNodes )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)_self;

    if(!Stg_Class_IsInstance( bNodes, VariableCondition_Type ))
        throw std::invalid_argument("Provided 'indexSet' does not appear to be of 'IndexSet' type.");
    self->bNodes = (VariableCondition*)bNodes;

    self->inc = IArray_New();
}
void _VectorSurfaceAssemblyTerm_NA__Fn__ni_SetFn( void* _self, Fn::Function* fn )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni*  self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)_self;

    // record fn to struct
    VectorAssemblyTerm_NA__Fn_cppdata* cppdata = (VectorAssemblyTerm_NA__Fn_cppdata*) self->cppdata;
    cppdata->fn = fn;

    FeMesh* mesh = self->forceVector->feVariable->feMesh;
    IntegrationPointsSwarm* swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    int dphi_dx_size;
    
    /* Work out the input size: 
       if scalar field, assume a gradient vector of size mesh's dim
       else assume vector field and use a correnponding 2nd order symmetric tensor size
    */
    switch ( self->forceVector->feVariable->fieldComponentCount ) {
        case 1:
            dphi_dx_size = Mesh_GetDimSize(mesh);
            break;
        case 2:
        case 3:
            dphi_dx_size = StGermain_nSymmetricTensorVectorComponents(Mesh_GetDimSize(mesh));
            break;
        default:
            std::stringstream ss;
            ss << "Neumann condition assembly term can only handle MeshVariable's with 1,2 or 3 components.\n";
            ss << "The input MeshVariable has " << self->forceVector->feVariable->fieldComponentCount << "\n";
            throw std::invalid_argument( ss.str() );
    }

    // setup fn
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( swarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)mesh, localCoord);
    cppdata->func = fn->getFunction(cppdata->input);

    // check output conforms
    std::shared_ptr<const FunctionIO> sampleguy = cppdata->func(cppdata->input);
    std::shared_ptr<const IO_double> iodub = std::dynamic_pointer_cast<const IO_double>(sampleguy);
    if( !iodub )
        throw std::invalid_argument( "Assembly term expects functions to return 'double' type values." );
    if( !self->forceVector )
        throw std::invalid_argument( "Assembly term does not appear to have AssembledVector set." );
    if( iodub->size() != dphi_dx_size ) {
        std::stringstream ss;
        ss << "Assembly term expects function to return array of size " << dphi_dx_size << ".\n";
        ss << "Provied function returns array of size " << iodub->size() << ".";
        throw std::invalid_argument( ss.str() );
    }
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Delete( void* forceTerm )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)forceTerm;

    Stg_Class_Delete( self->inc );
    delete (VectorAssemblyTerm_NA__Fn_cppdata*)self->cppdata;

    _ForceTerm_Delete( self );
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Print( void* forceTerm, Stream* stream ) {}

void* _VectorSurfaceAssemblyTerm_NA__Fn__ni_DefaultNew( Name name )
{
    /* Variables set in this function */
    SizeT                                              _sizeOfSelf = sizeof(VectorSurfaceAssemblyTerm_NA__Fn__ni);
    Type                                                      type = VectorSurfaceAssemblyTerm_NA__Fn__ni_Type;
    Stg_Class_DeleteFunction*                              _delete = _VectorSurfaceAssemblyTerm_NA__Fn__ni_Delete;
    Stg_Class_PrintFunction*                                _print = _VectorSurfaceAssemblyTerm_NA__Fn__ni_Print;
    Stg_Class_CopyFunction*                                  _copy = NULL;
    Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _VectorSurfaceAssemblyTerm_NA__Fn__ni_DefaultNew;
    Stg_Component_ConstructFunction*                    _construct = _VectorSurfaceAssemblyTerm_NA__Fn__ni_AssignFromXML;
    Stg_Component_BuildFunction*                            _build = _VectorSurfaceAssemblyTerm_NA__Fn__ni_Build;
    Stg_Component_InitialiseFunction*                  _initialise = _VectorSurfaceAssemblyTerm_NA__Fn__ni_Initialise;
    Stg_Component_ExecuteFunction*                        _execute = _VectorSurfaceAssemblyTerm_NA__Fn__ni_Execute;
    Stg_Component_DestroyFunction*                        _destroy = _VectorSurfaceAssemblyTerm_NA__Fn__ni_Destroy;
    ForceTerm_AssembleElementFunction*            _assembleElement = _VectorSurfaceAssemblyTerm_NA__Fn__ni_AssembleElement;

    /* Variables that are set to ZERO are variables that will be set eis by the current _New function or another parent _New function further up the hierachy */
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_VectorSurfaceAssemblyTerm_NA__Fn__ni_New(  FORCEASSEMBLYTERM_NA__FN_PASSARGS  );
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_AssignFromXML( void* forceTerm, Stg_ComponentFactory* cf, void* data )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni*  self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)forceTerm;

    /* Construct Parent */
    _ForceTerm_AssignFromXML( self, cf, data );
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Build( void* forceTerm, void* data )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)forceTerm;

    _ForceTerm_Build( self, data );
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Initialise( void* forceTerm, void* data )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)forceTerm;

    _ForceTerm_Initialise( self, data );
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Execute( void* forceTerm, void* data )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)forceTerm;

    _ForceTerm_Execute( self, data );
}

void _VectorSurfaceAssemblyTerm_NA__Fn__ni_Destroy( void* forceTerm, void* data )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = (VectorSurfaceAssemblyTerm_NA__Fn__ni*)forceTerm;

    _ForceTerm_Destroy( self, data );
}


void _VectorSurfaceAssemblyTerm_NA__Fn__ni_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec )
{
    VectorSurfaceAssemblyTerm_NA__Fn__ni* self = Stg_CheckType( forceTerm, VectorSurfaceAssemblyTerm_NA__Fn__ni );
    IntegrationPointsSwarm*    swarm = (IntegrationPointsSwarm*)self->integrationSwarm;
    IntegrationPoint*          particle;
    FeMesh*                    mesh;
    ElementType*               elementType;
    unsigned int               dim, dofsPerNode, ii, n_i, cell_I, A, cParticle_I, cellParticleCount, nodesPerEl;
    double                     fluxVector[3], jacDet, factor, *xi, *vector, localNormal[3], N[27];
    int                        nInc, *inc, dphi_dx_size;
    Bool                       assemble;
    double dphi_dx[6];

    dim         = forceVector->dim;
    dofsPerNode = forceVector->feVariable->fieldComponentCount;

    if (dofsPerNode > 1) {
        dphi_dx_size = StGermain_nSymmetricTensorVectorComponents(dim);
    } else {
        dphi_dx_size = dim;
    }

    // get the element's nodes
    mesh = forceVector->feVariable->feMesh;
    elementType = FeMesh_GetElementType( mesh, lElement_I );
    FeMesh_GetElementNodes( mesh, lElement_I, self->inc ); 

    // test if nodes are in the boundarySet bNodes (passed in from python)
    nodesPerEl = IArray_GetSize( self->inc );
    inc        = IArray_GetPtr( self->inc );
    assemble   = False;//flag for if we need to assemble this element
    for( n_i=0 ; n_i<nodesPerEl; n_i++ ) {
        // TODO: node sure about local or global numbers here
        if ( VariableCondition_IsCondition( self->bNodes, inc[n_i], 0 ) ) {
            assemble = True;
            break;
        }
    }
     
    // not a boundary element - don't calculate 
    if( assemble == False ) return;

    // set up the function input
    VectorAssemblyTerm_NA__Fn_cppdata* cppdata = (VectorAssemblyTerm_NA__Fn_cppdata*)self->cppdata;
    debug_dynamic_cast<ParticleInCellCoordinate>(cppdata->input->localCoord())->index() = lElement_I;
    cppdata->input->index() = lElement_I;  

    cell_I = CellLayout_MapElementIdToCellId( swarm->cellLayout, lElement_I );
    cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];

    for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
        debug_dynamic_cast<ParticleInCellCoordinate>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId
        particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
        xi       = particle->xi;

        // get the localNormal
        /* TODO: generalise the normal evaluation - the following assumes rectangular domain */
        ElementType_SurfaceNormal( elementType, lElement_I, dim, xi, localNormal );

        /* Calculate Determinant of Jacobian and Shape Functions */
        ElementType_EvaluateShapeFunctionsAt( elementType, xi, N );
        jacDet = ElementType_SurfaceJacobianDeterminant( elementType, mesh, lElement_I, xi, dim, localNormal );

        /* evaluate function */
        std::shared_ptr<const IO_double> funcout = debug_dynamic_cast<const IO_double>(cppdata->func(cppdata->input));
        for( ii=0; ii<dphi_dx_size; ii++ ) {
            dphi_dx[ii] = funcout->at(ii); // get the gradient solution definition
        }

        memset( fluxVector, 0, sizeof(double)*dim ); // zero the fluxVector
        if( dofsPerNode == 1 ) { // we can treat dphi_dx as a vector
            for( ii=0; ii<dim; ii++ ) 
                fluxVector[0] -= dphi_dx[ii]*localNormal[ii];
        } else if ( dofsPerNode == 2 || dofsPerNode == 3 ) {
            // treat dphi_dx as a symmetric tensor
            int index;
            for( ii=0; ii<dim; ii++ ) {
                for( A=0; A<dim; A++ ) {
                    // get the index of sym tensor to multiply
                    index = SymmetricTensor_TensorMap( ii, A, dim );
                    fluxVector[ii] += dphi_dx[index] * localNormal[ii];
                }
            }
        }

        factor = jacDet * particle->weight;
        for( A = 0 ; A < nodesPerEl ; A++ ) {
            if ( VariableCondition_IsCondition( self->bNodes, inc[A], 0 ) ) {
                for( ii = 0 ; ii < dofsPerNode ; ii++ )
                    elForceVec[A*dofsPerNode+ii] += factor*fluxVector[ii]*N[A] ;
            }
        }
    }
}
