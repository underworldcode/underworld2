/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_ParticleFound_h__
#define __Underworld_Function_ParticleFound_h__

#include "Function.hpp"

namespace Fn {

    class ParticleFound: public Function
    {
        public:
            ParticleFound( void* swarm );
            virtual func getFunction( IOsptr sample_input );
            virtual ~ParticleFound(){}
        private:
            void* _swarm;
    };

};

#endif // __Underworld_Function_ParticleFound_h__

