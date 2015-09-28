/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_FEMCoordinate_h__
#define __Underworld_Function_FEMCoordinate_h__

#include "FunctionIO.hpp"

class FEMCoordinate: public IO_double
{
    public:
        FEMCoordinate( void* mesh, std::shared_ptr<IO_double> localCoord );
        virtual FEMCoordinate *clone() const { return new FEMCoordinate(*this); }
        virtual       double* data();
        virtual const double* data()              const;
        virtual       double& at(std::size_t idx);
        virtual const double  at(std::size_t idx) const;

              unsigned&  index()            { _valueCalculated=false; return _index; };
        const unsigned   index()      const {                         return _index; };
        std::shared_ptr<      IO_double> localCoord()       { _valueCalculated=false; return _localCoord; };
        std::shared_ptr<const IO_double> localCoord() const {                         return _localCoord; };
        const void* mesh() const {return _mesh;};
    private:
        static unsigned _Check_GetDimSize(void* mesh);
        std::shared_ptr<IO_double> _localCoord;
        void* _mesh;
        unsigned _index;
        bool mutable _valueCalculated;
        void _calculate_value() const;
};

#endif //__Underworld_Function_FEMCoordinate_h__

