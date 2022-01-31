/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <cmath>
#include <sstream>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
}

#include "Shape.hpp"

Fn::Polygon::Polygon( Function* fn, double* IN_ARRAY2, int DIM1, int DIM2 )
{
    _fn = fn;
    if(DIM1 < 3)
    {
        std::stringstream ss;
        ss << "Vertex array must provide at least 3 vertices.\n";
        ss << "Array appears to provide only " << DIM1 <<" vertices.";
        throw std::invalid_argument(_pyfnerrorheader+ss.str());
    }
    if(DIM2 != 3)
    {
        std::stringstream ss;
        ss << "Vertex array of 3-Vectors is expected.\n";
        ss << "Array appears to provide vectors of dimension " << DIM2 <<".";
        throw std::invalid_argument(_pyfnerrorheader+ss.str());
    }
    
    XYZ centre   = { 0., 0., 0. };
    XYZ startXYZ = { 0., 0., 0. };
    XYZ endXYZ   = { 0., 0., 0. };
    
    _stgshape = (void*) PolygonShape_New( (Name) NULL,
                                   (Dimension_Index)2,
                                               centre, 0., 0., 0.,
                                (Coord_List)IN_ARRAY2,
                                          (Index)DIM1, startXYZ, endXYZ );
}


Fn::Polygon::~Polygon()
{
    Stg_Class_Delete(_stgshape);
    _stgshape = NULL;
}


Fn::Function::func  Fn::Polygon::getFunction( IOsptr sample_input ){
    // get lambda function.
    func _func;
    if (_fn) {
        _func = _fn->getFunction( sample_input );
    } else { // if no _fn, create lambda which simply returns input
        _func = [](IOsptr input)->IOsptr { return input; };
    }
    // test out to make sure it's double.. we should be able to relax this later
    const IO_double* funcio = dynamic_cast<const IO_double*>(_func(sample_input));
    if (!funcio)
        throw std::invalid_argument(_pyfnerrorheader+"Polygon shape function expects a 'double' type object as input.");

    unsigned size = funcio->size();
    if( !( (size==2) || (size==3) ) )
    {
        std::stringstream ss;
        ss << "Polygon shape expects input to be be 2 or 3 dimensional vector.\n";
        ss << "Provided input dimensionality is " << size <<".";
        throw std::invalid_argument(_pyfnerrorheader+ss.str());
    }
    // allocate memory for our output
    std::shared_ptr<IO_bool> _output_sp = std::make_shared<IO_bool>(1,FunctionIO::Scalar);
    IO_bool* _output = _output_sp.get();

    PolygonShape* polyshape = (PolygonShape*)_stgshape;
    
    return [_output, _output_sp, polyshape, _func](IOsptr input)->IOsptr {
        const FunctionIO* io = _func(input);

        _output->at() = (bool)_PolygonShape_IsCoordInside( polyshape, (double*)io->dataRaw() );

        return debug_dynamic_cast<const FunctionIO*>(_output);
    };

}
