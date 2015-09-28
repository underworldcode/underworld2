/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Swarm_CLLCellLayout_h__
#define __StgDomain_Swarm_CLLCellLayout_h__
	

	/* Textual name of this class */
	extern const Type CLLCellLayout_Type;
	
	/* CLLCellLayout information */
	#define __CLLCellLayout \
		/* General info */ \
      __ElementCellLayout \
		\
		/* Virtual info */ \
		\
      /* CLLCellLayout info */       \
      Mesh*			geometryMesh;      \
      double	   cellSize;          \
      unsigned    meshDim;           \
      Bool        parallelDecompose; \
      unsigned    expandFactor;

	struct CLLCellLayout { __CLLCellLayout };


	
	/* Create a new CLLCellLayout and initialise */
	void* _CLLCellLayout_DefaultNew( Name name );
	
   CLLCellLayout* CLLCellLayout_New( Name name, AbstractContext* context, void* mesh, double cellSize, unsigned meshDim, Bool parallelDecompose, unsigned expandFactor );
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CLLCELLLAYOUT_DEFARGS \
                ELEMENTCELLLAYOUT_DEFARGS

	#define CLLCELLLAYOUT_PASSARGS \
                ELEMENTCELLLAYOUT_PASSARGS

	CLLCellLayout* _CLLCellLayout_New(  CLLCELLLAYOUT_DEFARGS  ); 

	/* Initialise implementation */
   void _CLLCellLayout_Init( CLLCellLayout* self, void* mesh, double cellSize, unsigned meshDim, Bool parallelDecompose, unsigned expandFactor );

	/* Copy */
	#define CLLCellLayout_Copy( self ) \
		(CLLCellLayout*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define CLLCellLayout_DeepCopy( self ) \
		(CLLCellLayout*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void _CLLCellLayout_AssignFromXML( void *cLLCellLayout, Stg_ComponentFactory *cf, void* data );
	
	void _CLLCellLayout_Build( void *cLLCellLayout, void *data );
	
	void _CLLCellLayout_Initialise( void *cLLCellLayout, void *data );
	
	void _CLLCellLayout_Destroy( void *cLLCellLayout, void *data );
		
#endif /* __StgDomain_Swarm_CLLCellLayout_h__ */

