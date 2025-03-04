/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "StGermain/Base/Foundation/src/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "forwardDecl.h"
#include "Dictionary.h"
#include "Journal.h"
#include "File.h"
#include "Stream.h"
#include "Dictionary_Entry_Value.h"
#include "Dictionary_Entry.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Textual name of this class */
const Type Dictionary_Type = "dictionary";

/* Controlling struct behaviour */
static const int DEFAULT_INIT_SIZE = 10;
static const int DEFAULT_DELTA = 5;
static const int STRUCT_INIT_SIZE = 2;
static const int STRUCT_DELTA = 2;
static const char* OVERRIDE_ATTR = "overridden";

Dictionary* Dictionary_New( void ) {
   /* Variables set in this function */
   SizeT                                _sizeOfSelf = sizeof(Dictionary);
   Type                                        type = Dictionary_Type;
   Stg_Class_DeleteFunction*                _delete = _Dictionary_Delete;
   Stg_Class_PrintFunction*                  _print = _Dictionary_Print;
   Stg_Class_CopyFunction*                    _copy = _Dictionary_Copy;
   Dictionary_AddFunction*                      add = _Dictionary_Add;
   Dictionary_AddWithSourceFunction*  addWithSource = _Dictionary_AddWithSource;
   Dictionary_SetFunction*                      set = _Dictionary_Set;
   Dictionary_SetWithSourceFunction*  setWithSource = _Dictionary_SetWithSource;
   Dictionary_GetFunction*                      get = _Dictionary_Get;
   Dictionary_GetSourceFunction*          getSource = _Dictionary_GetSource;

   return _Dictionary_New(  DICTIONARY_PASSARGS  );
}

Dictionary* _Dictionary_New( DICTIONARY_DEFARGS ) {   
   Dictionary* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(Dictionary) );
   self = (Dictionary*)_Stg_Class_New( STG_CLASS_PASSARGS );
   
   /* General info */
   
   /* virtual functions */
   self->add = add;
   self->addWithSource = addWithSource;
   self->set = set;
   self->setWithSource = setWithSource;
   self->get = get;
   self->getSource = getSource;
   
   /* Dictionary info */
   _Dictionary_Init( self );
   
   return self;
}

void _Dictionary_Init( void* dictionary ) {
   Dictionary* self = (Dictionary*) dictionary;

   self->size = DEFAULT_INIT_SIZE;
   self->delta = DEFAULT_DELTA;
   self->count = 0;
   self->entryPtr = Memory_Alloc_Array( Dictionary_Entry*, self->size, "Dictionary->entryPtr" );
   self->debugStream = Journal_Register( Debug_Type, "DictionaryWarning" );
}

void _Dictionary_Delete( void* dictionary ) {
   Dictionary* self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   for( index = 0; index < self->count; index++ ) {
      Dictionary_Entry_Delete( self->entryPtr[index] );
   }
   Memory_Free( self->entryPtr );
   
   /* Stg_Class_Delete parent */
   _Stg_Class_Delete( self );
}

void _Dictionary_Print( void* dictionary, Stream* stream ) {
   Dictionary* self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   /* Print parent */
   _Stg_Class_Print( self, stream );

   Journal_Printf( (void*)stream, "Dictionary:\n" );
   Journal_Printf( (void*)stream, "\tsize: %u\n", self->size );
   Journal_Printf( (void*)stream, "\tdelta: %u\n", self->delta );
   Journal_Printf( (void*)stream, "\tcount: %u\n", self->count );
   Journal_Printf( (void*)stream, "\tentryPtr[0-%u]: {\n", self->count );
   Stream_Indent( stream );
   Stream_Indent( stream );

   for( index = 0; index < self->count; index++ ) {
      /*Journal_Printf( (void*)stream, "\t\t" );*/
      Dictionary_Entry_Print( self->entryPtr[index], stream ); 
      Journal_Printf( (void*)stream, "\n" );
   }
   Journal_Printf( (void*)stream, "\t}\n" );
   Stream_UnIndent( stream );
   Stream_UnIndent( stream );
}

