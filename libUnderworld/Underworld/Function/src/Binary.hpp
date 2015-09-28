/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Binary_hpp__
#define __Underworld_Function_Binary_hpp__

#include "Function.hpp"

namespace Fn {

    class MathBinary: public Function
    {
        public:
            enum MathBinaryFunc
            {
                add, subtract, multiply, divide, pow, fmax, fmin
            };
            MathBinary(Function* fn1, Function* fn2,  MathBinaryFunc mathFunc);
            virtual func getFunction( IOsptr sample_input );
            virtual ~MathBinary(){};
        protected:
            Function* _fn[2];
            MathBinaryFunc _mathFunc;

    };

}

#endif /* __Underworld_Function_Binary_hpp__ */

