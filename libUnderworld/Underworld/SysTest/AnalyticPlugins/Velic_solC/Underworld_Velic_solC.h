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

#ifndef __Underworld_Velic_solC_h__
#define __Underworld_Velic_solC_h__

   extern const Type Underworld_Velic_solC_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double eta;  /* Input parameters: density, viscosity A */
      double x_c;  
   } Underworld_Velic_solC;

   void* _Underworld_Velic_solC_DefaultNew( Name name );

   void _Underworld_Velic_solC_Init( Underworld_Velic_solC* self, double sigma, double eta, double x_c );

   void _Underworld_Velic_solC_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void Underworld_Velic_solC_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solC_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solC_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solC_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solC( 
      double  pos[], 
      double  sigma,
      double  eta,
      double  x_c,
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );

   Bool solC_checkInputParams( Underworld_Velic_solC* self );

#endif
