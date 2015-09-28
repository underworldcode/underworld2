/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_SwarmMap_h__
#define __PICellerator_MaterialPoints_SwarmMap_h__

	/** Textual name of this class */
	extern const Type SwarmMap_Type;

	/** Virtual function types */


	#define __SwarmMap				     \
		/* General info */			     \
		__Stg_Class;			      	 \
        IntegrationPointsSwarm* swarm;   \
        unsigned elementCount;           \
        int** mapArray;                  

	struct SwarmMap { __SwarmMap };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SWARMMAP_DEFARGS \
                STG_CLASS_DEFARGS, \
                IntegrationPointsSwarm* swarm

	#define SWARMMAP_PASSARGS \
                STG_CLASS_PASSARGS, \
                swarm

	SwarmMap* SwarmMap_New( IntegrationPointsSwarm* swarm );
	SwarmMap* _SwarmMap_New(  SWARMMAP_DEFARGS  );
	void _SwarmMap_Init( SwarmMap* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _SwarmMap_Delete( void* swarmMap );
	void _SwarmMap_Print( void* swarmMap, Stream* stream );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/
    void SwarmMap_Clear( SwarmMap* self ) ;

    void SwarmMap_Insert( SwarmMap* self, unsigned keyElementId, unsigned keyParticleCellId, unsigned insertValue );
    Bool SwarmMap_Map( SwarmMap* self, unsigned keyElementId, unsigned keyParticleCellId, unsigned *returnValue );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __PICellerator_MaterialPoints_SwarmMap_h__ */

