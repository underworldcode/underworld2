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

#ifndef __Underworld_Velic_solHy_h__
#define __Underworld_Velic_solHy_h__

   extern const Type Underworld_Velic_solHy_Type;

   typedef struct {
      __Codelet;
      double etaA; /* Input parameters: density, viscosity A */
      double etaB; /* Input parameters: density, viscosity B */
      double xc; /* Input parameters: viscosity jump location */
      int    n;
   } Underworld_Velic_solHy;

   Index Underworld_Velic_solHy_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solHy_DefaultNew( Name name );

   void _Underworld_Velic_solHy_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solHy_Init( Underworld_Velic_solHy* self, double etaA, double etaB, double xc, int n );

   void Underworld_Velic_solHy_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solHy_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solHy_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solHy_StrainRateFunction( void* _context, double* coord, double* strainRate );

#endif
