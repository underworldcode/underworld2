/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "types.h"
#include "forwardDecl.h"
                                                                                                                                    
#include "Memory.h"
#include "shortcuts.h"
                                                                                                                                    
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <math.h>

const char* StG_BoolToStringMap[2] = { "False", "True" };


double StG_RoundDoubleToNSigFigs( double value, unsigned int nSigFigs ) {
	double divisorPower;
	double sign = 1.0;
	double toleranceAroundZero = 1e-60;

	assert( nSigFigs >= 1 );
	
	/* Log goes to infinity at 0. Hence, we have to give up rounding if it gets too small */
	if ( (value > (0.0 - toleranceAroundZero)) && (value < (0.0 + toleranceAroundZero)) ) {
		return value;
	}	
	
	/* Since logs can't deal with negatives, need to save the sign */
	if ( value < 0 ) {
		sign = -1.0;
		value = fabs(value);
	}

	/* Since interested in significant figures, need to work out the number of 
	zeros in the answer */
	divisorPower = log10( value );
	divisorPower = floor( divisorPower );

	/* We now have the divisorPower to use if nSigFigs == 1. So adjust if this isn't
	the case */
	divisorPower -= ( nSigFigs - 1 );

	/* Similar approach to the decimal places rounder from here, except we use
	the divisorPower we've just calculated */
	return sign * floor( value / pow( 10., divisorPower ) + 0.5 ) * pow(10., divisorPower);
}


double StG_RoundDoubleToNDecimalPlaces( double value, unsigned int nDecimalPlaces ) {
	return floor( value * pow(10., nDecimalPlaces) + .5) / pow(10., nDecimalPlaces);
}


unsigned int StG_IntegerLength( int number ) {
	int tmpNumber;
	int numDigits;
	Bool isNegative;

	tmpNumber = abs( number );
	numDigits = 0;
	isNegative = ( number < 0 ) ? True : False;
	
	do {
		++numDigits;
		tmpNumber /= 10; /* Base 10 */
	} while ( tmpNumber > 0 );

	if ( isNegative )
		++numDigits;

	return numDigits;
}
	


char* StG_Strdup( const char* const str ) {
	int length;
	char* result;

        if( !str ) return NULL;
	length = strlen( str ) + 1;
	result = Memory_Alloc_Array_Unnamed( char, length );

	strcpy( result, str );

	return result;
}


int Stg_vasprintf( char** string, char* format, va_list ap ) {
	int       allocStringSize = 100;
	int       sizeOfString;

	va_list   apCopy;

	Bool      trying = True;

	/* Allocate Memory For String */
	*string = Memory_Alloc_Array( char, allocStringSize, "string" );

	while ( trying ) {
		/* See is we can print to this string with this size */
#ifdef NO_VA_COPY
                apCopy=ap;
#else
		va_copy( apCopy, ap );
#endif
		sizeOfString = vsnprintf( *string, allocStringSize, format, apCopy );
		va_end( apCopy );

		/* Check if string was truncated */
		if ( sizeOfString > -1 && sizeOfString < allocStringSize ) {
			return sizeOfString;
		}
		/* String was truncated - resize array and try again */
		if ( sizeOfString > -1 ) {
			/* glibc > 2.1 */
			allocStringSize = sizeOfString + 1;
		}
		else {
			/* glibc <= 2.0 */
			allocStringSize = sizeOfString * 2;
		}
		*string = Memory_Realloc_Array( *string, char, allocStringSize );
		if ( string == NULL ) {
			#ifdef DEBUG
				assert( string != NULL );
			#endif
			trying = False;
		}
	}
		
	return sizeOfString;
}


int Stg_asprintf( char** string, char* format, ... ) {
	int       sizeOfString;
	va_list   ap;
	
	va_start( ap, format );
	sizeOfString = Stg_vasprintf( string, format, ap ) ;
	va_end( ap );

	return sizeOfString;
}

Bool Stg_StringIsEmpty( char* string ) {
	size_t     length;
	const char whiteSpaceChars[] = " \t\n";

	if ( string == NULL ) 
		return True;
	
	/* Check if the length is Zero */
	length = strlen( string );
	if ( length == 0 )
		return True;

	/* Check if there are only white space characters */
	if ( length == strspn( string, whiteSpaceChars ) )
		return True;

	return False;
}

