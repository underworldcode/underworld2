/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <string.h>
#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>
#include <StgDomain/Mesh/src/Mesh.h>

#include "types.h"
#include "SobolGenerator.h"
#include "DomainContext.h"

#include <assert.h>

const Type SobolGenerator_Type = "SobolGenerator";

/* Some macros for clarity */
#define BITS_IN_A_BYTE 8

SobolGenerator* SobolGenerator_New( 
			Name                                               name,
			unsigned int                                       polynomialDegree,
			unsigned int                                       polynomialCoefficient,
			const unsigned int *                               initialDirectionNumbers )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(SobolGenerator);
	Type                              type = SobolGenerator_Type;
	Stg_Class_DeleteFunction*      _delete = _SobolGenerator_Delete;
	Stg_Class_PrintFunction*        _print = _SobolGenerator_Print;
	Stg_Class_CopyFunction*          _copy = _SobolGenerator_Copy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	SobolGenerator* self = _SobolGenerator_New(  SOBOLGENERATOR_PASSARGS  );
	_SobolGenerator_Init( self, polynomialDegree, polynomialCoefficient, initialDirectionNumbers );

	return self;
}

SobolGenerator* _SobolGenerator_New(  SOBOLGENERATOR_DEFARGS  )
{
	SobolGenerator*		self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SobolGenerator) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (SobolGenerator*)_Stg_Object_New(  STG_OBJECT_PASSARGS  );

	/** lets init this fella to zero here.. seems we want it to increment globally hence the global */

	return self;
}

void _SobolGenerator_Init( 
		SobolGenerator*                                    self,
		unsigned int                                       polynomialDegree,
		unsigned int                                       polynomialCoefficient,
		const unsigned int *                               initialDirectionNumbers ) 
{
	unsigned int        directionNum_I;

	self->directionalNumbersAlloced = 20;

	self->directionalNumberIsCalc = Memory_Alloc_Array( Bool, self->directionalNumbersAlloced, "Calculate Flag" );
	self->directionalNumberList   = Memory_Alloc_Array( unsigned int , self->directionalNumbersAlloced, "DirectionalNumbers" );
	memset( self->directionalNumberIsCalc, 0, sizeof( Bool ) * self->directionalNumbersAlloced );

	self->polynomialDegree      = polynomialDegree;
	self->polynomialCoefficient = polynomialCoefficient;

	memcpy( self->directionalNumberList, initialDirectionNumbers, polynomialDegree * sizeof( unsigned int ) );

	self->lastCalculated_I = 0;
	self->lastCalculatedNumber = 0.5;
		
	for ( directionNum_I = 0 ; directionNum_I < polynomialDegree ; directionNum_I++ )
		self->directionalNumberIsCalc[ directionNum_I ] = True;
	
}


void _SobolGenerator_Delete( void* sobolGenerator ) {
	SobolGenerator* self = (SobolGenerator*) sobolGenerator;

	Memory_Free( self->directionalNumberIsCalc );
	Memory_Free( self->directionalNumberList );

	_Stg_Object_Delete( self );
}

void _SobolGenerator_Print( void* sobolGenerator, Stream* stream ) {
	SobolGenerator* self  = (SobolGenerator*) sobolGenerator;
	unsigned int    index;

	Journal_Printf( stream, "Sobol Generator: '%s'\n", self->name );
	Stream_Indent( stream );

	_Stg_Object_Print( self, stream );

	Journal_PrintValue( stream, self->polynomialDegree );
	Journal_PrintValue( stream, self->polynomialCoefficient );
	Journal_Printf( stream, "This leads to polynomial: x^%u + ", self->polynomialDegree );
	for ( index = self->polynomialDegree - 1 ; index > 0 ; index-- ) {
		if ( self->polynomialCoefficient & 1 << (index - 1) )
			Journal_Printf( stream, "x^%u + ", index );
	}
	Journal_Printf( stream, "1\n" );

	Journal_PrintValue( stream, self->directionalNumbersAlloced );
	Journal_Printf( stream, "Directional Numbers Calculated:\n" );
	for ( index = 0 ; index < self->directionalNumbersAlloced ; index++ ) {
		if ( self->directionalNumberIsCalc[ index ] )
			Journal_Printf( stream, "\tDirectional Number %u = %lu\n", index, self->directionalNumberList[ index ] );
	}

	Journal_PrintValue( stream, self->lastCalculated_I );
	Journal_PrintValue( stream, self->lastCalculatedNumber );
	
	Stream_UnIndent( stream );
}

