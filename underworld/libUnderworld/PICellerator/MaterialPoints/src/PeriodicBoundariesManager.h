/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_PeriodicBoundariesManager_h__
#define __PICellerator_MaterialPoints_PeriodicBoundariesManager_h__
	
	/* Textual name of this class */
	extern const Type PeriodicBoundariesManager_Type;

	typedef struct PeriodicBoundary {
		Axis				axis; /* Which plane the BC is in */
		double			minWall;
		double			maxWall;
		unsigned int	particlesUpdatedMinEndCount;
		unsigned int	particlesUpdatedMaxEndCount;
	} PeriodicBoundary;

	typedef void (PeriodicBoundary_UpdateParticle)( void* self, Particle_Index particle_i );

	#define __PeriodicBoundariesManager \
		__Stg_Component \
		PICelleratorContext*	context; \
		\
	  PeriodicBoundary_UpdateParticle* _updateParticle; \
		Dictionary*				dictionary; \
		Mesh*						mesh; \
		Index						count; \
		Index						size; \
		Index						delta; \
		PeriodicBoundary*		boundaries; \
		Swarm*					swarm; \
		Stream*					debug; 

	struct PeriodicBoundariesManager { __PeriodicBoundariesManager };



	void* _PeriodicBoundariesManager_DefaultNew( Name name );

	PeriodicBoundariesManager* PeriodicBoundariesManager_New( 
		Name						name,
		PICelleratorContext*	context,
		Mesh*						mesh, 
		Swarm*					swarm,
		Dictionary*				dictionary );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PERIODICBOUNDARIESMANAGER_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define PERIODICBOUNDARIESMANAGER_PASSARGS \
                STG_COMPONENT_PASSARGS

	PeriodicBoundariesManager* _PeriodicBoundariesManager_New(  PERIODICBOUNDARIESMANAGER_DEFARGS  );

	void _PeriodicBoundariesManager_Init(
		void*						periodicBCsManager,
		PICelleratorContext*	context,
		Mesh*						mesh, 
		Swarm*					swarm,
		Dictionary*				dictionary );
		
	void _PeriodicBoundariesManager_AssignFromXML( void* periodicBCsManager, Stg_ComponentFactory* cf, void* data );
	
	void _PeriodicBoundariesManager_Delete( void* context );

	void _PeriodicBoundariesManager_Print( void* context, Stream* stream );

	void* _PeriodicBoundariesManager_Copy( void* periodicBCsManager, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _PeriodicBoundariesManager_Build( void* periodicBCsManager, void* data );

	void _PeriodicBoundariesManager_Initialise( void* periodicBCsManager, void* data );

	void _PeriodicBoundariesManager_Execute( void* periodicBCsManager, void* data );

	void _PeriodicBoundariesManager_Destroy( void* periodicBCsManager, void* data );

	void PeriodicBoundariesManager_AddPeriodicBoundary( void* periodicBCsManager, Axis axis );

	void PeriodicBoundariesManager_UpdateParticle( void* periodicBCsManager, Particle_Index lParticle_I );

#endif

