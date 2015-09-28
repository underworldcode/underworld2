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

#ifndef __Underworld_Velic_solJ_h__
#define __Underworld_Velic_solJ_h__

   extern const Type Underworld_Velic_solJ_Type;

   typedef struct {
      __Codelet;
      double sigmaB;
      double sigmaA;
      double etaB;
      double etaA;
      double dxB;
      double dxA;
      double x0B;
      double x0A; 
      double zc;
   } Underworld_Velic_solJ;

   Index Underworld_Velic_solJ_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solJ_DefaultNew( Name name );

   void _Underworld_Velic_solJ_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solJ_Init(
      Underworld_Velic_solJ* self,
      double           sigmaA,
      double           sigmaB,
      double           etaB,
      double           etaA,
      double           dxB,
      double           dxA,
      double           x0B,
      double           x0A,
      double           zc );

   void Velic_solJ_VelocityFunction( void* _context, double* coord, double* velocity );

   void Velic_solJ_PressureFunction( void* _context, double* coord, double* pressure );

   void Velic_solJ_StressFunction( void* _context, double* coord, double* stress );

   void Velic_solJ_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solJ( 
      double  pos[],
      double  _sigma_B,
      double  _sigma_A, /* density B, density A */
      double  _eta_B,
      double  _eta_A, /* viscosity B, viscosity A */
      double  _dx_B,
      double  _dx_A, /* width of the upper dense block, width of the lower dense block */
      double  _x_0_B,
      double  _x_0_A, /* centre of the upper dense block, centre of lower dense block */
      double  _z_c, /* bottom of the upper dense block */
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );

#endif
