/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/libStgFEM/src/StgFEM.h>
#include <PICellerator/libPICellerator/src/PICellerator.h>
#include <Underworld/libUnderworld/src/Underworld.h>
#include <Solvers/libSolvers/src/Solvers.h>
#include "Toolbox.h"

const Type Solvers_Toolbox_Type = "Solvers_Toolbox";

void _Solvers_Toolbox_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* ptrToContext ) {
	/* Do nothing! */
}


void* _Solvers_Toolbox_DefaultNew( Name name ) {
	return Codelet_New(
			Solvers_Toolbox_Type,
			_Solvers_Toolbox_DefaultNew,
			_Solvers_Toolbox_AssignFromXML,
			_Codelet_Build,
			_Codelet_Initialise,
			_Codelet_Execute,
			_Codelet_Destroy,
			name );
}

void Solvers_Toolbox_Initialise( PluginsManager* pluginsManager, int* argc, char*** argv ) {
	Solvers_Init( argc, argv );
}

void Solvers_Toolbox_Finalise( PluginsManager* pluginsManager ) {
	Solvers_Finalise();
}

Index Solvers_Toolbox_Register( PluginsManager* pluginsManager ) {
	return PluginsManager_Submit( pluginsManager, Solvers_Toolbox_Type, (Name)"0", _Solvers_Toolbox_DefaultNew  );
}


