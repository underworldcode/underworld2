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
        throw std::invalid_argument(_pyfnerrorheader+"Provided 'swarmavariable' does not appear to be of 'SwarmVariable' type.");

}


Fn::SwarmVariableFn::func Fn::SwarmVariableFn::getFunction( IOsptr sample_input )
{

    SwarmVariable* swarmvar = (SwarmVariable*)_swarmvariable;

    std::shared_ptr<FunctionIO> _output_sp;
	switch( swarmvar->variable->dataTypes[0] ) {
		case StgVariable_DataType_Double:
            _output_sp = std::make_shared<IO_double>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case StgVariable_DataType_Int:
            _output_sp = std::make_shared<IO_int>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case StgVariable_DataType_Char:
            _output_sp = std::make_shared<IO_char>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case StgVariable_DataType_Float:
            _output_sp = std::make_shared<IO_float>(swarmvar->dofCount, FunctionIO::Array);
			break;
		case StgVariable_DataType_Short:
            _output_sp = std::make_shared<IO_short>(swarmvar->dofCount, FunctionIO::Array);
			break;
		default:
			throw std::invalid_argument( _pyfnerrorheader+"SwarmVariable datatype does not appear to be supported." );
	}
    FunctionIO* _output = _output_sp.get();

    // this isn't great
    if (swarmvar->dofCount == 1) {
        _output->_iotype = FunctionIO::Scalar;
    } else if (swarmvar->dofCount <= 3)
        _output->_iotype = FunctionIO::Vector;
    else
        _output->_iotype = FunctionIO::Array;


    const FEMCoordinate*  meshCoord = dynamic_cast<const FEMCoordinate*>(sample_input);    
    if( meshCoord && !swarmvar->useKDTree )
    {
        const ParticleInCellCoordinate* partCoord = dynamic_cast<const ParticleInCellCoordinate*>(meshCoord->localCoord());
        if (!partCoord)
            throw std::invalid_argument( _pyfnerrorheader+"Provided 'FEMCoordinate' input to SwarmVariableFn does not appear to have 'ParticleInCellCoordinate' type local coordinate." );

        
        // return the lambda
        return [_output, _output_sp, swarmvar,this](IOsptr input)->IOsptr {
            const FEMCoordinate*            meshCoord = debug_dynamic_cast<const FEMCoordinate*>(input);
            const ParticleInCellCoordinate* partCoord = debug_dynamic_cast<const ParticleInCellCoordinate*>(meshCoord->localCoord());
            
            IntegrationPointsSwarm* intSwarm = (IntegrationPointsSwarm*)((SwarmVariable*)partCoord->object())->swarm;
            unsigned elementIndex = partCoord->index();
            unsigned partCellId   = partCoord->particle_cellId();
            
            // mapping 
            unsigned swarmVarLocalIndex = GeneralSwarm_IntegrationPointMap( swarmvar->swarm, intSwarm, elementIndex, partCellId );

            if ( swarmVarLocalIndex == (unsigned)-1 )
            {
                throw std::domain_error(  _pyfnerrorheader+"Error occurred while trying to evaluate swarm variable. "\
                                           "This can occur when there are no particles found in a given element. "\
                                           "You may wish to add population control mechanisms. "\
                                           "Please contact developers if this does not appear to be the issue." );
            }
    
            // find the position into the swarmvar array we need to copy from
            void* dataPtr = __StgVariable_GetStructPtr( swarmvar->variable, swarmVarLocalIndex );

            // copy swarmvariable datum into output
            memcpy( _output->dataRaw(), dataPtr, StgVariable_SizeOfDataType(swarmvar->variable->dataTypes[0]) * swarmvar->dofCount );

            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    }

    const ParticleCoordinate* partCoord = dynamic_cast<const ParticleCoordinate*>(sample_input);
    if (partCoord)
    {
        if (((SwarmVariable*)partCoord->object())->swarm != swarmvar->swarm)
            throw std::invalid_argument( _pyfnerrorheader+"'ParticleCoordinate' input and `SwarmVariableFn` function appear to be based on different swarms." );
        // return the lambda
        return [_output, _output_sp, swarmvar](IOsptr input)->IOsptr {
            const ParticleCoordinate*  partCoord = debug_dynamic_cast<const ParticleCoordinate*>(input);
            
            // find the position into the swarmvar array we need to copy from
            void* dataPtr = __StgVariable_GetStructPtr( swarmvar->variable, partCoord->index() );

            // copy swarmvariable datum into output
            memcpy( _output->dataRaw(), dataPtr, StgVariable_SizeOfDataType(swarmvar->variable->dataTypes[0]) * swarmvar->dofCount );

            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    }

    // if neither of the above worked, try plain old global coord
    const IO_double* iodouble = dynamic_cast<const IO_double*>(sample_input);
    if ( iodouble ){
        if ( iodouble->size() != swarmvar->swarm->dim )
        {
            std::stringstream streamguy;
            streamguy << "Function input dimensionality (" << iodouble->size() << ") ";
            streamguy << "does not appear to match swarm variable dimensionality (" << swarmvar->swarm->dim << ").";
            throw std::runtime_error(_pyfnerrorheader+streamguy.str());
        }
        return [_output,_output_sp,swarmvar,this](IOsptr input)->IOsptr {
            const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(input);
            
            // get nearest particle
            size_t part_index = GeneralSwarm_GetClosestParticles( swarmvar->swarm, iodouble->data(), 1 );

            // find the position into the swarmvar array we need to copy from
            void* dataPtr = __StgVariable_GetStructPtr( swarmvar->variable, part_index );

            // copy swarmvariable datum into output
            memcpy( _output->dataRaw(), dataPtr, StgVariable_SizeOfDataType(swarmvar->variable->dataTypes[0]) * swarmvar->dofCount );
 
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    }
    throw std::invalid_argument( _pyfnerrorheader+"Unable to evaluate SwarmVariable using provided input. );");
    
}