void* _SobolGenerator_Copy( void* sobolGenerator, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	SobolGenerator*	self = (SobolGenerator*)sobolGenerator;
	SobolGenerator*	newSobolGenerator;
	
	newSobolGenerator = (SobolGenerator*) _Stg_Object_Copy( self, dest, deep, nameExt, ptrMap );
	
	newSobolGenerator->lastCalculated_I          = self->lastCalculated_I;
	newSobolGenerator->lastCalculatedNumber      = self->lastCalculatedNumber;
	newSobolGenerator->directionalNumbersAlloced = self->directionalNumbersAlloced;
	newSobolGenerator->polynomialDegree          = self->polynomialDegree;
	newSobolGenerator->polynomialCoefficient     = self->polynomialCoefficient ;
	
	if (deep) {
		newSobolGenerator->directionalNumberIsCalc = Memory_Alloc_Array( Bool, self->directionalNumbersAlloced, "Flag" );
		memcpy( newSobolGenerator->directionalNumberIsCalc, self->directionalNumberIsCalc, 
				self->directionalNumbersAlloced * sizeof( Bool ) );

		newSobolGenerator->directionalNumberList = Memory_Alloc_Array( unsigned int, self->directionalNumbersAlloced, "V_i" );
		memcpy( newSobolGenerator->directionalNumberList, self->directionalNumberList, 
				self->directionalNumbersAlloced * sizeof( unsigned int ) );
	}
	else {
		newSobolGenerator->directionalNumberIsCalc      = self->directionalNumberIsCalc;
		newSobolGenerator->directionalNumberList        = self->directionalNumberList;
	}
	
	return (void*)newSobolGenerator;
}

Bit_Index _SobolGenerator_FindRightmostZeroBit( unsigned int number ) {
	Bit_Index bit = 0;

	while ( number & 1 ) {
		/* Shift All bits to right by one */
		number >>= 1;

		/* Increment counter to find bit */
		bit++;
		assert( bit < sizeof( unsigned int ) * BITS_IN_A_BYTE );
	}
	return bit;
}

unsigned int  SobolGenerator_GetDirectionalNumber( void* sobolGenerator, unsigned int directionalNum_I ) {
	SobolGenerator*	     self          = (SobolGenerator*)sobolGenerator;
	unsigned int         result;
	unsigned int         coefficient_I;
	unsigned int         powerOfTwo;
	Bit_Index            bitToTest;
	unsigned int         directionNumber = 0;

	if ( directionalNum_I >= self->directionalNumbersAlloced ) {
		unsigned int  difference       = 10;
		unsigned int  newNumberToAlloc = directionalNum_I + difference;
		
		/* Allocate More Memory */
		self->directionalNumberIsCalc = Memory_Realloc_Array( self->directionalNumberIsCalc, Bool, newNumberToAlloc );
		self->directionalNumberList   = Memory_Realloc_Array( self->directionalNumberList, unsigned int , newNumberToAlloc );

		/* Set Bools */
		memset( &self->directionalNumberIsCalc[ self->directionalNumbersAlloced ], 0, sizeof(Bool) * difference );
		
		self->directionalNumbersAlloced = newNumberToAlloc;
	}
	/* Check to see whether the directional number has already been calculated */
	else if ( self->directionalNumberIsCalc[ directionalNum_I ] )
		return self->directionalNumberList[ directionalNum_I ];

	/* If the code hasn't precalculated all the directional numbers up to polynomialDegree then there is something wrong */
	assert( directionalNum_I >= self->polynomialDegree );

	/* Calculate Directional Number */
	result = 0;
	for ( coefficient_I = 0 ; coefficient_I < self->polynomialDegree ; coefficient_I++ ) {
		/* Test for this coefficient is 1 */
		bitToTest = self->polynomialDegree - 2 - coefficient_I;
		if ( coefficient_I == self->polynomialDegree - 1 || self->polynomialCoefficient & 1 << bitToTest ) {
			powerOfTwo = 0;
			powerOfTwo |= 1 << (coefficient_I + 1);
			
			directionNumber = SobolGenerator_GetDirectionalNumber( self, directionalNum_I - 1 - coefficient_I );
			result ^= powerOfTwo * directionNumber;
		}
	}
	result ^= directionNumber;

	self->directionalNumberIsCalc[ directionalNum_I ] = True;

	return self->directionalNumberList[ directionalNum_I ] = result;
}

