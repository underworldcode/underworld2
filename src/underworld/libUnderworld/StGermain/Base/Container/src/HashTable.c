/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <StGermain/Base/Foundation/src/Foundation.h>
#include <StGermain/Base/IO/src/IO.h>

#include "types.h"
#include "HashTable.h"
#include "PtrMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

const Type HashTable_Type = "HashTable";
#define MAX_ENTRIES 255
#define SM_HEAP_TABLE_SIZE 256
/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/
HashTable* _HashTable_New( HASHTABLE_DEFARGS ) {
   HashTable *self = NULL;

   self = (HashTable*)_Stg_Class_New( STG_CLASS_PASSARGS );
   return self;
}

HashTable* HashTable_New(
   HashTable_dataCopyFunction*   dataCopyFunction,
   HashTable_dataPrintFunction*  dataPrintFunction,
   HashTable_dataDeleteFunction* dataDeleteFunction,
   KeyType                       keyType)
{
   /* Variables set in this function */
   SizeT                     _sizeOfSelf = sizeof(HashTable);
   Type                             type = HashTable_Type;
   Stg_Class_DeleteFunction*     _delete = _HashTable_DeleteFunc;
   Stg_Class_PrintFunction*       _print = _HashTable_PrintFunc;
   Stg_Class_CopyFunction*         _copy = _HashTable_CopyFunc;
   
   HashTable* self;
   
   /* General info */
   self = _HashTable_New( HASHTABLE_PASSARGS );

   /* List info */
   /* At the moment only String keys are supported */
   /*TODO 
    * add a hash function for hashing numeric keys */

   if( keyType == HASHTABLE_STRING_KEY ) {
      self->hashFunction = hashStringFunction;
   }
   else if( keyType == HASHTABLE_POINTER_KEY ) {
      self->hashFunction = hashPtrFunction;
   }

   self->dataCopyFunction = dataCopyFunction;
   self->dataPrintFunction = dataPrintFunction;
   self->dataDeleteFunction = dataDeleteFunction;
   self->keyType = keyType;
   
   /* Virtual functions */
   HashTable_Init( self );
   return self;
}

void _HashTable_Init( HashTable* self ) {
   /* General info */
   
   assert( self );
   _Stg_Class_Init ((Stg_Class*) self);
   
   self->max = MAX_ENTRIES;
   self->entries = Memory_Alloc_Array_Unnamed(HashTable_Entry*, sizeof( HashTable_Entry* ) * (self->max+1) );
   memset( self->entries, 0, sizeof( HashTable_Entry* ) * (self->max+1) );

   /* Dictionary info */
}

