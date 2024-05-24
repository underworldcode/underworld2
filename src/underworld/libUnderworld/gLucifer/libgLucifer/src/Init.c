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
#include <StgDomain/libStgDomain/src/StgDomain.h>

#include "gLucifer.h"

#include <stdio.h>

Bool gLucifer_Init( int* argc, char** argv[] ) {
   /*
    * This init function tells StGermain of all the component types,
    * etc this module contributes. Because it can be linked at compile 
    * time or linked in by a toolbox at runtime, we need to make sure it isn't run twice 
    * (compiled in and loaded through a toolbox.
    */
   if( !ToolboxesManager_IsInitialised( stgToolboxesManager, "gLucifer" ) ) {
      int   tmp;
      char* directory;

      
      

      Stream_Flush( Journal_Register( Info_Type, (Name)"Context" ) );
      Stream_SetPrintingRank( Journal_Register( Info_Type, (Name)"Context" ), tmp );

      lucBase_Init();
      lucDrawingObjects_Init();

      /* Add the gLucifer path to the global xml path dictionary */
      directory = Memory_Alloc_Array( char, 200, "xmlDirectory" ) ;
      sprintf(directory, "%s%s", LIB_DIR, "/StGermain" );
      XML_IO_Handler_AddDirectory( "gLucifer", directory );
      Memory_Free(directory);

      /* Add the plugin path to the global plugin list */
      ModulesManager_AddDirectory( "gLucifer", LIB_DIR );

      return True;
   }
   return False;
}



