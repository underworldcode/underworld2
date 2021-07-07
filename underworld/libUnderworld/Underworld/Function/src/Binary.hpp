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

#include <functional>
#include <stdexcept>

#include "FunctionIO.hpp"
#include "Function.hpp"

namespace Fn {

    class Binary: public Function
    {
        public:
            Binary( Function *fn1, Function *fn2 );
            virtual func getFunction( IOsptr sample_input ) = 0;
            virtual ~Binary(){};
        protected:
            Function* _fn[2];
            void initGetFunction( IOsptr sample_input, const IO_double* doubleio[2], func (&_func)[2] );
    };

    class Add: public Binary
    {
        public:
            Add( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Add(){};
    };

    class Subtract: public Binary
    {
        public:
            Subtract( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Subtract(){};
    };

    class Multiply: public Binary
    {
        public:
            Multiply( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Multiply(){};
    };

    class Divide: public Binary
    {
        public:
            Divide( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Divide(){};
    };

    class Dot: public Binary
    {
        public:
            Dot( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Dot(){};
    };

    class Pow: public Binary
    {
        public:
            Pow( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Pow(){};
    };

    class Min: public Binary
    {
        public:
            Min( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Min(){};
    };

    class Max: public Binary
    {
        public:
            Max( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Max(){};
    };

    class Atan2: public Binary
    {
        public:
            Atan2( Function *fn1, Function *fn2 ) : Binary( fn1, fn2) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~Atan2(){};
    };
 
}

#endif /* __Underworld_Function_Binary_hpp__ */

