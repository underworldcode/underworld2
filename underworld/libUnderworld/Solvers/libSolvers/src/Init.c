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
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>

#include "Solvers.h"

#include <stdio.h>

/** Initialises this package, then any init for this package
such as streams etc */

Bool Solvers_Init( int* argc, char** argv[] ) {
	/* This init function tells StGermain of all the component types, etc this module contributes. Because it can be linked at compile
	   time or linked in by a toolbox at runtime, we need to make sure it isn't run twice (compiled in and loaded through a toolbox.*/
	if( !ToolboxesManager_IsInitialised( stgToolboxesManager, "Solvers" ) ) {
		int tmp;
		char* directory;

		Solvers_SLE_Init(argc, argv);
		Solvers_Assembly_Init(argc, argv);
		Solvers_KSPSolvers_Init(argc, argv);

                
		

		/* Add the Solvers path to the global xml path dictionary */
		directory = Memory_Alloc_Array( char, 200, "xmlDirectory" ) ;
		sprintf(directory, "%s%s", LIB_DIR, "/StGermain" );
		XML_IO_Handler_AddDirectory( "Solvers", directory );
		Memory_Free(directory);

		/* Add the plugin path to the global plugin list */
		ModulesManager_AddDirectory( "Solvers", LIB_DIR );

                #ifdef HAVE_PETSCEXT
		PetscExtInitialize( );
                #endif
	
		return True;
	}
	return False;
}


