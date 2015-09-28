/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "StGermain/Base/Base.h"

#define OUTPUT_FILE_FLAG "-output_file="
#define DEFAULT_OUTPUT_FILE "./output.xml"

int main( int argc, char* argv[] ) {
   Dictionary*     dictionary=NULL;
   Dictionary*     sources;
   XML_IO_Handler* ioHandler;
   Stream*         msgs;
   char*           outputFilename = DEFAULT_OUTPUT_FILE;
   int             ii;

   MPI_Init( &argc, &argv );

   if( !StGermainBase_Init( &argc, &argv ) ) {
      fprintf( stderr, "Error initialising StGermain, exiting.\n" );
      exit( EXIT_FAILURE );
   }
   msgs = Journal_Register( Info_Type, "Messages" );
   
   for( ii = 0; ii < argc; ++ii ) {
      if( strstr( argv[ii], OUTPUT_FILE_FLAG ) == argv[ii] ) {
         outputFilename = argv[ii] + strlen( OUTPUT_FILE_FLAG );
         if( strlen( outputFilename ) < 1 ) {
            Journal_Printf( msgs, "Invalid outputfile name: %s\n", outputFilename );
            Journal_Printf( msgs, "Exiting...\n" );
            StGermainBase_Finalise();
            return 1;
         }
         argv[ii] = " "; /* remove it from the arg list */
      }
   }

   dictionary = Dictionary_New();
   sources = Dictionary_New();

   /* Read input */
   ioHandler = XML_IO_Handler_New();
   IO_Handler_ReadAllFromCommandLine( ioHandler, argc, argv, dictionary, sources );
   IO_Handler_WriteAllToFile( ioHandler, outputFilename, dictionary, sources );

   Stg_Class_Delete( dictionary );
   Stg_Class_Delete( sources );
   Stg_Class_Delete( ioHandler );

   StGermainBase_Finalise();
   MPI_Finalize();

   return 0; /* success */
}


