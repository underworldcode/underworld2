/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_Grid_h__
#define __StgDomain_Mesh_Grid_h__

	/** Textual name of this class */
	extern const Type Grid_Type;

	/** Virtual function types */

	/** Grid class contents */
	#define __Grid				\
		/* General info */		\
		__Stg_Class			\
						\
		/* Virtual info */		\
						\
		/* Grid info */			\
		unsigned	nDims;		\
		unsigned*	sizes;		\
		unsigned*	basis;		\
		unsigned	nPoints;	\
						\
		HashTable*	map;		\
		HashTable*	invMap;

	struct Grid { __Grid };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define GRID_DEFARGS \
                STG_CLASS_DEFARGS

	#define GRID_PASSARGS \
                STG_CLASS_PASSARGS

	Grid* Grid_New();
	Grid* _Grid_New(  GRID_DEFARGS  );
	void _Grid_Init( Grid* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Grid_Delete( void* grid );
	void _Grid_Print( void* grid, Stream* stream );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Grid_SetNumDims( void* grid, unsigned nDims );
	void Grid_SetSizes( void* grid, unsigned* sizes );
	void Grid_SetMapping( void* grid, HashTable* mapping, HashTable* inverse );

	unsigned Grid_GetNumDims( void* grid );
	unsigned* Grid_GetSizes( void* grid );
	unsigned Grid_GetNumPoints( void* grid );
	void Grid_GetMapping( void* grid, HashTable** mapping, HashTable** inverse );

	void Grid_Lift( void* grid, unsigned ind, unsigned* params );
	unsigned Grid_Project( void* grid, unsigned* params );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgDomain_Mesh_Grid_h__ */

