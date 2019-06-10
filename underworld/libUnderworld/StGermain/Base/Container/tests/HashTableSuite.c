/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pcu/pcu.h"
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/IO/IO.h"
#include "StGermain/Base/Container/Container.h"
#include "HashTableSuite.h"

unsigned int NumWords_Global = 0; 
/* Expect to read in ~100 words */
char* WordList_Global[1000];

typedef struct {
   HashTable* table;
   HashTable* ptrTable;
} HashTableSuiteData;

typedef struct DataRef {
   void* ptr;
   Bool  found;
} DataRef;

void deleteFunction( void *entry ) {
   assert( entry );

   Memory_Free( entry );
}

void printFunction( void *entry, void *stream ) {
   
   assert( entry );
   assert( stream );

   Journal_Printf( (Stream*)stream, "\t\t%s", (void*)entry );
}

void ptrPrintFunction( void *entry, void *stream ) {
   assert( entry );
   assert( stream );

   Journal_Printf( (Stream*)stream, "\t\t(ptr)%p\n", (void*)entry );
}

void checkFunction( void *entry, void *arg ) {
   int*  wordFoundArray = (int*)arg;
   Index ii;
   
   assert( entry );

   for (ii=0; ii<NumWords_Global; ii++) {
      if ( 0 == strcmp( (char*)entry, (char*)WordList_Global[ii] ) ) {
         wordFoundArray[ii] = True;
         break;
      }
   }
}

void checkPtrFunction( void *entry, void *arg ) {
   DataRef* dataRefs = (DataRef*)arg;
   Index    ii;
   
   assert( entry );

   for (ii=0; ii<NumWords_Global; ii++) {
      /* Deliberately comparing ptrs here, rather than strings */
      if ( entry == dataRefs[ii].ptr ) {
         dataRefs[ii].found = True;
         break;
      }
   }
}

void HashTableSuite_Setup( HashTableSuiteData* data ) {
   char  dictFilename[PCU_PATH_MAX];
   FILE* dictFile=NULL;
   char  wordBuffer[1024];

   /* Read in the words from the dictionary file */
   pcu_filename_input( "dictionary.txt", dictFilename );
   dictFile = fopen( dictFilename, "r+" );
   NumWords_Global=0;
   while( (fgets( wordBuffer, sizeof(wordBuffer), dictFile ) != NULL) ) {
      WordList_Global[NumWords_Global++] = StG_Strdup(wordBuffer);
   }
   fclose( dictFile );

   data->table = HashTable_New( NULL, printFunction, NULL, HASHTABLE_STRING_KEY );
   data->ptrTable = HashTable_New( NULL, ptrPrintFunction, deleteFunction, HASHTABLE_POINTER_KEY );
}

void HashTableSuite_Teardown( HashTableSuiteData* data ) {
   Index ii;

   Stg_Class_Delete( data->table );
   Stg_Class_Delete( data->ptrTable );
   for ( ii=0; ii < NumWords_Global; ii++ ) {
      Memory_Free( WordList_Global[ii] );
   }
   NumWords_Global=0;
}

void HashTableSuite_TestInsert( HashTableSuiteData* data ) {
   char  *word = NULL;
   int   result = 0;
   int   sum = 0;
   Bool  wordFoundArray[NumWords_Global];
   Index ii=0;
   
   /* Testing hash table with string keys..\n\n */
   /* Inserting data into the hash table\n\n */
   for (ii=0; ii<NumWords_Global; ii++) {
      word = StG_Strdup(WordList_Global[ii]);
      result = HashTable_InsertEntry( data->table, (void*)word, strlen(word), (void*)word, strlen(word));
      sum += result;
   }
   
   pcu_check_true( NumWords_Global == data->table->count );

   /* The count of insertion operations should be at least the number of entries we inserted */
   pcu_check_true( sum >= 0 );

   /* This is a basic check that the number of collisions was within reason. Perhaps a stricter
    * mathematical calc should really be performed here */
   /* Hash table has 255 places, only inserted 100 entries, therefore not many collisions expected */
   pcu_check_true( sum <= (data->table->count * 0.3) );
   
   /* Now parse the tree, and check the actual entries - note they won't necessarily be in same 'order'
    * while being parsed */
   for (ii=0; ii<NumWords_Global; ii++) {
      wordFoundArray[ii] = False;
   }   
   HashTable_ParseTable( data->table, checkFunction, wordFoundArray );
   for (ii=0; ii<NumWords_Global; ii++) {
      pcu_check_true( wordFoundArray[ii] == True );
   }
}

