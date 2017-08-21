/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Function_hpp__
#define __Underworld_Function_Function_hpp__

#include <functional>
#include <stdexcept>

#include "FunctionIO.hpp"

namespace Fn {

    class Function
    {
        public:
            typedef const FunctionIO* IOsptr;
            typedef std::function<IOsptr( const IOsptr &input )> func;
            virtual func getFunction( IOsptr input )=0;
            virtual ~Function(){};
        protected:
            Function(){};
    };

    class Input: public Function
    {
        public:
            Input(){};
            virtual ~Input(){};
            virtual func getFunction( IOsptr sample_input );
        protected:
    };

    class SafeMaths: public Function
    {
        public:
            SafeMaths( Function *fn ): _fn(fn) {};
            virtual ~SafeMaths(){};
            virtual func getFunction( IOsptr sample_input );
        protected:
            Function* _fn;
    };

    class CustomException: public Function
    {
        public:
            CustomException( Function *fn_input, Function *fn_condition, Function *fn_print=NULL ):
                _fn_input(fn_input), _fn_condition(fn_condition), _fn_print(fn_print) {};
            virtual ~CustomException(){};
            virtual func getFunction( IOsptr sample_input );
        protected:
            Function* _fn_input;
            Function* _fn_condition;
            Function* _fn_print;
    };
    
}


#endif /* __Underworld_Function_Function_hpp__ */

