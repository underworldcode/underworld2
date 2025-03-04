/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisaton_C2Generator_h__
#define __StgFEM_Discretisaton_C2Generator_h__

	/** Textual name of this class */
	extern const Type C2Generator_Type;

	/** Virtual function types */

	/** C2Generator class contents */
	#define __C2Generator			\
		/* General info */		\
		__CartesianGenerator		\
						\
		/* Virtual info */		\
						\
		/* C2Generator info */		\

	struct C2Generator { __C2Generator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define C2GENERATOR_DEFARGS \
                CARTESIANGENERATOR_DEFARGS

	#define C2GENERATOR_PASSARGS \
                CARTESIANGENERATOR_PASSARGS

	C2Generator* C2Generator_New( Name name, AbstractContext* context );
	C2Generator* _C2Generator_New(  C2GENERATOR_DEFARGS  );
	void _C2Generator_Init( C2Generator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _C2Generator_Delete( void* meshGenerator );
	void _C2Generator_Print( void* meshGenerator, Stream* stream );
	void _C2Generator_AssignFromXML( void* meshGenerator, Stg_ComponentFactory* cf, void* data );
	void _C2Generator_Build( void* meshGenerator, void* data );
	void _C2Generator_Initialise( void* meshGenerator, void* data );
	void _C2Generator_Execute( void* meshGenerator, void* data );
	void _C2Generator_Destroy( void* meshGenerator, void* data );

	void C2Generator_SetTopologyParams( void* meshGenerator, unsigned* sizes, 
					    unsigned maxDecompDims, unsigned* minDecomp, unsigned* maxDecomp );
	void C2Generator_GenElementVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void C2Generator_GenFaceVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void C2Generator_GenEdgeVertexInc( void* meshGenerator, IGraph* topo, Grid*** grids );
	void C2Generator_GenElementTypes( void* meshGenerator, Mesh* mesh );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgFEM_Discretisaton_C2Generator_h__ */

