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

#ifndef __Underworld_Velic_solHA_h__
#define __Underworld_Velic_solHA_h__

   extern const Type Underworld_Velic_solHA_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double eta;
      double dx;
      double dy;
      double x0;
      double y0;
   } Underworld_Velic_solHA;

   Index Underworld_Velic_solHA_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solHA_DefaultNew( Name name );

   void _Underworld_Velic_solHA_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solHA_Init( Underworld_Velic_solHA* self, double sigma, double eta, double dx, double dy, double x0, double y0 );

   void Underworld_Velic_solHA_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solHA_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solHA_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solHA_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solHA( 
      double pos[],
      double _sigma,
      double _eta,
      double _dx,
      double _dy,
      double _x_0,
      double _y_0,
      double vel[],
      double* presssure, 
      double total_stress[],
      double strain_rate[] );

#endif
