/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_Map_h__
#define __Underworld_Function_Map_h__

#include "Function.hpp"

namespace Fn {

    class Map: public Function
    {
        public:
            Map(Function* keyFunc, Function* defaultFunc=NULL);
            virtual func getFunction( IOsptr sample_input );
            void insert( unsigned key, Function* value );
            virtual ~Map(){}
        private:
            std::vector<Function*> _funcArray;
            std::vector<bool>      _isIndexInMap;
            Function*              _keyFunc;
            Function*              _defaultFunc;
        
    };

}

#endif // __Underworld_Function_Map_h__