void Dictionary_PrintConcise( void* dictionary, Stream* stream ) {
   Dictionary*      self = (Dictionary*) dictionary;
   Dictionary_Index index;

   Journal_Printf( stream, "Dictionary contains %u entries:\n", self->count );

   Stream_Indent( stream );
   for( index = 0; index < self->count; index++ ) {
      Journal_Printf( stream, "\t" );
      Dictionary_Entry_Print( self->entryPtr[index], stream ); 
      Journal_Printf( stream, "\n" );
   }
   Journal_Printf( stream, "}\n" );
   Stream_UnIndent( stream );
}

void Dictionary_Empty( void* dictionary ) {
   Dictionary*      self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   for( index = 0; index < self->count; index++ ) {
      Dictionary_Entry_Delete( self->entryPtr[index] );
   }
   
   self->count=0;
   self->size = DEFAULT_INIT_SIZE;
   self->entryPtr = Memory_Realloc_Array( self->entryPtr, Dictionary_Entry*, self->size );
}

void* _Dictionary_Copy(
   void*          dictionary,
   void*          dest,
   Bool           deep,
   Name           nameExt,
   struct PtrMap* ptrMap )
{
   Dictionary*      self = (Dictionary*) dictionary;
   Dictionary_Index index;
   Dictionary*      newDict;
   Bool             ownMap = False;
   
   if( !ptrMap ) {
      ptrMap = PtrMap_New( 10 );
      ownMap = True;
   }

   newDict = (Dictionary*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );

   newDict->add = self->add;
   newDict->addWithSource = self->addWithSource;
   newDict->set = self->set;
   newDict->setWithSource = self->setWithSource;
   newDict->get = self->get;
   newDict->getSource = self->getSource;

   newDict->size = self->size;
   newDict->delta = self->delta;

   if( False == deep ) {
      newDict->debugStream = self->debugStream;
   }
   else {
      newDict->debugStream = (Stream*)Stg_Class_Copy( self->debugStream, NULL, deep, nameExt, ptrMap );
   }

   newDict->entryPtr = Memory_Alloc_Array( Dictionary_Entry*, self->size, "Dictionary->entryPtr" );
   if( False == deep ) {
      newDict->count = self->count;
      memcpy( newDict->entryPtr, self->entryPtr, sizeof(Dictionary_Entry*) * self->count );
      for( index = 0; index < self->count; index++ ) {
         newDict->entryPtr[index] = self->entryPtr[index];
      }
   }
   else {
      newDict->count = 0;
      for( index = 0; index < self->count; index++ ) {
         Dictionary_Add( newDict, self->entryPtr[index]->key,
            Dictionary_Entry_Value_Copy( self->entryPtr[index]->value, True ) );
      }
   }

   if( ownMap ) {
      Stg_Class_Delete( ptrMap );
   }

   return(void*)newDict;
}

void Dictionary_Add(
   void*                   dictionary,
   Dictionary_Entry_Key    key,
   Dictionary_Entry_Value* value )
{
   Dictionary* self = (Dictionary*)dictionary;
   self->add( dictionary, key, value );
}

void Dictionary_AddWithSource(
   void*                   dictionary,
   Dictionary_Entry_Key    key,
   Dictionary_Entry_Value* value, 
   Dictionary_Entry_Units  units,
   Dictionary_Entry_Source source ) 
{
   Dictionary* self = (Dictionary*)dictionary;
   self->addWithSource( dictionary, key, value, units, source );
}

Bool Dictionary_Set( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value ) {
   Dictionary* self = (Dictionary*)dictionary;
   return self->set( dictionary, key, value );
}

Dictionary_Entry_Value* Dictionary_Get( void* dictionary, Dictionary_Entry_Key key ) {
   Dictionary* self = (Dictionary*)dictionary;
   /* Be less sensitive to whether the dictionary is NULL... yields nicer user code */
   return self ? self->get( dictionary, key ) : NULL;
}

