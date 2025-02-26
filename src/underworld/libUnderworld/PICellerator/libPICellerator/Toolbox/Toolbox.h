/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_Toolbox_h__
#define __PICellerator_Toolbox_h__

extern const Type PICellerator_Toolbox_Type;

   typedef struct {
	   __Codelet
   } PICellerator_Toolbox;

   void PICellerator_Toolbox_Initialise();

   Index PICellerator_Toolbox_Register( ToolboxesManager* toolboxesManager );

   void PICellerator_Toolbox_Finalise();

   char* PICellerator_Toolbox_GetDeps() {return "StgDomain StgFEM";};


#endif	
