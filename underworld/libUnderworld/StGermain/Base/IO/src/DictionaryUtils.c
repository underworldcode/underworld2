/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>
#include <math.h>
#include <StGermain/libStGermain/src/StGermain.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "DictionaryUtils.h"

void _DictionaryUtils_AliasDereferenceStruct( Dictionary* masterDict, Dictionary_Entry_Value* valuePtr){
   Dictionary* subDictPtr = valuePtr->as.typeStruct;
   Dictionary_Entry* entryPtr=NULL;
   unsigned int index;

   for (index=0; index < subDictPtr->count; index++) {
      entryPtr = subDictPtr->entryPtr[index]; 
      _DictionaryUtils_AliasDereferenceEntryValue( masterDict, entryPtr->value );
   }
}

void _DictionaryUtils_AliasDereferenceList( Dictionary* masterDict, Dictionary_Entry_Value* valuePtr){
   Dictionary_Entry_Value* anode=NULL;
   Dictionary_Entry_Value_List* listPtr = valuePtr->as.typeList;

   /* if the list is empty do nothing */
   if( listPtr->count == 0  ) return ;

   anode = listPtr->first;
   do{
	   _DictionaryUtils_AliasDereferenceEntryValue( masterDict, anode ); 
	} while( (anode = anode->next) != NULL); 
}

Dictionary_Entry_Value* _DictionaryUtils_GetRecursive( Dictionary* dict, char* str ){
   char* strPoint = strchr( str, '.' );
   if(strPoint){
      Dictionary_Entry_Value* entryVal=NULL;
      /* Create the struct and member strings from the source. */
      char* structString = StG_Strdup( str );
      char* strPointNew = strchr( structString, '.' );
      *strPointNew = 0;
      char* memberString = strPointNew + 1;
      Dictionary_Entry_Value* subDict = Dictionary_Get( dict, structString );
      if( subDict ) entryVal = _DictionaryUtils_GetRecursive( Dictionary_Entry_Value_AsDictionary(subDict), memberString );
      Memory_Free( structString );
      return entryVal;
   } else {
      return Dictionary_Get( dict, str );
   }
}

void _DictionaryUtils_AliasDereferenceString( Dictionary* masterDict, Dictionary_Entry_Value* valuePtr){
   /* check if first character is @, otherwise nothing to do */
   if( valuePtr->as.typeString && (valuePtr->as.typeString)[0]=='@' ){ 
      char *aliasStr = valuePtr->as.typeString + 1;
      Dictionary_Entry_Value* refVal = _DictionaryUtils_GetRecursive( masterDict, aliasStr ); 

      /* If there isn't an entry, fail */
      Journal_Firewall(
         (refVal!=NULL),
         Journal_Register( Error_Type, Dictionary_Type ),
         "Alias \"%s\" not found in dictionary.",
         aliasStr-1 );
      /* check we arent referencing ourself! */
      Journal_Firewall(
         (refVal!=valuePtr) ,
         Journal_Register( Error_Type, Dictionary_Type ),
         "Alias \"%s\" references itself!",
         aliasStr-1 );
      /* If is found in dictionary - fist check if it requires dereferencing */
      /* note that circular dependencies are possible */
      _DictionaryUtils_AliasDereferenceString( masterDict, refVal );
      Dictionary_Entry_Value_SetFromStringKeepCurrentType( valuePtr, refVal->as.typeString );
   }
}

void _DictionaryUtils_AliasDereferenceEntryValue( Dictionary* masterDict, Dictionary_Entry_Value* valuePtr ) {
   switch( valuePtr->type ) {
   case Dictionary_Entry_Value_Type_Struct:
      /* recursive case - to scale a dictionary */
      _DictionaryUtils_AliasDereferenceStruct( masterDict, valuePtr );
      break;
   case Dictionary_Entry_Value_Type_List:
      /* recursive case - to scale a list */
      _DictionaryUtils_AliasDereferenceList( masterDict, valuePtr );
      break;
   case Dictionary_Entry_Value_Type_String:
      /* dereference value as required */
      _DictionaryUtils_AliasDereferenceString( masterDict, valuePtr );
      break;
   default:
      /* if otherwise, nothing to do */
      break;
   }
}

void DictionaryUtils_AliasDereferenceDictionary( Dictionary* masterDict ) {
   Dictionary_Entry* entryPtr=NULL;
   unsigned int index;

   for (index=0; index < masterDict->count; index++) {
      entryPtr = masterDict->entryPtr[index]; 
      _DictionaryUtils_AliasDereferenceEntryValue( masterDict, entryPtr->value );
   }
}
