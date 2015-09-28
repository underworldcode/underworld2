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
#include <gLucifer/gLucifer.h>
#include "Toolbox.h"

const Type gLucifer_Toolbox_Type = "gLucifer_Toolbox";

void _gLucifer_Toolbox_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data )  {
}

void* _gLucifer_Toolbox_DefaultNew( Name name ) {
   return Codelet_New(
      gLucifer_Toolbox_Type,
      _gLucifer_Toolbox_DefaultNew,
      _gLucifer_Toolbox_AssignFromXML,
      _Codelet_Build,
      _Codelet_Initialise,
      _Codelet_Execute,
      _Codelet_Destroy,
      name );
}

void gLucifer_Toolbox_Initialise( ToolboxesManager* toolboxesManager, int* argc, char*** argv ) {
   gLucifer_Init( argc, argv );
}

void gLucifer_Toolbox_Finalise( ToolboxesManager* toolboxesManager ) {
   gLucifer_Finalise();
   
   Journal_RPrintf(
      Journal_Register( Info_Type, (Name)gLucifer_Toolbox_Type  ),
         "Finalised: gLucifer (Visualisation Framework).\n" );
}

Index gLucifer_Toolbox_Register( ToolboxesManager* toolboxesManager ) {
   return ToolboxesManager_Submit( toolboxesManager, gLucifer_Toolbox_Type, (Name)"0", _gLucifer_Toolbox_DefaultNew );
}



