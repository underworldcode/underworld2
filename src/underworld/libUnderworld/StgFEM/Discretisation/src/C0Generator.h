/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisaton_C0Generator_h__
#define __StgFEM_Discretisaton_C0Generator_h__

	/** Textual name of this class */
	extern const Type C0Generator_Type;

	/** Virtual function types */

	/** C0Generator class contents */
	#define __C0Generator		\
		/* General info */		\
		__MeshGenerator			\
						\
		/* Virtual info */		\
						\
		/* C0Generator info */		\
		Mesh*		elMesh;

	struct C0Generator { __C0Generator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define C0GENERATOR_DEFARGS \
                MESHGENERATOR_DEFARGS

	#define C0GENERATOR_PASSARGS \
                MESHGENERATOR_PASSARGS

	C0Generator* C0Generator_New( Name name, AbstractContext* context );
	C0Generator* _C0Generator_New(  C0GENERATOR_DEFARGS  );
	void _C0Generator_Init( C0Generator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _C0Generator_Delete( void* generator );
	void _C0Generator_Print( void* generator, Stream* stream );
	void _C0Generator_AssignFromXML( void* generator, Stg_ComponentFactory* cf, void* data );
	void _C0Generator_Build( void* generator, void* data );
	void _C0Generator_Initialise( void* generator, void* data );
	void _C0Generator_Execute( void* generator, void* data );
	void _C0Generator_Destroy( void* generator, void* data );

	void C0Generator_Generate( void* generator, void* _mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void C0Generator_SetElementMesh( void* generator, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void C0Generator_BuildTopology( C0Generator* self, FeMesh* mesh );
	void C0Generator_BuildGeometry( C0Generator* self, FeMesh* mesh );
	void C0Generator_BuildElementTypes( C0Generator* self, FeMesh* mesh );

#endif /* __StgFEM_Discretisaton_C0Generator_h__ */

