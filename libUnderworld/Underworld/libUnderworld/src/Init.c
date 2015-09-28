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
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>

#include "Underworld/Rheology/Rheology.h"
#include "Underworld/Utils/Utils.h"

#include "Init.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>

/** Initialises this package, then any init for this package
such as streams etc */

Stream* Underworld_Debug = NULL;
Stream* Underworld_Info = NULL;
Stream* Underworld_Error = NULL;

Bool Underworld_Init( int* argc, char** argv[] ) {
   /* 
    * This init function tells StGermain of all the component types, 
    * etc this module contributes. Because it can be linked at compile
    * time or linked in by a toolbox at runtime, we need to make sure it isn't run twice 
    * (compiled in and loaded through a toolbox).
    */
   if( !ToolboxesManager_IsInitialised( stgToolboxesManager, "Underworld" ) ) {
      char* argString;
      int   arg_I;
      int   tmp;
      Bool  useSignalHandler = True;
      char* directory;

      for( arg_I = 0; argc && arg_I < *argc; arg_I++ ) {
         argString = (*argv)[arg_I];
         /* Leverage from PETSC's no signal flag */
         if( strcmp( argString, "-no_signal_handler" ) == 0 ) 
            useSignalHandler = False;
      }

      if( useSignalHandler ) {
         signal( SIGSEGV, Underworld_SignalHandler );
         signal( SIGTERM, Underworld_SignalHandler );
         signal( SIGINT,  Underworld_SignalHandler );
      }

      Underworld_Rheology_Init( argc, argv );
      Underworld_Utils_Init( argc, argv );

         
      

      Stream_Flush( Journal_Register( InfoStream_Type, (Name)"Context" ) );
      Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ), tmp );

      /* Create Streams */
      Underworld_Debug = Journal_Register( Debug_Type, (Name)"Context" );
      Underworld_Info = Journal_Register( Info_Type, (Name)"Context" );
      Underworld_Error = Journal_Register( Error_Type, (Name)"Context" );
   
      /* Add the Underworld path to the global xml path dictionary */
      directory = Memory_Alloc_Array( char, 400, "xmlDirectory" ) ;
      sprintf( directory, "%s%s", LIB_DIR, "/StGermain" );
      XML_IO_Handler_AddDirectory( "Underworld", directory );
      Memory_Free( directory );

      /* Add the plugin path to the global plugin list */
      #ifdef GLUCIFER_LIBDIR
         ModulesManager_AddDirectory( "gLucifer", GLUCIFER_LIBDIR );
      #endif
   
      ModulesManager_AddDirectory( "Underworld", LIB_DIR );

      return True;
   }
   return False;
}

void Underworld_SignalHandler( int signal ) {
   fprintf(
      stderr, 
      "\n\n=====================================================================================\n"
      "Error running Underworld - Signal %d ",
      signal );

   switch( signal ) {
      case SIGSEGV:
         fprintf(
            stderr, 
            "'SIGSEGV' (Segmentation Fault).\n" 
            "This is probably caused by an illegal access of memory.\n"
            "We recommend running the code in a debugger to work out where the problem is (e.g. 'gdb')\n"
            "and also to contact the developers.\n" );
         break;
      case SIGTERM:
         fprintf(
            stderr, 
            "'SIGTERM' (Termination Request).\n" 
            "This is caused by an external call to terminate the code.\n"
            "This could have happened by a queueing system (e.g. if the code has run longer than allowed),\n"
            "the code might have been killed on another processor or it may have been killed by the user.\n" );
         break;
      case SIGINT:
        fprintf(
          stderr, 
          "'SIGINT' (Termination Request).\n"
          "Isn't it wonderbubble to have CTRL-C?\n" );
        break;
   }
   exit( EXIT_FAILURE );
}


