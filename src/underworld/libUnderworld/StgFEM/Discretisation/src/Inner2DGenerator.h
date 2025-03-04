/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisaton_Inner2DGenerator_h__
#define __StgFEM_Discretisaton_Inner2DGenerator_h__

	/** Textual name of this class */
	extern const Type Inner2DGenerator_Type;

	/** Virtual function types */

	/** Inner2DGenerator class contents */
	#define __Inner2DGenerator		\
		/* General info */		\
		__MeshGenerator			\
						\
		/* Virtual info */		\
						\
		/* Inner2DGenerator info */		\
		Mesh*		elMesh;

	struct Inner2DGenerator { __Inner2DGenerator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define INNER2DGENERATOR_DEFARGS \
                MESHGENERATOR_DEFARGS

	#define INNER2DGENERATOR_PASSARGS \
                MESHGENERATOR_PASSARGS

	Inner2DGenerator* Inner2DGenerator_New( Name name, AbstractContext* context );
	Inner2DGenerator* _Inner2DGenerator_New(  INNER2DGENERATOR_DEFARGS  );
	void _Inner2DGenerator_Init( Inner2DGenerator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Inner2DGenerator_Delete( void* generator );
	void _Inner2DGenerator_Print( void* generator, Stream* stream );
	void _Inner2DGenerator_AssignFromXML( void* generator, Stg_ComponentFactory* cf, void* data );
	void _Inner2DGenerator_Build( void* generator, void* data );
	void _Inner2DGenerator_Initialise( void* generator, void* data );
	void _Inner2DGenerator_Execute( void* generator, void* data );
	void _Inner2DGenerator_Destroy( void* generator, void* data );

	void Inner2DGenerator_Generate( void* generator, void* _mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Inner2DGenerator_SetElementMesh( void* generator, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void Inner2DGenerator_BuildTopology( Inner2DGenerator* self, FeMesh* mesh );
	void Inner2DGenerator_BuildGeometry( Inner2DGenerator* self, FeMesh* mesh );
	void Inner2DGenerator_BuildElementTypes( Inner2DGenerator* self, FeMesh* mesh );

#endif /* __StgFEM_Discretisaton_Inner2DGenerator_h__ */

