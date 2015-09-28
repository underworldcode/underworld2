/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <string>
#include <sstream>
#include <algorithm>

#include "Unary.hpp"

Fn::At::func Fn::At::getFunction( IOsptr sample_input )
{
    // get lambda function.
    func _func;
    if (_fn) {
        _func = _fn->getFunction( sample_input );
    } else { // if no _fn, create lambda which simply returns input
        _func = [](IOsptr input)->IOsptr { return input; };
    }
    // test out to make sure it's double.. we should be able to relax this later
    std::shared_ptr<const IO_double> funcio;
    funcio = std::dynamic_pointer_cast<const IO_double>(_func(sample_input));
    if (!funcio)
        throw std::invalid_argument("At function expects functions to return a 'double' type object.");

    if ( funcio->size() <= _component ){
        std::stringstream ss;
        ss << "Trying to extract component " << _component << " from from object with size " << funcio->size() << ".\n";
        ss << "Index must be in [0," << funcio->size()-1 << "].";
        throw std::invalid_argument( ss.str() );
    }
    // allocate memory for our output
    std::shared_ptr<IO_double> _output = std::make_shared<IO_double>(1,FunctionIO::Scalar);
    unsigned compforLambda = _component;
    unsigned iodatasize = funcio->_dataSize;
    std::size_t startOffset = iodatasize*(compforLambda    );
    std::size_t   endOffset = iodatasize*(compforLambda + 1);

    return [_output,_func,startOffset,endOffset](IOsptr input)->IOsptr {
        std::shared_ptr<const FunctionIO> io = _func(input);

        // extract value
        std::copy( (char*)io->dataRaw() + startOffset,
                   (char*)io->dataRaw() +   endOffset,
                   (char*)_output->dataRaw() );
        
        return _output;
    };
}
