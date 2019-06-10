/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StgFEM_Discretisaton_dQ1Generator_h__
#define __StgFEM_Discretisaton_dQ1Generator_h__

	/** Textual name of this class */
	extern const Type dQ1Generator_Type;

	/** Virtual function types */

	/** dQ1Generator class contents */
	#define __dQ1Generator		\
		/* General info */		\
		__MeshGenerator			\
						\
		/* Virtual info */		\
						\
		/* dQ1Generator info */		\
		Mesh*		elMesh;

	struct dQ1Generator { __dQ1Generator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define DQ1GENERATOR_DEFARGS \
                MESHGENERATOR_DEFARGS

	#define DQ1GENERATOR_PASSARGS \
                MESHGENERATOR_PASSARGS

	dQ1Generator* dQ1Generator_New( Name name, AbstractContext* context );
	dQ1Generator* _dQ1Generator_New(  DQ1GENERATOR_DEFARGS  );
	void _dQ1Generator_Init( dQ1Generator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _dQ1Generator_Delete( void* generator );
	void _dQ1Generator_Print( void* generator, Stream* stream );
	void _dQ1Generator_AssignFromXML( void* generator, Stg_ComponentFactory* cf, void* data );
	void _dQ1Generator_Build( void* generator, void* data );
	void _dQ1Generator_Initialise( void* generator, void* data );
	void _dQ1Generator_Execute( void* generator, void* data );
	void _dQ1Generator_Destroy( void* generator, void* data );

	void dQ1Generator_Generate( void* generator, void* _mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void dQ1Generator_SetElementMesh( void* generator, void* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void dQ1Generator_BuildTopology( dQ1Generator* self, FeMesh* mesh );
	void dQ1Generator_BuildGeometry( dQ1Generator* self, FeMesh* mesh );
	void dQ1Generator_BuildElementTypes( dQ1Generator* self, FeMesh* mesh );

#endif /* __StgFEM_Discretisaton_dQ1Generator_h__ */

