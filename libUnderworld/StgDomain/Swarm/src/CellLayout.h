/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_CellLayout_h__
#define __StgDomain_Swarm_CellLayout_h__
	
	/* Child classes must define these abstract functions */
	typedef Cell_Index (CellLayout_CellCountFunction) ( void* cellLayout );
	typedef Cell_PointIndex	(CellLayout_PointCountFunction) ( void* cellLayout, Cell_Index cell_I );
	typedef void (CellLayout_InitialisePointsFunction)	( void* cellLayout, 
		Cell_Index cell_I, 
		Cell_PointIndex pointCount, 
		Cell_Points points );
	typedef Cell_Index (CellLayout_MapElementIdToCellIdFunction) ( void* cellLayout, unsigned element_dI ); 
	typedef Bool (CellLayout_IsInCellFunction) ( void* cellLayout, Cell_Index cell_I, void* particle );
	typedef Cell_Index (CellLayout_CellOfFunction) ( void* cellLayout, void* particle );
	typedef ShadowInfo* (CellLayout_GetShadowInfoFunction) ( void* cellLayout );
	
	/* Textual name of this class */
	extern const Type CellLayout_Type;
	
	/* CellLayout information */
	#define __CellLayout \
		/* General info */ \
		__Stg_Component\
		\
		AbstractContext*									context; \
		/* Virtual info */ \
		CellLayout_CellCountFunction*					_cellLocalCount; \
		CellLayout_CellCountFunction*					_cellShadowCount; \
		CellLayout_PointCountFunction*				_pointCount; \
		CellLayout_InitialisePointsFunction*		_initialisePoints; \
		CellLayout_MapElementIdToCellIdFunction*	_mapElementIdToCellId; \
		CellLayout_IsInCellFunction*					_isInCell; \
		CellLayout_CellOfFunction*						_cellOf; \
		CellLayout_GetShadowInfoFunction*			_getShadowInfo; \
		\
		/* CellLayout info */ \

	struct CellLayout { __CellLayout };


	
	/* No "CellLayout_New" and "CellLayout_Init" as this is an abstract class */
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CELLLAYOUT_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                CellLayout_CellCountFunction*                   _cellLocalCount, \
                CellLayout_CellCountFunction*                  _cellShadowCount, \
                CellLayout_PointCountFunction*                      _pointCount, \
                CellLayout_InitialisePointsFunction*          _initialisePoints, \
                CellLayout_MapElementIdToCellIdFunction*  _mapElementIdToCellId, \
                CellLayout_IsInCellFunction*                          _isInCell, \
                CellLayout_CellOfFunction*                              _cellOf, \
                CellLayout_GetShadowInfoFunction*                _getShadowInfo

	#define CELLLAYOUT_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _cellLocalCount,       \
	        _cellShadowCount,      \
	        _pointCount,           \
	        _initialisePoints,     \
	        _mapElementIdToCellId, \
	        _isInCell,             \
	        _cellOf,               \
	        _getShadowInfo       

	CellLayout* _CellLayout_New(  CELLLAYOUT_DEFARGS  ); 

	/* Initialise implementation */
	void _CellLayout_Init( CellLayout* self, AbstractContext* context );
	
	/* Stg_Class_Delete a CellLayout construct */
	void _CellLayout_Delete( void* cellLayout );
	
	/* Print a CellLayout construct */
	void _CellLayout_Print( void* cellLayout, Stream* stream );
	
	/* CellLayout Destroy function. */
	void _CellLayout_Destroy( void* cellLayout, void* data );

	/* Copy */
	#define CellLayout_Copy( self ) \
		(CellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CellLayout_DeepCopy( self ) \
		(CellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _CellLayout_Copy( void* cellLayout, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	void _CellLayout_AssignFromXML( void* cellLayout, Stg_ComponentFactory *cf, void* data );	
	
	/* Get the cell counts */
	Cell_Index CellLayout_CellDomainCount( void* cellLayout );
	
	Cell_Index CellLayout_CellLocalCount( void* cellLayout );
	
	Cell_Index CellLayout_CellShadowCount( void* cellLayout );
	
	/** Get the point count for a given cell */
	Cell_PointIndex CellLayout_PointCount( void* cellLayout, Cell_Index cell_I );
	
	/** Initialise cell points (geometry) */
	void CellLayout_InitialiseCellPoints( void* cellLayout, Cell_Index cell_I, Cell_PointIndex pointCount, Cell_Points points );

	/** Get the Cell ID corresponding to a mesh element (only makes sense when the cell layout is used with a mesh
	 * in the same app */
	Cell_Index CellLayout_MapElementIdToCellId( void* cellLayout, unsigned element_dI ); 

	/* Specify whether a particle is in a given cell */
	Bool CellLayout_IsInCell( void* cellLayout, Cell_Index cellIndex, void* particle );
	
	/* Obtain which cell a given coord lives in */
	Cell_Index CellLayout_CellOf( void* cellLayout, void* particle );

	ShadowInfo* CellLayout_GetShadowInfo( void* cellLayout );
	
#endif /* __StgDomain_Swarm_CellLayout_h__ */

