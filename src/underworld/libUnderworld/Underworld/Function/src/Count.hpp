/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Count_hpp__
#define __Underworld_Function_Count_hpp__

#include <iostream>

#include "FunctionIO.hpp"

namespace Fn {

    class Count: public Function
    {
        public:
            Count(Function *fn_input):count(0),_fn_input(fn_input){};
            virtual ~Count(){};
            int count;
            virtual func getFunction( IOsptr sample_input )
            {
                func func_input = _fn_input->getFunction( sample_input );
                // Do a decrement as the lambda will be called (to determine
                // what it itself returns) after this `getFunction` returns. 
                // I'm not 100% how robust this is. JM.
                count-=1;
                return [func_input,this](IOsptr input)->IOsptr 
                    { 
                        count+=1; 
                        return func_input(input); 
                    };
            };
            void reset(){count=0;};
        protected:
            Function* _fn_input;
    };
        
}


#endif /* __Underworld_Function_Count_hpp__ */

