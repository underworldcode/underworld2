/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_DiscreteCoordinate_h__
#define __Underworld_Function_DiscreteCoordinate_h__

#include "FunctionIO.hpp"

class DiscreteCoordinate: public IO_double
{
    public:
        DiscreteCoordinate( size_t dim, void* object );
              unsigned& index()       { return _index; };
        const unsigned  index() const { return _index; };
        void* object()          const { return _object; };
        virtual       double* data()                    =0;
        virtual const double* data()              const =0;
        virtual       double& at(std::size_t idx)       =0;
        virtual const double  at(std::size_t idx) const =0;

    protected:
        void*    _object;
        unsigned _index;
};



#endif /* __Underworld_Function_DiscreteCoordinate_h__ */

