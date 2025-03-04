/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_ElementCellLayout_h__
#define __StgDomain_Swarm_ElementCellLayout_h__
	

	/* Textual name of this class */
	extern const Type ElementCellLayout_Type;
	
	/* ElementCellLayout information */
	#define __ElementCellLayout \
		/* General info */ \
		__CellLayout \
		\
		/* Virtual info */ \
		\
		/* ElementCellLayout info */ \
		Mesh*			mesh; \
		ShadowInfo	cellShadowInfo; \
		IArray*		incArray; \

	struct ElementCellLayout { __ElementCellLayout };


	
	/* Create a new ElementCellLayout and initialise */
	void* _ElementCellLayout_DefaultNew( Name name );
	
	ElementCellLayout* ElementCellLayout_New( Name name, AbstractContext* context, void* mesh );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ELEMENTCELLLAYOUT_DEFARGS \
                CELLLAYOUT_DEFARGS

	#define ELEMENTCELLLAYOUT_PASSARGS \
                CELLLAYOUT_PASSARGS

	ElementCellLayout* _ElementCellLayout_New(  ELEMENTCELLLAYOUT_DEFARGS  ); 

	/* Initialise implementation */
	void _ElementCellLayout_Init( ElementCellLayout* self, void* mesh );
	
	/* Stg_Class_Delete implementation */
	void _ElementCellLayout_Delete( void* elementCellLayout );
	
	/* Print implementation */
	void _ElementCellLayout_Print( void* elementCellLayout, Stream* stream );
	
	/* Copy */
	#define ElementCellLayout_Copy( self ) \
		(ElementCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ElementCellLayout_DeepCopy( self ) \
		(ElementCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _ElementCellLayout_Copy( void* elementCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _ElementCellLayout_AssignFromXML( void *elementCellLayout, Stg_ComponentFactory *cf, void* data );
	
	void _ElementCellLayout_Build( void *elementCellLayout, void *data );
	
	void _ElementCellLayout_Initialise( void *elementCellLayout, void *data );
	
	void _ElementCellLayout_Execute( void *elementCellLayout, void *data );

	void _ElementCellLayout_Destroy( void *elementCellLayout, void *data );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _ElementCellLayout_CellLocalCount( void* elementCellLayout );
	
	/* Returns the number of cells in this layout*/
	Cell_Index _ElementCellLayout_CellShadowCount( void* elementCellLayout );
	
	/* Returns the cell point count... this is the mesh's element node count */
	Cell_PointIndex _ElementCellLayout_PointCount( void* elementCellLayout, Cell_Index cellIndex );
	
	/* Returns the cell point array... this is the mesh's element node array */
	void _ElementCellLayout_InitialisePoints( void* elementCellLayout, Cell_Index cellIndex, Cell_PointIndex pointCount, Cell_Points points );
	
	/** Implements CellLayout_MapElementIdToCellId(): just returns a cell number, after checking size */
	Cell_Index _ElementCellLayout_MapElementIdToCellId( void* cellLayout, unsigned element_dI ); 
	
	/* Specify whether a particle is in a given cell enough */
	Bool _ElementCellLayout_IsInCell( void* elementCellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in - regular meshes */
	Cell_Index _ElementCellLayout_CellOf( void* elementCellLayout, void* particle );
	/* Obtain which cell a given coord lives in - irregular meshes */
	 Cell_Index _ElementCellLayout_CellOf_Irregular( void* elementCellLayout, void* _particle );

	/* Get the shadow info: uses the mesh's element one */
	ShadowInfo* _ElementCellLayout_GetShadowInfo( void* elementCellLayout );

	void ElementCellLayout_BuildShadowInfo( ElementCellLayout* self );
   void ElementCellLayout_DestroyShadowInfo( ElementCellLayout* self );
	
#endif /* __StgDomain_Swarm_ElementCellLayout_h__ */

