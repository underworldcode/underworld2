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
#include <petsc.h>

#include <Underworld/Function/src/FunctionIO.hpp>
#include <Underworld/Function/src/FEMCoordinate.hpp>
#include <Underworld/Function/src/ParticleInCellCoordinate.hpp>
#include <Underworld/Function/src/Function.hpp>

#include "Fn_Integrate.h"

/* Textual name of this class */
const Type Fn_Integrate_Type = (char*) "Fn_Integrate";

/* Creation implementation / Virtual constructor */
Fn_Integrate* _Fn_Integrate_New(  FN_INTEGRATE_DEFARGS  )
{
    Fn_Integrate* self;

    /* Allocate memory */
    assert( _sizeOfSelf >= sizeof(Fn_Integrate) );
    /* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
    /* This means that any values of these parameters that are passed into this function are not passed onto the parent function
    and so should be set to ZERO in any children of this class. */
    nameAllocationType = NON_GLOBAL;

    self = (Fn_Integrate*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
    self->cppdata = (void*) new Fn_Integrate_cppdata;
    Fn_Integrate_cppdata* cppdata = (Fn_Integrate_cppdata*)self->cppdata;
    cppdata->sumLocal = NULL;
    cppdata->sumGlobal = NULL;
    self->dim = 0;
    self->isSurfaceIntegral = False;

    /* Virtual info */

    return self;
    
}

void* _Fn_Integrate_DefaultNew( Name name ) {
    /* Variables set in this function */
    SizeT                                              _sizeOfSelf = sizeof(Fn_Integrate);
    Type                                                      type = Fn_Integrate_Type;
    Stg_Class_DeleteFunction*                              _delete = _Stg_Component_Delete;
    Stg_Class_PrintFunction*                                _print = _Stg_Component_Print;
    Stg_Class_CopyFunction*                                  _copy = NULL;
    Stg_Component_DefaultConstructorFunction*  _defaultConstructor = _Fn_Integrate_DefaultNew;
    Stg_Component_ConstructFunction*                    _construct = _Fn_Integrate_AssignFromXML;
    Stg_Component_BuildFunction*                            _build = _Fn_Integrate_Build;
    Stg_Component_InitialiseFunction*                  _initialise = _Fn_Integrate_Initialise;
    Stg_Component_ExecuteFunction*                        _execute = _Fn_Integrate_Execute;
    Stg_Component_DestroyFunction*                        _destroy = _Fn_Integrate_Destroy;

    /* Variables that are set to ZERO are variables that will be set eis by the current _New function or another parent _New function further up the hierachy */
    AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

    return (void*)_Fn_Integrate_New(  FN_INTEGRATE_PASSARGS  );
}

void _Fn_Integrate_SetFn( void* _self, Fn::Function* fn ){
    Fn_Integrate*  self = (Fn_Integrate*)_self;
    
    // record fn to struct
    Fn_Integrate_cppdata* cppdata = (Fn_Integrate_cppdata*) self->cppdata;
    cppdata->fn = fn;
    
    // setup fn
    std::shared_ptr<ParticleInCellCoordinate> localCoord = std::make_shared<ParticleInCellCoordinate>( self->integrationSwarm->localCoordVariable );
    cppdata->input = std::make_shared<FEMCoordinate>((void*)self->mesh, localCoord);
    cppdata->func  = fn->getFunction(cppdata->input.get());
    
    // check output conforms
    const FunctionIO* io = dynamic_cast<const FunctionIO*>(cppdata->func(cppdata->input.get()));
    cppdata->sumLocal  = new IO_double(io->size(), io->iotype());
    cppdata->sumGlobal = new IO_double(io->size(), io->iotype());
}


void _Fn_Integrate_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data ){
}

void _Fn_Integrate_Build( void* _self, void* data ) {}

void _Fn_Integrate_Initialise( void* _self, void* data ) {}

void _Fn_Integrate_Destroy( void* _self, void* data ){
    Fn_Integrate*  self = (Fn_Integrate*)_self;
    Fn_Integrate_cppdata* cppdata = (Fn_Integrate_cppdata*)self->cppdata;
    
    if (cppdata->sumLocal)
        delete cppdata->sumLocal;
    if (cppdata->sumGlobal)
        delete cppdata->sumGlobal;
    delete cppdata;
}

void _Fn_Integrate_Execute( void* _self, void* data ) {}

IO_double* Fn_Integrate_Integrate( void* fn_integrate ) {
    Fn_Integrate*  self = (Fn_Integrate*)fn_integrate;
    IntegrationPoint*          particle;
    IntegrationPointsSwarm*    swarm = self->integrationSwarm;
    FeMesh*                    mesh  = self->mesh;
    double*                    xi;
    Particle_InCellIndex       cParticle_I;
    Particle_InCellIndex       cellParticleCount;
    ElementType*               elementType;
    Cell_Index                 cell_I;
    double                     jacDet;
    double                     factor;
    double                     N[27];
    int                        nElements;
    int                        lElement_I;

    Fn_Integrate_cppdata* cppdata = (Fn_Integrate_cppdata*)self->cppdata;
    
    // init to zero
    for( unsigned ii = 0 ; ii < cppdata->sumLocal->size() ; ii++ )
    {
        cppdata->sumLocal->at(ii) = 0.;  // set new value
        cppdata->sumGlobal->at(ii) = 0.;  // set new value
    }

    nElements = FeMesh_GetElementLocalSize( mesh );
    // printf("n_elements = %d \n", n_elements );

    for( lElement_I=0; lElement_I<nElements; lElement_I++ ) {
        cell_I = CellLayout_MapElementIdToCellId( self->integrationSwarm->cellLayout, lElement_I );
        cellParticleCount = swarm->cellParticleCountTbl[ cell_I ];
        elementType = FeMesh_GetElementType( mesh, lElement_I );
    
        debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->index() = lElement_I;  // set the elementId as the owning cell for the particleCoord
        cppdata->input->index()                                                             = lElement_I;    // set the elementId for the fem coordinate
        
        for ( cParticle_I = 0 ; cParticle_I < cellParticleCount ; cParticle_I++ ) {
            debug_dynamic_cast<ParticleInCellCoordinate*>(cppdata->input->localCoord())->particle_cellId(cParticle_I);  // set the particleCoord cellId

            particle = (IntegrationPoint*) Swarm_ParticleInCellAt( swarm, cell_I, cParticle_I );
            xi       = particle->xi;

            /* Calculate Determinant of Jacobian and Shape Functions */
            if (!self->isSurfaceIntegral) {
                jacDet = ElementType_JacobianDeterminant( elementType, mesh, lElement_I, xi, self->dim );
            } else {
                double localNormal[3];
                ElementType_SurfaceNormal( elementType, lElement_I, self->dim, xi, localNormal );
                jacDet = ElementType_SurfaceJacobianDeterminant( elementType, mesh, lElement_I, xi, self->dim, localNormal );
            }
            /* evaluate function */
            const FunctionIO* funcout = debug_dynamic_cast<const FunctionIO*>(cppdata->func(cppdata->input.get()));

            factor = jacDet * particle->weight;
            for( unsigned ii = 0 ; ii < funcout->size() ; ii++ )
            {
                cppdata->sumLocal->at(ii) += funcout->at<double>(ii)*factor;
            }

        }
    }

    (void)MPI_Allreduce ( cppdata->sumLocal->data(), cppdata->sumGlobal->data(), cppdata->sumGlobal->size(), MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD );
   
    return cppdata->sumGlobal;
}

