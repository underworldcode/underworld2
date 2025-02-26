/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_SROpGenerator_h__
#define __StgFEM_SLE_SystemSetup_SROpGenerator_h__

	/** Textual name of this class */
	extern const Type SROpGenerator_Type;

	/** Virtual function types */

	/** SROpGenerator class contents */
	#define __SROpGenerator				\
		/* General info */			\
		__MGOpGenerator				\
							\
		/* Virtual info */			\
							\
		/* SROpGenerator info */		\
		FeVariable*		fineVar;	\
		FeEquationNumber*	fineEqNum;	\
		Mesh**			meshes;		\
		unsigned**		topMaps;	\
		unsigned***		eqNums;		\
		unsigned*		nLocalEqNums;	\
		unsigned*		eqNumBases;

	struct SROpGenerator { __SROpGenerator };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/



	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SROPGENERATOR_DEFARGS \
                MGOPGENERATOR_DEFARGS

	#define SROPGENERATOR_PASSARGS \
                MGOPGENERATOR_PASSARGS

	SROpGenerator* SROpGenerator_New( Name name );
	SROpGenerator* _SROpGenerator_New(  SROPGENERATOR_DEFARGS  );
	void _SROpGenerator_Init( SROpGenerator* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/

	void _SROpGenerator_Delete( void* srOpGenerator );
	void _SROpGenerator_Print( void* srOpGenerator, Stream* stream );
	void _SROpGenerator_AssignFromXML( void* srOpGenerator, Stg_ComponentFactory* cf, void* data );
	void _SROpGenerator_Build( void* srOpGenerator, void* data );
	void _SROpGenerator_Initialise( void* srOpGenerator, void* data );
	void _SROpGenerator_Execute( void* srOpGenerator, void* data );
	void _SROpGenerator_Destroy( void* srOpGenerator, void* data );

	Bool SROpGenerator_HasExpired( void* srOpGenerator );
	//void SROpGenerator_Generate( void* srOpGenerator, Matrix*** pOps, Matrix*** rOps );
	void SROpGenerator_Generate( void* srOpGenerator, Mat** pOps, Mat** rOps );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public functions
	*/

	void SROpGenerator_SetFineVariable( void* srOpGenerator, void* variable );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions
	*/

	void SROpGenerator_GenMeshes( SROpGenerator* self );
	void SROpGenerator_GenLevelMesh( SROpGenerator* self, unsigned level );
	void SROpGenerator_GenLevelTopMap( SROpGenerator* self, unsigned level );
	void SROpGenerator_GenLevelEqNums( SROpGenerator* self, unsigned level );

	//void SROpGenerator_GenOps( SROpGenerator* self, Matrix** pOps, Matrix** rOps );
	void SROpGenerator_GenOps( SROpGenerator* self, Mat* pOps, Mat* rOps );
	//void SROpGenerator_GenLevelOp( SROpGenerator* self, unsigned level, Matrix* P );
	void SROpGenerator_GenLevelOp( SROpGenerator* self, unsigned level, Mat P );
	void SROpGenerator_CalcOpNonZeros( SROpGenerator* self, unsigned level, 
					   unsigned** nDiagNonZeros, unsigned** nOffDiagNonZeros );
	void SROpGenerator_DestructMeshes( SROpGenerator* self );
	//void SROpGenerator_Simple( SROpGenerator *self, Matrix **pOps, Matrix **rOps );
	void SROpGenerator_Simple( SROpGenerator *self, Mat* pOps, Mat* rOps );
	//Matrix *SROpGenerator_SimpleFinestLevel( SROpGenerator *self );
	//Matrix *SROpGenerator_SimpleCoarserLevel( SROpGenerator *self, int level );
	Mat SROpGenerator_SimpleFinestLevel( SROpGenerator *self );
	Mat SROpGenerator_SimpleCoarserLevel( SROpGenerator *self, int level );

#endif /* __StgFEM_SLE_SystemSetup_SROpGenerator_h__ */

