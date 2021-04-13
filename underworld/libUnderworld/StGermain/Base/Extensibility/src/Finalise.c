/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "ExtensionManager_Register.h"
#include "Init.h"
#include "Finalise.h"

#include <stdio.h>

Bool BaseExtensibility_Finalise( void ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 
	
	Stg_Class_Delete( stgToolboxesManager );
	stgToolboxesManager = NULL;
	Stg_Class_Delete( extensionMgr_Register );
	extensionMgr_Register = NULL;
	
	return True;
}


