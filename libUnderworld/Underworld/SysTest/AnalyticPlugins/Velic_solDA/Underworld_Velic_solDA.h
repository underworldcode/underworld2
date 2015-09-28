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

#ifndef __Underworld_Velic_solDA_h__
#define __Underworld_Velic_solDA_h__

   extern const Type Underworld_Velic_solDA_Type;

   typedef struct {
      __Codelet;
      double sigma;
      double etaA;  
      double etaB; 
      double zc;    
      double dx;    
      double x0;    
   } Underworld_Velic_solDA;

   Index Underworld_Velic_solDA_Register( PluginsManager* pluginsManager );

   void* _Underworld_Velic_solDA_DefaultNew( Name name );

   void _Underworld_Velic_solDA_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

   void _Underworld_Velic_solDA_Init( Underworld_Velic_solDA* self, double sigma, double etaA, double etaB, double zc, double dx, double x0 );

   void Underworld_Velic_solDA_VelocityFunction( void* _context, double* coord, double* velocity );

   void Underworld_Velic_solDA_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solDA_StressFunction( void* _context, double* coord, double* stress );

   void Underworld_Velic_solDA_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void _Velic_solDA(      
      double  pos[],
      double  _sigma, /* density */
      double  _eta_A,
      double  _eta_B, /* viscosity A, viscosity B */ 
      double  _z_c,
      double  _dx,
      double  _x_0, /* viscosity jump location, width of dense column, centre of dense column */
      double  vel[],
      double* presssure, 
      double  total_stress[],
      double  strain_rate[] );   

#endif
