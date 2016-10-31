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
#include "FEMCoordinate.hpp"

FEMCoordinate::FEMCoordinate( void* mesh, std::shared_ptr<IO_double> localCoord )
   : IO_double( _Check_GetDimSize(mesh), FunctionIO::Vector), _localCoord_sp(localCoord), _localCoord(NULL), _mesh(mesh), _index(0), _valueCalculated(false)
{
    if( _localCoord_sp->iotype() != FunctionIO::Vector)
        throw std::invalid_argument("Provided local coordinate must be of 'Vector' type.");
    
    if(!Stg_Class_IsInstance( mesh, FeMesh_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'FeMesh' type.");
    
    _localCoord = _localCoord_sp.get();

};

unsigned FEMCoordinate::_Check_GetDimSize(void* mesh){
    if(!Stg_Class_IsInstance( mesh, FeMesh_Type ))
        throw std::invalid_argument("Provided 'object' does not appear to be of 'FeMesh' type.");
        
    return Mesh_GetDimSize(mesh);
}

double&
FEMCoordinate::at(size_t idx)
{
    throw std::runtime_error("The FEMCoordinate 'value' should never be set explicitly.");
}

const double
FEMCoordinate::at(size_t idx) const
{
    if (!_valueCalculated)
        _calculate_value();
    
    return IO_double::at(idx);
    
}

double*
FEMCoordinate::data()
{
    throw std::runtime_error("Non const access to underlying data is not permitted for 'FEMCoordinate' objects.");
    return NULL;
}

const double*
FEMCoordinate::data() const
{
    if (!_valueCalculated)
        _calculate_value();

    return IO_double::data();
}

void
FEMCoordinate::_calculate_value() const
{

    FeMesh_CoordLocalToGlobal( _mesh,
        _index,
        const_cast<const IO_double*>(_localCoord)->data(),
        _vector.data() );
    _valueCalculated = false;
}

