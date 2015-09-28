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

#ifndef __Underworld_Velic_solI_h__
#define __Underworld_Velic_solI_h__

   extern const Type Underworld_Velic_solI_Type;

   typedef struct {
      __Codelet;
      double sigma; /* density */
      double B;
      double xc; /* width of dense column */
   } Underworld_Velic_solI;

   Index Underworld_Velic_solI_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solI_DefaultNew( Name name );

   void _Underworld_Velic_solI_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solI_Init( Underworld_Velic_solI* self, double sigma, double B, double xc );

   void Underworld_Velic_solI_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solI_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solI_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solI_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solI( 
      double pos[],
      double _sigma, /* density */
      double _B,
      double _x_c, /* viscosity parameter, width of dense column */
      double vel[],
      double* presssure, 
      double total_stress[],
      double strain_rate[] );

#endif