Dictionary_Entry_Source Dictionary_GetSource( void* dictionary, Dictionary_Entry_Key key ) {
   Dictionary* self = (Dictionary*)dictionary;
   /* Be less sensitive to whether the dictionary is NULL... yields nicer user code */
   return self ? self->getSource( dictionary, key ) : NULL;
}

Dictionary_Entry_Value* Dictionary_GetByIndex( void* dictionary, Dictionary_Index index ) {
   Dictionary* self = (Dictionary*)dictionary;
   /* Be less sensitive to whether the dictionary is NULL... yields nicer user code */
   return self ? _Dictionary_GetByIndex( dictionary, index ) : NULL;
}

Dictionary_Entry* Dictionary_GetEntry( void* dictionary, Dictionary_Entry_Key key ) {
   Dictionary*      self = (Dictionary*)dictionary;
   Dictionary_Index index;
   Dictionary_Index count;

   /* Be less sensitive to whether the dictionary is NULL... yields nicer user code */
   count = self ? self->count : 0;
   
   for( index = 0; index < count; index++ ) {
      if( Dictionary_Entry_Compare( self->entryPtr[index], key ) != 0 ) {
         return self->entryPtr[index];
      }
   }
   return NULL;
}

Dictionary_Entry* Dictionary_GetEntryByIndex( void* dictionary, Dictionary_Index index) {
   Dictionary* self = (Dictionary*)dictionary;

   /* Be less sensitive to whether the dictionary is NULL... yields nicer user code */
   /* Check that self->entryPtr[index] is NOT NULL */
   if( !self || !self->entryPtr[index] )
      return NULL;
   else
      return self->entryPtr[index];
}

Index Dictionary_GetCount( void* dictionary ) {
   Dictionary* self = (Dictionary*) dictionary;
   /* Be less sensitive to whether the dictionary is NULL... yields nicer user code */
   return self ? self->count : 0;
}

Dictionary_Entry_Value* Dictionary_GetDefault(
   void*                   dictionary,
   Dictionary_Entry_Key    key,
   Dictionary_Entry_Value* value )
{
   Dictionary* self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   for( index = 0; index < self->count; index++ ) {
      if( Dictionary_Entry_Compare( self->entryPtr[index], key ) != 0 ) {
         /* key found, so delete the default value */
         Dictionary_Entry_Value_Delete( value );
         return Dictionary_Entry_Get( self->entryPtr[index] );
      }
   }
   
   Journal_Printf(
      self->debugStream,
      "Warning - value %s not found in dictionary, using default value of ",
      key );

   Dictionary_Entry_Value_Print( value, self->debugStream );
   Journal_Printf( self->debugStream, "\n" );
   
   Dictionary_Add( dictionary, key, value );
   return value;
}

void _Dictionary_Add( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value ) {
   Dictionary* self = (Dictionary*) dictionary;
   assert( self->count <= self->size );

   if( self->count == self->size ) {
      self->size += self->delta;
      self->entryPtr = Memory_Realloc_Array( self->entryPtr, Dictionary_Entry*, self->size );
   }
   
   self->entryPtr[self->count] = Dictionary_Entry_New( key, value );
   self->count++;
}

void _Dictionary_AddWithSource(
   void*                   dictionary,
   Dictionary_Entry_Key    key,
   Dictionary_Entry_Value* value,
   Dictionary_Entry_Units  units,
   Dictionary_Entry_Source source ) 
{
   Dictionary* self = (Dictionary*) dictionary;
   assert( self->count <= self->size );

   if( self->count == self->size ) {
      self->size += self->delta;
      self->entryPtr = Memory_Realloc_Array( self->entryPtr, Dictionary_Entry*, self->size );
   }
   
   self->entryPtr[self->count] = Dictionary_Entry_NewWithSource( key, value, source );
   /* wow */
   self->entryPtr[self->count]->units = StG_Strdup(units);
   self->count++;
}

