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
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
}
#include "DiscreteCoordinate.hpp"
#include "MeshCoordinate.hpp"

MeshCoordinate::MeshCoordinate( void* mesh )
   : DiscreteCoordinate(_Check_GetDimSize(mesh), mesh)
{
    if(!Stg_Class_IsInstance( mesh, Mesh_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'Mesh' type.");

};

unsigned MeshCoordinate::_Check_GetDimSize(void* mesh){
   if(!Stg_Class_IsInstance( mesh, Mesh_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'Mesh' type.");
    
    return Mesh_GetDimSize( mesh );
}

double &
MeshCoordinate::at( size_t idx ){
    throw std::runtime_error("The MeshCoordinate 'value' should never be set explicitly.");
}

const double
MeshCoordinate::at( size_t idx) const{
    return data()[idx];
}

double*
MeshCoordinate::data(){
    throw std::runtime_error("Non const access to underlying data is not permitted for 'MeshCoordinate' objects.");
    return NULL;
};

const double*
MeshCoordinate::data() const {
    return 	Mesh_GetVertex( _object, _index );
};

