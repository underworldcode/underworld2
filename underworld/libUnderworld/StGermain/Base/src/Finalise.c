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
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"
#include "StGermain/Base/Context/src/Context.h"
#include "StGermain/Base/Context/src/Finalise.h"

#include "Finalise.h"

#include <stdio.h>

/* Note: this function is a duplicate of StGermain_Finalise currently - since StGermain_Finalise doesn't call Base_Finalise */
Bool StGermainBase_Finalise( void ) {
   Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 
   
   /* Delete the global objects : xmlSearchPaths and moduleDirectories */
   Stg_ObjectList_DeleteAllObjects( moduleDirectories );
   Stg_Class_Delete( moduleDirectories );
   moduleDirectories=NULL;
   
   Stg_ObjectList_DeleteAllObjects( Project_XMLSearchPaths );
   Stg_Class_Delete( Project_XMLSearchPaths );
   Project_XMLSearchPaths=NULL;

   /* Finalise all the bits and pieces */
   BaseContext_Finalise();
//#if 0
   BaseExtensibility_Finalise();
//#endif
   BaseAutomation_Finalise();
   BaseContainer_Finalise();
   BaseIO_Finalise();
   BaseFoundation_Finalise();

   return True;
}


