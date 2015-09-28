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

#ifndef __Underworld_Velic_solS_h__
#define __Underworld_Velic_solS_h__

   extern const Type Underworld_Velic_solS_Type;

   typedef struct {
      __Codelet;
      int    _n;
      double _eta;
   } Underworld_Velic_solS;

   Index Underworld_Velic_solS_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solS_DefaultNew( Name name );

   void _Underworld_Velic_solS_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solS_Init( Underworld_Velic_solS* self, double eta, int _n );

   void Underworld_Velic_solS_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solS_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solS_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solS_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solS( 
      double  pos[],
      int     _n,
      double  _eta,
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );

   Bool _checkInputParams( Underworld_Velic_solS* self );

#endif
