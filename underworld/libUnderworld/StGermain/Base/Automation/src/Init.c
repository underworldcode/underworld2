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

#include "Automation.h"

#include <stdio.h>

Bool BaseAutomation_Init( int* argc, char** argv[] ) 
{
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 

	/** Initializing the Component Register singleton */
	stgComponentRegister = Stg_ComponentRegister_New( );
	
	/** Initializing the Hierarchy Table singleton */
	stgHierarchyTable = HierarchyTable_New();

	/** Initializing the Call Graph singleton */
	stgCallGraph = Stg_CallGraph_New();

	/** Initializing the ComponentRegister singleton */
	
	/** Register Parents for All Classes */
	RegisterParent( Stg_ComponentFactory_Type,           Stg_Class_Type );
	RegisterParent( Stg_ComponentRegister_Type,          Stg_Class_Type );
	RegisterParent( Stg_Component_Type,                  Stg_Object_Type );
	RegisterParent( HierarchyTable_Type,             HashTable_Type );
	RegisterParent( Stg_CallGraph_Type,              Stg_Class_Type );

	return True;
}


