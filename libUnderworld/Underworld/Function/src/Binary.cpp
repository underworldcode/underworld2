/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <cmath>
#include "Binary.hpp"

Fn::MathBinary::MathBinary( Function* fn1, Function* fn2, MathBinaryFunc mathFunc )
{
    _fn[0] = fn1;
    _fn[1] = fn2;
    _mathFunc = mathFunc;
}


Fn::MathBinary::func  Fn::MathBinary::getFunction( IOsptr sample_input ){
    std::shared_ptr<const IO_double> doubleio[2];
    func _func[2];
    for (unsigned ii=0; ii<2; ii++) {
        // get lambda function
        _func[ii] = _fn[ii]->getFunction( sample_input );
        // test evaluation
        doubleio[ii] = std::dynamic_pointer_cast<const IO_double>(_func[ii](sample_input));
        if (!doubleio[ii])
            throw std::invalid_argument("Operand in binary function does not appear to return a 'double' type value, as required. "
                                        "Note that where the operand Function you have constructed uses Python numeric objects, those objects "
                                        "must be of 'float' type (so for example '2.' instead of '2').");

    }
    
    bool _identicalSize = ( doubleio[0]->size() == doubleio[1]->size() );

    unsigned _minGuy = doubleio[0]->size() < doubleio[1]->size() ? 0 : 1;
    unsigned _maxGuy = doubleio[0]->size() > doubleio[1]->size() ? 0 : 1;
    
    if ( !_identicalSize && (doubleio[_minGuy]->size()!=1) )
        throw std::invalid_argument("Binary function can only be constructed where the both argument functions return resuls of identical\n"
                                    "size (pointwise operation) or where the one function is scalar.");

    // allocate memory for our output
    auto _output = std::make_shared<IO_double>(doubleio[_maxGuy]->size(), doubleio[_maxGuy]->iotype());

    // setup arrays to walk results as required
    unsigned bigsize = doubleio[_maxGuy]->size();
    auto pos0 = std::make_shared< std::vector<unsigned> >(bigsize);
    auto pos1 = std::make_shared< std::vector<unsigned> >(bigsize);
    for (unsigned ii=0; ii<bigsize; ii++) {
        pos0->at(ii) = ii % doubleio[0]->size();
        pos1->at(ii) = ii % doubleio[1]->size();
    }

    switch (_mathFunc) {
        case MathBinary::add:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = io[0]->at((*pos0)[ii]) + io[1]->at((*pos1)[ii]) ;
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        case MathBinary::subtract:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = io[0]->at((*pos0)[ii]) - io[1]->at((*pos1)[ii]) ;
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        case MathBinary::multiply:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = io[0]->at((*pos0)[ii]) * io[1]->at((*pos1)[ii]) ;
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        case MathBinary::divide:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = io[0]->at((*pos0)[ii]) / io[1]->at((*pos1)[ii]) ;
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        case MathBinary::pow:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = std::pow( io[0]->at((*pos0)[ii]), io[1]->at((*pos1)[ii]) );
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        case MathBinary::fmax:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = std::fmax( io[0]->at((*pos0)[ii]), io[1]->at((*pos1)[ii]) );
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        case MathBinary::fmin:
            return [_func,_output, pos0, pos1](IOsptr input)->IOsptr {
                std::shared_ptr<const IO_double> io[2];
                io[0] = debug_dynamic_cast<const IO_double>( _func[0](input) ) ;
                io[1] = debug_dynamic_cast<const IO_double>( _func[1](input) ) ;

                for (unsigned ii=0; ii<io[0]->size(); ii++)
                    _output->at(ii) = std::fmin( io[0]->at((*pos0)[ii]), io[1]->at((*pos1)[ii]) );
                
                return debug_dynamic_cast<const FunctionIO>(_output);
            };
        default:
                throw std::invalid_argument("MathBinary function does not appear to be valid. Please contact developers.");


    }

}
