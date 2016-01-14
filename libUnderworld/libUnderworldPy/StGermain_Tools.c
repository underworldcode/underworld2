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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const Type StGermain_Type = "StGermain";

#include "StGermain_Tools.h"

static int hasBeenMPIInit = 0;

StgData* StgInit( int argc, char* argv[] ) {
   StgData* data = (StgData*) malloc(sizeof(StgData));
   *data = (StgData){.comm = NULL, .rank=-1, .nProcs=-1, .dictionary=NULL, .argcCpy=NULL, .argvCpy=NULL }; 

   //lets copy all this data for safety
   data->argcCpy = (int*) malloc(sizeof(int));
   *(data->argcCpy) = argc;
   data->argvCpy = (char***) malloc(sizeof(char**)); 
   *(data->argvCpy) = (char**) malloc((argc+1)*sizeof(char*));
   int ii;
   for(ii = 0; ii<argc; ii++){
      (*(data->argvCpy))[ii] = (char*)malloc(strlen(argv[ii])+1);
      strcpy((*(data->argvCpy))[ii],argv[ii]);
   }
   (*(data->argvCpy))[argc]=NULL;  //add sentinel

   Index                 i;
   Stg_ObjectList*       inputPaths = NULL;
   char*                 inputPath = NULL;
   /* Initialise PETSc, get world info */
   if(!hasBeenMPIInit){
	   //MPI_Init( data->argcCpy, data->argvCpy );
	   hasBeenMPIInit = 1;
   }
   MPI_Comm_dup( MPI_COMM_WORLD, &data->comm );
   MPI_Comm_size( data->comm, &data->nProcs );
   MPI_Comm_rank( data->comm, &data->rank );
   StGermain_Init( data->argcCpy, data->argvCpy );

   /* Ensures copyright info always come first in output */
   MPI_Barrier( data->comm );

   /* 
    * Parse the input path command line argument... needed before we start parsing the input.
    * And add the path to the global xml path dictionary. 
    */
   inputPaths = stgParseInputPathCmdLineArg( data->argcCpy, data->argvCpy );
   for( i = 0; i < Stg_ObjectList_Count( inputPaths ); i++ ) {
      inputPath = (char*)Stg_ObjectAdaptor_Object( (Stg_ObjectAdaptor*)Stg_ObjectList_At( inputPaths, i ) );
      XML_IO_Handler_AddDirectory( (Name)"--inputPath", inputPath );
      File_AddPath( inputPath );
   }
   Stg_Class_Delete( inputPaths );

   /* Create the application's dictionary & read input. */
   data->dictionary = Dictionary_New();
   
   Dictionary* sources = Dictionary_New();

   XML_IO_Handler* ioHandler = XML_IO_Handler_New();
   IO_Handler_ReadAllFromCommandLine( ioHandler, *(data->argcCpy), *(data->argvCpy), data->dictionary, sources );
   Stg_Class_Delete( ioHandler );
   Stg_Class_Delete( sources );
   
   return data;
}

int StgFinalise(StgData* data){
   /* Close off everything */
   StGermain_Finalise();
   MPI_Finalize();

   /* free up these guys created earlier */
   int ii;
   for(ii = 0; ii<*(data->argcCpy); ii++)
   	  free((*(data->argvCpy))[ii]);
   free(data->argvCpy);
   free(data->argcCpy);
   free(data);   
   return 0; /* success */
}


void StgAbort(StgData* data){
     MPI_Abort( data->comm, EXIT_FAILURE );
}


