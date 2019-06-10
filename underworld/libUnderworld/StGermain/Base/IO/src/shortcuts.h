/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_shortcuts_h__
#define __StGermain_Base_IO_shortcuts_h__

   #define Printf Journal_Printf
   #define Write  Journal_Write
   #define Dump   Journal_Dump

   /* Macros to add things to the dictionary more easily */
   #define Dictionary_AddFromUnsignedInt( dict, key, uintVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromUnsignedInt( (uintVal) ) ) )
   
   #define Dictionary_AddFromInt( dict, key, intVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromInt( (intVal) ) ) )
   
   #define Dictionary_AddFromDouble( dict, key, doubleVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromDouble( (doubleVal) ) ) )

   #define Dictionary_AddFromUnsignedLong( dict, key, uLongVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromUnsignedLong( (uLongVal) ) ) )

   #define Dictionary_AddFromString( dict, key, stringVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromString( (stringVal) ) ) )

   #define Dictionary_AddFromBool( dict, key, boolVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromBool( (boolVal) ) ) )

   #define Dictionary_AddFromDictionary( dict, key, dictVal ) \
      ( Dictionary_Add( (dict), (key), Dictionary_Entry_Value_FromStruct( (dictVal) ) ) )

   /* Macros to get things out of the dictionary more easily */
   #define Dictionary_GetUnsignedInt( dictionary, key ) \
      ( Dictionary_Entry_Value_AsUnsignedInt( Dictionary_Get( dictionary, key ) ) )
   
   #define Dictionary_GetInt( dictionary, key ) \
      ( Dictionary_Entry_Value_AsInt( Dictionary_Get( dictionary, key ) ) )
   
   #define Dictionary_GetDouble( dictionary, key ) \
      ( Dictionary_Entry_Value_AsDouble( Dictionary_Get( dictionary, key ) ) )

   #define Dictionary_GetUnsignedLong( dictionary, key ) \
      ( Dictionary_Entry_Value_AsUnsignedLong( Dictionary_Get( dictionary, key ) ) )

   #define Dictionary_GetString( dictionary, key ) \
      ( Dictionary_Entry_Value_AsString( Dictionary_Get( dictionary, key ) ) )

   #define Dictionary_GetBool( dictionary, key ) \
      ( Dictionary_Entry_Value_AsBool( Dictionary_Get( dictionary, key ) ) )

   #define Dictionary_GetDictionary( dictionary, key ) \
      ( Dictionary_Entry_Value_AsDictionary( Dictionary_Get( dictionary, key ) ) )

   /* shortcuts to easily get a value from the dictionary, using a default
   if not found, and converting it to the correct type */

   int Dictionary_GetUnsignedInt_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const unsigned int defaultVal );
   
   int Dictionary_GetInt_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const int defaultVal );

   double Dictionary_GetDouble_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const double defaultVal );

   /** Public function **/
   double Dictionary_GetDouble_WithScopeDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const double defaultVal );
   /** private function **/
   Dictionary_Entry_Value* _Dictionary_GetDouble_WithScopeDefault( Dictionary* dictionary, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal );
   
   unsigned long Dictionary_GetUnsignedLong_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const unsigned long defaultVal );
   
   float Dictionary_GetFloat_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const float defaultVal );
   
   Bool Dictionary_GetBool_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const Bool defaultVal );

   char* Dictionary_GetString_WithDefault( Dictionary* dictionary, Dictionary_Entry_Key key, const char* const defaultVal );

   /** Wrapper to Dictionary_GetString_WithDefault which creates a string using sprintf */
   char* Dictionary_GetString_WithPrintfDefault( Dictionary* dictionary, Dictionary_Entry_Key key, char* format, ... );

   /** Redirects file and puts 'prependedPath' in front of the filename 
    * e.g. Stream_RedirectFile_WithPrependedPath( stream, "./output", "file.dat" ) redirects to './output/file.dat' */
   Bool Stream_RedirectFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename );
   Bool Stream_AppendFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename );

   /** Prints a string with a certain maximum length - it truncates it in the middle with an ellipsis if it is too long */
   void Journal_PrintString_WithLength( Stream* stream, char* string, int length );
      
   /* shortcuts for printing names and values */
   #define Journal_PrintValue( stream, value ) \
      Journal_Printf( stream, #value " = %.5g\n", (double) (value) )

   #define Journal_PrintDouble      Journal_PrintValue
   #define Journal_PrintFloat       Journal_PrintValue
   #define Journal_PrintUnsignedInt Journal_PrintValue
   #define Journal_PrintInt         Journal_PrintValue

   #define Journal_PrintChar( stream, value ) \
      Journal_Printf( stream, #value " = %c\n", (char) value )
   #define Journal_PrintBool( stream, value ) \
      Journal_Printf( stream, #value " = %s\n", (value) ? "True" : "False" )
   #define Journal_PrintPointer( stream, value ) \
      Journal_Printf( stream, #value " = (ptr)%p\n", value )
   #define Journal_PrintString( stream, value ) \
      Journal_Printf( stream, #value " = %s\n", value )
   #define Journal_PrintArray( stream, array, count ) \
      do {   \
         Index journalPrintArray_array_I; \
         Journal_Printf( stream, #array " = { " ); \
         for( journalPrintArray_array_I = 0; journalPrintArray_array_I < count - 1; journalPrintArray_array_I++ ) \
            Journal_Printf( stream, "%.5g, ", (double)array[ journalPrintArray_array_I ] ); \
         Journal_Printf( stream, "%.5g }\n", (double)array[ journalPrintArray_array_I ] ); \
      } while(0)

#endif /* __StGermain_Base_IO_shortcuts_h__ */
