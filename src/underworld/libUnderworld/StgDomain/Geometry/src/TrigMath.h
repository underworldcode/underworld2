/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StgDomain_Geometry_TrigMath_h__
#define __StgDomain_Geometry_TrigMath_h__

	#define UPPER_RIGHT_QUADRANT 	0
	#define UPPER_LEFT_QUADRANT 	1
	#define LOWER_LEFT_QUADRANT 	2
	#define LOWER_RIGHT_QUADRANT 	3
	
	#ifndef M_PI
		#define M_PI   3.14159265358979323846
	#endif
	#ifndef M_1_PI
		#define M_1_PI 0.31830988618379067154
	#endif
	#ifndef M_2_PI
		#define M_2_PI 0.63661977236758134308
	#endif

	#define StGermain_RadianToDegree(RAD) (180.0 * M_1_PI * (RAD))
	#define StGermain_DegreeToRadian(DEG) (M_PI / 180.0 * (DEG))

	void StGermain_RectangularToSpherical(double* sphericalCoords, double* rectangularCoords, Index dim) ;
	void StGermain_SphericalToRectangular(double* rectangularCoords, double* sphericalCoords, Index dim) ;

	double StGermain_TrigDomain(double angle) ;
	char StGermain_TrigQuadrant(double angle) ;

#endif /* __StgDomain_Geometry_TrigMath_h__ */
