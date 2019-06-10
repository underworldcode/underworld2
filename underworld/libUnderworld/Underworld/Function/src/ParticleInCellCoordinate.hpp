/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Function_ParticleInCellCoordinate_h__
#define __Underworld_Function_ParticleInCellCoordinate_h__

#include "FunctionIO.hpp"
#include "DiscreteCoordinate.hpp"

class ParticleInCellCoordinate: public DiscreteCoordinate
{
   public:
        ParticleInCellCoordinate( void* swarmvariable );
        virtual       double* data();
        virtual const double* data() const;
        virtual       double& at(std::size_t idx);
        virtual const double  at(std::size_t idx) const;
        virtual  ParticleInCellCoordinate *clone() const { return new ParticleInCellCoordinate(*this); }
        void     particle_cellId( unsigned particle_cellId ){ _particle_cellId = particle_cellId; };
        unsigned particle_cellId() const {  return _particle_cellId; };
    private:
        static unsigned _Check_GetDimSize(void* swarmvariable);
        unsigned _particle_cellId;
};

#endif // __Underworld_Function_ParticleInCellCoordinate_h__
