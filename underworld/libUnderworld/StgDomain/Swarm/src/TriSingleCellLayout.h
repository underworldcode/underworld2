/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_TriSingleCellLayout_h__
#define __StgDomain_Swarm_TriSingleCellLayout_h__

	
	/* Textual name of this class */
	extern const Type TriSingleCellLayout_Type;
	
	/* TriSingleCellLayout information */
	#define __TriSingleCellLayout \
		/* General info */ \
		__CellLayout \
		\
		/* Virtual info */ \
		\
		/* TriSingleCellLayout info */ \
		Dictionary*	dictionary; \
		int			dim;

	struct TriSingleCellLayout { __TriSingleCellLayout };


	
	/* Create a new TriSingleCellLayout and initialise */
	TriSingleCellLayout* _TriSingleCellLayout_DefaultNew( Name name );
	
	TriSingleCellLayout* TriSingleCellLayout_New( Name name, AbstractContext* context, int dim, Dictionary* dictionary );
		
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define TRISINGLECELLLAYOUT_DEFARGS \
                CELLLAYOUT_DEFARGS, \
                Dictionary*  dictionary

	#define TRISINGLECELLLAYOUT_PASSARGS \
                CELLLAYOUT_PASSARGS, \
	        dictionary

	TriSingleCellLayout* _TriSingleCellLayout_New(  TRISINGLECELLLAYOUT_DEFARGS  );
	
	/* Initialise implementation */
	
	/* Stg_Class_Delete implementation */
	void _TriSingleCellLayout_Delete( void* triSingleCellLayout );
	
	/* Print implementation */
	void _TriSingleCellLayout_Print( void* triSingleCellLayout, Stream* stream );
	
	/* Copy */
	#define TriSingleCellLayout_Copy( self ) \
		(TriSingleCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define TriSingleCellLayout_DeepCopy( self ) \
		(TriSingleCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _TriSingleCellLayout_Copy( void* triSingleCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _TriSingleCellLayout_AssignFromXML( void *triSingleCellLayout, Stg_ComponentFactory *cf, void* data );
	
	void _TriSingleCellLayout_Build( void* triSingleCellLayout, void* data );
	
	void _TriSingleCellLayout_Initialise( void* triSingleCellLayout, void* data );
	
	void _TriSingleCellLayout_Execute( void* triSingleCellLayout, void* data );
	
	void _TriSingleCellLayout_Destroy( void* triSingleCellLayout, void* data );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _TriSingleCellLayout_CellLocalCount( void* triSingleCellLayout );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _TriSingleCellLayout_CellShadowCount( void* triSingleCellLayout );
	
	/* Returns the cell point count... this is the layout's single node count */
	Cell_PointIndex _TriSingleCellLayout_PointCount( void* triSingleCellLayout, Cell_Index cellIndex );
	
	/* Returns the cell point array... this is the layout's single node array */
	void _TriSingleCellLayout_InitialisePoints( 
		void*			triSingleCellLayout, 
		Cell_Index		cellIndex, 
		Cell_PointIndex		pointCount, 
		Cell_Points		points );
	
	/** Implements CellLayout_MapElementIdToCellId(): always return 0, since all elements correspond to the same cell */
	Cell_Index _TriSingleCellLayout_MapElementIdToCellId( void* cellLayout, unsigned element_dI );		

	/* Specify whether a particle is in a given cell */
	Bool _TriSingleCellLayout_IsInCell( void* triSingleCellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index _TriSingleCellLayout_CellOf( void* triSingleCellLayout, void* particle );
	
	/* Get the shadow info */
	ShadowInfo* _TriSingleCellLayout_GetShadowInfo( void* triSingleCellLayout );
	
#endif /* __StgDomain_Swarm_TriSingleCellLayout_h__ */

