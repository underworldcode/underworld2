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

#ifndef __Underworld_Velic_solKx_h__
#define __Underworld_Velic_solKx_h__

   extern const Type Underworld_Velic_solKx_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double _m;
      int    n;
      double B;
   } Underworld_Velic_solKx;

   Index Underworld_Velic_solKx_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solKx_DefaultNew( Name name );

   void _Underworld_Velic_solKx_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solKx_Init( Underworld_Velic_solKx* self, double sigma, int _m, double B, double n );

   void Underworld_Velic_solKx_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solKx_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solKx_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solKx_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void Underworld_Velic_solKx_ViscosityFunction( void* _context, double* coord, double* viscosity );

   void _Velic_solKx( 
      double  pos[],
      double  _sigma, /* density */
      int     _m, /* wavenumber in z */
      double  _n,  /* wavenumber in x */
      double  _B, /* viscosity parameter */
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[],
      double* viscosity );

#endif
