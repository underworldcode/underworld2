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

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>

#include "types.h"

#include "Random.h"

#include <assert.h>

/* Park and Miller "minimul standard" constants */
const long int Swarm_Random_RandMax = 2147483647;	/* Swarm_Random_RandMax = (2^31)-1, taken from max int */
static const long int a = 16807;			/* a = 7^5, taken from algorithm */
static const unsigned long int mask = 123459876;	/* taken from algorithm */

/* Schrage's algorithm constants */
static const long int q = 127773;			/* q = [Swarm_Random_RandMax / a] */
static const long int r = 2836;				/* r = m mod a */

/* Park and Miller "minimul standard" variables */
static long Swarm_Random_maskedSeed;


void Swarm_Random_Seed( long seed ) {
	assert( mask - seed != 0 );
	Swarm_Random_maskedSeed = seed ^ mask;
}


double Swarm_Random_Random() {
	const long int tmp = Swarm_Random_maskedSeed / q;
	Swarm_Random_maskedSeed = a * (Swarm_Random_maskedSeed - tmp * q) - r * tmp;
	if( Swarm_Random_maskedSeed < 0 ) {
		Swarm_Random_maskedSeed += Swarm_Random_RandMax;
	}
	return Swarm_Random_maskedSeed;
}

double Swarm_Random_Random_WithMinMax( double min, double max ) {
	return ( max - min ) * Swarm_Random_Random() / (double)Swarm_Random_RandMax + min;
}


