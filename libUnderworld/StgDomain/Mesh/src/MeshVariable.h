/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Mesh_MeshVariable_h__
#define __StgDomain_Mesh_MeshVariable_h__

	/** Textual name of this class */
	extern const Type MeshVariable_Type;

	/** Virtual function types */

	/** Class contents */
	#define __MeshVariable \
		/* General info */ \
		__Variable \
		\
		/* Virtual info */ \
		\
		/* MeshVariable info */ \
		Mesh*					mesh; \
		MeshTopology_Dim	topoDim;	\
		unsigned				meshArraySize;

	struct MeshVariable { __MeshVariable };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MESHVARIABLE_DEFARGS \
		VARIABLE_DEFARGS

	#define MESHVARIABLE_PASSARGS \
		VARIABLE_PASSARGS

	MeshVariable* MeshVariable_New( Name name );

	MeshVariable* _MeshVariable_New(  MESHVARIABLE_DEFARGS  );

	void _MeshVariable_Init( MeshVariable* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _MeshVariable_Delete( void* meshVariable );

	void _MeshVariable_Print( void* meshVariable, Stream* stream );

	void _MeshVariable_AssignFromXML( void* meshVariable, Stg_ComponentFactory* cf, void* data );

	void _MeshVariable_Build( void* meshVariable, void* data );

	void _MeshVariable_Initialise( void* meshVariable, void* data );

	void _MeshVariable_Execute( void* meshVariable, void* data );

	void _MeshVariable_Destroy( void* meshVariable, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void MeshVariable_SetMesh( void* meshVariable, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	Index _MeshVariable_GetMeshArraySize( void* meshVariable );

#endif /* __StgDomain_Mesh_MeshVariable_h__ */

