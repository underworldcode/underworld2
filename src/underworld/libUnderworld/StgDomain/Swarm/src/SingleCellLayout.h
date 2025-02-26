/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_SingleCellLayout_h__
#define __StgDomain_Swarm_SingleCellLayout_h__
	

	/* Textual name of this class */
	extern const Type SingleCellLayout_Type;
	
	/* SingleCellLayout information */
	#define __SingleCellLayout \
		/* General info */ \
		__CellLayout \
		\
		/* Virtual info */ \
		\
		/* SingleCellLayout info */ \
		Bool            dimExists[3];    /** Defines whether each dimension exists in current sim*/\
		XYZ             min;             /** Max top-right-back corner of cell */\
		XYZ             max;             /** Min bottom-left-front corner of cell */ \
		double**        cellPointCoords; /** Generated cell points in single cell */\
		Cell_PointIndex pointCount;	     /** Count of points in single cell */\

	struct SingleCellLayout { __SingleCellLayout };


	
	/* Create a new SingleCellLayout and initialise */
	
	SingleCellLayout* SingleCellLayout_New( Name name, AbstractContext* context,  const Bool dimExists[3], const XYZ min, const XYZ max ) ;

	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SINGLECELLLAYOUT_DEFARGS \
                CELLLAYOUT_DEFARGS

	#define SINGLECELLLAYOUT_PASSARGS \
                CELLLAYOUT_PASSARGS

	SingleCellLayout* _SingleCellLayout_New(  SINGLECELLLAYOUT_DEFARGS  );

	/* Initialise implementation */
	void _SingleCellLayout_Init( void* cellLayout, const Bool dimExists[3], const XYZ min, const XYZ max );
	
	/* Stg_Class_Delete implementation */
	void _SingleCellLayout_Delete( void* singleCellLayout );

	void _SingleCellLayout_Print( void* singleCellLayout, Stream* stream );

	#define SingleCellLayout_Copy( self ) \
		(SingleCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define SingleCellLayout_DeepCopy( self ) \
		(SingleCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _SingleCellLayout_Copy( void* singleCellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/* 'Stg_Component' Class Info */
	void* _SingleCellLayout_DefaultNew( Name name );

	void _SingleCellLayout_AssignFromXML( void* singleCellLayout, Stg_ComponentFactory* cf, void* data );

	void _SingleCellLayout_Build( void* singleCellLayout, void* data );

	void _SingleCellLayout_Initialise( void* singleCellLayout, void* data );

	void _SingleCellLayout_Execute( void* singleCellLayout, void* data );

	void _SingleCellLayout_Destroy( void* singleCellLayout, void* data );
	
	/** Returns the number of local cells in this cellLayout */
	Cell_Index _SingleCellLayout_CellLocalCount( void* singleCellLayout );
	
	/** Returns the number of shadow cells in this cellLayout */
	Cell_Index _SingleCellLayout_CellShadowCount( void* singleCellLayout );
	
	/** Returns the cell point count... this is the cellLayout's single node count */
	Cell_PointIndex _SingleCellLayout_PointCount( void* singleCellLayout, Cell_Index cellIndex );
	
	/** Returns the cell point array... this is the cellLayout's single node array */
	void _SingleCellLayout_InitialisePoints( void* singleCellLayout, Cell_Index cellIndex, Cell_PointIndex pointCount, Cell_Points points );
	
	/** Implements CellLayout_MapElementIdToCellId(): always return 0, since all elements correspond to the same cell */
	Cell_Index _SingleCellLayout_MapElementIdToCellId( void* cellLayout, unsigned element_dI );
	
	/* Specify whether a particle is in a given cell */
	Bool _SingleCellLayout_IsInCell( void* singleCellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index _SingleCellLayout_CellOf( void* singleCellLayout, void* particle );
	
	/* Get the shadow info */
	ShadowInfo* _SingleCellLayout_GetShadowInfo( void* singleCellLayout );
	
	/* --- Private Functions --- */

	void _SingleCellLayout_CalculateGlobalPointCount( SingleCellLayout* self );

	void _SingleCellLayout_InitialiseGlobalCellPointPositions( SingleCellLayout* self );
	
#endif /* __StgDomain_Swarm_SingleCellLayout_h__ */

