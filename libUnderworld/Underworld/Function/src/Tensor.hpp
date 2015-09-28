/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __Underworld_Function_Tensor_hpp__
#define __Underworld_Function_Tensor_hpp__

#include "Function.hpp"

namespace Fn {
    
    class TensorFunc: public Function
    {
        public:
            enum TensorFuncFunc
            {
                get_symmetric, get_antisymmetric, second_invariant, get_deviatoric
            };
            TensorFunc(Function* fn, TensorFuncFunc partFunc): _fn(fn), _partFunc(partFunc) {};
            virtual func getFunction( IOsptr sample_input );
            virtual ~TensorFunc(){};
        protected:
            Function* _fn;
            TensorFuncFunc _partFunc;
    };

}

#endif /* __Underworld_Function_Tensor_hpp__ */