void HashTable_Init( HashTable *self ) {
   assert( self );
   _HashTable_Init (self);
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual Functions
*/
void _HashTable_PrintFunc ( void *ht, Stream *stream ) {
   HashTable *self = NULL;
   
   self = (HashTable*) ht;
   assert( self );
   assert( stream );

   /* print parent */
   _Stg_Class_Print( (void*) self, stream );

   /* general info */
   Journal_Printf( stream, "HashTable (ptr): (%p)\n", self );

   /* Virtual Info */

   /* HashTable Info */
   Journal_Printf( stream, "\tEntryCount\t\t - %d\n", self->count );
   Journal_Printf( stream, "\tHashTable KeyType\t - %s\n", (self->keyType == HASHTABLE_STRING_KEY)?"STRING":"INTEGER" );
   
   Journal_Printf( stream, "\tHashTable data\t - \n");
   if (self->dataPrintFunction)
      HashTable_ParseTable( self, (HashTable_parseFunction*)self->dataPrintFunction, (void*)stream );
}

void _HashTable_DeleteFunc( void *ht ) {
   HashTable *self = NULL;
   HashTable_Entry *he = NULL, *heTemp = NULL;
   int i = 0;

   self = (HashTable*)ht;
   assert (self);
   
   /* In the case of a ptrHash table at least, it's possible to have keys that hash to 255 - the default max. So,
    * need to check that entry for possible deletion. -- PatrickSunter, 5 Jun 2009 */
   for( i=0; i<self->max+1; i++ ) {
      he = self->entries[i];
      
      if(he) {
         while( he ) {
            if( self->dataDeleteFunction ) {
               self->dataDeleteFunction( (void*) he->data );
            }
            if( self->keyType == HASHTABLE_POINTER_KEY) {
               /* TODO: anything to be freed here? */
            }
            else {
               Memory_Free( (char*)he->key );
            }
            
            heTemp = he->next;
            /** Freeing the Entry without calling the Class_Delete function, because HashTable_Entry does not inherit __Class */
            Memory_Free( he ); 

            he = heTemp;
         }
      }
   }
   Memory_Free( self->entries );
   _Stg_Class_Delete( self );
}

void* _HashTable_CopyFunc( void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
   HashTable* self = (HashTable*)source;
   HashTable* newTable;
   HashTable_Index* hi;
   void* key;

   newTable = (HashTable*)_Stg_Class_Copy( self, NULL, deep, nameExt, ptrMap ) ;

   newTable->hashFunction = self->hashFunction;
   newTable->dataCopyFunction = self->dataCopyFunction;
   newTable->dataPrintFunction = self->dataPrintFunction;
   newTable->dataDeleteFunction = self->dataDeleteFunction;

   newTable->count = self->count;
   newTable->max = self->max;
   newTable->keyType = self->keyType;

   newTable->entries = Memory_Alloc_Array_Unnamed(HashTable_Entry*, sizeof( HashTable_Entry* ) * (self->max+1) );
   memset( newTable->entries, 0, sizeof( HashTable_Entry* ) * (self->max+1) );

   for ( hi = HashTable_First( self ); hi; hi = HashTable_Next( hi ) ) {
      void* dataPtr = PtrMap_Find( ptrMap, hi->curr->data );
      if ( dataPtr == NULL ) {
         if ( self->dataCopyFunction ) {
            /* data copy */
            dataPtr = self->dataCopyFunction( hi->curr->data, NULL, deep, nameExt, ptrMap );
         }
         else {
            dataPtr = hi->curr->data;
         }
      }
      key = PtrMap_Find( ptrMap, (void*)hi->curr->key );
      if ( key == NULL ) {
         /* Assume a const key so just assign */
         key = (void*)hi->curr->key;
      }
         
      HashTable_InsertEntry( newTable, key, hi->curr->keyLength, dataPtr, hi->curr->dataSize );
   }
   
   return newTable;
}

static unsigned char hashtab[SM_HEAP_TABLE_SIZE] = {
   161, 71, 77,187, 15,229,  9,176,221,119,239, 21, 85,138,203, 86,
   102, 65, 80,199,235, 32,140, 96,224, 78,126,127,144,  0, 11,179,
   64, 30,120, 23,225,226, 33, 50,205,167,130,240,174, 99,206, 73,
   231,210,189,162, 48, 93,246, 54,213,141,135, 39, 41,192,236,193,
   157, 88, 95,104,188, 63,133,177,234,110,158,214,238,131,233, 91,
   125, 82, 94, 79, 66, 92,151, 45,252, 98, 26,183,  7,191,171,106,
   145,154,251,100,113,  5, 74, 62, 76,124, 14,217,200, 75,115,190,
   103, 28,198,196,169,219, 37,118,150, 18,152,175, 49,136,  6,142,
   89, 19,243,254, 47,137, 24,166,180, 10, 40,186,202, 46,184, 67,
   148,108,181, 81, 25,241, 13,139, 58, 38, 84,253,201, 12,116, 17,
   195, 22,112, 69,255, 43,147,222,111, 56,194,216,149,244, 42,173,
   232,220,249,105,207, 51,197,242, 72,211,208, 59,122,230,237,170,
   165, 44, 68,123,129,245,143,101,  8,209,215,247,185, 57,218, 53,
   114,121,  3,128,  4,204,212,146,  2,155, 83,250, 87, 29, 31,159,
   60, 27,107,156,227,182,  1, 61, 36,160,109, 97, 90, 20,168,132,
   223,248, 70,164, 55,172, 34, 52,163,117, 35,153,134, 16,178,228
};

/*
**  PTRHASH -- hash a pointer value
**
**Parameters:
**p -- pointer.
**
**Returns:
**hash value.
**
**  ptrhash hashes a pointer value to a uniformly distributed random
**  number between 0 and 255.
**
**  This hash algorithm is based on Peter K. Pearson,
**  "Fast Hashing of Variable-Length Text Strings",
**  in Communications of the ACM, June 1990, vol 33 no 6.
*/

unsigned int hashPtrFunction( const void *p, const unsigned int keyLen ) {
   int h;
   unsigned long n = (unsigned long)p;

   /* NOTE: these #if makes keyLen unused */

#if defined(SYSTEM_SIZEOF_LONG) && SYSTEM_SIZEOF_LONG == 8
   h = hashtab[n & 0xFF];
   h = hashtab[h ^ ((n >> 8) & 0xFF)];
   h = hashtab[h ^ ((n >> 16) & 0xFF)];
   h = hashtab[h ^ ((n >> 24) & 0xFF)];
   h = hashtab[h ^ ((n >> 32) & 0xFF)];
   h = hashtab[h ^ ((n >> 40) & 0xFF)];
   h = hashtab[h ^ ((n >> 48) & 0xFF)];
   h = hashtab[h ^ ((n >> 56) & 0xFF)];
#else
   /* Else assume 32 bit system. Watch this get invalidated one day =) */
   h = hashtab[n & 0xFF];
   h = hashtab[h ^ ((n >> 8) & 0xFF)];
   h = hashtab[h ^ ((n >> 16) & 0xFF)];
   h = hashtab[h ^ ((n >> 24) & 0xFF)];
#endif
      
   return h;
}

unsigned int hashStringFunction( const void *voidKey, const unsigned int keyLen ) {
   /*
    * This is the popular `times 33' hash algorithm which is used by
    * perl and also appears in Berkeley DB. This is one of the best
    * known hash functions for strings because it is both computed
    * very fast and distributes very well.
    *
    * The originator may be Dan Bernstein but the code in Berkeley DB
    * cites Chris Torek as the source. The best citation I have found
    * is "Chris Torek, Hash function for text in C, Usenet message
    * <27038@mimsy.umd.edu> in comp.lang.c , October, 1990." in Rich
    * Salz's USENIX 1992 paper about INN which can be found at
    * <http://citeseer.nj.nec.com/salz92internetnews.html>.
    *
    * The magic of number 33, i.e. why it works better than many other
    * constants, prime or not, has never been adequately explained by
    * anyone. So I try an explanation: if one experimentally tests all
    * multipliers between 1 and 256 (as I did while writing a low-level
    * data structure library some time ago) one detects that even
    * numbers are not useable at all. The remaining 128 odd numbers
    * (except for the number 1) work more or less all equally well.
    * They all distribute in an acceptable way and this way fill a hash
    * table with an average percent of approx. 86%.
    *
    * If one compares the chi^2 values of the variants (see
    * Bob Jenkins ``Hashing Frequently Asked Questions'' at
    * http://burtleburtle.net/bob/hash/hashfaq.html for a description
    * of chi^2), the number 33 not even has the best value. But the
    * number 33 and a few other equally good numbers like 17, 31, 63,
    * 127 and 129 have nevertheless a great advantage to the remaining
    * numbers in the large set of possible multipliers: their multiply
    * operation can be replaced by a faster operation based on just one
    * shift plus either a single addition or subtraction operation. And
    * because a hash function has to both distribute good _and_ has to
    * be very fast to compute, those few numbers should be preferred.
    *
    *                  -- Ralf S. Engelschall <rse@engelschall.com>
    */
   
   /*
    * The above comment and the hash algorithm has been taken from 
    * subversion's hashFunction.
    */
   
   unsigned int hash = 0;
   unsigned char *key;
   unsigned char *p = (unsigned char*) 0;
   unsigned int i = 0;
   
   key = (unsigned char*)voidKey;

   hash = 0;
   for( p=key, i=keyLen; i; i--, p++ ) {
      hash = hash * 33 + *p;
   }

   return hash;
}

/*----------------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/
int HashTable_InsertEntry ( HashTable *ht, const void *voidKey, unsigned int keyLen, void *data, SizeT dataSize ) {
   unsigned int hash = 0;
   HashTable_Entry *he = NULL, **hep = NULL;
   unsigned char *key;
   int count;
   
   assert( ht );
   assert( data );
   assert( ht->hashFunction );

   key = (unsigned char*) voidKey;
   
   if( ht->keyType == HASHTABLE_POINTER_KEY ) {
      hash = ht->hashFunction( voidKey, sizeof( void* ) );
      
      count = 0;
      for (hep = &ht->entries[hash], he = *hep; he; hep = &he->next, he = *hep) {
         if (he->hash == (unsigned int)hash && ((void*)he->key == (void*)voidKey)) {
            fprintf( stdout, "trying to enter duplicate keys\n" );
            return 0;
         }
         count++;
      }
   }
   else {
      hash = ht->hashFunction( voidKey, keyLen );

      while( (hash & ht->max) > ht->max - 1 ) {
         expandArray( ht );
      }
   
      count = 0;
      for (hep = &ht->entries[hash & ht->max], he = *hep; he; hep = &he->next, he = *hep) {
         if (he->hash == hash && he->keyLength == keyLen && memcmp(he->key, key, keyLen) == 0) {
            fprintf( stderr, "trying to enter duplicate keys\n" );
            return 0;
         }
         count++;
      }
   }
   
   he = Memory_Alloc( HashTable_Entry , "HashTable_Entry");
   memset( he, 0, sizeof( HashTable_Entry ) );
   he->next = NULL;
   he->hash = hash;
   
   if( ht->keyType== HASHTABLE_POINTER_KEY) {
      he->key = (void*)key;
   }
   else {
      he->key = StG_Strdup( (char *)key );
   }
   
   he->keyLength = keyLen;

   he->data = data;
   he->dataSize = dataSize;
   ht->count++;
   *hep = he;
      
   /*returning the number of collisions incurred. 0 means no collisions and a new entry
    * at that index, 1 means 1 collision, and so on */    
   return count; 
}

