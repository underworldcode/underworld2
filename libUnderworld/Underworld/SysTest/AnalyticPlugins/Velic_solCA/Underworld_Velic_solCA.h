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

#ifndef __Underworld_Velic_solCA_h__
#define __Underworld_Velic_solCA_h__

   extern const Type Underworld_Velic_solCA_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double eta;  /* Input parameters: density, viscosity A */
      double dx;  
      double x0;  
   } Underworld_Velic_solCA;

   Index Underworld_Velic_solCA_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solCA_DefaultNew( Name name );

   void _Underworld_Velic_solCA_Init( Underworld_Velic_solCA* self, double sigma, double eta, double dx, double x0 ); 

   void _Underworld_Velic_solCA_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void Underworld_Velic_solCA_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solCA_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solCA_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solCA_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solCA( 
      double pos[], 
      double _sigma,
      double _eta,
      double _dx,
      double _x_0,
      double vel[],
      double* presssure, 
      double total_stress[],
      double strain_rate[] );

   Bool _checkInputParams( Underworld_Velic_solCA* self );   

#endif