Dictionary_Entry_Value* Dictionary_AddMerge( 
   void*                   dictionary, 
   Dictionary_Entry_Key    key, 
   Dictionary_Entry_Value* value,
   Dictionary_MergeType    mergeType )
{
   Dictionary*             self = (Dictionary*)dictionary;
   Dictionary_Entry*       existingEntry = Dictionary_GetEntry( self, key );
   Dictionary_Entry_Value* newValue = value;
   Dictionary_Entry_Value* updatedValue = value;
   Dictionary_Entry_Value* curValue;
   
   if( existingEntry ) {
      Dictionary_Entry_Value* existingValue = Dictionary_Entry_Get( existingEntry );
      Index                   i;
      
      switch( existingValue->type ) {
         /* parameters... all behave the same */
         case Dictionary_Entry_Value_Type_String:
         case Dictionary_Entry_Value_Type_Double:
         case Dictionary_Entry_Value_Type_UnsignedInt:
         case Dictionary_Entry_Value_Type_Int:
         case Dictionary_Entry_Value_Type_Bool:
         case Dictionary_Entry_Value_Type_UnsignedLong:
         case Dictionary_Entry_Value_Type_VoidPtr:
            switch( mergeType ) {
               /* "Append", means add to end... default add behaviour */
               case Dictionary_MergeType_Append:
                  _Dictionary_Add( dictionary, key, newValue );
                  break;
               /* "Merge" and "Replace" means override */
               case Dictionary_MergeType_Merge:
               case Dictionary_MergeType_Replace:
                  Dictionary_Entry_Set( existingEntry, newValue );
                  break;
            };
            break;
         case Dictionary_Entry_Value_Type_Struct:
            switch( mergeType ) {
               /* "Append", means add to end... default add behaviour */
               case Dictionary_MergeType_Append: /* create at end */
                  _Dictionary_Add( dictionary, key, newValue );
                  break;
               /* "Merge" means for all struct items, add to end... i.e. do nothing here .*/
               case Dictionary_MergeType_Merge:
                  for( i = 0; i < newValue->as.typeStruct->count; i++ ) {
                     Dictionary_AddMerge( existingValue->as.typeStruct,
                        newValue->as.typeStruct->entryPtr[i]->key, 
                        newValue->as.typeStruct->entryPtr[i]->value, mergeType );
                  }
                  
                  newValue->as.typeStruct->count = 0;
                  Stg_Class_Delete( newValue->as.typeStruct );
                  Memory_Free( newValue );
                  updatedValue = existingValue;
                  break;
               /* "Replace" means remove current entry and create a newey */
               case Dictionary_MergeType_Replace:
                  Dictionary_Entry_Set( existingEntry, newValue );
                  break;
            };
            break;
         case Dictionary_Entry_Value_Type_List:
            switch( mergeType ) {
               /* "Append", means add to end... default add behaviour */
               case Dictionary_MergeType_Append: /* create at end */
                  _Dictionary_Add( dictionary, key, newValue );
                  break;
               /* "Merge": 2 different cases... */
               /* means for all list items, add to end... i.e. do nothing here .*/
               case Dictionary_MergeType_Merge:
                  curValue = newValue->as.typeList->first;
                  for( i = 0; i < newValue->as.typeList->count; i++ ) {
                     Dictionary_Entry_Value_AddElement( existingValue, curValue );
                     curValue = curValue->next;
                  }
                  
                  Memory_Free( newValue );
                  updatedValue = existingValue;
                  break;
               /* "Replace" means remove current entry and create a newey */
               case Dictionary_MergeType_Replace:
                  Dictionary_Entry_Set( existingEntry, newValue );
                  break;
            };
            break;
         default:
            Journal_Firewall(
               False,
               Journal_Register( Error_Type, self->type ),
               "Error in func %s: Bad existingValue type '%u'\n",
               __func__,
               existingValue->type ); 
            break;
      }
   }
   else {
      _Dictionary_Add( self, key, value );
   }
   
   return updatedValue;
}

