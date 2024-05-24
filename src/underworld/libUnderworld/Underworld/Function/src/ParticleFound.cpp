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
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
}

#include "ParticleFound.hpp"
#include "FunctionIO.hpp"
#include "FEMCoordinate.hpp"
#include "ParticleCoordinate.hpp"
#include "ParticleInCellCoordinate.hpp"

Fn::ParticleFound::ParticleFound( void* swarm ):Function(), _swarm(swarm){
    // setup output
    if(!Stg_Class_IsInstance( _swarm, Swarm_Type ))
        throw std::invalid_argument(_pyfnerrorheader+"Provided 'swarm' does not appear to be of 'Swarm' type.");

}

Fn::ParticleFound::func Fn::ParticleFound::getFunction( IOsptr sample_input )
{
    Swarm* swarm = (Swarm*)_swarm;

    std::shared_ptr<IO_bool> _output_sp = std::make_shared<IO_bool>(1, FunctionIO::Scalar);
    IO_bool* _output = _output_sp.get();

    const FEMCoordinate*      meshCoord = dynamic_cast<const FEMCoordinate*>(sample_input);
    const ParticleCoordinate* partCoord = dynamic_cast<const ParticleCoordinate*>(sample_input);
    
    if( meshCoord )
    {
        const ParticleInCellCoordinate* partCoord = dynamic_cast<const ParticleInCellCoordinate*>(meshCoord->localCoord());
        if (partCoord)
            // return the lambda
            return [_output, _output_sp, swarm](IOsptr input)->IOsptr {
                const FEMCoordinate*            meshCoord = debug_dynamic_cast<const FEMCoordinate*>(input);
                const ParticleInCellCoordinate* partCoord = debug_dynamic_cast<const ParticleInCellCoordinate*>(meshCoord->localCoord());
                
                IntegrationPointsSwarm* intSwarm = (IntegrationPointsSwarm*)((SwarmVariable*)partCoord->object())->swarm;
                unsigned elementIndex = partCoord->index();
                unsigned partCellId   = partCoord->particle_cellId();
                
                // mapping 
                unsigned swarmVarLocalIndex = GeneralSwarm_IntegrationPointMap( swarm, intSwarm, elementIndex, partCellId );
                
                if (swarmVarLocalIndex == (unsigned)-1)
                    _output->at() = false;
                else
                    _output->at() = true;

                return debug_dynamic_cast<const FunctionIO*>(_output);
            };
    }
    
    if (partCoord)
    {
        if (((SwarmVariable*)partCoord->object())->swarm == swarm)
            // set to output true, as this will always be the case
            _output->at() = true;
            return [_output, _output_sp](IOsptr input)->IOsptr {
                return debug_dynamic_cast<const FunctionIO*>(_output);
            };
    }


    // if we have gotten to this point, none of the supplied inputs can be used
    // to access a particle, therefore always return false.
    _output->at() = false;
    return [_output, _output_sp](IOsptr input)->IOsptr {
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
    
    
    
}


