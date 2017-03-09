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

#include "Base.h"

Stream* lucInfo  = NULL;
Stream* lucDebug = NULL;
Stream* lucError = NULL;

/* Alias placeholders */
const Type lucDefaultWindow_Type = "lucDefaultWindow";

Bool lucBase_Init()
{
   Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

   Journal_Printf( Journal_Register( Debug_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

   /* Set up streams */
   lucInfo  = Journal_Register( Info_Type, (Name)"lucInfo"  );
   /* let's disable lucInfo */
   lucInfo->_enable = (Bool) 0;
   lucDebug = Journal_Register( Debug_Type, (Name)"lucDebug"  );
   lucError = Journal_Register( Error_Type, (Name)"lucError" );

   Stg_ComponentRegister_Add( componentRegister, lucDrawingObject_Type, (Name)"0", _lucDrawingObject_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucColourMap_Type, (Name)"0", _lucColourMap_DefaultNew  );
   Stg_ComponentRegister_Add( componentRegister, lucDatabase_Type, (Name)"0", _lucDatabase_DefaultNew  );

   /* Register Parents for type checking */
   RegisterParent( lucColourMap_Type,         Stg_Component_Type );
   RegisterParent( lucDatabase_Type,          Stg_Component_Type );
   RegisterParent( lucDrawingObject_Type,     Stg_Component_Type );

   RegisterParent( lucDrawingObject_Register_Type, NamedObject_Register_Type );

   return True;
}