Dictionary_Entry_Value* Dictionary_AddMergeWithSource( 
   void*                   dictionary, 
   Dictionary_Entry_Key    key, 
   Dictionary_Entry_Value* value,
   Dictionary_Entry_Units  units,
   Dictionary_MergeType    mergeType,
   Dictionary_Entry_Source source )
{
   Dictionary*             self = (Dictionary*)dictionary;
   Dictionary_Entry*       existingEntry = Dictionary_GetEntry( self, key );
   Dictionary_Entry_Value* newValue = value;
   Dictionary_Entry_Value* updatedValue = value;
   Dictionary_Entry_Value* curValue;
   
   if( existingEntry ) {
      Dictionary_Entry_Value* existingValue = Dictionary_Entry_Get( existingEntry );
      Index                   i;
      
      switch( existingValue->type ) {
         /* parameters... all behave the same */
         case Dictionary_Entry_Value_Type_String:
         case Dictionary_Entry_Value_Type_Double:
         case Dictionary_Entry_Value_Type_UnsignedInt:
         case Dictionary_Entry_Value_Type_Int:
         case Dictionary_Entry_Value_Type_Bool:
         case Dictionary_Entry_Value_Type_UnsignedLong:
         case Dictionary_Entry_Value_Type_VoidPtr:
            switch( mergeType ) {
               /* "Append", means add to end... default add behaviour */
               case Dictionary_MergeType_Append:
                  _Dictionary_AddWithSource( dictionary, key, newValue, units, source );
                  break;
               /* "Merge" and "Replace" means override */
               case Dictionary_MergeType_Merge:
               case Dictionary_MergeType_Replace:
                  Dictionary_Entry_SetWithSource( existingEntry, newValue, source );
                  break;
            };
            break;
         case Dictionary_Entry_Value_Type_Struct:
            switch( mergeType ) {
               /* "Append", means add to end... default add behaviour */
               case Dictionary_MergeType_Append: /* create at end */
                  _Dictionary_AddWithSource( dictionary, key, newValue, units, source );
                  break;
               /* "Merge" means for all struct items, add to end... i.e. do nothing here .*/
               case Dictionary_MergeType_Merge:
                  for( i = 0; i < newValue->as.typeStruct->count; i++ ) {
                     Dictionary_AddMergeWithSource( existingValue->as.typeStruct,
                        newValue->as.typeStruct->entryPtr[i]->key, 
                        newValue->as.typeStruct->entryPtr[i]->value,
                        units,
                        mergeType, source );
                  }
                  
                  newValue->as.typeStruct->count = 0;
                  Stg_Class_Delete( newValue->as.typeStruct );
                  Memory_Free( newValue );
                  updatedValue = existingValue;
                  break;
               /* "Replace" means remove current entry and create a newey */
               case Dictionary_MergeType_Replace:
                  Dictionary_Entry_SetWithSource( existingEntry, newValue, source );
                  break;
            };
            break;
         case Dictionary_Entry_Value_Type_List:
            switch( mergeType ) {
               /* "Append", means add to end... default add behaviour */
               case Dictionary_MergeType_Append: /* create at end */
                  _Dictionary_AddWithSource( dictionary, key, newValue, units, source );
                  break;
               /* "Merge": 2 different cases... */
               /* means for all list items, add to end... i.e. do nothing here .*/
               case Dictionary_MergeType_Merge:
                  curValue = newValue->as.typeList->first;

                  for( i = 0; i < newValue->as.typeList->count; i++ ) {
                     Dictionary_Entry_Value_AddElementWithSource(
                        existingValue,
                        curValue, 
                        source );
                     curValue = curValue->next;
                  }
                  
                  Memory_Free( newValue );
                  updatedValue = existingValue;
                  break;
               /* "Replace" means remove current entry and create a newey */
               case Dictionary_MergeType_Replace:
                  Dictionary_Entry_SetWithSource( existingEntry, newValue, source );
                  break;
            };
            break;
         default:
            Journal_Firewall(
               False,
               Journal_Register( Error_Type, self->type ),
               "Error in func %s: Bad existingValue type '%u'\n",
               __func__,
               existingValue->type ); 
            break;
      }
   }
   else {
      _Dictionary_AddWithSource( self, key, value, units, source );
   }
   
   return updatedValue;
}

