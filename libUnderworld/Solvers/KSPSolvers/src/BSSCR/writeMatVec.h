/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef _writeMat_h
#define _writeMat_h
#include <petsc.h>
#include <petscmat.h>
#include <petscvec.h>

void bsscr_writeMat(Mat A, char name[], char message[]);
void bsscr_writeVec(Vec V, char name[], char message[]);
void bsscr_dirwriteMat(Mat A, char name[], char dir[], char message[]);
void bsscr_dirwriteVec(Vec V, char name[], char dir[], char message[]);
#endif
