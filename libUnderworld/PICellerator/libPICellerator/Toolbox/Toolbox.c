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
#include "Toolbox.h"

const Type PICellerator_Toolbox_Type = "PICellerator_Toolbox";

void _PICellerator_Toolbox_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
}

void* _PICellerator_Toolbox_DefaultNew( Name name ) {
   return Codelet_New(
      PICellerator_Toolbox_Type,
      _PICellerator_Toolbox_DefaultNew,
      _PICellerator_Toolbox_AssignFromXML,
      _Codelet_Build,
      _Codelet_Initialise,
      _Codelet_Execute,
      _Codelet_Destroy,
      name );
}

void PICellerator_Toolbox_Initialise( ToolboxesManager* toolboxesManager, int* argc, char*** argv ) {
   PICellerator_Init( argc, argv );
}

void PICellerator_Toolbox_Finalise( ToolboxesManager* toolboxesManager ) {
   PICellerator_Finalise();
}

Index PICellerator_Toolbox_Register( ToolboxesManager* toolboxesManager ) {
   return ToolboxesManager_Submit( toolboxesManager, PICellerator_Toolbox_Type, (Name)"0", _PICellerator_Toolbox_DefaultNew );
}



