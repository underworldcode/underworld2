/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "ContextEntryPoint.h"
#include <mpi.h>  /* subsequent files need this */
#include "DictionaryCheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



void CheckDictionaryKeys( Dictionary* dictionary, const char* errorMessage)
{
	Dictionary_Index index_I, index_J;
	/* Put in the Journal_Firewall stream */
	Stream* errStream = Journal_Register( Error_Type, "DictionaryCheck");
	int errCount;
	int** keyIndexArray;
	Index errIndex = 0;
	Bool  alreadyFoundRepeat = False;

	keyIndexArray = Memory_Alloc_2DArray( int, ((dictionary->count)*(dictionary->count - 1)), 
					2, "Key Index Array" );
	/* Iterate through the whole dictionary*/
	errCount = 0;
	for ( index_I = 0; index_I < dictionary->count; ++index_I )
	{
		alreadyFoundRepeat = False;
		/* First, check if the current key has already been found & flagged as a repeat*/
		for ( errIndex=0; errIndex < errCount; errIndex++) {
			if ( index_I == keyIndexArray[errIndex][1] ) {
				alreadyFoundRepeat = True;
				break;
			}
		}
		if (alreadyFoundRepeat==True) continue; 

		/*For Each key, search through dictionary to see if there is another 
		key the same*/
		for (index_J = index_I+1; index_J < dictionary->count; ++index_J ) {
			if (index_J != index_I)
			{
				/* If there are two keys with the same name */
				if ( (0 == strcasecmp( dictionary->entryPtr[index_I]->key, 
					dictionary->entryPtr[index_J]->key)) )
				{
					/* Check for the case that this is a 2nd or later 
					 * repeat - in which case we don't need to add it */ 	
					for ( errIndex=0; errIndex < errCount; errIndex++) {
						if ( index_J == keyIndexArray[errIndex][0] ) {
							alreadyFoundRepeat = True;
							break;
						}
					}
					if (alreadyFoundRepeat==True) break;

/* 					preserve indexes index_I, index_J */
					keyIndexArray[errCount][0] = index_I;
					keyIndexArray[errCount][1] = index_J;					
/* 					increment counter */
					errCount++;
					break;
				}
			}
		}
	}
	/*if keyIndexArray is not empty, then do a print to error stream 
	for each problem then call Journal_Firewall */
	

	if (errCount > 0) {
		Index errIndex;
		Journal_Printf(errStream, errorMessage);
		Journal_Printf(errStream,"Error found in given dictionary:\n", dictionary);
		Journal_Printf(errStream,"The following keys were repeated:\n");
		Stream_Indent(errStream);
		for (errIndex = 0; errIndex < errCount; errIndex++) {

			Journal_Printf(errStream, "\"%s\"\n",					 
				dictionary->entryPtr[keyIndexArray[errIndex][1]]->key );
		}
/* 		Do I need this one here if Journal_Firewall exits prog? */
		Stream_UnIndent(errStream);
		Journal_Printf(errStream, "Error in %s with %d entries in dictionary keys\n",
			       __func__, errCount);
		
	}
	Memory_Free(keyIndexArray);
}


