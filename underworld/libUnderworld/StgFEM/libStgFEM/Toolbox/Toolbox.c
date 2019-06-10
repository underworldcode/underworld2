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
#include "Toolbox.h"

const Type StgFEM_Toolbox_Type = "StgFEM_Toolbox";

void _StgFEM_Toolbox_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data )  {
}

void* _StgFEM_Toolbox_DefaultNew( Name name ) {
   return Codelet_New(
      StgFEM_Toolbox_Type,
      _StgFEM_Toolbox_DefaultNew,
      _StgFEM_Toolbox_AssignFromXML,
      _Codelet_Build,
      _Codelet_Initialise,
      _Codelet_Execute,
      _Codelet_Destroy,
      name );
}

void StgFEM_Toolbox_Initialise( ToolboxesManager* toolboxesManager, int* argc, char*** argv ) {
   StgFEM_Init( argc, argv );
}

void StgFEM_Toolbox_Finalise( ToolboxesManager* toolboxesManager ) {
   StgFEM_Finalise();
}

Index StgFEM_Toolbox_Register( ToolboxesManager* toolboxesManager ) {
   return ToolboxesManager_Submit( toolboxesManager, StgFEM_Toolbox_Type, (Name)"0", _StgFEM_Toolbox_DefaultNew );
}


