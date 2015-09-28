/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __solCx_h__
#define __solCx_h__

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void _Velic_solCx( 
  const double pos[],
  double _eta_A, double _eta_B, 
  double _x_c, int _n,
  double vel[], double* presssure, 
  double total_stress[], double strain_rate[] );

void _solCx_A(
  const double pos[],
  double _eta_A, double _eta_B, 	/* Input parameters: density, viscosity A, viscosity B */
  double _x_c, int _n, 			/* Input parameters: viscosity jump location, wavenumber in x */
  double vel[], double* presssure, 
  double total_stress[], double strain_rate[] );

void _solCx_B(
  const double pos[],
  double _eta_A, double _eta_B, 	/* Input parameters: density, viscosity A, viscosity B */
  double _x_c, int _n, 			/* Input parameters: viscosity jump location, wavenumber in x */
  double vel[], double* presssure, 
  double total_stress[], double strain_rate[] );

#endif