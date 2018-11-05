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
#include <petsc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Type StGermain_Type = "StGermain";

#include "StGermain_Tools.h"

StgData* StgInit( int argc, char* argv[] ) {
   StgData* data = (StgData*) malloc(sizeof(StgData));
   *data = (StgData){.comm = NULL, .rank=-1, .nProcs=-1, .dictionary=NULL, .argcCpy=NULL, .argvCpy=NULL }; 

   //lets copy all this data for safety
   data->argcCpy = argc;
   data->argvCpy = (char**) malloc((argc+1)*sizeof(char*));
   int ii;
   for(ii = 0; ii<argc; ii++){
      data->argvCpy[ii] = (char*)malloc(strlen(argv[ii])+1);
      strcpy(data->argvCpy[ii],argv[ii]);
   }
   data->argvCpy[argc] = NULL;  //add sentinel

   MPI_Comm_dup( MPI_COMM_WORLD, &data->comm );
   MPI_Comm_size( data->comm, &data->nProcs );
   MPI_Comm_rank( data->comm, &data->rank );
   StGermain_Init( &(data->argcCpy), &(data->argvCpy) );
   
   return data;
}

int StgFinalise(StgData* data){
   /* Close off everything */
//   StGermain_Finalise();
    PetscFinalize();
   MPI_Finalize();

   /* free up these guys created earlier */
   int ii;
   for(ii = 0; ii<data->argcCpy; ii++)
   	  free(data->argvCpy[ii]);
   free(data->argvCpy);
   free(data);
   return 0; /* success */
}


void StgAbort(StgData* data){
     MPI_Abort( data->comm, EXIT_FAILURE );
}


