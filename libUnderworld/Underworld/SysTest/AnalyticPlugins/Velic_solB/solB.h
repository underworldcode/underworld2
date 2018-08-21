/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __solB_h__
#define __solB_h__

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void _Velic_solB(
                 const double* pos,
                 double  sigma,
                 double  Z,
                 int     n,
                 double  km,
                 double* velocity,
                 double* pressure,
                 double* Tstress,
                 double* strainRate );
#endif
