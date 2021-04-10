/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"

#include "units.h"
#include "types.h"
#include "Finalise.h"

#include <stdio.h>

Bool BaseContainer_Finalise( void ) {
	Journal_Printf( Journal_Register( DebugStream_Type, "Context" ), "In: %s\n", __func__ ); 
	
	return True;
}


