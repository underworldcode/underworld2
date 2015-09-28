/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Arithmetic_hpp__
#define __Underworld_Function_Arithmetic_hpp__

#include <functional>
#include <stdexcept>

#include "FunctionIO.hpp"
#include "Function.hpp"

namespace Fn {

    class Arithmetic: public Function
    {
        public:
            Arithmetic( Function *fn1, Function *fn2 );
            virtual func getFunction( IOsptr sample_input ) = 0;
            virtual ~Arithmetic(){};
        protected:
            Function* _fn[2];
    };

    class Add: public Arithmetic
    {
        public:
            Add( Function *fn1, Function *fn2 ) : Arithmetic( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Add(){};
    };

    class Multiply: public Arithmetic
    {
        public:
            Multiply( Function *fn1, Function *fn2 ) : Arithmetic( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Multiply(){};
    };

    class Divide: public Arithmetic
    {
        public:
            Divide( Function *fn1, Function *fn2 ) : Arithmetic( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Divide(){};
    };

    class Dot: public Arithmetic
    {
        public:
            Dot( Function *fn1, Function *fn2 ) : Arithmetic( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Dot(){};
    };

}

#endif /* __Underworld_Function_Arithmetic_hpp__ */

