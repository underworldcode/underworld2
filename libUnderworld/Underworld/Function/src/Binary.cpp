/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <cmath>
#include <cfloat>

#include "FunctionIO.hpp"
#include "Function.hpp"
#include "Binary.hpp"

Fn::Binary::Binary( Function *fn1, Function *fn2 )
{ _fn[0] = fn1; _fn[1]=fn2;};

void Fn::Binary::initGetFunction( IOsptr sample_input, const IO_double* doubleio[2], func (&_func)[2] )
{
    for (unsigned ii=0; ii<2; ii++) {
        // get lambda function
        _func[ii] = _fn[ii]->getFunction( sample_input );
        // test evaluation
        doubleio[ii] = dynamic_cast<const IO_double*>(_func[ii](sample_input));
        if (!doubleio[ii])
            throw std::invalid_argument("Operand in binary function does not appear to return a 'double' type value, as required. "
                                        "Note that where the operand Function you have constructed uses Python numeric objects, those objects "
                                        "must be of 'float' type (so for example '2.' instead of '2').");
    }
}

Fn::Add::func Fn::Add::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    if (doubleio[0]->size() != doubleio[1]->size())
        throw std::invalid_argument("Added functions must return identical sized objects.");

    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[0]->size(), doubleio[0]->iotype());
    // get the raw pointer from the shared pointer
    auto _output    = _output_sp.get();

    // create and return the lambda
    unsigned size = doubleio[1]->size();
    return [_func, _output, _output_sp, size](IOsptr input)->IOsptr {
        const IO_double* io1 = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
        const IO_double* io2 = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

        // perform sum
        for (unsigned ii=0; ii<size; ii++)
            _output->at(ii) = io1->at(ii) + io2->at(ii);
            
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
}

Fn::Subtract::func Fn::Subtract::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    if (doubleio[0]->size() != doubleio[1]->size())
        throw std::invalid_argument("Subtracted functions must return identical sized objects.");

    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[0]->size(), doubleio[0]->iotype());
    auto _output    = _output_sp.get();

    // create and return the lambda
    unsigned size = doubleio[1]->size();
    return [_func, _output, _output_sp, size](IOsptr input)->IOsptr {
        const IO_double* io1 = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
        const IO_double* io2 = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

        // perform sum
        for (unsigned ii=0; ii<size; ii++)
            _output->at(ii) = io1->at(ii) - io2->at(ii);
            
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
}

Fn::Multiply::func  Fn::Multiply::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    unsigned _minGuy = doubleio[0]->size() < doubleio[1]->size() ? 0 : 1;
    unsigned _maxGuy = doubleio[0]->size() > doubleio[1]->size() ? 0 : 1;
    bool _identicalSize = ( _minGuy == _maxGuy );
    
    if ( !_identicalSize && (doubleio[_minGuy]->size()!=1) )
        throw std::invalid_argument("Function multiplication is only possible between functions of identical " \
                                    "size (pointwise) or where one function is scalar.");
    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[_maxGuy]->size(), doubleio[_maxGuy]->iotype());
    auto _output    = _output_sp.get();

    // create and return the lambda
    unsigned size = doubleio[_maxGuy]->size();
    if ( _identicalSize ) {
        return [_output, _output_sp, _func, size](IOsptr input)->IOsptr {
            const IO_double* io[2];
            io[0] = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

            // perform product
            for (unsigned ii=0; ii<size; ii++)
                _output->at(ii) = io[0]->at(ii) * io[1]->at(ii);
            
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    } else {
        return [_output, _output_sp, _func, _minGuy, _maxGuy, size](IOsptr input)->IOsptr {
            const IO_double* io[2];
            io[0] = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

            // perform product
            double ioscalar = io[_minGuy]->at();
            for (unsigned ii=0; ii<size; ii++)
                _output->at(ii) = ioscalar * io[_maxGuy]->at(ii);
            
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    }
}


Fn::Divide::func  Fn::Divide::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    unsigned _minGuy = doubleio[0]->size() < doubleio[1]->size() ? 0 : 1;
    unsigned _maxGuy = doubleio[0]->size() > doubleio[1]->size() ? 0 : 1;
    bool _identicalSize = _minGuy == _maxGuy;
    
    if ( !_identicalSize && (doubleio[1]->size()!=1) )
        throw std::invalid_argument("Function division is only possible between functions of identical " \
                                    "size (pointwise) or where denominator function returns scalars.");
    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[_maxGuy]->size(), doubleio[_maxGuy]->iotype());
    auto _output    = _output_sp.get();
    
    // create and return the lambda
    unsigned size = doubleio[_maxGuy]->size();
    if (_identicalSize)  // first lambda function is for pointwise
        return [_output,_output_sp,_func,size](IOsptr input)->IOsptr {
            const IO_double* io[2];
            io[0] = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

            // perform division
            for (unsigned ii=0; ii<size; ii++){
                double result = io[0]->at(ii) / io[1]->at(ii);
                _output->at(ii) = result;
            }
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
    else                  // this one is for scalar denominator
        return [_output,_output_sp,_func,size](IOsptr input)->IOsptr {
            const IO_double* io[2];
            io[0] = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

            // perform division
            double denominator = io[1]->at(0);
            for (unsigned ii=0; ii<size; ii++){
                double result = io[0]->at(ii) / denominator;
                _output->at(ii) = result;
            }
            return debug_dynamic_cast<const FunctionIO*>(_output);
        };
}

