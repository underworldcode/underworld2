/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Shape_ConvexHullClass_h__
#define __StgDomain_Shape_ConvexHullClass_h__

	/* Textual name of this class */
	extern const Type ConvexHull_Type;

	/* ConvexHull information */
	#define __ConvexHull \
		/* General info */ \
		__Stg_Shape \
		/* Virtual Info */\
		\
		Coord_List              vertexList;    \
		Index                   vertexCount;   \
		XYZ*                    facesList;     \

	struct ConvexHull { __ConvexHull };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/
	ConvexHull* ConvexHull_New(
		Name                                  name,
		Dimension_Index                       dim,
		XYZ                                   centre, 
		double                                alpha,
		double                                beta,
		double                                gamma,
		Coord_List                            vertexList,
		Index                                 vertexCount);
		
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CONVEXHULL_DEFARGS \
                STG_SHAPE_DEFARGS

	#define CONVEXHULL_PASSARGS \
                STG_SHAPE_PASSARGS

	ConvexHull* _ConvexHull_New(  CONVEXHULL_DEFARGS  );
	
	void _ConvexHull_Init( void* convexHull, Coord_List vertexList, Index vertexCount);
		
	/* Stg_Class_Delete ConvexHull implementation */
	void _ConvexHull_Delete( void* convexHull );
	void _ConvexHull_Print( void* convexHull, Stream* stream );
	#define ConvexHull_Copy( self ) \
		(ConvexHull*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define ConvexHull_DeepCopy( self ) \
		(ConvexHull*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _ConvexHull_Copy( void* convexHull, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	void* _ConvexHull_DefaultNew( Name name ) ;
	void _ConvexHull_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _ConvexHull_Build( void* convexHull, void* data ) ;
	void _ConvexHull_Initialise( void* convexHull, void* data ) ;
	void _ConvexHull_Execute( void* convexHull, void* data );
	void _ConvexHull_Destroy( void* convexHull, void* data ) ;
	
	Bool _ConvexHull_IsCoordInside( void* convexHull, Coord coord ) ;
	double _ConvexHull_CalculateVolume( void* convexHull );
	void _ConvecHull_DistanceFromCenterAxis( void* self, Coord coord, double* disVec );

	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/
	
	
#endif 

