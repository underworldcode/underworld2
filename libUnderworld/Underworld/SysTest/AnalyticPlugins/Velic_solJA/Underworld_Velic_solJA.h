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

#ifndef __Underworld_Velic_solJA_h__
#define __Underworld_Velic_solJA_h__

   extern const Type Underworld_Velic_solJA_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double etaA;
      double etaB;
      double dx;
      double x0;
      double zc;
   } Underworld_Velic_solJA;

   Index Underworld_Velic_solJA_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solJA_DefaultNew( Name name );

   void _Underworld_Velic_solJA_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solJA_Init( Underworld_Velic_solJA* self, double sigma, double etaA, double etaB, double dx, double x0, double zc );

   void Underworld_Velic_solJA_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solJA_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solJA_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solJA_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solJA( 
      double  pos[],
      double  _sigma, /* density */
      double  _eta_A,
      double  _eta_B, /* viscosity A, viscosity B */
      double  _dx, /* width of upper dense block */
      double  _x_0,
      double  _z_c, /* centre of upper dense block, bottom of upper dense block */
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );

#endif
