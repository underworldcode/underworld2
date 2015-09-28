/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __solKx_h__
#define __solKx_h__

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void _Velic_solKx( 
		const double pos[],
		double _sigma,
		int _m, double _n, 
		double _B,
		double vel[], double* presssure, 
		double total_stress[], double strain_rate[], double* viscosity );


#endif
