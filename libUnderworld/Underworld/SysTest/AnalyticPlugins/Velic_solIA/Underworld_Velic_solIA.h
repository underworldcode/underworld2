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

#ifndef __Underworld_Velic_solIA_h__
#define __Underworld_Velic_solIA_h__

   extern const Type Underworld_Velic_solIA_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double B;
      double dx;
      double x0;
   } Underworld_Velic_solIA;

   Index Underworld_Velic_solIA_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solIA_DefaultNew( Name name );

   void _Underworld_Velic_solIA_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solIA_Init( Underworld_Velic_solIA* self, double sigma, double B, double dx, double x0 );

   void Underworld_Velic_solIA_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solIA_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solIA_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solIA_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solIA( 
      double  pos[],
      double  _sigma,
      double  _B, /* density, viscosity parameter */
      double  _dx,
      double  _x_0, /* width of dense column, centre of dense column */
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );

#endif
