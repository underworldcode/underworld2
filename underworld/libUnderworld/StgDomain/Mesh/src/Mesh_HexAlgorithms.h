/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_HexAlgorithms_h__
#define __StgDomain_Mesh_HexAlgorithms_h__

	/** Textual name of this class */
	extern const Type Mesh_HexAlgorithms_Type;

	/** Virtual function types */

	/** Class contents */
	#define __Mesh_HexAlgorithms			\
		/* General info */			\
		__Mesh_Algorithms			\
							\
		/* Virtual info */			\
							\
		/* Mesh_HexAlgorithms info */

	struct Mesh_HexAlgorithms { __Mesh_HexAlgorithms };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	Mesh_HexAlgorithms* Mesh_HexAlgorithms_New( Name name, AbstractContext* context );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESH_HEXALGORITHMS_DEFARGS \
                MESH_ALGORITHMS_DEFARGS

	#define MESH_HEXALGORITHMS_PASSARGS \
                MESH_ALGORITHMS_PASSARGS

	Mesh_HexAlgorithms* _Mesh_HexAlgorithms_New(  MESH_HEXALGORITHMS_DEFARGS  );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Mesh_HexAlgorithms_Init( void* hexAlgorithms );

	void _Mesh_HexAlgorithms_Delete( void* hexAlgorithms );

	void _Mesh_HexAlgorithms_Print( void* hexAlgorithms, Stream* stream );

	void _Mesh_HexAlgorithms_AssignFromXML( void* hexAlgorithms, Stg_ComponentFactory* cf, void* data );

	void _Mesh_HexAlgorithms_Build( void* hexAlgorithms, void* data );

	void _Mesh_HexAlgorithms_Initialise( void* hexAlgorithms, void* data );

	void _Mesh_HexAlgorithms_Execute( void* hexAlgorithms, void* data );

	void _Mesh_HexAlgorithms_Destroy( void* hexAlgorithms, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgDomain_Mesh_HexAlgorithms_h__ */

