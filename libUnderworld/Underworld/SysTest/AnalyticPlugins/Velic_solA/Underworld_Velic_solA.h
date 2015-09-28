/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#ifndef __Underworld_Velic_solA_h__
#define __Underworld_Velic_solA_h__

   extern const Type Underworld_Velic_solA_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double Z;
      double km;
      int    n;
   } Underworld_Velic_solA;

   void Underworld_Velic_solA_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solA_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solA_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solA_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solA(
      double* pos, 
      double  sigma,
      double  Z,
      int     n,
      double  km,
      double* velocity,
      double* pressure,
      double* Tstress,
      double* strainRate );

   Bool solA_checkInputParams( Underworld_Velic_solA* self );

#endif
