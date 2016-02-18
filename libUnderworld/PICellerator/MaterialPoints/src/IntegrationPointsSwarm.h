/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_IntegrationPointsSwarm_h__
#define __PICellerator_MaterialPoints_IntegrationPointsSwarm_h__

    #include <PICellerator/PopulationControl/PopulationControl.h>
    #include <PICellerator/Weights/Weights.h>


	/* Textual name of this class */
	extern const Type IntegrationPointsSwarm_Type;

	/* IntegrationPointsSwarm information */
	#define __IntegrationPointsSwarm \
		__Swarm \
		\
		FeMesh*                               mesh;                 \
		WeightsCalculator*                    weights;              \
		SwarmVariable*                        localCoordVariable;    /** Set only if a local coord system swarm. */ \
		SwarmVariable*                        weightVariable;       \
        List*                                 swarmsMappedTo;


	struct IntegrationPointsSwarm { __IntegrationPointsSwarm };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	
	void* _IntegrationPointsSwarm_DefaultNew( Name name ) ;

	/** Private New */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define INTEGRATIONPOINTSSWARM_DEFARGS \
                SWARM_DEFARGS

	#define INTEGRATIONPOINTSSWARM_PASSARGS \
                SWARM_PASSARGS

	IntegrationPointsSwarm* _IntegrationPointsSwarm_New(  INTEGRATIONPOINTSSWARM_DEFARGS  );

	void _IntegrationPointsSwarm_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;

	void _IntegrationPointsSwarm_Init(
		void*                                           swarm,
		FeMesh*                             mesh, 
		WeightsCalculator*                              weights );

	/* Stg_Class_Delete IntegrationPointsSwarm implementation */
	void _IntegrationPointsSwarm_Delete( void* integrationPoints );
	void _IntegrationPointsSwarm_Print( void* integrationPoints, Stream* stream );
	#define IntegrationPointsSwarm_Copy( self ) \
		(IntegrationPointsSwarm*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define IntegrationPointsSwarm_DeepCopy( self ) \
		(IntegrationPointsSwarm*) Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void _IntegrationPointsSwarm_Build( void* integrationPoints, void* data ) ;
	void _IntegrationPointsSwarm_Initialise( void* integrationPoints, void* data ) ;
	void _IntegrationPointsSwarm_Execute( void* integrationPoints, void* data );
	void _IntegrationPointsSwarm_Destroy( void* integrationPoints, void* data ) ;

	void _IntegrationPointsSwarm_UpdateHook( void* timeIntegrator, void* swarm );
	
    void IntegrationPointsSwarm_ClearSwarmMaps( void* integrationPoints );

#endif

