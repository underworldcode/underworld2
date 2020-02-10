/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Conditional_h__
#define __Underworld_Function_Conditional_h__

#include "Function.hpp"

namespace Fn {

    class Conditional: public Function
    {
        public:
            Conditional(){};
            virtual func getFunction( IOsptr sample_input );
            void insert( Function* condition, Function* value );
            virtual ~Conditional(){}
        private:
            std::vector< std::pair<Function*,Function*> > _clause;
    };

}

#endif // __Underworld_Function_Conditional_h__

