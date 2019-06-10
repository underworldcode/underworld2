/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_BaseExtensibility_Init_h__
#define __StGermain_Base_BaseExtensibility_Init_h__

	/** The toolboxes singleton... this is the manager of loading and unloading toolboxes */
	extern ToolboxesManager* stgToolboxesManager;

   ToolboxesManager* GetToolboxManagerInstance();
	Bool BaseExtensibility_Init( int* argc, char** argv[] );
	
#endif /* __StGermain_Base_BaseExtensibility_Init_h__ */
