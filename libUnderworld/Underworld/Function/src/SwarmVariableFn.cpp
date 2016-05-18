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
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}

#include "SwarmVariableFn.hpp"
#include "FunctionIO.hpp"
#include "FEMCoordinate.hpp"
#include "ParticleCoordinate.hpp"
#include "ParticleInCellCoordinate.hpp"

Fn::SwarmVariableFn::SwarmVariableFn( void* swarmvariable ):Function(), _swarmvariable(swarmvariable){
    // setup output
    if(!Stg_Class_IsInstance( _swarmvariable, SwarmVariable_Type ))
        throw std::invalid_argument("Provided 'swarmavariable' does not appear to be of 'SwarmVariable' type.");

}


Fn::SwarmVariableFn::func Fn::SwarmVariableFn::getFunction( IOsptr sample_input )
{

    SwarmVariable* swarmvar = (SwarmVariable*)_swarmvariable;

    std::shared_ptr<FunctionIO> _output;
	switch( swarmvar->variable->dataTypes[0] ) {
		case Variable_DataType_Double:
            _output = std::make_shared<IO_double>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case Variable_DataType_Int:
            _output = std::make_shared<IO_int>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case Variable_DataType_Char:
            _output = std::make_shared<IO_char>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case Variable_DataType_Float:
            _output = std::make_shared<IO_float>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case Variable_DataType_Short:
            _output = std::make_shared<IO_short>(swarmvar->dofCount, FunctionIO::Array);
			break;
		default:
			throw std::invalid_argument( "SwarmVariable datatype does not appear to be supported." );
	}
    
    // this isn't great
    if (swarmvar->dofCount == 1) {
        _output->_iotype = FunctionIO::Scalar;
    } else if (swarmvar->dofCount <= 3)
        _output->_iotype = FunctionIO::Vector;
    else
        _output->_iotype = FunctionIO::Array;


    std::shared_ptr<const FEMCoordinate>      meshCoord = std::dynamic_pointer_cast<const FEMCoordinate>(sample_input);
    std::shared_ptr<const ParticleCoordinate> partCoord = std::dynamic_pointer_cast<const ParticleCoordinate>(sample_input);
    if (!(meshCoord||partCoord))
        throw std::invalid_argument( "Provided input to SwarmVariableFn does not appear to be of supported type.\nSupported types are 'FEMCoordinate' and 'ParticleCoordinate'." );
    
    if( meshCoord )
    {
        std::shared_ptr<const ParticleInCellCoordinate> partCoord = std::dynamic_pointer_cast<const ParticleInCellCoordinate>(meshCoord->localCoord());
        if (!partCoord)
            throw std::invalid_argument( "Provided 'FEMCoordinate' input to SwarmVariableFn does not appear to have 'ParticleInCellCoordinate' type local coordinate." );

        
        // return the lambda
        return [_output, swarmvar](IOsptr input)->IOsptr {
            std::shared_ptr<const FEMCoordinate>            meshCoord = debug_dynamic_cast<const FEMCoordinate>(input);
            std::shared_ptr<const ParticleInCellCoordinate> partCoord = debug_dynamic_cast<const ParticleInCellCoordinate>(meshCoord->localCoord());
            
            IntegrationPointsSwarm* intSwarm = (IntegrationPointsSwarm*)((SwarmVariable*)partCoord->object())->swarm;
            unsigned elementIndex = partCoord->index();
            unsigned partCellId   = partCoord->particle_cellId();
            
            // mapping 
            unsigned swarmVarLocalIndex = GeneralSwarm_IntegrationPointMap( swarmvar->swarm, intSwarm, elementIndex, partCellId );

            // find the position into the swarmvar array we need to copy from
            void* dataPtr = __Variable_GetStructPtr( swarmvar->variable, swarmVarLocalIndex );

            // copy swarmvariable datum into output
            memcpy( _output->dataRaw(), dataPtr, Variable_SizeOfDataType(swarmvar->variable->dataTypes[0]) * swarmvar->dofCount );

            return debug_dynamic_cast<const FunctionIO>(_output);
        };
    }
    
    if (partCoord)
    {
        // return the lambda
        return [_output, swarmvar](IOsptr input)->IOsptr {
            std::shared_ptr<const ParticleCoordinate>  partCoord = debug_dynamic_cast<const ParticleCoordinate>(input);
            
            // find the position into the swarmvar array we need to copy from
            void* dataPtr = __Variable_GetStructPtr( swarmvar->variable, partCoord->index() );

            // copy swarmvariable datum into output
            memcpy( _output->dataRaw(), dataPtr, Variable_SizeOfDataType(swarmvar->variable->dataTypes[0]) * swarmvar->dofCount );

            return debug_dynamic_cast<const FunctionIO>(_output);
        };
    }
    
}


