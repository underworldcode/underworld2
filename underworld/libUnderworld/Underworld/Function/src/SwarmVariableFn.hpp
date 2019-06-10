/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_SwarmVariableFn_h__
#define __Underworld_Function_SwarmVariableFn_h__

#include "Function.hpp"

namespace Fn {

    class SwarmVariableFn: public Function
    {
        public:
            SwarmVariableFn( void* swarmvariable );
            virtual func getFunction( IOsptr sample_input );
            virtual ~SwarmVariableFn(){}
        private:
            void* _swarmvariable;
    };

};

#endif // __Underworld_Function_SwarmVariableFn_h__

