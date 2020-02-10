/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "types.h"
#include "Memory.h"
#include "TimeMonitor.h"
#include "Init.h"

#include <stdio.h>
#include <assert.h>

/* constants */
const int DEFAULT_LIST_INITIAL_SIZE = 8;
const int DEFAULT_LIST_DELTA = 8;

Bool BaseFoundation_Init( int* argc, char** argv[] ) {

	Stg_TimeMonitor_Initialise();
	
	return True;
}


