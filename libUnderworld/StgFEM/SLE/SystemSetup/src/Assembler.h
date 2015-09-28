/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_Assembler_h__
#define __StgFEM_SLE_SystemSetup_Assembler_h__

	/** Textual name of this class */
	extern const Type Assembler_Type;

	/** Virtual function types */

	/** Assembler class contents */
	typedef Bool (Assembler_CallbackType)( void* object, Assembler* assm );

	#define __Assembler				\
		/* General info */			\
		__Stg_Class				\
							\
		/* Virtual info */			\
							\
		/* Assembler info */			\
		FeVariable*		rowVar;		\
		FeVariable*		colVar;		\
		Swarm*			swarm;		\
		Assembler_CallbackType*	partCB;		\
		Assembler_CallbackType*	rowRCB;		\
		Assembler_CallbackType*	rowUCB;		\
		Assembler_CallbackType*	colRCB;		\
		Assembler_CallbackType*	colUCB;		\
		void*			obj;		\
							\
		unsigned		elInd;		\
		IntegrationPoint*	particle;	\
		double*			shapeFuncs;	\
		double			detJac;		\
		double**		globalDerivs;	\
		unsigned		rowInd;		\
		unsigned		rowElNodeInd;	\
		unsigned		rowNodeInd;	\
		unsigned		rowDofInd;	\
		unsigned		rowEq;		\
		unsigned		colInd;		\
		unsigned		colElNodeInd;	\
		unsigned		colNodeInd;	\
		unsigned		colDofInd;	\
		unsigned		colEq;		\
							\
		IArray*			rowInc;		\
		IArray*			colInc;

	struct Assembler { __Assembler };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define ASSEMBLER_DEFARGS \
                STG_CLASS_DEFARGS

	#define ASSEMBLER_PASSARGS \
                STG_CLASS_PASSARGS

	Assembler* Assembler_New();
	Assembler* _Assembler_New(  ASSEMBLER_DEFARGS  );
	void _Assembler_Init( Assembler* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _Assembler_Delete( void* assembler );
	void _Assembler_Print( void* assembler, Stream* stream );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void Assembler_SetVariables( void* assembler, FeVariable* rowVar, FeVariable* columnVar );
	void Assembler_SetIntegrationSwarm( void* assembler, Swarm* swarm );
	void Assembler_SetCallbacks( void* assembler, 
				     Assembler_CallbackType* particle, 
				     Assembler_CallbackType* rowRestricted, 
				     Assembler_CallbackType* rowUnrestricted, 
				     Assembler_CallbackType* colRestricted, 
				     Assembler_CallbackType* colUnrestricted, 
				     void* object );
	void Assembler_Update( void* assembler );

	void Assembler_IntegrateMatrixElement( void* assembler, unsigned element );
	void Assembler_LoopMatrixElement( void* assembler, unsigned element );
	void Assembler_LoopMatrixDiagonal( void* assembler );

	#define Assembler_LoopVector Assembler_LoopMatrixDiagonal

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

#endif /* __StgFEM_SLE_SystemSetup_Assembler_h__ */

