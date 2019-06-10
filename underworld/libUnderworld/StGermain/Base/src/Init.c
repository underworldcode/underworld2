/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include <mpi.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"
#include "Base/Context/Context.h"

#include "Init.h"

#include <stdio.h>

Bool StGermainBase_Init( int* argc, char** argv[] ) {
   char* directory;
   int   tmp;
   
   /* Initialise enough bits and pieces to get IO going */
   BaseFoundation_Init( argc, argv );
   BaseIO_Init( argc, argv );

   /* Write out the copyright message */

   
   Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ );
   tmp = Stream_GetPrintingRank( Journal_Register( InfoStream_Type, "Context" ) );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, "Context" ), 0 );
 
   Stream_Flush( Journal_Register( InfoStream_Type, "Context" ) );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, "Context" ), tmp );
   
   /* Initialise the remaining bits and pieces */
   BaseContainer_Init( argc, argv );
   BaseAutomation_Init( argc, argv );
   BaseExtensibility_Init( argc, argv );
   BaseContext_Init( argc, argv );
   
   /* Add the StGermain path to the global xml path dictionary */
   directory = Memory_Alloc_Array( char, 200, "xmlDirectory" ) ;
   sprintf( directory, "%s%s", LIB_DIR, "/StGermain" );
   XML_IO_Handler_AddDirectory( "StGermain", directory  );
   Memory_Free( directory );
   
   /* Add the plugin path to the global plugin list */
   ModulesManager_AddDirectory( "StGermain", LIB_DIR );
   
   return True;
}


