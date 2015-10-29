/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_FeMesh_h__
#define __StgFEM_Discretisation_FeMesh_h__

	/** Textual name of this class */
	extern const Type FeMesh_Type;

	/** Virtual function types */

	/** Class contents */
	#define __FeMesh				\
		/* General info */			\
		__Mesh					\
							\
		/* Virtual info */			\
							\
		/* FeMesh info */			\
		char*			feElFamily;	\
		ElementType*		feElType;	\
		Bool		elementMesh;		\
                Bool useFeAlgorithms; \
		IArray*	inc; \
		IndexSet*           bndNodeSet;   	  /* IndexSet for mesh boundary nodes */ \
		IndexSet*           bndElementSet;	  /* IndexSet for mesh boundary elements */ \
		

	struct FeMesh { __FeMesh };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define FEMESH_DEFARGS \
                MESH_DEFARGS

	#define FEMESH_PASSARGS \
                MESH_PASSARGS

	FeMesh* FeMesh_New( Name name );
	FeMesh* _FeMesh_New(  FEMESH_DEFARGS  );
	void _FeMesh_Init( FeMesh* self, ElementType* elType, const char* family, Bool elementMesh );
   FeMesh* _FeMesh_DefaultNew( Name name );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _FeMesh_Delete( void* feMesh );
	void _FeMesh_Print( void* feMesh, Stream* stream );
	void _FeMesh_AssignFromXML( void* feMesh, Stg_ComponentFactory* cf, void* data );
	void _FeMesh_Build( void* feMesh, void* data );
	void _FeMesh_Initialise( void* feMesh, void* data );
	void _FeMesh_Execute( void* feMesh, void* data );
	void _FeMesh_Destroy( void* feMesh, void* data );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void FeMesh_SetElementFamily( void* feMesh, const char* family );
	void FeMesh_SetElementType( void* feMesh, ElementType* elType );

	ElementType* FeMesh_GetElementType( void* feMesh, unsigned element );

	unsigned FeMesh_GetNodeLocalSize( void* feMesh );
	unsigned FeMesh_GetNodeRemoteSize( void* feMesh );
	unsigned FeMesh_GetNodeDomainSize( void* feMesh );
	unsigned FeMesh_GetNodeGlobalSize( void* feMesh );
	unsigned FeMesh_GetElementLocalSize( void* feMesh );
	unsigned FeMesh_GetElementDomainSize( void* feMesh );
	unsigned FeMesh_GetElementRemoteSize( void* feMesh );
	unsigned FeMesh_GetElementGlobalSize( void* feMesh );

	unsigned FeMesh_GetElementNodeSize( void* feMesh, unsigned element );
	unsigned FeMesh_GetNodeElementSize( void* feMesh, unsigned node );
	void FeMesh_GetElementNodes( void* feMesh, unsigned element, IArray* inc );
	void FeMesh_GetNodeElements( void* feMesh, unsigned node, IArray* inc );

	unsigned FeMesh_ElementDomainToGlobal( void* feMesh, unsigned domain );
	Bool FeMesh_ElementGlobalToDomain( void* feMesh, unsigned global, unsigned* domain );
	unsigned FeMesh_NodeDomainToGlobal( void* feMesh, unsigned domain );
	Bool FeMesh_NodeGlobalToDomain( void* feMesh, unsigned global, unsigned* domain );

	void FeMesh_CoordGlobalToLocal( void* feMesh, unsigned element, const double* global, double* local );
	void FeMesh_CoordLocalToGlobal( void* feMesh, unsigned element, const double*  local, double* global );
	void FeMesh_EvalBasis( void* feMesh, unsigned element, double* localCoord, double* basis );
	void FeMesh_EvalLocalDerivs( void* feMesh, unsigned element, double* localCoord, double** derivs );
	void FeMesh_EvalGlobalDerivs( void* feMesh, unsigned element, double* localCoord, double** derivs, double* jacDet );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void FeMesh_Destruct( FeMesh* self );
    void _FeMesh_DumpMeshHDF5( FeMesh* meshl, const char* filename );

#endif /* __StgFEM_Discretisaton_FeMesh_h__ */

