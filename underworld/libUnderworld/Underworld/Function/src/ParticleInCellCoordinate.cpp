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
#include "DiscreteCoordinate.hpp"
#include "ParticleInCellCoordinate.hpp"

ParticleInCellCoordinate::ParticleInCellCoordinate( void* swarmvariable )
    : DiscreteCoordinate(_Check_GetDimSize(swarmvariable), swarmvariable), _particle_cellId(0)
{
    if(!Stg_Class_IsInstance( swarmvariable, SwarmVariable_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'SwarmVariable' type.");
    
    if ( ((SwarmVariable*)swarmvariable)->dim != ((SwarmVariable*)swarmvariable)->dofCount )
        throw std::invalid_argument("Provided swarm variable does not appear to have consistent data ( dim != dofcount ).\n  \
                                     Variable should provide particle positional data.");

};

unsigned ParticleInCellCoordinate::_Check_GetDimSize(void* swarmvariable){
   if(!Stg_Class_IsInstance( swarmvariable, SwarmVariable_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'SwarmVariable' type.");
    return ((SwarmVariable*)swarmvariable)->dim;
}

double &
ParticleInCellCoordinate::at( size_t idx ){
    throw std::runtime_error("The ParticleInCellCoordinate 'value' should never be set explicitly.");
}

const double
ParticleInCellCoordinate::at( size_t idx ) const{
    return data()[idx];
}

double*
ParticleInCellCoordinate::data(){
    throw std::runtime_error("Non const access to underlying data is not permitted for 'ParticleInCellCoordinate' objects.");
    return NULL;
};

const double*
ParticleInCellCoordinate::data() const{
    unsigned cellId = CellLayout_MapElementIdToCellId( ((SwarmVariable*)_object)->swarm->cellLayout, _index );
    unsigned localId = Swarm_ParticleCellIDtoLocalID( ((SwarmVariable*)_object)->swarm, cellId, _particle_cellId );
    // this will copy the coord into _vector.  not stricly necessary, but safer
    _SwarmVariable_ValueAtDouble( _object, localId, _vector.data() ) ;
    return _vector.data();
};

