/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_SwarmAdvector_h__
#define __PICellerator_MaterialPoints_SwarmAdvector_h__

	/* Textual name of this class */
	extern const Type SwarmAdvector_Type;

	/* SwarmAdvector information */
	#define __SwarmAdvector \
		/* General info */ \
		__TimeIntegrand \
		/* Virtual Info */\
		/* Other Info */\
		GeneralSwarm*                  swarm;                \
		FeVariable*                           velocityField;        \
		PeriodicBoundariesManager*            periodicBCsManager;   \

	struct SwarmAdvector { __SwarmAdvector };
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	SwarmAdvector* SwarmAdvector_New(
		Name                                       name,
		DomainContext*                             context,
		TimeIntegrator*                            timeIntegrator,
		FeVariable*                                velocityField,
		Bool                                       allowFallbackToFirstOrder,
		GeneralSwarm*                       swarm,
		PeriodicBoundariesManager*                 periodicBCsManager );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SWARMADVECTOR_DEFARGS \
                TIMEINTEGRAND_DEFARGS

	#define SWARMADVECTOR_PASSARGS \
                TIMEINTEGRAND_PASSARGS

	SwarmAdvector* _SwarmAdvector_New(  SWARMADVECTOR_DEFARGS  );

	void _SwarmAdvector_Init( 
		SwarmAdvector*                             self,
		FeVariable*                                velocityField,
		GeneralSwarm*                       swarm,
		PeriodicBoundariesManager*                 periodicBCsManager );

	void _SwarmAdvector_Delete( void* materialSwarm );
	void _SwarmAdvector_Print( void* materialSwarm, Stream* stream );
	#define SwarmAdvector_Copy( self ) \
		(SwarmAdvector*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define SwarmAdvector_DeepCopy( self ) \
		(SwarmAdvector*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _SwarmAdvector_Copy( void* materialSwarm, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _SwarmAdvector_DefaultNew( Name name ) ;
void _SwarmAdvector_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _SwarmAdvector_Build( void* materialSwarm, void* data ) ;
	void _SwarmAdvector_Initialise( void* materialSwarm, void* data ) ;
	void _SwarmAdvector_Execute( void* materialSwarm, void* data );
	void _SwarmAdvector_Destroy( void* materialSwarm, void* data ) ;
	Bool _SwarmAdvector_TimeDeriv( void* swarmAdvector, Index array_I, double* timeDeriv ) ;
   Bool _SwarmAdvector_TimeDeriv_Quicker4IrregularMesh( void* swarmAdvector, Index array_I, double* timeDeriv );
	void _SwarmAdvector_Intermediate( void* swarmAdvector, Index array_I ) ;
	
		
	/*---------------------------------------------------------------------------------------------------------------------
	** Private functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Entry Point Hooks
	*/
    double SwarmAdvector_MaxDt( void* swarmAdvector ) ;

	/*---------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

#endif 

