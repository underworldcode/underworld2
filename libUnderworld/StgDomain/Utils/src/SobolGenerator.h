/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_SobolGenerator_h__
#define __StgDomain_Utils_SobolGenerator_h__

	extern int SobolGeneratorGeneratorCount;

	/** Textual name of this class */
	extern const Type SobolGenerator_Type;
	
	/** SobolGenerator contents */
	#define __SobolGenerator \
		/* General info */ \
		__Stg_Object \
		\
		/* Virtual info */ \
		\
		/* Object Info */ \
		unsigned int        polynomialDegree;                     \
		unsigned int        polynomialCoefficient;                \
		Bool*               directionalNumberIsCalc;              \
		unsigned int *      directionalNumberList;                \
		unsigned int        directionalNumbersAlloced;            \
		unsigned int        lastCalculated_I;                     \
		unsigned int        lastCalculatedNumber;                 \


	struct SobolGenerator { __SobolGenerator };	

	/** Public constructor */
	SobolGenerator* SobolGenerator_NewFromTable( Name name ) ;
	
	SobolGenerator* SobolGenerator_New( 
			Name                                               name,
			unsigned int                                       polynomialDegree,
			unsigned int                                       polynomialCoefficient,
			const unsigned int *                               initialDirectionNumbers );

	/** Private Constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define SOBOLGENERATOR_DEFARGS \
                STG_OBJECT_DEFARGS

	#define SOBOLGENERATOR_PASSARGS \
                STG_OBJECT_PASSARGS

	SobolGenerator* _SobolGenerator_New(  SOBOLGENERATOR_DEFARGS  );

	void _SobolGenerator_Init( 
		SobolGenerator*                                    self,
		unsigned int                                       polynomialDegree,
		unsigned int                                       polynomialCoefficient,
		const unsigned int *                               initialDirectionNumbers )	;		

	/** Stg_Class Virtual Functions */
	void _SobolGenerator_Delete( void* sobolGenerator ) ;
	void _SobolGenerator_Print( void* sobolGenerator, Stream* stream ) ;
	void* _SobolGenerator_Copy( void* sobolGenerator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) ;
	
	/** +++ Private Functions +++ **/
	Bit_Index _SobolGenerator_FindRightmostZeroBit( unsigned int number ) ;

	/** +++ Public Functions +++ **/
	/** See  B.L. Fox P. Bratley. Implementing Sobol's Quasirandom Sequence Generator. 
	 * ACM Transactions on Mathematical Software, 14(1):88 - 100, March 1988. For details of algorithm */
	unsigned int SobolGenerator_GetDirectionalNumber( void* sobolGenerator, unsigned int directionalNum_I ) ;
	double SobolGenerator_GetNextNumber( void* sobolGenerator ) ;
	double SobolGenerator_GetNumberByIndex( void* sobolGenerator, unsigned int index ) ;

	double SobolGenerator_GetNextNumber_WithMinMax( void* sobolGenerator, double min, double max ) ;
	double SobolGenerator_GetNumberByIndex_WithMinMax( void* sobolGenerator, unsigned int index, double min, double max ) ;

    void _ZeroSobolGeneratorGeneratorCount();

#endif /* __StgDomain_Utils_SobolGenerator_h__ */

