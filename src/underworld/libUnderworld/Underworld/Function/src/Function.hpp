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
#include <iostream>

#include "FunctionIO.hpp"

namespace Fn {

    class Function
    {
        public:
            typedef const FunctionIO* IOsptr;
            typedef std::function<IOsptr( const IOsptr &input )> func;
            virtual func getFunction( IOsptr input )=0;
            virtual ~Function(){};
            void set_pyfnerrorheader( char* pyfnerrorheader ){ _pyfnerrorheader = pyfnerrorheader; }
        protected:
            Function(): _pyfnerrorheader("Error in function of class 'Function'\nError message:\n"){};
            std::string _pyfnerrorheader;  // this member records the python stack error message at construction time
    };

    class Input: public Function
    {
        public:
            Input(){};
            virtual ~Input(){};
            virtual func getFunction( IOsptr sample_input )
            {
                return [](IOsptr input)->IOsptr { return input; };
            };
    };
        
}


#endif /* __Underworld_Function_Function_hpp__ */

