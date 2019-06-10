/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Toolbox_h__
#define __StgDomain_Toolbox_h__

   extern const Type StgDomain_Toolbox_Type;

   typedef struct {
   __Codelet
   } StgDomain_Toolbox;

   void StgDomain_Toolbox_Initialise();

   Index StgDomain_Toolbox_Register( ToolboxesManager* toolboxesManager );

   void StgDomain_Toolbox_Finalise();

   char* StgDomain_Toolbox_GetDeps() {return "";};

#endif	