#define WORDLENGTH 30
double SobolGenerator_GetNextNumber( void* sobolGenerator ) {
	SobolGenerator*   self          = (SobolGenerator*)sobolGenerator;
	Bit_Index         differentBit;
	unsigned int      directionalNumber;

	/* The Gray Code of the index last calculated quasi-random number will differ by only one bit 
	 * from the Gray Code of the index of the current quasi-random number to calculate
	 * this bit is the right most zero bit in lastCalcuated_I */
	differentBit = _SobolGenerator_FindRightmostZeroBit( self->lastCalculated_I );

	/* This bit can be used to find the next quasi-random number in Sobol's sequence 
	 * x_{n+1} = x_{n} ^ v_{c}, where
	 * x_{j} is the j^{th} quasi-random number in the sequence
	 * v_{j} is the j^{th} directional number
	 * c is the single different bit between the Gray codes for n and n+1 */

	self->lastCalculated_I++;
	directionalNumber = SobolGenerator_GetDirectionalNumber( self, differentBit );
	directionalNumber <<= (WORDLENGTH - differentBit - 1);
	self->lastCalculatedNumber ^= directionalNumber;

	return (double) self->lastCalculatedNumber / (double) (1L << (WORDLENGTH));
}
	
double SobolGenerator_GetNumberByIndex( void* sobolGenerator, unsigned int index ) {
	SobolGenerator*   self          = (SobolGenerator*)sobolGenerator;
	unsigned int      grayCode;
	Bit_Index         bit_I;
	unsigned int      directionalNumber;
	unsigned int      result   =  0;

	/* have to shift index because index 0 is the initial zero for the sequence to begin */
	index++;

	/* Get Gray Code of 'index' */
	grayCode = index ^ (index >> 1);

	for ( bit_I = 0 ; bit_I < sizeof( unsigned int ) * BITS_IN_A_BYTE ; bit_I++ ) {
		if ( grayCode & 1 << bit_I ) {
			directionalNumber = SobolGenerator_GetDirectionalNumber( self, bit_I );
			directionalNumber <<= (WORDLENGTH - bit_I - 1);

			result ^= directionalNumber;
		}
	}

	return (double) result / (double) (1L << (WORDLENGTH));
}

double SobolGenerator_GetNextNumber_WithMinMax( void* sobolGenerator, double min, double max ) {
	return (max - min) * SobolGenerator_GetNextNumber( sobolGenerator ) + min;
}

double SobolGenerator_GetNumberByIndex_WithMinMax( void* sobolGenerator, unsigned int index, double min, double max ) {
	return (max - min) * SobolGenerator_GetNumberByIndex( sobolGenerator, index ) + min;
}

/* Some global variables that drive the random number generator */
/* These numbers for initialisation come  from:
 * Stephen Joe and Frances Y. Juo. Remark on Algorithm 659: Implementing Sobol¿s Quasirandom Sequence Generator. ACM Transactions on Mathematical Software, 29(1), March 2003.*/
const unsigned int SobolGenerator_PolynomialDegree[] = {
	1,2,3,3,4, 4,5,5,5,5,
	5,5,6,6,6, 6,6,6,7,7,
	7,7,7,7,7, 7,7,7,7,7,
	7,7,7,7,7, 7,8,8,8,8,
	8,8,8,8,8, 8,8,8,8,8,
	
	8,8,9,9,9, 9,9,9,9,9,
	9,9,9,9,9, 9,9,9,9,9,
	9,9,9,9,9, 9,9,9,9,9,
	9,9,9,9,9, 9,9,9,9,9,
	9,9,9,9,9, 9,9,9,9,9
	};

const unsigned int SobolGenerator_PolynomialCoefficient[] = {
	0,1,1,2,1, 4,2,13,7,14,
	11,4,1,16,13, 22,19,25,1,32,
	4,8,7,56,14, 28,19,50,21,42,
	31,62,37,41,55, 59,14,56,21,22,
	38,47,49,50,52, 67,70,84,97,103,

	115,122,8,13,16, 22,25,44,47,52,
	55,59,62,67,74, 81,82,87,91,94,
	103,104,109,122,124, 137,138,143,145,152,
	157,167,173,176,181, 182,185,191,194,199,
	218,220,227,229,230, 234,236,241,244,253
	};

