/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgDomain_Geometry_VectorMath_h__
#define __StgDomain_Geometry_VectorMath_h__

	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/

	#define Vec_Set2D( dst, src )			\
		((dst)[0] = (src)[0],			\
		 (dst)[1] = (src)[1], 0)

	#define Vec_Set3D( dst, src )			\
		(Vec_Set2D( dst, src ),			\
		 (dst)[2] = (src)[2], 0)

	#define Vec_SetScalar2D( dst, x, y )		\
		((dst)[0] = x,				\
		 (dst)[1] = y, 0)

	#define Vec_SetScalar3D( dst, x, y, z )		\
		(Vec_SetScalar2D( dst, x, y ),		\
		 (dst)[2] = z, 0)

	#define Vec_Add2D( dst, a, b )			\
		((dst)[0] = (a)[0] + (b)[0],		\
		 (dst)[1] = (a)[1] + (b)[1], 0)

	#define Vec_Add3D( dst, a, b )			\
		(Vec_Add2D( dst, a, b ),		\
		 (dst)[2] = (a)[2] + (b)[2], 0)

	#define Vec_Sub2D( dst, a, b )			\
		((dst)[0] = (a)[0] - (b)[0],		\
		 (dst)[1] = (a)[1] - (b)[1], 0)

	#define Vec_Sub3D( dst, a, b )			\
		(Vec_Sub2D( dst, a, b ),		\
		 (dst)[2] = (a)[2] - (b)[2], 0)

	#define Vec_Dot2D( a, b )			\
		((a)[0] * (b)[0] + (a)[1] * (b)[1])

	#define Vec_Dot3D( a, b )			\
		(Vec_Dot2D( a, b ) + (a)[2] * (b)[2])

	#define Vec_Scale2D( dst, a, s )		\
		((dst)[0] = (a)[0] * (s),		\
		 (dst)[1] = (a)[1] * (s), 0)

	#define Vec_Scale3D( dst, a, s )		\
		(Vec_Scale2D( dst, a, s ),		\
		 (dst)[2] = (a)[2] * (s), 0)

	#define Vec_MagSq2D( a )			\
		Vec_Dot2D( a, a )

	#define Vec_MagSq3D( a )			\
		Vec_Dot3D( a, a )

	#define Vec_Mag2D( a )				\
		sqrt( Vec_MagSq2D( a ) )

	#define Vec_Mag3D( a )				\
		sqrt( Vec_MagSq3D( a ) )

	#define Vec_Proj2D( dst, a, b )					\
		(Vec_Norm2D( dst, b ),					\
		 Vec_Scale2D( dst, dst, Vec_Dot2D( a, b ) ), 0)

	#define Vec_Proj3D( dst, a, b )					\
		(Vec_Norm3D( dst, b ),					\
		 Vec_Scale3D( dst, dst, Vec_Dot3D( a, b ) ), 0)


	/*--------------------------------------------------------------------------------------------------------------------------
	** Functions
	*/

	void Vec_Cross3D( double* dst, double* a, double* b );
	void Vec_Div2D( double* dst, double* a, double s );
	void Vec_Div3D( double* dst, double* a, double s );
	void Vec_Norm2D( double* dst, double* a );
	void Vec_Norm3D( double* dst, double* a );

	void StGermain_RotateVector(double* rotatedVector, double* vector, double* w, double theta) ;
	void StGermain_RotateCoordinateAxis( double* rotatedVector, double* vector, Index axis, double theta ) ;
	void StGermain_VectorSubtraction(double* destination, double* vector1, double* vector2, Index dim) ;
	void StGermain_VectorAddition(double* destination, double* vector1, double* vector2, Index dim) ;
	double StGermain_VectorMagnitude(double* vector, Index dim) ;
	double StGermain_VectorDotProduct(double* vector1, double* vector2, Index dim) ;
	void StGermain_VectorCrossProduct(double* destination, double* vector1, double* vector2) ;
	double StGermain_VectorCrossProductMagnitude( double* vector1, double* vector2, Dimension_Index dim ) ;
	double StGermain_ScalarTripleProduct( double* vectorA, double* vectorB, double* vectorC ) ;

	void StGermain_VectorNormalise(double* vector, Index dim) ;
	double StGermain_AngleBetweenVectors( double* vectorA, double* vectorB, Index dim ) ;
	double StGermain_DistanceBetweenPoints( double* pos1, double* pos2, Index dim) ;
	void StGermain_NormalToPlane( double* normal, double* pos0, double* pos1, double* pos2) ;

	void StGermain_TriangleCentroid( double* centroid, double* pos0, double* pos1, double* pos2, Index dim) ;
	double StGermain_TriangleArea( double* pos0, double* pos1, double* pos2, Index dim ) ;
	double StGermain_ConvexQuadrilateralArea( double* vertexCoord1, double* vertexCoord2, 
						  double* vertexCoord3, double* vertexCoord4, 
						  Dimension_Index dim ) ;
	double StGermain_ParallelepipedVolume( double* coordLeftBottomFront, 
					       double* coordRightBottomFront, 
					       double* coordLeftTopFront, 
					       double* coordLeftBottomBack );
	double StGermain_ParallelepipedVolumeFromCoordList( Coord_List list ) ;
	
	void StGermain_AverageCoord( double* coord, double** coordList, Index count, Dimension_Index dim ) ;
	void StGermain_PrintVector( Stream* stream, double* vector, Index dim ) ;

   void StGermain_RotateAroundPoint( Coord coord, double matrix[][3], double centre[3], Coord transformedCoord );

	/** Print a named vector. Name comes from vector variable in file*/
	#define StGermain_PrintNamedVector(stream, vector, dim)		\
		do {							\
			Journal_Printf( stream, #vector " - " );	\
			StGermain_PrintVector( stream, vector, dim );	\
		} while(0)

#endif /* __StgDomain_Geometry_VectorMath_h__ */
