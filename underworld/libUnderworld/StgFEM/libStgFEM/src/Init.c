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
#include "StgFEM/Discretisation/Discretisation.h"
#include "StgFEM/SLE/SLE.h"
#include "StgFEM/Assembly/Assembly.h"
#include "StgFEM/Utils/Utils.h"
#include "Init.h"

#include <stdio.h>

/* 
 * Initialises the Linear Algebra package, then any init for this package
 * such as streams etc.
 */
Bool StgFEM_Init( int* argc, char** argv[] ) {
   /* 
    * This init function tells StGermain of all the component types, 
    * etc this module contributes. Because it can be linked at compile
    * time or linked in by a toolbox at runtime, we need to make sure it isn't run twice
    * (compiled in and loaded through a toolbox.
    */
   if( !ToolboxesManager_IsInitialised( stgToolboxesManager, "StgFEM" ) ) {
      int   tmp;
      char* directory;
 
      
      

      Stream_Flush( Journal_Register( InfoStream_Type, (Name)"Context" ) );
      Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ), tmp );
   
      StgFEM_Discretisation_Init( argc, argv );
      StgFEM_SLE_Init( argc, argv );
      StgFEM_Assembly_Init( argc, argv );
      StgFEM_Utils_Init( argc, argv );

      /* Add the StgFEM path to the global xml path dictionary */
      directory = Memory_Alloc_Array( char, 200, "xmlDirectory" );
      sprintf(directory, "%s%s", LIB_DIR, "/StGermain" );
      XML_IO_Handler_AddDirectory("StgFEM", directory );
      Memory_Free(directory);
   
      /* Add the plugin path to the global plugin list */
      ModulesManager_AddDirectory( "StgFEM", LIB_DIR );

      return True;
   }
   return False;
}