Fn::Dot::func Fn::Dot::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    if ( doubleio[0]->size() != doubleio[1]->size() )
        std::invalid_argument("Function dot products can only be constructed between functions of identical\n"
                              "size.");
    // setup our output
    auto _output_sp = std::shared_ptr<IO_double>(new IO_double(1, FunctionIO::Scalar ));
    auto _output    = _output_sp.get();

    // create and return the lambda
    unsigned size = doubleio[0]->size();
    return [_output,_output_sp,_func,size](IOsptr input)->IOsptr {
        const IO_double* io[2];
        io[0] = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
        io[1] = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

        // perform product
        double sum = 0.;
        for (unsigned ii=0; ii<size; ii++)
            sum += io[0]->at(ii) * io[1]->at(ii);

        _output->at(0) = sum;
        
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
}

Fn::Pow::func Fn::Pow::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    if (doubleio[1]->size() != 1 )
        throw std::invalid_argument("Power function exponent must be a scalar.");

    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[0]->size(), doubleio[0]->iotype());
    auto _output    = _output_sp.get();

    // create and return the lambda
    unsigned size = doubleio[0]->size();
    return [_func, _output, _output_sp,size](IOsptr input)->IOsptr {
        const IO_double* io1 = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
        const IO_double* io2 = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

        // perform sum
        double power = io2->at();
        for (unsigned ii=0; ii<size; ii++)
            _output->at(ii) = std::pow( io1->at(ii), power );
            
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
}


Fn::Min::func Fn::Min::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    if ( (doubleio[0]->size()!=1) || (doubleio[1]->size()!=1) )
        throw std::invalid_argument("Min function requires scalar inputs.");

    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[0]->size(), doubleio[0]->iotype());
    auto _output    = _output_sp.get();

    // create and return the lambda
    return [_func, _output, _output_sp](IOsptr input)->IOsptr {
        const IO_double* io1 = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
        const IO_double* io2 = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

        _output->at() = std::fmin( io1->at(), io2->at() );
        
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
}


Fn::Max::func Fn::Max::getFunction( IOsptr sample_input )
{
    const IO_double* doubleio[2];
    func _func[2];
    initGetFunction( sample_input, doubleio, _func );
    
    if ( (doubleio[0]->size()!=1) || (doubleio[1]->size()!=1) )
        throw std::invalid_argument("Max function requires scalar inputs.");

    // allocate memory for our output
    auto _output_sp = std::make_shared<IO_double>(doubleio[0]->size(), doubleio[0]->iotype());
    auto _output    = _output_sp.get();

    // create and return the lambda
    return [_func, _output, _output_sp](IOsptr input)->IOsptr {
        const IO_double* io1 = debug_dynamic_cast<const IO_double*>( _func[0](input) ) ;
        const IO_double* io2 = debug_dynamic_cast<const IO_double*>( _func[1](input) ) ;

        _output->at() = std::fmax( io1->at(), io2->at() );
        
        return debug_dynamic_cast<const FunctionIO*>(_output);
    };
}