Bool _Dictionary_Set( void* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* value ) {
   Dictionary*      self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   for( index = 0; index < self->count; index++ ) {
      if( Dictionary_Entry_Compare( self->entryPtr[index], key ) != 0 ) {
         Dictionary_Entry_Set( self->entryPtr[index], value );
         return True;
      }
   }

   /* If we reach here and haven't found it, add new entry */
   Dictionary_Add( self, key, value );

   return True;
}

Bool Dictionary_SetWithSource(
    void*                   dictionary,
    Dictionary_Entry_Key    key,
    Dictionary_Entry_Value* value,
    Dictionary_Entry_Source source )
{
   Dictionary* self = (Dictionary*) dictionary;
   self->setWithSource( dictionary,
                        key,
                        value,
                        source );

   
}

Bool _Dictionary_SetWithSource(
   void*                   dictionary,
   Dictionary_Entry_Key    key,
   Dictionary_Entry_Value* value,
   Dictionary_Entry_Source source ) 
{
   Dictionary* self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   for( index = 0; index < self->count; index++ ) {
      if( Dictionary_Entry_CompareWithSource( self->entryPtr[index], key, source ) != 0 ) {
         Dictionary_Entry_SetWithSource( self->entryPtr[index], value, source );
         return True;
      }
   }

   /* If we reach here and haven't found it, add new entry */
   Dictionary_AddWithSource( self, key, value, NULL, source );

   return True;
}

Dictionary_Entry_Value* _Dictionary_Get( void* dictionary, Dictionary_Entry_Key key ) {
   Dictionary* self = (Dictionary*) dictionary;
   Dictionary_Index index;
   
   for( index = 0; index < self->count; index++ ) {
      if( Dictionary_Entry_Compare( self->entryPtr[index], key ) != 0 ) {
         return Dictionary_Entry_Get( self->entryPtr[index] );
      }
   }
   return 0;
}

Dictionary_Entry_Source _Dictionary_GetSource( void* dictionary, Dictionary_Entry_Key key) {
   Dictionary* self = (Dictionary*) dictionary;
   Dictionary_Index index;

   for( index = 0; index < self->count; index++ ) {
      if( Dictionary_Entry_Compare( self->entryPtr[index], key ) != 0 ) {
         return Dictionary_Entry_GetSource( self->entryPtr[index] );
      }
   }
   return 0;
}

Dictionary_Entry_Value* _Dictionary_GetByIndex( void* dictionary, Dictionary_Index index ) {
   Dictionary* self = (Dictionary*) dictionary;
   return Dictionary_Entry_Get(self->entryPtr[index]);
}

