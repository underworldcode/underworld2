/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
/* \file
** <b>Role:</b>
**   Facilitates a dictionary style repository of information.
**
** <b>Assumptions:</b>
**   calling Dictionary_Entry_Value_AsString puts the result into a temporary
**      buffer:- if you want to use it for other than printing, you should copy
**      it to your own string.
**
** <b>Comments:</b>
**   Made to be compatible/comparable/interchangable with Pythia's PropertySheets.
**   Currently, error handling is very basic - input values checked and set to
**   defaults if necessary, pointers checked, 0 returned when getting an invalid
**   value. Needs to be extended once we decide how we're simulating Journal.
**   Have made some facility to have the content as a void*... but not completed and not sure if necessary.
**   May need to add methods to remove entries from dictionaries and structs, but
**   not yet.
**   The Dictionary_Entry_Value_As... functions can accept the "self" as null, and will provide a zero-like or empty
**   string-like answer. This enables a simple line for accessing values (no need for error checking).
**
**
**/

#ifndef __StGermain_Base_IO_Dictionary_h__
#define __StGermain_Base_IO_Dictionary_h__
  
   /*< Index type for Dictionary, */ 
   typedef Index Dictionary_Index;
   
   /* Enum to dictionary add/merging type */
   typedef enum { 
      Dictionary_MergeType_Append, 
      Dictionary_MergeType_Merge, 
      Dictionary_MergeType_Replace 
   } Dictionary_MergeType;
   
   /* Textual name for Dictionary class. */
   extern const Type Dictionary_Type;
   
   /* Virtual Function pointers */
   typedef void (Dictionary_AddFunction) (void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
   typedef void (Dictionary_AddWithSourceFunction) (void* dictionary, Dictionary_Entry_Key key, 
      Dictionary_Entry_Value* value,
      Dictionary_Entry_Units unit,
      Dictionary_Entry_Source source );
   typedef Bool (Dictionary_SetFunction) (void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
   typedef Bool (Dictionary_SetWithSourceFunction) (void* dictionary, Dictionary_Entry_Key key, 
      Dictionary_Entry_Value* value, Dictionary_Entry_Source source );
   typedef Dictionary_Entry_Value* (Dictionary_GetFunction) (void* dictionary, Dictionary_Entry_Key key );
   typedef Dictionary_Entry_Source (Dictionary_GetSourceFunction) (void* dictionary, Dictionary_Entry_Key key );

   /* \def __Dictionary See Dictionary */
   #define __Dictionary \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      Dictionary_AddFunction*           add; \
      Dictionary_AddWithSourceFunction* addWithSource; \
      Dictionary_SetFunction*           set; \
      Dictionary_SetWithSourceFunction* setWithSource; \
      Dictionary_GetFunction*           get; \
      Dictionary_GetSourceFunction*     getSource; \
      /* Dictionary info */ \
      Dictionary_Index                  size; \
      Dictionary_Index                  delta; \
      Dictionary_Index                  count; \
      Dictionary_Entry**                entryPtr; \
      Stream*                           debugStream; \

   struct _Dictionary { __Dictionary };
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Dictionary
   */
   
   /* Initialise a Dictionary construct */
   Dictionary* Dictionary_New( void );
   
   /* Constructor interface. */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define DICTIONARY_DEFARGS \
      STG_CLASS_DEFARGS, \
      Dictionary_AddFunction*           add, \
      Dictionary_AddWithSourceFunction* addWithSource, \
      Dictionary_SetFunction*           set, \
      Dictionary_SetWithSourceFunction* setWithSource, \
      Dictionary_GetFunction*           get, \
      Dictionary_GetSourceFunction*     getSource

   #define DICTIONARY_PASSARGS \
      STG_CLASS_PASSARGS, \
      add, \
      addWithSource, \
      set, \
      setWithSource, \
      get, \
      getSource    
   
   Dictionary* _Dictionary_New(  DICTIONARY_DEFARGS  );
   
   /* Init interface. */
   void _Dictionary_Init( void* dictionary );
   
   /* Stg_Class_Delete dictionary implementation */
   void _Dictionary_Delete( void* dictionary );
   
   /* Print dictionary implementation */
   void _Dictionary_Print( void* dictionary, Stream* stream );
   
   /* Concise dictionary info print */
   void Dictionary_PrintConcise( void* dictionary, Stream* stream );
   
   /* Implementation of Stg_Class_Copy() */
   void* _Dictionary_Copy( void* indexMap, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );

   /* Empty the Dictionary's entry list and re-set count to zero.
    * Will also call Delete() on each entry currently in the dictionary.
    */     
   void Dictionary_Empty( void* dictionary );

   /* Add an entry to the dictionary... orignal implementation... appends keys */
   void Dictionary_Add( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );

   /* Add an entry to the dictionary... orignal implementation... appends keys, with source file */
   void Dictionary_AddWithSource(
      void*                   dictionary,
      Dictionary_Entry_Key    key,
      Dictionary_Entry_Value* value,
      Dictionary_Entry_Units  units,
      Dictionary_Entry_Source source );
   
   /* Add an entry to the dictionary... specifying how the entry values are merged if key present already */
   Dictionary_Entry_Value* Dictionary_AddMerge( 
      void*                   dictionary, 
      Dictionary_Entry_Key    key, 
      Dictionary_Entry_Value* value,
      Dictionary_MergeType    mergeType );
   
   /* Add an entry to the dictionary... specifying how the entry values are merged if key present already */
   Dictionary_Entry_Value* Dictionary_AddMergeWithSource( 
      void*                   dictionary, 
      Dictionary_Entry_Key    key, 
      Dictionary_Entry_Value* value,
      Dictionary_Entry_Units  units,
      Dictionary_MergeType    mergeType,
      Dictionary_Entry_Source source );

   /* Set a value in the dictionary */
   Bool Dictionary_Set( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
   
   /* Set a value in the dictionary */
   Bool Dictionary_SetWithSource(
      void*                   dictionary,
      Dictionary_Entry_Key    key,
      Dictionary_Entry_Value* value,
      Dictionary_Entry_Source source );

   /* Get a value from the dictionary */
   Dictionary_Entry_Value* Dictionary_Get( void* dictionary, Dictionary_Entry_Key key );

   /* Get a source from the dictionary */
   Dictionary_Entry_Source Dictionary_GetSource( void* dictionary, Dictionary_Entry_Key key );

   /* Get a value from the dictionary by index */
   Dictionary_Entry_Value* Dictionary_GetByIndex( void* dictionary, Dictionary_Index index );

   /* Get a value from the dictionary */
   Dictionary_Entry_Value* Dictionary_GetDefault(
      void*                   dictionary,
      Dictionary_Entry_Key    key,
      Dictionary_Entry_Value* value );
   
   /* Add an entry to the dictionary implementation */
   void _Dictionary_Add( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
   
   /* Add an entry to the dictionary implementation */
   void _Dictionary_AddWithSource(
      void*                   dictionary,
      Dictionary_Entry_Key    key,
      Dictionary_Entry_Value* value,
      Dictionary_Entry_Units  units,
      Dictionary_Entry_Source source );
   
   /* Set a value in the dictionary implementation */
   Bool _Dictionary_Set( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value );
   
   /* Set a value in the dictionary implementation */
   Bool _Dictionary_SetWithSource(
      void*                   dictionary,
      Dictionary_Entry_Key    key,
      Dictionary_Entry_Value* value,
      Dictionary_Entry_Source source );

   /* Get a value from the dictionary implementation */
   Dictionary_Entry_Value* _Dictionary_Get( void* dictionary, Dictionary_Entry_Key key );
   
   /* Get a source from the dictionary implementation */
   Dictionary_Entry_Source _Dictionary_GetSource( void* dictionary, Dictionary_Entry_Key key );
   
   Dictionary_Entry_Value* _Dictionary_GetByIndex( void* dictionary, Dictionary_Index index );
      
   /* Get an entry from the dictionary */
   Dictionary_Entry* Dictionary_GetEntry( void* dictionary, Dictionary_Entry_Key key );

   /* Get an entry from the dictionary by index */
   Dictionary_Entry* Dictionary_GetEntryByIndex( void* dictionary, Dictionary_Index index );

   /* Get count of entries in the dictionary */
   Index Dictionary_GetCount( void* dictionary );

   /* Loops over command line arguments and reads in values with format "--param=value" */
   void Dictionary_ReadAllParamFromCommandLine( void* dictionary, int argc, char* argv[] ) ;

   /* Compares two dictionaries, returns True if all entries are identical. Default to loose
     *  type check */
   #define Dictionary_CompareAllEntries( dict1, dict2 ) \
       ( Dictionary_CompareAllEntriesFull( (dict1), (dict2), False ) )

   /* Compares two dictionaries, returns True if all entries are identical */
   Bool Dictionary_CompareAllEntriesFull( void* dictionary1, void* dictionary2, Bool strictTypeCheck );

#endif /* __StGermain_Base_IO_Dictionary_h__ */

