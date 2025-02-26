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

#include "types.h"
#include "shortcuts.h"
#include "Finalise.h"

#include "Stg_Component.h"
#include "Stg_ComponentRegister.h"
#include "HierarchyTable.h"
#include "CallGraph.h"
#include <stdio.h>

Bool BaseAutomation_Finalise( void ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 
	
	/* Clean up global variable */
	Stg_Class_Delete( stgComponentRegister );
	stgComponentRegister=NULL;
	Stg_Class_Delete( stgHierarchyTable );
	stgHierarchyTable=NULL;
	Stg_Class_Delete( stgCallGraph );
	stgCallGraph=NULL;

	return True;
}


