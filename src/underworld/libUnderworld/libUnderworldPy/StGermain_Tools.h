/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StgData
{
   MPI_Comm comm;
   int rank;
   int nProcs;
   Dictionary* dictionary;
   int argcCpy;
   char** argvCpy;
} StgData;

StgData* StgInit( int argc, char* argv[] ) ;
int StgFinalise(StgData* data) ;
void StgAbort(StgData* data) ;