void Dictionary_ParseCommandLineParam( void* self, char* paramString, char* valueString ) {
   Dictionary*             dictionary = (Dictionary*)self;
   char*                   point;
   char*                   lbracket;
   char*                   structString;
   char*                   memberString;
   char*                   point2;
   char*                   listString;
   char*                   lbracket2;
   char*                   indexString;
   char*                   rbracket2;
   char*                   remainderString;
   Dictionary*             structDictionary; 
   Dictionary_Entry_Value* ev;
   Dictionary_Entry*       entry;
   Index                   index;
   Bool                    emptyIndex;
   Bool                    isStruct;

   point = strchr( paramString, '.' );
   lbracket = strchr( paramString, '[' );

   /* 
    * If there's neither a point or left-bracket in the(supposed) param string, 
    * them treat as a parameter.
    */
   if( !point && !lbracket ) {
      ev = Dictionary_Get( dictionary, paramString );

      /* Add value to dictionary */
      if( ev == NULL ) {
         /* If there isn't already an entry in dictionary - create one */
         Dictionary_AddWithSource(
            dictionary,
            paramString,
            Dictionary_Entry_Value_FromString( valueString ),
            NULL,
            (Dictionary_Entry_Source)OVERRIDE_ATTR );
      }
      else {
         /* If is already in dictionary - then update value */
         Dictionary_Entry_Value_SetFromStringKeepCurrentType( ev, valueString );

         /* Also update the source to state "overriden". */
         entry = Dictionary_GetEntry( dictionary, (Dictionary_Entry_Key)paramString );
         Dictionary_Entry_SetSource( entry, (Dictionary_Entry_Source)OVERRIDE_ATTR ); 
      }
   }
   else {
      /* It is either a struct or list or both. Work out one at a time, from the left. */
      if( point && lbracket ) {
         if( point < lbracket ) {
            lbracket = 0;
         }
         else {
            point = 0;
         }
      }

      if( point ) {
         /* Create the struct and member strings from the source. */
         structString = StG_Strdup( paramString );
         point2 = strchr( structString, '.' );
         *point2 = 0;
         memberString = point2 + 1;
         
         /* Add value to dictionary */
         ev = Dictionary_Get( dictionary, structString );
         if( !ev ) {
            /* If there isn't already an entry in the dictionary - create one. */
            structDictionary = Dictionary_New();
            Dictionary_Add(
               dictionary,
               structString,
               Dictionary_Entry_Value_FromStruct( structDictionary ) );
         }
         else {
            /* Entry found. Ensure its a struct. */
            structDictionary = Dictionary_Entry_Value_AsDictionary( ev );

            Journal_Firewall( 
               (Bool)structDictionary, 
               Journal_Register( Error_Type, Dictionary_Type ), 
               "Command line argument \"%s\" parsed as struct, "
               "and is in dictionary, but not as a struct",
               structString );
         }
         Dictionary_ParseCommandLineParam( structDictionary, memberString, valueString );
         Memory_Free( structString );
      }
      else if( lbracket ) {
         /* Create the list string and index from the source */
         listString = StG_Strdup( paramString );
         lbracket2 = strchr( listString, '[' );
         *lbracket2 = 0;
         indexString = lbracket2 + 1;
         
         /* Make sure there is a right bracket in the string */
         rbracket2 = strchr( indexString, ']' );
         Journal_Firewall( 
            (Bool)rbracket2, 
            Journal_Register( Error_Type, Dictionary_Type ), 
            "Command line argument \"%s\" parsed as list but missing closing \']\'.",
            listString );

         *rbracket2 = 0;
         remainderString = rbracket2 + 1;
         
         /* Obtain the index */
         if( strlen( indexString ) ) {
            emptyIndex = False;
            index = strtoul( indexString, 0, 0 );
         }
         else {
            emptyIndex = True;
            index = 0;
         }
         
         if( strlen( remainderString ) ) {
            /* TODO: parse out recursive lists SQ20060206 */
            /* TODO: parse out structures(properly) SQ20060206 */
            if( remainderString[0] == '.' ) {
               memberString = remainderString + 1;
               isStruct = True;
            }
            else {
               memberString = 0;
               isStruct = False;
            }
         }
         else {
            memberString = 0;
            isStruct = False;
         }
         
         /* Assume it is a parameter */
         ev = Dictionary_Get( dictionary, listString );

         /* Add value to dictionary */
         if( ev == NULL ) {
            /* 
             * If there isn't already an entry in dictionary - create one. 
             * Verify index is 0(0th/add).
             */
            Journal_Firewall( 
               emptyIndex, 
               Journal_Register( Error_Type, Dictionary_Type ), 
               "Command line argument \"%s\" parsed as list but list not found and "\
               "index not specified as add(i.e. '[]').",
               listString );

            ev = Dictionary_Entry_Value_NewList();
            Dictionary_Add( dictionary, listString, ev );
            if( isStruct ) {
               Dictionary* newStruct = Dictionary_New();
               
               Dictionary_Entry_Value_AddElement( 
                  ev, 
                  Dictionary_Entry_Value_FromStruct( newStruct ) );
               Dictionary_ParseCommandLineParam( newStruct, memberString, valueString );
            }
            else {
               Dictionary_Entry_Value_AddElement( 
                  ev, 
                  Dictionary_Entry_Value_FromString( valueString ) );
            }
         }
         else {
            /* Obtain the ith element or add if specified */
            if( emptyIndex ) {
               if( isStruct ) {
                  Dictionary* newStruct = Dictionary_New();
               
                  Dictionary_Entry_Value_AddElement( 
                     ev, 
                     Dictionary_Entry_Value_FromStruct( newStruct ) );
                  Dictionary_ParseCommandLineParam( newStruct, memberString, valueString );
               }
               else {
                  Dictionary_Entry_Value_AddElement( 
                     ev, 
                     Dictionary_Entry_Value_FromString( valueString ) );
               }
            }
            else {
               Dictionary_Entry_Value* item;
            
               item = Dictionary_Entry_Value_GetElement( ev, index );

               Journal_Firewall( 
                  (Bool)item, 
                  Journal_Register( Error_Type, Dictionary_Type ), 
                  "Command line argument \"%s\" parsed as list, requesting index \"%u\", "\
                  "but that index does not exist.",
                  listString,
                  index );
               
               if( isStruct ) {
                  Dictionary* newStruct = Dictionary_Entry_Value_AsDictionary( item );
                  Dictionary_ParseCommandLineParam( newStruct, memberString, valueString );
               }
               else {
                  Dictionary_Entry_Value_SetFromStringKeepCurrentType( item, valueString );
               }
            }
         }
         Memory_Free( listString );
      }
   }
}

