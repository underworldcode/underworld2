/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_CommonRoutines_h__
#define __StGermain_Base_Foundation_CommonRoutines_h__
	#include <stdarg.h>

	/** Global map from a Boolean's enum value to a string: for printing purposes */
	extern const char* StG_BoolToStringMap[2];
	
	/** Rounds a double to the specified number of significant figures */
	double StG_RoundDoubleToNSigFigs( double value, unsigned int nSigFigs );

	/** Rounds a double to a certain number of decimal places */
	double StG_RoundDoubleToNDecimalPlaces( double value, unsigned int nDecimalPlaces );
	
	/** Counts the number of characters required to display the given base 10 value. */
	unsigned int StG_IntegerLength( int number );

	/** StGermain's version of strdup() which uses Memory Module */
	char* StG_Strdup( const char* const str );

	/** StGermain's version of asprintf which allocates enough space for a string before printing to it */
	int Stg_asprintf( char** string, char* format, ... ) ;
	int Stg_vasprintf( char** string, char* format, va_list ap ) ;

	/** Check to see whether string is empty or only has white space */
	Bool Stg_StringIsEmpty( char* string ) ;

	/** Check to see whether string is can be considered to be a number - 
	 * allows digits, plus and minus signs, and scientific notation */
	Bool Stg_StringIsNumeric( char* string ) ;

	/** This function calculates the of the longest matching subsequence between string1 and string2.
	 *  A subsequence of a string is when all the characters of the subsequence appear in order within the string,
	 *  but possibly with gaps between occurrences of each character. */
	unsigned int Stg_LongestMatchingSubsequenceLength( char* string1, char* string2, Bool caseSensitive ) ;

#endif /* __StGermain_Base_Foundation_CommonRoutines_h__ */
