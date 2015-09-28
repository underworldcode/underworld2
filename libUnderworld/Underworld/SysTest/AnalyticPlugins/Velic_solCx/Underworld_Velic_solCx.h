/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#ifndef __Underworld_Velic_solCx_h__
#define __Underworld_Velic_solCx_h__

   extern const Type Underworld_Velic_solCx_Type;

   typedef struct {
      __Codelet;
      double etaA; /* Input parameters: density, viscosity A */
      double etaB; /* Input parameters: density, viscosity B */
      double xc; /* Input parameters: viscosity jump location */
      int    n;
   } Underworld_Velic_solCx;

   Index Underworld_Velic_solCx_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solCx_DefaultNew( Name name );

   void _Underworld_Velic_solCx_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solCx_Init( Underworld_Velic_solCx* self, double etaA, double etaB, double xc, int n );

   void Underworld_Velic_solCx_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solCx_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solCx_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solCx_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void Underworld_Velic_solCx_ViscosityFunction( void* _context, double* coord, double* viscosity );

   void _Velic_solCx(
      double pos[], 
      double _eta_A,
      double _eta_B, /* Input parameters: density, viscosity A, viscosity B */
      double _x_c,
      int _n, /* Input parameters: viscosity jump location, wavenumber in x */
      double vel[],
      double* presssure, 
      double total_stress[],
      double strain_rate[] );

#endif
