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

#ifndef __Underworld_Velic_solKz_h__
#define __Underworld_Velic_solKz_h__

	extern const Type Underworld_Velic_solKz_Type;

	typedef struct {
      __Codelet;
		double sigma;
		double km;
		int    n;
		double B;
	} Underworld_Velic_solKz;

	Index Underworld_Velic_solKz_Register( PluginsManager* pluginsManager );

	void* _Underworld_Velic_solKz_DefaultNew( Name name );

	void _Underworld_Velic_solKz_AssignFromXML( void* analyticSolution, Stg_ComponentFactory* cf, void* data );

	void _Underworld_Velic_solKz_Init( Underworld_Velic_solKz* self, double sigma, double km, double B, int n );

	void Underworld_Velic_solKz_VelocityFunction( void* _context, double* coord, double* velocity );

	void Underworld_Velic_solKz_PressureFunction( void* _context, double* coord, double* pressure );

   void Underworld_Velic_solKz_StressFunction( void* _context, double* coord, double* stress );

	void Underworld_Velic_solKz_StrainRateFunction( void* _context, double* coord, double* strainRate );

   void Underworld_Velic_solKz_ViscosityFunction( void* _context, double* coord, double* viscosity );

	void _Velic_solKz( 
		double  pos[],
		double  _sigma, /* density */
		double  _km,
      int     _n, /* wavelength in z, wavenumber in x */
		double  _B, /* viscosity parameter */
		double  vel[],
      double* presssure, 
		double  total_stress[],
      double  strain_rate[] );

#endif
