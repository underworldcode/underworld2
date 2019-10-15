/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_CustomException_hpp__
#define __Underworld_Function_CustomException_hpp__

#include "Function.hpp"

namespace Fn {
    
    class CustomException: public Function
    {
    public:
        CustomException( Function *fn_input, Function *fn_condition, Function *fn_print=NULL ):
        _fn_input(fn_input), _fn_condition(fn_condition), _fn_print(fn_print) {};
        virtual ~CustomException(){};
        virtual func getFunction( IOsptr sample_input );
    protected:
        Function* _fn_input;
        Function* _fn_condition;
        Function* _fn_print;
    };

}

#endif /* __Underworld_Function_CustomException_hpp__ */


