/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __StgDomain_Swarm_Swarm_Register_h__
#define __StgDomain_Swarm_Swarm_Register_h__

	extern const Type Swarm_Register_Type;
	extern Swarm_Register* stgSwarm_Register;		/**< Swarm register singleton. */
	
	#define __Swarm_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Class info */ \
		Stg_ObjectList*		swarmList;
		
	struct Swarm_Register { __Swarm_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SWARM_REGISTER_DEFARGS \
                STG_CLASS_DEFARGS

	#define SWARM_REGISTER_PASSARGS \
                STG_CLASS_PASSARGS

	Swarm_Register* _Swarm_Register_New(  SWARM_REGISTER_DEFARGS  );
	
	Swarm_Register* Swarm_Register_New();

	void _Swarm_Register_Init( Swarm_Register* self );
	
	void Swarm_Register_Init( Swarm_Register* self );

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Swarm_Register_Delete( void* swarmRegister );
	
	void _Swarm_Register_Print( void* swarmRegister, Stream* stream );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/

	Index Swarm_Register_Add( Swarm_Register* self, void* swarm );
	void Swarm_Register_RemoveIndex( Swarm_Register* self, unsigned int index );
	
	Swarm* Swarm_Register_Get( Swarm_Register* self, Name name );
	Swarm* Swarm_Register_At( void* swarmRegister, Index index ) ;
	
	unsigned int Swarm_Register_GetCount( Swarm_Register* self );
	Index Swarm_Register_GetIndexFromPointer( Swarm_Register* self, void* ptr );
	Swarm_Register* Swarm_Register_GetSwarm_Register();

	void Swarm_Register_SaveAllRegisteredSwarms( Swarm_Register* self, void* context );

#endif /* __StgDomain_Swarm_Swarm_Register_h__ */



