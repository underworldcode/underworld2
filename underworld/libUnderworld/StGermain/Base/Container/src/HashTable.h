/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_HashTable_h__
#define __StGermain_Base_Container_HashTable_h__

   /* Textual name for List class. */
   extern const Type HashTable_Type;
   
   /* Virtual Function Pointers */
   
   /* Hash function */   
   typedef unsigned int (HashTable_keyHashFunction) ( const void*, const unsigned int );
   
   /* 
    * This is a function type to be provided by the user for copying node data, 
    * when a new entry is being created.
    */
   /* typedef void (HashTable_dataCopyFunction) ( void **, void *, SizeT );*/
   typedef Stg_Class_CopyFunction HashTable_dataCopyFunction;

   /* This is a function type to be provided by the user for printing the contents of a entry's data */
   typedef void (HashTable_dataPrintFunction) ( void *, void * );
      
   /* This is a function type to be provided by the user for deleting node data, when an entry is being deleted */
   typedef void (HashTable_dataDeleteFunction) ( void * );
   
   /* 
    * This is a function type to be provided by the user, which can be applied to each node on the tree later on.
    * The first argument is the data stored inside a node and the second argument can be a single argument or a 
    * struct of arguments that need to be passed to the user-supplied function.
    */
   typedef void (HashTable_parseFunction) ( void *, void * );

   /* \def HashTable_Entry see __HashTable_Entry */
   #define __HashTable_Entry \
      struct HashTable_Entry *next; \
      unsigned int           hash; \
      const void             *key; \
      size_t                 keyLength; \
      void                   *data; \
      size_t                 dataSize;   

   struct HashTable_Entry { __HashTable_Entry };

   /* \def HashTable_Index see __HashTable_Index */
   #define __HashTable_Index \
      struct HashTable       *ht; \
      struct HashTable_Entry *curr, *next; \
      unsigned int           index;

   struct HashTable_Index { __HashTable_Index };

   /* \def __HashTable See __HashTable */
   #define __HashTable \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      \
      /* HashTable info */ \
      HashTable_keyHashFunction*    hashFunction; \
      HashTable_dataCopyFunction*   dataCopyFunction; \
      HashTable_dataPrintFunction*  dataPrintFunction; \
      HashTable_dataDeleteFunction* dataDeleteFunction; \
      struct HashTable_Entry**      entries; \
      struct HashTable_Index        iterator; \
      unsigned int                  count; \
      unsigned int                  max; \
      KeyType                       keyType;

   struct HashTable { __HashTable };

   /* Constructor interface. */
   HashTable* HashTable_New(
      HashTable_dataCopyFunction*   dataCopyFunction,
      HashTable_dataPrintFunction*  dataPrintFunction,
      HashTable_dataDeleteFunction* dataDeleteFunction,
      KeyType keyType);
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define HASHTABLE_DEFARGS \
      STG_CLASS_DEFARGS

   #define HASHTABLE_PASSARGS \
      STG_CLASS_PASSARGS

   HashTable* _HashTable_New( HASHTABLE_DEFARGS );

   /* Init interface. */
   void HashTable_Init( HashTable* self );
   
   void _HashTable_Init( HashTable* self );
   
   /* Delete interface. */
      /** Class delete function */
   void _HashTable_DeleteFunc( void *ht );
   
   /* Print interface. */
   /* Class print function */
   void _HashTable_PrintFunc( void *ht, Stream* stream );

   void* _HashTable_CopyFunc( void* source, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );

   /* Public functions */
   /* Inserts a new entry into the hashTable  */
   int HashTable_InsertEntry( HashTable *ht, const void *voidKey, unsigned int keyLen, void *data, SizeT dataSize );
      
   /* Public functions */
   /* Inserts a new entry into the hashTable copying the data with the copy function if any */
   int HashTable_InsertEntryCopyData( HashTable *ht, const void *voidKey, unsigned int keyLen, void *data, SizeT dataSize );
      
   /* Attempts to find a key and returns the data within, if found */
   void *HashTable_FindEntryFunction( HashTable *ht, const void *voidKey, unsigned int keyLen );
      
   #define HashTable_FindEntry( ht, voidKey, keyLen, type ) \
      (type*) HashTable_FindEntryFunction( ht, voidKey, keyLen )
   
   /* Deletes a node from the list with the node-data being passed in as a parameter */
   int HashTable_DeleteEntry( HashTable *ht, const void *voidKey, unsigned int keyLen );

   /* Replaces the data contents for given key */
   int HashTable_ReplaceEntry( HashTable *ht, const void *voidKey, unsigned int keyLen, void *data, SizeT dataSize );
      
   /* Parses the list and executes a user-supplied function */
   void HashTable_ParseTable( HashTable *ht, HashTable_parseFunction *parseFunction, void *args );
   
   unsigned int hashStringFunction( const void *voidKey, const unsigned int keyLen );

   unsigned int hashPtrFunction( const void *p, const unsigned int keyLen );
   
   void expandArray( HashTable * );
   
   struct HashTable_Index* HashTable_Next( struct HashTable_Index *hi );
   struct HashTable_Index* HashTable_First( struct HashTable *ht );

#endif /* __StGermain_Base_Container_HashTable_h__ */


