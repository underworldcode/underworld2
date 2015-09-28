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

#ifndef __Underworld_Velic_solE_h__
#define __Underworld_Velic_solE_h__

	extern const Type Underworld_Velic_solE_Type;

	typedef struct {
      __Codelet;
		double sigma;
		double etaA;
		double etaB;
		double zc;
		double km;
		int    n;
	} Underworld_Velic_solE;

	Index Underworld_Velic_solE_Register( PluginsManager* pluginsManager );

	void* _Underworld_Velic_solE_DefaultNew( Name name );

	void _Underworld_Velic_solE_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

	void _Underworld_Velic_solE_Init( Underworld_Velic_solE* self, double sigma, double etaA, double etaB, double zc, double km, double n );

	void Underworld_Velic_solE_VelocityFunction( void* _context, double* coord, double* velocity );

	void Underworld_Velic_solE_PressureFunction( void* _context, double* coord, double* pressure );

	void Underworld_Velic_solE_StressFunction( void* _context, double* coord, double* stress );

	void Underworld_Velic_solE_StrainRateFunction( void* _context, double* coord, double* strainRate );

	void _Velic_solE(
		double  pos[],
		double  _sigma,
		double  _eta_A,
      double  _eta_B, 
		double  _z_c,
      double  _km,
      int     _n,
		double  vel[],
      double* presssure, 
		double  total_stress[],
      double  strain_rate[] );
 
#endif
