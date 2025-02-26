/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_CoincidentMapper_h__
#define __PICellerator_MaterialPoints_CoincidentMapper_h__

	/* Textual name of this class */
	extern const Type CoincidentMapper_Type;

	#define __CoincidentMapper \
		__Stg_Component \
		IntegrationPointsSwarm*												integrationSwarm; \
		GeneralSwarm*	materialSwarm; \

	struct CoincidentMapper { __CoincidentMapper };

	#ifndef ZERO
	#define ZERO 0
	#endif

	#define COINCIDENTMAPPER_DEFARGS \
                STG_COMPONENT_DEFARGS

	#define COINCIDENTMAPPER_PASSARGS \
                STG_COMPONENT_PASSARGS

	CoincidentMapper* _CoincidentMapper_New(  COINCIDENTMAPPER_DEFARGS  );

	void _CoincidentMapper_Delete( void* mapper );

	#define CoincidentMapper_Copy( self ) \
		(CoincidentMapper*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CoincidentMapper_DeepCopy( self ) \
		(CoincidentMapper*) Stg_Class_Copy( self, NULL, True, NULL, NULL )

	void* _CoincidentMapper_DefaultNew( Name name );

	void _CoincidentMapper_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data );

	void _CoincidentMapper_Build( void* mapper, void* data );

	void _CoincidentMapper_Initialise( void* mapper, void* data );

	void _CoincidentMapper_Map( void* mapper );
	
#endif

