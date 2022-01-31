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
#include "StGermain/Base/Extensibility/src/Extensibility.h"

#include "types.h"
#include "VariableCondition.h"
#include "VariableCondition_Register.h"
#include "ConditionFunction_Register.h"
#include "Finalise.h"

#include <stdio.h>

Bool BaseContext_Finalise( void ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 
	
	Stg_Class_Delete( condFunc_Register );
	condFunc_Register = NULL;
	Stg_Class_Delete( variableCondition_Register );
	variableCondition_Register = NULL;

	return True;
}

