/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_FeVariableFn_h__
#define __Underworld_Function_FeVariableFn_h__

#include "Function.hpp"

namespace Fn {

    class FeVariableFn: public Function
    {
        public:
            FeVariableFn( void* fevariable );
            virtual ~FeVariableFn(){};
            virtual func getFunction( IOsptr sample_input );
        private:
            void* _fevariable;
    };

};

#endif // __Underworld_Function_FeVariableFn_h__

