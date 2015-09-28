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
#include "Toolbox.h"


const Type StgDomain_Toolbox_Type = "StgDomain_Toolbox";

void _StgDomain_Toolbox_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) { }

void* _StgDomain_Toolbox_DefaultNew( Name name ) {
   return Codelet_New(
      StgDomain_Toolbox_Type,
      _StgDomain_Toolbox_DefaultNew,
      _StgDomain_Toolbox_AssignFromXML,
      _Codelet_Build,
      _Codelet_Initialise,
      _Codelet_Execute,
      _Codelet_Destroy,
      name );
}

void StgDomain_Toolbox_Initialise( ToolboxesManager* toolboxesManager, int* argc, char*** argv ) {
   StgDomain_Init( argc, argv );
}

void StgDomain_Toolbox_Finalise( ToolboxesManager* toolboxesManager ) {
   StgDomain_Finalise();
   
   Journal_RPrintf(
      Journal_Register( Info_Type, (Name)StgDomain_Toolbox_Type ),
         "Finalised: StgDomain (Domain Library).\n" );
}

Index StgDomain_Toolbox_Register( ToolboxesManager* toolboxesManager ) {
   return ToolboxesManager_Submit( toolboxesManager, StgDomain_Toolbox_Type, (Name)"0", _StgDomain_Toolbox_DefaultNew );
}


