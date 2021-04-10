/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __solJ_h__
#define __solJ_h__

void _Velic_solJ(
        double pos[],
        double _sigma_B, double _sigma_A,
        double _eta_B, double _eta_A,
        double _dx_B, double _dx_A,
        double _x_0_B, double _x_0_A,
        double _z_c,
        double vel[], double* presssure,
        double total_stress[], double strain_rate[] );

#endif
