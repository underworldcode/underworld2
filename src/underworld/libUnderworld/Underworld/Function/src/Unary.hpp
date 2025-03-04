/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Unary_hpp__
#define __Underworld_Function_Unary_hpp__

#include <cmath>
#include "Function.hpp"

namespace Fn {

    template <double F( double )>
    class MathUnary: public Function
    {
        public:
            MathUnary(Function* fn): _fn(fn){};
            virtual func getFunction( IOsptr sample_input )
                {
                    // get lambda function.
                    unsigned outsize = sample_input->size();
                    func _func;
                    if (_fn) {
                        _func = _fn->getFunction( sample_input );

                        // test out to make sure it's double
                        const IO_double* doubleio;
                        doubleio = dynamic_cast<const IO_double*>(_func(sample_input));
                        if (!doubleio)
                            throw std::invalid_argument(_pyfnerrorheader+"Argument function is expected to return 'double' type object.");
                        outsize = doubleio->size();
                    }
                    // allocate memory for our output
                    std::shared_ptr<IO_double> _output_sp = std::make_shared<IO_double>(outsize,FunctionIO::Scalar);
                    IO_double* _output = _output_sp.get();
                    
                    if (_fn) {  // return this version where the user has supplied an input function
                        return [_output,_output_sp,_func](IOsptr input)->IOsptr {
                            const IO_double* io = debug_dynamic_cast<const IO_double*>( _func(input) ) ;

                            // perform function
                            for (unsigned ii=0; ii<_output->size(); ii++)
                                _output->at(ii) = F( io->at(ii) );
                                
                            return debug_dynamic_cast<const FunctionIO*>(_output);
                        };
                    } else {  // return this version where no input function has been provided.  input get's processed directly.
                    
                        return [_output,_output_sp](IOsptr input)->IOsptr {
                            const IO_double* io = debug_dynamic_cast<const IO_double*>( input ) ;

                            // perform function
                            for (unsigned ii=0; ii<_output->size(); ii++)
                                _output->at(ii) = F( io->at(ii) );
                                
                            return debug_dynamic_cast<const FunctionIO*>(_output);
                        };
                    }
                }
            virtual ~MathUnary(){};
        protected:
            Function* _fn;

    };
    
    class At: public Function
    {
        public:
            At(Function* fn, unsigned component): _fn(fn),_component(component){};
            virtual func getFunction( IOsptr sample_input );
            virtual ~At(){};
        protected:
            Function* _fn;
            unsigned _component;

    };
    

}

#endif /* __Underworld_Function_Unary_hpp__ */

