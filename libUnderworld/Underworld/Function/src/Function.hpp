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
            typedef std::shared_ptr<const FunctionIO> IOsptr;
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

    class MinMax: public Function
    {
        public:
            MinMax( Function *fn ): _fn(fn) {};
            virtual ~MinMax(){};
            virtual func getFunction( IOsptr sample_input );
            double getMin();
            double getMax();
            double getMinGlobal();
            double getMaxGlobal();
            void reset();
        protected:
            Function* _fn;
            double _minVal;
            double _maxVal;
            unsigned _size;
    };
    
}


#endif /* __Underworld_Function_Function_hpp__ */

