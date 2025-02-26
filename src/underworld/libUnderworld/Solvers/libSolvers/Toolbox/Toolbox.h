/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Solvers_Toolbox_h__
#define __Solvers_Toolbox_h__

extern const Type Solvers_Toolbox_Type;

typedef struct {
	__Codelet
} Solvers_Toolbox;

void Solvers_Toolbox_Initialise();

Index Solvers_Toolbox_Register( PluginsManager* pluginsManager );

void Solvers_Toolbox_Finalise();

char* Solvers_Toolbox_GetDeps() {return "StgDomain StgFEM PICellerator Underworld";};

#endif	