int HashTable_InsertEntryCopyData ( HashTable *ht, const void *voidKey, unsigned int keyLen, void *data, SizeT dataSize ) {
   void* dataToInsert = data;
   if ( ht->dataCopyFunction ) {
      dataToInsert = ht->dataCopyFunction( data, NULL, True, NULL, NULL );
   }
   return HashTable_InsertEntry( ht, voidKey, keyLen, dataToInsert, dataSize );
}

void *HashTable_FindEntryFunction( HashTable *ht, const void *voidKey, unsigned int keyLen ) {
   unsigned char *key;
   unsigned int hash = 0;
   HashTable_Entry *he = NULL, **hep = NULL;
   
   assert( ht );
   assert( ht->hashFunction );

   if( ht->keyType == HASHTABLE_POINTER_KEY ) {
      hash = ht->hashFunction( voidKey, sizeof( void* ) );
      
      for (hep = &ht->entries[hash], he = *hep;
         he; hep = &he->next, he = *hep) {
      
         if (he->hash == hash
            && ((void*)he->key == (void*)voidKey)) {
         
            break;
         }
      }
   }
   else {
      key = (unsigned char*)voidKey;
      hash = ht->hashFunction( voidKey, keyLen );

      for (hep = &ht->entries[hash & ht->max], he = *hep; he; hep = &he->next, he = *hep) {
         if (he->hash == hash && he->keyLength == keyLen && memcmp(he->key, key, keyLen) == 0) {
            break;
         }
      }
   }

   if( he ) {
      return (void*)he->data;
   }
   
   return NULL;
}

