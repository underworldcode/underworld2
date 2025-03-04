/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_HexType_h__
#define __StgDomain_Mesh_HexType_h__

	/** Textual name of this class */
	extern const Type Mesh_HexType_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Mesh_HexType							\
		/* General info */						\
		__Mesh_ElementType						\
										\
		/* Virtual info */						\
										\
		/* Mesh_HexType info */						\
		unsigned				mapSize;		\
		unsigned*				vertMap;		\
		unsigned*				inc;			\
		Mesh_ElementType_ElementHasPointFunc*	elementHasPoint;	\
		unsigned**				triInds;		\
		unsigned**				tetInds;		\
		IArray*					incArray;

	struct Mesh_HexType { __Mesh_HexType };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_HEXTYPE_DEFARGS \
                MESH_ELEMENTTYPE_DEFARGS

	#define MESH_HEXTYPE_PASSARGS \
                MESH_ELEMENTTYPE_PASSARGS

	Mesh_HexType* Mesh_HexType_New();
	Mesh_HexType* _Mesh_HexType_New(  MESH_HEXTYPE_DEFARGS  );
	void _Mesh_HexType_Init( Mesh_HexType* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_HexType_Delete( void* hexType );
	void _Mesh_HexType_Print( void* hexType, Stream* stream );

	void Mesh_HexType_Update( void* hexType );
	Bool Mesh_HexType_ElementHasPoint( void* hexType, unsigned elInd, double* point, 
					   MeshTopology_Dim* dim, unsigned* ind );
	double Mesh_HexType_GetMinimumSeparation( void* hexType, unsigned elInd, double* perDim );
	double Mesh_HexType_GetMinimumSeparationGeneral( void* hexType, unsigned elInd, double* perDim );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Mesh_HexType_SetVertexMap( void* hexType, unsigned* map );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	Bool Mesh_HexType_ElementHasPoint3DGeneral( Mesh_HexType* self, unsigned elInd, double* point, 
						    MeshTopology_Dim* dim, unsigned* ind );
	Bool Mesh_HexType_ElementHasPoint3DWithIncidence( Mesh_HexType* self, unsigned elInd, double* point, 
							  MeshTopology_Dim* dim, unsigned* ind );
	Bool Mesh_HexType_ElementHasPoint2DGeneral( Mesh_HexType* self, unsigned elInd, double* point, 
						    MeshTopology_Dim* dim, unsigned* ind );
	Bool Mesh_HexType_ElementHasPoint2DWithIncidence( Mesh_HexType* self, unsigned elInd, double* point, 
							  MeshTopology_Dim* dim, unsigned* ind );
	Bool Mesh_HexType_ElementHasPoint1DGeneral( Mesh_HexType* self, unsigned elInd, double* point, 
						    MeshTopology_Dim* dim, unsigned* ind );
	Bool Mesh_HexType_ElementHasPoint1DWithIncidence( Mesh_HexType* self, unsigned elInd, double* point, 
							  MeshTopology_Dim* dim, unsigned* ind );

#endif /* __StgDomain_Mesh_HexType_h__ */