const unsigned int SobolGenerator_InitialDirectionNumbers[][13] = {
	 { 1 },
	 { 1, 1 },
	 { 1, 3, 7 },
	 { 1, 1, 5 },
	 { 1, 3, 1, 1 },

	 { 1, 1, 3, 7 },
	 { 1, 3, 3, 9, 9 },
	 { 1, 3, 7, 13, 3 },
	 { 1, 1, 5, 11, 27 },
	 { 1, 3, 5, 1, 15 },

	 /*10*/
	 { 1, 1, 7, 3, 29 },
	 { 1, 3, 7, 7, 21 },
	 { 1, 1, 1, 9, 23, 37 },
	 { 1, 3, 3, 5, 19, 33 },
	 { 1, 1, 3, 13, 11, 7 },

	 { 1, 1, 7, 13, 25, 5 },
	 { 1, 3, 5, 11, 7, 11 },
	 { 1, 1, 1, 3, 13, 39 },
	 { 1, 3, 1, 15, 17, 63, 13 },
	 { 1, 1, 5, 5, 1, 59, 33 },

	 /*20*/
	 { 1, 3, 3, 3, 25, 17, 115 },
	 { 1, 1, 7, 15, 29, 15, 41 },
	 { 1, 3, 1, 7, 3, 23, 79 },
	 { 1, 3, 7, 9, 31, 29, 17 },
	 { 1, 1, 5, 13, 11, 3, 29 },
	 
	 { 1, 1, 1, 9, 5, 21, 119 },
	 { 1, 1, 3, 1, 23, 13, 75 },
	 { 1, 3, 7, 11, 27, 31, 73 },
	 { 1, 1, 7, 7, 19, 25, 105 },
	 { 1, 3, 1, 5, 21, 9, 7 },
	 
	 /*30*/
	 { 1, 1, 1, 15, 5, 49, 59 },
	 { 1, 3, 1, 1, 1, 33, 65 },
	 { 1, 3, 5, 15, 17, 19, 21 },
	 { 1, 1, 7, 11, 13, 29, 3 },
	 { 1, 3, 7, 5, 7, 11, 113 },
	 
	 { 1, 1, 5, 11, 15, 19, 61 },
	 { 1, 1, 1, 1, 9, 27, 89, 7 },
	 { 1, 1, 3, 7, 31, 15, 45, 23 },
	 { 1, 3, 3, 9, 25, 25, 107, 39 },
	 { 1, 1, 7, 7, 3, 63, 21, 217 },
	 
	 /*40*/
	 { 1, 3, 5, 7, 5, 55, 71, 141 },
	 { 1, 1, 5, 1, 23, 17, 79, 27 },
	 { 1, 1, 5, 15, 7, 63, 19, 53 },
	 { 1, 1, 3, 15, 3, 49, 71, 181 },
	 { 1, 3, 3, 15, 17, 19, 61, 169 },
	 
	 { 1, 3, 3, 13, 23, 41, 41, 35 },
	 { 1, 1, 1, 3, 3, 59, 57, 15 },
	 { 1, 3, 1, 3, 3, 3, 121, 207 },
	 { 1, 3, 5, 15, 21, 57, 87, 45 },
	 { 1, 1, 1, 5, 25, 33, 119, 247 },
	 
	 /*50*/
	 { 1, 1, 1, 9, 25, 49, 55, 185 },
	 { 1, 3, 5, 7, 23, 53, 85, 117 },
	 { 1, 3, 3, 13, 11, 57, 121, 41, 235 },
	 { 1, 1, 3, 3, 19, 57, 119, 81, 307 },
	 { 1, 3, 3, 7, 3, 39, 11, 223, 495 },
	 
	 { 1, 3, 3, 5, 11, 21, 23, 151, 417 },
	 { 1, 3, 1, 11, 31, 7, 61, 81, 57 },
	 { 1, 1, 3, 9, 7, 53, 11, 189, 151 },
	 { 1, 3, 7, 1, 9, 9, 35, 61, 19 },
	 { 1, 1, 5, 9, 5, 55, 33, 95, 119 },
	 
	 /*60*/
	 { 1, 3, 7, 1, 17, 15, 43, 185, 375 },
	 { 1, 1, 3, 5, 23, 59, 107, 23, 451 },
	 { 1, 1, 7, 7, 17, 19, 113, 73, 55 },
	 { 1, 3, 1, 13, 17, 49, 101, 113, 449 },
	 { 1, 3, 3, 9, 25, 31, 29, 239, 501 },
	 
	 { 1, 1, 3, 9, 13, 3, 87, 85, 53 },
	 { 1, 1, 5, 1, 11, 39, 119, 9, 185 },
	 { 1, 1, 1, 7, 31, 5, 97, 201, 317 },
	 { 1, 1, 3, 3, 27, 5, 29, 83, 17 },
	 { 1, 3, 5, 5, 19, 41, 17, 53, 21 },
	 
	 /*70*/
	 { 1, 1, 5, 1, 17, 9, 89, 183, 487 },
	 { 1, 1, 7, 11, 23, 19, 5, 203, 13 },
	 { 1, 3, 7, 11, 7, 9, 127, 91, 347 },
	 { 1, 1, 7, 13, 5, 57, 89, 149, 393 },
	 { 1, 1, 1, 7, 11, 25, 119, 101, 15 },
	 
	 { 1, 1, 1, 7, 19, 1, 117, 13, 391 },
	 { 1, 3, 3, 9, 19, 15, 103, 111, 307 },
	 { 1, 3, 3, 9, 7, 51, 105, 239, 189 },
	 { 1, 1, 1, 1, 13, 11, 41, 3, 381 },
	 { 1, 3, 1, 1, 21, 19, 83, 205, 71 },
	 
	 /*80*/
	 { 1, 3, 5, 3, 21, 61, 25, 253, 163 },
	 { 1, 1, 1, 9, 7, 53, 41, 247, 99 },
	 { 1, 3, 5, 15, 9, 29, 55, 121, 467 },
	 { 1, 3, 7, 1, 11, 19, 69, 189, 167 },
	 { 1, 3, 5, 5, 1, 11, 117, 169, 433 },
	 
	 { 1, 1, 1, 13, 5, 9, 49, 179, 337 },
	 { 1, 3, 7, 1, 21, 21, 127, 197, 257 },
	 { 1, 3, 5, 9, 11, 19, 29, 175, 179 },
	 { 1, 3, 3, 9, 13, 43, 1, 217, 47 },
	 { 1, 1, 3, 9, 25, 13, 99, 249, 385 },
	 
	 /*90*/
	 { 1, 3, 1, 9, 9, 13, 53, 195, 23 },
	 { 1, 3, 5, 9, 7, 41, 83, 95, 117 },
	 { 1, 1, 7, 13, 7, 25, 15, 63, 369 },
	 { 1, 3, 1, 11, 27, 31, 31, 19, 425 },
	 { 1, 3, 7, 3, 15, 9, 73, 7, 207 },
	 
	 { 1, 3, 5, 5, 25, 11, 115, 5, 433 },
	 { 1, 1, 1, 11, 15, 19, 35, 75, 301 },
	 { 1, 3, 7, 11, 21, 5, 21, 217, 147 },
	 { 1, 1, 3, 13, 17, 53, 89, 245, 333 },
	 { 1, 3, 1, 5, 19, 37, 5, 111, 85 }
	 };


SobolGenerator* SobolGenerator_NewFromTable( Name name, unsigned int generatorIndex ) {
	/* Make sure that the number generators used from this function is smaller than the number in the table */
	Journal_Firewall(
		generatorIndex < sizeof(SobolGenerator_PolynomialDegree)/sizeof(unsigned int),
		Journal_Register( Error_Type, (Name)SobolGenerator_Type  ),
		"Error in func '%s' - Trying to instantiate a %dth SobolGenerator ('%s') using table, "
		"but only %d initial directional numbers have been implemented. Please contact developers.\n",
		__func__, generatorIndex + 1, name, sizeof(SobolGenerator_PolynomialDegree)/sizeof(unsigned int) );

	return SobolGenerator_New(
			name,
			SobolGenerator_PolynomialDegree[ generatorIndex ], 
			SobolGenerator_PolynomialCoefficient[ generatorIndex ],
			SobolGenerator_InitialDirectionNumbers[ generatorIndex ] );

}
