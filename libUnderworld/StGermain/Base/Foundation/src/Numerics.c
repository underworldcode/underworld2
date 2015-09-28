/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <assert.h>

#include "types.h"
#include "Numerics.h"


const double	Num_Epsilon = 1e-12;


Bool Num_Approx( double var, double val ) {
	return (Bool)(var >= val - Num_Epsilon && var <= val + Num_Epsilon);
}

Bool Num_InRange( double var, double low, double upp ) {
	return (Bool)(var >= low - Num_Epsilon && var <= upp + Num_Epsilon);
}


