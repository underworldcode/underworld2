/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_VelicIC_h__
#define __Underworld_VelicIC_h__

extern const Type Underworld_VelicIC_Type;

typedef struct {
	__Codelet
} Underworld_VelicIC;

void Underworld_VelicIC_Sinusoidal( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) ;
void Underworld_VelicIC_Hyperbolic( Node_LocalIndex node_lI, StgVariable_Index var_I, void* _context, void* _data, void* _result ) ;

void _Underworld_VelicIC_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) ;
void* _Underworld_VelicIC_DefaultNew( Name name ) ;
Index Underworld_VelicIC_Register( PluginsManager* pluginsManager );


#endif	
