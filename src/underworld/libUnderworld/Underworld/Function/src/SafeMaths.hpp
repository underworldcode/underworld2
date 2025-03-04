/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
 **                                                                                  **
 ** This file forms part of the Underworld geophysics modelling application.         **
 **                                                                                  **
 ** For full license and copyright information, please refer to the LICENSE.md file  **
 ** located at the project root, or contact the authors.                             **
 **                                                                                  **
 **~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_SafeMaths_hpp__
#define __Underworld_Function_SafeMaths_hpp__

#include "Function.hpp"

namespace Fn {

    class SafeMaths: public Function
    {
    public:
        SafeMaths( Function *fn ): _fn(fn) {};
        virtual ~SafeMaths(){};
        virtual func getFunction( IOsptr sample_input );
    protected:
        Function* _fn;
    };

}

#endif /* __Underworld_Function_SafeMaths_hpp__ */


