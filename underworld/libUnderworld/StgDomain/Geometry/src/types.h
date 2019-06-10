/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Geometry_types_h__
#define __StgDomain_Geometry_types_h__


	typedef Index		NeighbourIndex;

	/** Stores three indices per triangle, each index refering to a coordinate stored in an external list */
	typedef Index		Triangle[3];
	/** Type used for indexing into lists of triangles */
	typedef Index		Triangle_Index;
	/** A type used to point to the head of a list of triangles */
	typedef Triangle*	Triangle_List;
	
	typedef double		Stg_Line[4];
	typedef Index		Line_Index;
	typedef Stg_Line*	Stg_Line_List;

	typedef Coord		Line3[2];
	
	typedef Index		Dimension_Index;
	typedef Index		IJK[3];				/* ijk indices, positive, not constrained */
	typedef Index*		Indices;			/* array/list of indices */
	typedef enum Axis { I_AXIS=0, J_AXIS=1, K_AXIS=2 } Axis;

	typedef Coord*				Coord_List;
	typedef Index				Coord_Index;
	
#endif /* __StgDomain_Geometry_types_h__ */