void Dictionary_ReadAllParamFromCommandLine( void* dictionary, int argc, char* argv[] ) {
   Index        arg_I;
   const char*  preceedingString = "--";
   char*        argumentString;
   char*        paramString;
   char*        valueString;
   char*        equalsSign;
   unsigned int preceedingStringLength = strlen( preceedingString );

   /* Loop over all the arguments from command line and reads all arguments of form "--param=value" */
   for( arg_I = 1 ; arg_I < argc ; arg_I++ ) {
      argumentString = argv[ arg_I ];

      /* Check is string has preceeding string "--" if not then continue in loop */
      if( strncmp( preceedingString, argumentString , preceedingStringLength ) != 0 )
         continue;

      /* Make sure there is an equals sign in the string */
      equalsSign = strchr( argumentString, '=' ) ;

      /* Make sure that there are characters after equals sign */
      if( equalsSign == NULL ) 
         continue;

      /* Paramater name starts right after these dashes */
      paramString = StG_Strdup(&argumentString[ preceedingStringLength ]);

      /* Find Pointer to equals sign in this new string we've just created */
      equalsSign = strchr( paramString, '=' ) ;

      /* Put in null terminator at equals sign */
      *equalsSign = '\0';

      /* Get Value String - which is everything after the "=" */
      valueString = equalsSign + 1;

      Dictionary_ParseCommandLineParam( dictionary, paramString, valueString );
      
      /* Free memory that we have allocated */
      Memory_Free( paramString );
   }
}

Bool Dictionary_CompareAllEntriesFull( void* dictionary1, void* dictionary2, Bool strictTypeCheck ) {
   Dictionary*       dict1 = (Dictionary*)dictionary1;
   Dictionary*       dict2 = (Dictionary*)dictionary2;
   Dictionary_Entry* entryPtr1 = NULL;   
   Dictionary_Entry* entryPtr2 = NULL;   
   Bool              retValue = True;
   Dictionary_Index  index;

   if( dict1->count != dict2->count ) return False;

   for( index = 0; index < dict1->count; index++ ) {
      entryPtr1 = dict1->entryPtr[index];
      entryPtr2 = dict2->entryPtr[index];
      retValue = Dictionary_Entry_Compare( entryPtr1, entryPtr2->key );

      if( retValue == False )
         break;

      retValue = Dictionary_Entry_Value_CompareFull(
         entryPtr1->value,
         entryPtr2->value,
         strictTypeCheck );

      if( retValue == False )
         break;
   }
   return retValue;
}


