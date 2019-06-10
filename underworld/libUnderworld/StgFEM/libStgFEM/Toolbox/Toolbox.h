/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Toolbox_h__
#define __StgFEM_Toolbox_h__

   extern const Type StgFEM_Toolbox_Type;

   typedef struct {
      __Codelet
   } StgFEM_Toolbox;

   void StgFEM_Toolbox_Initialise();

   Index StgFEM_Toolbox_Register( ToolboxesManager* toolboxesManager );

   void StgFEM_Toolbox_Finalise();

   char* StgFEM_Toolbox_GetDeps() {return "StgDomain";};

#endif	
