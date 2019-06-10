/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Toolbox_h__
#define __Underworld_Toolbox_h__

extern const Type Underworld_Toolbox_Type;

typedef struct {
	__Codelet
} Underworld_Toolbox;

void Underworld_Toolbox_Initialise();

Index Underworld_Toolbox_Register( ToolboxesManager* toolboxesManager );

void Underworld_Toolbox_Finalise();

char* Underworld_Toolbox_GetDeps() {return "StgDomain StgFEM PICellerator";};

#endif	