int HashTable_DeleteEntry( HashTable* ht, const void *voidKey, unsigned int keyLen ) {
   unsigned char*    key;
   unsigned int      hash = 0;
   HashTable_Entry*  he   = NULL; 
   HashTable_Entry** hep  = NULL;
   
   assert( ht );
   assert( ht->hashFunction );

   key = (unsigned char*)voidKey;
   assert( key );
   hash = ht->hashFunction( voidKey, keyLen );

   for (hep = &ht->entries[hash & ht->max], he = *hep; he; hep = &he->next, he = *hep) {
      if ( he->hash == hash && he->keyLength == keyLen && memcmp(he->key, key, keyLen) == 0) {
         break;
      }
   }

   if( he ) {
      *hep = he->next;
      ht->count--;
      if( ht->dataDeleteFunction ) {
         ht->dataDeleteFunction( he->data );
      }
      else {
         /* Leaving the data inside the entry */
      }
      
      Memory_Free( he );
      return 1;
   }

   return 0;
}

int HashTable_ReplaceEntry( HashTable *ht, const void *voidKey, unsigned int keyLen, void *data, SizeT dataSize ) {
   if ( HashTable_FindEntryFunction( ht, voidKey, keyLen ) ) {
      HashTable_DeleteEntry( ht, voidKey, keyLen );
   }
   return HashTable_InsertEntry( ht, voidKey, keyLen, data, dataSize );
}

