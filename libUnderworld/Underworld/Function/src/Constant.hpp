/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Constant_h__
#define __Underworld_Function_Constant_h__

#include "Function.hpp"

namespace Fn {

    class Constant: public Function
    {
        public:
            Constant( const FunctionIO& constio ): Function() { _constIO = std::shared_ptr<FunctionIO>(constio.clone()); };
            virtual func getFunction( IOsptr sample_input );
            void set_value( const FunctionIO& value ){ _constIO = std::shared_ptr<FunctionIO>(value.clone()); };
            virtual ~Constant(){};
        private:
            std::shared_ptr<FunctionIO> _constIO;
    };

};


#endif // __Underworld_Function_Constant_h__