void HashTableSuite_TestSearch( HashTableSuiteData* data ) {
   char* word = NULL;
   Index ii=0;
   char* searchResult = NULL;

   /* Inserting data into the hash table\n\n */
   for (ii=0; ii<NumWords_Global; ii++) {
      word = StG_Strdup(WordList_Global[ii]);
      HashTable_InsertEntry( data->table, (void*)word, strlen(word), (void*)word, strlen(word));
   }
   
   for (ii=0; ii<NumWords_Global; ii++) {
      searchResult = (char*)HashTable_FindEntry( data->table, WordList_Global[ii], strlen(WordList_Global[ii]), char* );
      pcu_check_true( searchResult != NULL );
      pcu_check_streq( WordList_Global[ii], searchResult );
   }
}

void HashTableSuite_TestInsertPointers( HashTableSuiteData* data ) {
   char    *word = NULL;
   int     result = 0;
   int     sum = 0;
   DataRef dataRefs[NumWords_Global];
   Index   ii=0;

   for (ii=0; ii<NumWords_Global; ii++) {
      dataRefs[ii].ptr = NULL;
      dataRefs[ii].found = False;
   }
   /* Testing hash table with pointer keys..\n\n */
   /* Inserting data into the hash table\n\n */
   
   for (ii=0; ii<NumWords_Global; ii++) {
      word = StG_Strdup(WordList_Global[ii]);
      result = HashTable_InsertEntry( data->ptrTable, (void*)word, sizeof(void*), (void*)word, strlen(word));
      sum += result;
      dataRefs[ii].ptr = word;
   }
   
   pcu_check_true( NumWords_Global == data->ptrTable->count );

   /* The count of insertion operations should be at least the number of entries we inserted */
   pcu_check_true( sum >= 0 );

   /* This is a basic check that the number of collisions was within reason. Perhaps a stricter
    *  mathematical calc should really be performed here */
   /* Hash table has 255 places, only inserted 100 entries, therefore not many collisions expected */
   pcu_check_true( sum <= (int)(data->ptrTable->count * 0.4) );
   
   /* Now parse the tree, and check the actual entries - note they won't necessarily be in same 'order'
    * while being parsed */
   HashTable_ParseTable( data->ptrTable, checkPtrFunction, dataRefs );
   for (ii=0; ii<NumWords_Global; ii++) {
      pcu_check_true( dataRefs[ii].found == True );
   }
}

/* Searching for items in the table:\n\n */
void HashTableSuite_TestSearchPointers( HashTableSuiteData* data ) {
   char    *word = NULL;
   DataRef dataRefs[NumWords_Global];
   Index   ii=0;
   char*   searchResult = NULL;

   for (ii=0; ii<NumWords_Global; ii++) {
      dataRefs[ii].ptr = NULL;
      dataRefs[ii].found = False;
   }
   /* Inserting data into the hash table\n\n */
   for (ii=0; ii<NumWords_Global; ii++) {
      word = StG_Strdup(WordList_Global[ii]);
      HashTable_InsertEntry( data->ptrTable, (void*)word, sizeof(void*), (void*)word, strlen(word));
      dataRefs[ii].ptr = word;
   }
 
   for (ii=0; ii<NumWords_Global; ii++) {
      searchResult = (char*)HashTable_FindEntry( data->ptrTable, (void*)dataRefs[ii].ptr, 0, char* );
      pcu_check_true( searchResult != NULL );
      pcu_check_streq( WordList_Global[ii], searchResult );
   }
}

void HashTableSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, HashTableSuiteData );
   pcu_suite_setFixtures( suite, HashTableSuite_Setup, HashTableSuite_Teardown );
   pcu_suite_addTest( suite, HashTableSuite_TestInsert );
   pcu_suite_addTest( suite, HashTableSuite_TestSearch );
   pcu_suite_addTest( suite, HashTableSuite_TestInsertPointers );
   pcu_suite_addTest( suite, HashTableSuite_TestSearchPointers );
}