void HashTable_ParseTable( HashTable *ht, HashTable_parseFunction *parseFunction, void *args ) {
   HashTable *self = NULL;
   HashTable_Entry *he = NULL;
   int i = 0;

   self = (HashTable*)ht;
   assert( self );
   assert( parseFunction );
   
   /* In the case of a ptrHash table at least, it's possible to have keys that hash to 255 - the default max. So,
    * need to check that entry for possible handling. -- PatrickSunter, 5 Jun 2009 */
   for( i=0; i<(self->max+1); i++ ) {
      he = self->entries[i];
      
      if(he) {
         while( he ) {
            parseFunction( he->data, args );
            he = he->next;
         }
      }
   }
}

void expandArray( HashTable *ht ) {
   unsigned int newMax = 0;
   HashTable_Entry **newArray = NULL;
   HashTable_Index *hi = NULL;

   assert( ht );
   
   newMax = ht->max * 2 + 1;
   newArray = Memory_Alloc_Array_Unnamed(HashTable_Entry*, sizeof( HashTable_Entry* ) * newMax );
   if( !newArray ) {
      Journal_Firewall(0, Journal_Register(ErrorStream_Type,"HashTable"), "Out of memory in '%s'\n", __func__ );
   }
   memset( newArray, 0, sizeof( HashTable_Entry* ) * newMax );
   
    for (hi = HashTable_First( ht ); hi; hi = HashTable_Next( hi )) {
        unsigned int i = hi->curr->hash & newMax;
        hi->curr->next = newArray[i];
        newArray[i] = hi->curr;
    }
   
   ht->entries = newArray;
   ht->max = newMax;
}

HashTable_Index* HashTable_Next( HashTable_Index *hi ) {
   assert( hi );
   
   hi->curr= hi->next;
   while (!hi->curr) {
      if (hi->index > hi->ht->max - 1)
         return NULL;

      hi->curr= hi->ht->entries[hi->index++];
   }
   hi->next = hi->curr->next;

   return hi;
}

HashTable_Index* HashTable_First( HashTable *ht ) {
   HashTable_Index *hi;
    
   assert( ht );
   hi = &ht->iterator;

   hi->ht = ht;
   hi->index = 0;
   hi->curr= NULL;
   hi->next = NULL;
    
   return HashTable_Next( hi );
}


