/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Shape_ShapeClass_h__
#define __StgDomain_Shape_ShapeClass_h__

	typedef Bool (Stg_Shape_IsCoordInsideFunction) ( void* shape, Coord coord );

	typedef void (Stg_Shape_DistanceFromCenterAxisFunction) ( void* shape, Coord coord, double* disVec );

	typedef double (Stg_Shape_CalculateVolumeFunction) ( void* shape );


	/* Textual name of this class */
	extern const Type Stg_Shape_Type;

   /* Stg_Shape information */
#define __Stg_Shape \
   /* General info */ \
   __Stg_Component \
   AbstractContext*                      context;             \
   /* Virtual Info */                                         \
   Stg_Shape_IsCoordInsideFunction*      _isCoordInside;      \
   Stg_Shape_CalculateVolumeFunction*    _calculateVolume;    \
   Stg_Shape_DistanceFromCenterAxisFunction* _distanceFromCenterAxis; \
   /* Other info */                                           \
   Dimension_Index                       dim;                 \
   XYZ                                   centre;              \
   Bool                                  invert;              \
   double                                rotationMatrix[3][3]; \
   double                                rotations[3]; 		/* store the rotations */ \

   struct Stg_Shape { __Stg_Shape };
	
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STG_SHAPE_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                Stg_Shape_IsCoordInsideFunction*                    _isCoordInside, \
                Stg_Shape_CalculateVolumeFunction*                _calculateVolume, \
                Stg_Shape_DistanceFromCenterAxisFunction*  _distanceFromCenterAxis

	#define STG_SHAPE_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        _isCoordInside,          \
	        _calculateVolume,        \
	        _distanceFromCenterAxis

Stg_Shape* _Stg_Shape_New(  STG_SHAPE_DEFARGS  );
	
	void _Stg_Shape_Init( void* shape, Dimension_Index dim, Coord centre, Bool invert, double alpha, double beta, double gamma ) ;
	void Stg_Shape_InitAll( void* shape, Dimension_Index dim, Coord centre, double alpha, double beta, double gamma );

	void _Stg_Shape_Delete( void* shape );
	void _Stg_Shape_Print( void* shape, Stream* stream );
	#define Stg_Shape_Copy( self ) \
		(Stg_Shape*) Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Stg_Shape_DeepCopy( self ) \
		(Stg_Shape*) Stg_Class_Copy( self, NULL, True, NULL, NULL )
	void* _Stg_Shape_Copy( void* shape, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

	/* 'Stg_Component' Virtual Function Implementations */
	void _Stg_Shape_AssignFromXML( void* shape, Stg_ComponentFactory* cf, void* data ) ;
	void _Stg_Shape_Build( void* shape, void* data ) ;
	void _Stg_Shape_Initialise( void* shape, void* data ) ;
	void _Stg_Shape_Execute( void* shape, void* data ) ;
	void _Stg_Shape_Destroy( void* shape, void* data ) ;

	
	/*---------------------------------------------------------------------------------------------------------------------
	** Virtual Function Implementation 
	*/
	Bool Stg_Shape_IsCoordInside( void* shape, Coord coord ) ;

	double Stg_Shape_CalculateVolume( void* shape );
	void Stg_Shape_DistanceFromCenterAxis( void* shape, Coord coord, double* disVec );
	
	/*---------------------------------------------------------------------------------------------------------------------
	** Public member functions
	*/
	void Stg_Shape_TransformCoord( void* shape, Coord coord, Coord transformedCoord ) ;
	void Stg_Shape_TranslateCoord( void* shape, Coord coord, Coord translatedCoord ) ;
	
#endif 

