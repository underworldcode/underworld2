/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Shape_PolygonShapeClass_h__
#define __StgDomain_Shape_PolygonShapeClass_h__

	/* Textual name of this class */
	extern const Type PolygonShape_Type;

	/* PolygonShape information */
	#define __PolygonShape \
		/* General info */ \
		__Stg_Shape \
		/* Virtual Info */\
		\
		Coord_List              vertexList;    \
		Index                   vertexCount;   \
		XYZ                     start;        \
		XYZ                     end;          \
		XYZ                     centroid;     \

	struct PolygonShape { __PolygonShape };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	PolygonShape* PolygonShape_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount,
		XYZ                                   start,
		XYZ                                   end );
		
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define POLYGONSHAPE_DEFARGS \
                STG_SHAPE_DEFARGS

	#define POLYGONSHAPE_PASSARGS \
                STG_SHAPE_PASSARGS

	PolygonShape* _PolygonShape_New(  POLYGONSHAPE_DEFARGS  );
	
	void _PolygonShape_Init( void* polygon, Coord_List vertexList, Index vertexCount, XYZ start, XYZ end ) ;
		
	/* Stg_Class_Delete PolygonShape implementation */
	void _PolygonShape_Delete( void* polygon );
	void _PolygonShape_Print( void* polygon, Stream* stream );
	#define PolygonShape_Copy( self ) \
		(PolygonShape*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define PolygonShape_DeepCopy( self ) \
		(PolygonShape*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _PolygonShape_Copy( void* polygon, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _PolygonShape_DefaultNew( Name name ) ;
	void _PolygonShape_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _PolygonShape_Build( void* polygon, void* data ) ;
	void _PolygonShape_Initialise( void* polygon, void* data ) ;
	void _PolygonShape_Execute( void* polygon, void* data );
	void _PolygonShape_Destroy( void* polygon, void* data ) ;
	
	Bool _PolygonShape_IsCoordInside( void* polygon, const Coord coord ) ;
	double _PolygonShape_CalculateVolume( void* polygon );
	void _PolygonShape_DistanceFromCenterAxis( void* shape, Coord coord, double* disVec );

	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif 

