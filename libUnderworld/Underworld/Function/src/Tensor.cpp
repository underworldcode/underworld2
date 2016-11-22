/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <sstream> 
#include <cmath>
#include <string>
#include <sstream>
#include <algorithm>

#include <mpi.h>
#include <petsc.h>
extern "C" {
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
}

#include "Tensor.hpp"

Fn::TensorFunc::func Fn::TensorFunc::getFunction( IOsptr sample_input )
{
    func _func;
    if (_fn) {
        _func = _fn->getFunction( sample_input );
    } else { // if no _fn, create lambda which simply returns input
        _func = [](IOsptr input)->IOsptr { return input; };
    }
    // test out to make sure it's double.
    const IO_double* funcio;
    funcio = dynamic_cast<const IO_double*>(_func(sample_input));
    if (!funcio)
        throw std::invalid_argument("TensorFunc expects input function to return a 'double' type object.");
    
    FunctionIO::IOType iotype;
    unsigned dim;
    if ( funcio->iotype() == FunctionIO::Tensor )
    {
        iotype = FunctionIO::Tensor;
        // lets try take a sqrt
        dim = (unsigned) std::sqrt(funcio->size());
        if (!( dim*dim == funcio->size() && (dim==2 || dim==3) ))
            throw std::invalid_argument("TensorFunc Tensor input does not appear to be valid.");
    }
    else if ( funcio->iotype() == FunctionIO::SymmetricTensor )
    {
        iotype = FunctionIO::SymmetricTensor;
        if ( ! ( funcio->size()==3 || funcio->size()==6 ))
            throw std::invalid_argument("TensorFunc SymmetricTensor input does not appear to be valid.");
        dim = ( funcio->size()==3 ) ? 2 : 3;
    }
    else
        throw std::invalid_argument("TensorFunc expects input function to return a 'Tensor' or 'SymmetricTensor' object.");
    
    if ( _partFunc == get_symmetric ) {
        if (iotype!=FunctionIO::Tensor)
            throw std::invalid_argument("TensorFunc expects Tensor input for 'get_symmetric' function.");
        unsigned outsize = (dim = 2) ? 3 : 6;
        std::shared_ptr<IO_double> _output_sp = std::make_shared<IO_double>(outsize,FunctionIO::SymmetricTensor);
        IO_double* _output = _output_sp.get();
        return [_output,_output_sp,_func,dim](IOsptr input)->IOsptr {
            const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(_func(input));

            TensorArray_GetSymmetricPart( iodouble->data(), dim, _output->data() );
            
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    } else if ( _partFunc == get_antisymmetric ) {
        if (iotype!=FunctionIO::Tensor)
            throw std::invalid_argument("TensorFunc expects Tensor input for 'get_antisymmetric' function.");
        std::shared_ptr<IO_double> _output_sp = std::make_shared<IO_double>(funcio->size(),FunctionIO::Tensor);
        IO_double* _output = _output_sp.get();
        return [_output,_output_sp,_func,dim](IOsptr input)->IOsptr {
            const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(_func(input));

            TensorArray_GetAntisymmetricPart( iodouble->data(), dim, _output->data() );
            
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    } else if (_partFunc == second_invariant) {
        std::shared_ptr<IO_double> _output_sp = std::make_shared<IO_double>(1,FunctionIO::Scalar);
        IO_double* _output = _output_sp.get();
        if (iotype==FunctionIO::Tensor)
            return [_output,_output_sp,_func, dim](IOsptr input)->IOsptr {
                const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(_func(input));

                _output->at() = TensorArray_2ndInvariant( iodouble->data(), dim );
                
                return debug_dynamic_cast<const FunctionIO*>(_output);
            };
        else if (iotype==FunctionIO::SymmetricTensor)
            return [_output,_output_sp, _func, dim](IOsptr input)->IOsptr {
                const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(_func(input));

                _output->at() = SymmetricTensor_2ndInvariant( iodouble->data(), dim );
                
                return debug_dynamic_cast<const FunctionIO*>(_output);
            };
    } else if (_partFunc == get_deviatoric) {
        if (iotype!=FunctionIO::SymmetricTensor)
            throw std::invalid_argument("TensorFunc expects SymmetricTensor input for 'get_deviatoric' function.");
        std::shared_ptr<IO_double> _output_sp = std::make_shared<IO_double>(funcio->size(),FunctionIO::SymmetricTensor);
        IO_double* _output = _output_sp.get();
        return [_output, _output_sp, _func, dim](IOsptr input)->IOsptr {
            const IO_double* iodouble = debug_dynamic_cast<const IO_double*>(_func(input));

            double meanStress;
            const double* dubdata = iodouble->data();
            SymmetricTensor_GetTrace( dubdata, dim, &meanStress );
            meanStress = meanStress / dim ;

            std::copy( dubdata, dubdata + iodouble->size(), _output->data() ) ;

            _output->at(0) -= meanStress;
            _output->at(1) -= meanStress;
            if( dim == 3 )
                _output->at(2) -= meanStress;
            
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    }
    
    // something amiss if we get here
    throw std::invalid_argument("TensorFunc input does not appear to be valid..");
}
