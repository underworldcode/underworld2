/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_SwarmVariable_Register_h__
#define __StgDomain_Swarm_SwarmVariable_Register_h__
	
	
	extern const Type SwarmVariable_Register_Type;
	
	#define __SwarmVariable_Register \
		/* General info */ \
		__NamedObject_Register \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		Variable_Register* variable_Register;

	struct SwarmVariable_Register { __SwarmVariable_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	
	SwarmVariable_Register*	SwarmVariable_Register_New( Variable_Register* variable_Register );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
	#define SwarmVariable_Register_Add NamedObject_Register_Add

	#define SwarmVariable_Register_GetIndex NamedObject_Register_GetIndex 

	#define SwarmVariable_Register_GetByName( self, swarmVariableName ) \
		( (SwarmVariable*) NamedObject_Register_GetByName( self, swarmVariableName ) ) 

	#define SwarmVariable_Register_GetByIndex( self, swarmVariableIndex ) \
		( (SwarmVariable*) NamedObject_Register_GetByIndex( self, swarmVariableIndex ) )

	#define SwarmVariable_Register_PrintAllEntryNames NamedObject_Register_PrintAllEntryNames

#endif /* __StgDomain_Swarm_SwarmVariable_Register_h__ */
