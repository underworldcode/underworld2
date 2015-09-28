/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_PopulationControl_EscapedRoutine_h__
#define __PICellerator_PopulationControl_EscapedRoutine_h__

	typedef void (EscapedRoutine_SelectFunction)( void* escapedRoutine, void* _swarm );

	/* Textual name of this class */
	extern const Type EscapedRoutine_Type;

	/* EscapedRoutine information */
	#define __EscapedRoutine \
		/* General info */ \
		__Stg_Component \
		/* Virtual Info */\
		EscapedRoutine_SelectFunction*	_select; \
		/* Other Info */\
		Stream*									debug; \
		Dimension_Index						dim; \
		/* Removal Info */  \
		Particle_Index							particlesToRemoveCount; \
		Particle_Index							particlesToRemoveAlloced; \
		Particle_Index							particlesToRemoveDelta; \
		unsigned*								particlesToRemoveList;    

	struct EscapedRoutine { __EscapedRoutine };

	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ESCAPEDROUTINE_DEFARGS \
  		STG_COMPONENT_DEFARGS, \
		EscapedRoutine_SelectFunction*  _select

	#define ESCAPEDROUTINE_PASSARGS \
		STG_COMPONENT_PASSARGS, \
		_select

	EscapedRoutine* _EscapedRoutine_New(  ESCAPEDROUTINE_DEFARGS  );

	void* _EscapedRoutine_DefaultNew( Name name );

	void _EscapedRoutine_Init(                                                                                            
		void*             escapedRoutine,
		Dimension_Index   dim,                                                                                             
		Particle_Index    particlesToRemoveDelta );         
	
	/* Stg_Class_Delete EscapedRoutine implementation */
	void _EscapedRoutine_Delete( void* escapedRoutine );

	void _EscapedRoutine_Print( void* escapedRoutine, Stream* stream );

	#define EscapedRoutine_Copy( self ) \
		(EscapedRoutine*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define EscapedRoutine_DeepCopy( self ) \
		(EscapedRoutine*) Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void* _EscapedRoutine_Copy( void* escapedRoutine, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void _EscapedRoutine_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ); 

	void _EscapedRoutine_Build( void* escapedRoutine, void* data );

	void _EscapedRoutine_Initialise( void* escapedRoutine, void* data );

	void _EscapedRoutine_Execute( void* escapedRoutine, void* data );

	void _EscapedRoutine_Destroy( void* escapedRoutine, void* data );
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/

	void EscapedRoutine_Select( void* escapedRoutine, void* _swarm );

	void _EscapedRoutine_Select( void* escapedRoutine, void* _swarm );

	void EscapedRoutine_RemoveFromSwarm( void* escapedRoutine, void* _swarm );

	void EscapedRoutine_InitialiseParticleList( void* escapedRoutine );

	void EscapedRoutine_SetParticleToRemove( void* escapedRoutine, Swarm* swarm, Particle_Index lParticle_I );

	int _EscapedRoutine_CompareParticles( const void* _aParticleInfo, const void* _bParticleInfo ); 

	void EscapedRoutine_SortParticleList( void* escapedRoutine );

	void EscapedRoutine_RemoveParticles( void* escapedRoutine, Swarm* swarm );

#endif 