Bool Stg_StringIsNumeric( char* string ) {
	size_t         length;
	Index          char_I;
	char           ch;
	unsigned int   eCount            = 0;
	unsigned int   decimalPointCount = 0;
	unsigned int   digitCount        = 0;

	if ( string == NULL ) 
		return False;

	length = strlen( string );
	if ( length == 0 ) 
		return False;

	/* The conversion of HUGE_VAL by printf is 'inf' which requires this line to 
	 * pick up that this is actually a numerical string */
	if ( strcmp( string, "-inf" ) == 0 || strcmp( string, "inf" ) == 0  )
		return True;

	for ( char_I = 0 ; char_I < length ; char_I++ ) {
		ch = string[ char_I ];

		/* Check for digits from 0 - 9 */
		if ( isdigit( ch ) ) {
			digitCount++;
			continue;
		}

		/* Check for 'e' or 'E' for scientific notation - only one allowed */
		if ( ( ch == 'e' || ch == 'E' ) && eCount == 0 ) {
			eCount++;

			/* Reset counters because another number is allowed */
			decimalPointCount = 0;
			digitCount        = 0;
			continue;
		}
		
		/* Check for decimal point - one allowed before 'e' and one allowed after 'e' */
		if ( ch == '.' && decimalPointCount == 0 ) {
			decimalPointCount++;
			continue;
		}

		/* Check for plus for minus sign - only allowed at beginning of number */
		if ( (ch == '+' || ch == '-') && (digitCount == 0 && decimalPointCount == 0) )
			continue;

		return False;
	}

	/* If all chars pass - then return happy */
	return True;
}

/** This function calculates the of the longest matching subsequence between string1 and string2.
 *  A subsequence of a string is when all the characters of the subsequence appear in order within the string,
 *  but possibly with gaps between occurrences of each character. */
unsigned int Stg_LongestMatchingSubsequenceLength( char* string1, char* string2, Bool caseSensitive ) {
	size_t         stringLength1 = strlen( string1 );
	size_t         stringLength2 = strlen( string2 );
	unsigned int** lmsArray;
	Index          charString1_I;
	Index          charString2_I;
	char           charString1;
	char           charString2;
	Bool           areEqual;
	unsigned int   lmsLength;

	/* This array 'lmsArray' is defined as follows: 
	 * lmsArray[i][j] = the length of the longest matching subsequence in string1[ 0 ... (i-1) ] and string[ 0 ... (j-1) ]
	 * lmsArray[0][j] and lmsArray[i][0] are initialised to zero for all i, j */
	lmsArray = Memory_Alloc_2DArray( unsigned int, stringLength1 + 1, stringLength2 + 1, "lmsArray" );
	memset( lmsArray[0], 0, sizeof( unsigned int ) * (stringLength1 + 1) * (stringLength2 + 1) );

	for ( charString1_I = 1 ; charString1_I <= stringLength1 ; charString1_I++ ) {
		charString1 = string1[ charString1_I - 1 ];
		for ( charString2_I = 1 ; charString2_I <= stringLength2 ; charString2_I++ ) {
			charString2 = string2[ charString2_I - 1 ];

			areEqual = (Bool)( caseSensitive ? charString1 == charString2 :  tolower( charString1 ) == tolower( charString2 ) );

			/* If the two characters are equal then this character is the last in the longest subsequence of 
			 * string1[0 ... charString1_I - 1 ] and string2[0 ... charString2_I - 1 ] therefore the length is 
			 * one more than the previous longest subsequence - 
			 * namely lmsArray[ charString1_I - 1 ][ charString2_I - 1 ] + 1 
			 *
			 * If the characters arn't equal then the length is the longest subseqence from 
			 * string1[0 ... charString1_I - 1 ] or string2[0 ... charString2_I - 1 ] */
			
			lmsArray[ charString1_I ][ charString2_I ] = 
				( areEqual )
					? lmsArray[ charString1_I - 1 ][ charString2_I - 1 ] + 1
					: MAX( lmsArray[ charString1_I - 1 ][ charString2_I ], lmsArray[ charString1_I ][ charString2_I - 1 ] );
		}
	}

	lmsLength = lmsArray[ stringLength1 ][ stringLength2 ];
	Memory_Free( lmsArray );

	return lmsLength;
}
					


