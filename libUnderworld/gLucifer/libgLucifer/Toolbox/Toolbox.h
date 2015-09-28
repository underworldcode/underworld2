/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __gLucifer_Toolbox_h__
#define __gLucifer_Toolbox_h__

   extern const Type gLucifer_Toolbox_Type;

   typedef struct {
	   __Codelet
   } gLucifer_Toolbox;

   void gLucifer_Toolbox_Initialise();

   Index gLucifer_Toolbox_Register( ToolboxesManager* toolboxesManager );

   void gLucifer_Toolbox_Finalise();

   char* gLucifer_Toolbox_GetDeps() {return "StgDomain StgFEM";};


#endif	
