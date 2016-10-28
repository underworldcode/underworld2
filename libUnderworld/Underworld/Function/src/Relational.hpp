/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Relational_hpp__
#define __Underworld_Function_Relational_hpp__

#include <cmath>
#include <sstream>
#include <functional>

#include "Function.hpp"

namespace Fn {

    /* add this because it isn't include in the stl */
    template <class T> struct logical_xor {
      bool operator() (const T& x, const T& y) const {return x!=y;}
      typedef T first_argument_type;
      typedef T second_argument_type;
      typedef bool result_type;
    };


    template <typename F>
    class MathRelational: public Function
    {
        public:
            MathRelational(Function* fn1, Function* fn2)
            {
                _fn[0] = fn1;
                _fn[1] = fn2;
            };
            virtual func getFunction( IOsptr sample_input )
                {
                    // get lambda function.
                    const FunctionIO*  ioguy[2];
                    func _func[2];
                    for (unsigned ii=0; ii<2; ii++) {
                        // get lambda function
                        _func[ii] = _fn[ii]->getFunction( sample_input );
                        // test evaluation
                        ioguy[ii] = dynamic_cast<const FunctionIO *>(_func[ii](sample_input));
                        if (!ioguy[ii])
                            throw std::invalid_argument("Operand in relational function does not appear to return a supported type.");
                    }
                    
                    if( ioguy[0]->size() != ioguy[1]->size() ){
                        std::stringstream ss;
                        ss << "Inputs to relational function should return objects of identical size.\n";
                        ss << "Function one returns object of size " << ioguy[0]->size() << ".\n";
                        ss << "Function two returns object of size " << ioguy[1]->size() << ".\n";
                        throw std::invalid_argument(ss.str());
                    }
                   
                    // allocate memory for our output
                    std::shared_ptr<IO_bool> _output_sp = std::make_shared<IO_bool>(1,FunctionIO::Scalar);
                    IO_bool* _output = _output_sp.get();
                    
                    // create functional object
                    auto relationalfunc = F();
                    unsigned count = ioguy[0]->size();
                    return [_output,_output_sp,_func,relationalfunc,count](IOsptr input)->IOsptr {
                         const FunctionIO * io[2];
                        io[0] = debug_dynamic_cast< const FunctionIO *>( _func[0](input) ) ;
                        io[1] = debug_dynamic_cast< const FunctionIO *>( _func[1](input) ) ;

                        // perform function.. note that we are implementing AND behaviour
                        // for vector objects.  this will possibly be generalised in the future.
                        _output->at() = true;
                        for (unsigned ii=0; ii<count; ii++)
                            if ( relationalfunc( io[0]->at<double>(ii), io[1]->at<double>(ii) ) )
                                continue;
                            else
                            {
                                _output->at() = false;
                                break;
                            }
                                
                            
                        return debug_dynamic_cast<const FunctionIO*>(_output);
                    };
                }
            virtual ~MathRelational(){};
        protected:
            Function* _fn[2];

    };
    
}

#endif /* __Underworld_Function_Relational_hpp__ */

