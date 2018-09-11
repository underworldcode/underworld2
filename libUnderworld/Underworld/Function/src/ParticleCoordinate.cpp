/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <stdexcept>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}
#include "ParticleCoordinate.hpp"
#include "DiscreteCoordinate.hpp"


ParticleCoordinate::ParticleCoordinate( void* particleCoordVariable )
   : DiscreteCoordinate(_Check_GetDimSize(particleCoordVariable), particleCoordVariable)
{};

unsigned ParticleCoordinate::_Check_GetDimSize(void* particleCoordVariable){
   if(!Stg_Class_IsInstance( particleCoordVariable, SwarmVariable_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'SwarmVariable' type.");
    
    return ((SwarmVariable*) particleCoordVariable)->dim;
}

double &
ParticleCoordinate::at( size_t idx ){
    throw std::runtime_error("The ParticleCoordinate 'value' should never be set explicitly.");
}

const double
ParticleCoordinate::at( size_t idx) const{
    return data()[idx];
}

double*
ParticleCoordinate::data(){
    throw std::runtime_error("Non const access to underlying data is not permitted for 'ParticleCoordinate' objects.");
    return NULL;
};

const double*
ParticleCoordinate::data() const {
    return StgVariable_GetPtrDouble( ((SwarmVariable*) _object)->variable, _index );
};

