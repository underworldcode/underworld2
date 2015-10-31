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
#include "Arithmetic.hpp"

Fn::Arithmetic::Arithmetic( Function *fn1, Function *fn2 )
{ _fn[0] = fn1; _fn[1]=fn2;};

Fn::Add::func Fn::Add::getFunction( IOsptr sample_input ){
    
    std::shared_ptr<const IO_double> doubleio[2];
    func _func[2];
    for (unsigned ii=0; ii<2; ii++) {
        // get lambda function
        _func[ii] = _fn[ii]->getFunction( sample_input );
        // test evaluation
        doubleio[ii] = std::dynamic_pointer_cast<const IO_double>(_func[ii](sample_input));
        if (!doubleio[ii])
            throw std::invalid_argument("Operand in arithmetic function (Add) does not appear to return a 'double' type value, as required.");
    }
    
    if (doubleio[0]->size() != doubleio[1]->size())
        throw std::invalid_argument("Added functions must return identical sized objects.");

    // allocate memory for our output
    auto _output = std::make_shared<IO_double>(doubleio[0]->size(), doubleio[0]->iotype());

    // create and return the lambda
    return [_func, _output](IOsptr input)->IOsptr {
        std::shared_ptr<const IO_double> io1 = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
        std::shared_ptr<const IO_double> io2 = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

        // perform sum
        for (unsigned ii=0; ii<io1->size(); ii++)
            _output->at(ii) = io1->at(ii) + io2->at(ii);
            
        return debug_dynamic_cast<const FunctionIO>(_output);
    };
}

Fn::Multiply::func  Fn::Multiply::getFunction( IOsptr sample_input ){

    std::shared_ptr<const IO_double> doubleio[2];
    func _func[2];
    for (unsigned ii=0; ii<2; ii++) {
        // get lambda function
        _func[ii] = _fn[ii]->getFunction( sample_input );
        // test evaluation
        doubleio[ii] = std::dynamic_pointer_cast<const IO_double>(_func[ii](sample_input));
        if (!doubleio[ii])
            throw std::invalid_argument("Operand in arithmetic function (Multiply) does not appear to return a 'double' type value, as required.");
    }
    
    unsigned _minGuy = doubleio[0]->size() < doubleio[1]->size() ? 0 : 1;
    unsigned _maxGuy = doubleio[0]->size() > doubleio[1]->size() ? 0 : 1;
    bool _identicalSize = ( _minGuy == _maxGuy );
    
    if ( !_identicalSize && (doubleio[_minGuy]->size()!=1) )
        throw std::invalid_argument("Function products can only be constructed between functions of identical\n"
                                    "size (pointwise) or where one function is scalar.");
    // allocate memory for our output
    auto _output = std::make_shared<IO_double>(doubleio[_maxGuy]->size(), doubleio[_maxGuy]->iotype());

    // create and return the lambda
    if ( _identicalSize ) {
        return [_output,_func](IOsptr input)->IOsptr {
            std::shared_ptr<const IO_double> io[2];
            io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

            // perform product
            for (unsigned ii=0; ii<io[0]->size(); ii++)
                _output->at(ii) = io[0]->at(ii) * io[1]->at(ii);
            
            return debug_dynamic_cast<const FunctionIO>(_output);
        };
    } else {
        return [_output,_func, _minGuy, _maxGuy](IOsptr input)->IOsptr {
            std::shared_ptr<const IO_double> io[2];
            io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

            // perform product
            double ioscalar = io[_minGuy]->at();
            for (unsigned ii=0; ii<io[_maxGuy]->size(); ii++)
                _output->at(ii) = ioscalar * io[_maxGuy]->at(ii);
            
            return debug_dynamic_cast<const FunctionIO>(_output);
        };
    }

}


Fn::Divide::func  Fn::Divide::getFunction( IOsptr sample_input ){

    std::shared_ptr<const IO_double> doubleio[2];
    func _func[2];
    for (unsigned ii=0; ii<2; ii++) {
        // get lambda function
        _func[ii] = _fn[ii]->getFunction( sample_input );
        // test evaluation
        doubleio[ii] = std::dynamic_pointer_cast<const IO_double>(_func[ii](sample_input));
        if (!doubleio[ii])
            throw std::invalid_argument("Operand in arithmetic function (Divide) does not appear to return a 'double' type value, as required.");
    }
    
    unsigned _minGuy = doubleio[0]->size() < doubleio[1]->size() ? 0 : 1;
    unsigned _maxGuy = doubleio[0]->size() > doubleio[1]->size() ? 0 : 1;
    bool _identicalSize = _minGuy == _maxGuy;
    
    if ( !_identicalSize && (doubleio[1]->size()!=1) )
        throw std::invalid_argument("Function division can only be constructed between functions of identical\n"
                                    "size (pointwise) or where denominator function returns scalars.");
    // allocate memory for our output
    auto _output = std::make_shared<IO_double>(doubleio[_maxGuy]->size(), doubleio[_maxGuy]->iotype());
    
    // create and return the lambda
    if (_identicalSize)  // first lambda function is for pointwise
        return [_output,_func](IOsptr input)->IOsptr {
            std::shared_ptr<const IO_double> io[2];
            io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

            // perform division
            for (unsigned ii=0; ii<io[0]->size(); ii++){
                double result = io[0]->at(ii) / io[1]->at(ii);
//                if (std::isinf(result))
//                    throw std::runtime_error("Divide by zero encountered while evaluating Divide function.");
                _output->at(ii) = result;
            }
            return debug_dynamic_cast<const FunctionIO>(_output);
        };
    else                  // this one is for scalar denominator
        return [_output,_func](IOsptr input)->IOsptr {
            std::shared_ptr<const IO_double> io[2];
            io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
            io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

            // perform division
            double denominator = io[1]->at(0);
            for (unsigned ii=0; ii<io[0]->size(); ii++){
                double result = io[0]->at(ii) / denominator;
//                if (std::isinf(result))
//                    throw std::runtime_error("Divide by zero encountered while evaluating Divide function.");
                _output->at(ii) = result;
            }
            return debug_dynamic_cast<const FunctionIO>(_output);
        };

}

Fn::Dot::func Fn::Dot::getFunction( IOsptr sample_input ){

    std::shared_ptr<const IO_double> doubleio[2];
    func _func[2];
    for (unsigned ii=0; ii<2; ii++) {
        // get lambda function
        _func[ii] = _fn[ii]->getFunction( sample_input );
        // test evaluation
        doubleio[ii] = std::dynamic_pointer_cast<const IO_double>(_func[ii](sample_input));
        if (!doubleio[ii])
            throw std::invalid_argument("Operand in arithmetic function (Dot) does not appear to return a 'double' type value, as required.");
    }
    
    if ( doubleio[0]->size() != doubleio[1]->size() )
        std::invalid_argument("Function dot products can only be constructed between functions of identical\n"
                              "size.");
    // setup our output
    auto _output = std::shared_ptr<IO_double>(new IO_double(1, FunctionIO::Scalar ));

    // create and return the lambda
    return [_output,_func](IOsptr input)->IOsptr {
        std::shared_ptr<const IO_double> io[2];
        io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
        io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

        // perform product
        double sum = 0.;
        for (unsigned ii=0; ii<io[0]->size(); ii++)
            sum += io[0]->at(ii) * io[1]->at(ii);

        _output->at(0) = sum;
        
        return debug_dynamic_cast<const FunctionIO>(_output);
    };

}



