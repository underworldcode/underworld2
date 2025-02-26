/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_GradFeVariableFn_h__
#define __Underworld_Function_GradFeVariableFn_h__

#include "Function.hpp"

namespace Fn {

    class GradFeVariableFn: public Function
    {
        public:
            GradFeVariableFn( void* fevariable );
            virtual ~GradFeVariableFn(){};
            virtual func getFunction( IOsptr sample_input );
        private:
            void* _fevariable;
    };

};

#endif // __Underworld_Function_GradFeVariableFn_h__

