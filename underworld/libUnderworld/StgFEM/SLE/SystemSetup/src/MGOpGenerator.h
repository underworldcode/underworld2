/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_MGOpGenerator_h__
#define __StgFEM_SLE_SystemSetup_MGOpGenerator_h__

	/** Textual name of this class */
	extern const Type MGOpGenerator_Type;

	/** Virtual function types */
	typedef void (MGOpGenerator_SetNumLevelsFunc)( void* mgOpGenerator, unsigned nLevels );
	typedef Bool (MGOpGenerator_HasExpiredFunc)( void* mgOpGenerator );
	//typedef void (MGOpGenerator_GenerateFunc)( void* mgOpGenerator, Matrix*** pOps, Matrix*** rOps );
	typedef void (MGOpGenerator_GenerateFunc)( void* mgOpGenerator, Mat** pOps, Mat** rOps );

	/** MGOpGenerator class contents */
	#define __MGOpGenerator							\
		/* General info */						\
		__Stg_Component							\
										\
		/* Virtual info */						\
		MGOpGenerator_SetNumLevelsFunc*		setNumLevelsFunc;	\
		MGOpGenerator_HasExpiredFunc*		hasExpiredFunc;		\
		MGOpGenerator_GenerateFunc*		generateFunc;		\
										\
		/* MGOpGenerator info */					\
		MGSolver_PETScData*	solver;					\
		unsigned		nLevels;

	struct MGOpGenerator { __MGOpGenerator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/





	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define MGOPGENERATOR_DEFARGS \
                STG_COMPONENT_DEFARGS, \
                MGOpGenerator_SetNumLevelsFunc*  setNumLevelsFunc, \
                MGOpGenerator_HasExpiredFunc*      hasExpiredFunc, \
                MGOpGenerator_GenerateFunc*          generateFunc

	#define MGOPGENERATOR_PASSARGS \
                STG_COMPONENT_PASSARGS, \
	        setNumLevelsFunc, \
	        hasExpiredFunc,   \
	        generateFunc    

	MGOpGenerator* _MGOpGenerator_New(  MGOPGENERATOR_DEFARGS  );
	void _MGOpGenerator_Init( MGOpGenerator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _MGOpGenerator_Delete( void* mgOpGenerator );
	void _MGOpGenerator_Print( void* mgOpGenerator, Stream* stream );
	void _MGOpGenerator_AssignFromXML( void* mgOpGenerator, Stg_ComponentFactory* cf, void* data );
	void _MGOpGenerator_Build( void* mgOpGenerator, void* data );
	void _MGOpGenerator_Initialise( void* mgOpGenerator, void* data );
	void _MGOpGenerator_Execute( void* mgOpGenerator, void* data );
	void _MGOpGenerator_Destroy( void* mgOpGenerator, void* data );

	void _MGOpGenerator_SetNumLevels( void* mgOpGenerator, unsigned nLevels );

	#define MGOpGenerator_SetNumLevels( self, nLevels )		\
		VirtualCall( self, setNumLevelsFunc, self, nLevels )

	#define MGOpGenerator_HasExpired( self )			\
		VirtualCall( self, hasExpiredFunc, self )

	#define MGOpGenerator_Generate( self, pOps, rOps )		\
		VirtualCall( self, generateFunc, self, pOps, rOps )

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void MGOpGenerator_SetMatrixSolver( void* mgOpGenerator, void* solver );
	unsigned MGOpGenerator_GetNumLevels( void* mgOpGenerator );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgFEM_SLE_SystemSetup_MGOpGenerator_h__ */

