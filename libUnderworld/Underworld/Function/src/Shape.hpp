/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Shape_hpp__
#define __Underworld_Function_Shape_hpp__

#include "Function.hpp"

namespace Fn {

    class Polygon: public Function
    {
        public:
            Polygon(Function* _fn, double* IN_ARRAY2, int DIM1, int DIM2, char* name);
            virtual func getFunction( IOsptr sample_input );
            virtual ~Polygon();
        private:
            void* _stgshape;
            Function* _fn;
    };

}

#endif /* __Underworld_Function_Shape_hpp__ */

