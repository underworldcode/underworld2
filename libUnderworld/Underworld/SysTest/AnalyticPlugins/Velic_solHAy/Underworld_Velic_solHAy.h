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

#ifndef __Underworld_Velic_solHAy_h__
#define __Underworld_Velic_solHAy_h__

   extern const Type  Underworld_Velic_solHAy_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double eta;
      double dx;
      double dy;
      double x0;
      double y0;
   } Underworld_Velic_solHAy;

   Index Underworld_Velic_solHAy_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solHAy_DefaultNew( Name name );

   void _Underworld_Velic_solHAy_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solHAy_Init( Underworld_Velic_solHAy* self, double sigma, double eta, double dx, double dy, double x0, double y0 );

   void Underworld_Velic_solHAy_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solHAy_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solHAy_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solHAy_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solHAy( 
      double  pos[],
      double  _sigma,
      double  _eta,
      double  _dx,
      double  _dy,
      double  _x_0,
      double  _y_0,
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );

#endif
