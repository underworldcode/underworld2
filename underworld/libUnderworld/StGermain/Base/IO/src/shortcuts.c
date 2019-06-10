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
#include <stdarg.h>
#include <string.h>

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry_Value.h"
#include "Dictionary_Entry.h"
#include "File.h"
#include "Stream.h"
#include "Journal.h"

int Dictionary_GetUnsignedInt_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const unsigned int   defaultVal )
{
   return Dictionary_Entry_Value_AsUnsignedInt( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromUnsignedInt( defaultVal ) ) );
}

int Dictionary_GetInt_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const int            defaultVal )
{
   return Dictionary_Entry_Value_AsInt( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromInt( defaultVal ) ) );
}

Dictionary_Entry_Value* _Dictionary_GetDouble_WithScopeDefault(
   Dictionary*             dictionary,
   Dictionary_Entry_Key    key,
   Dictionary_Entry_Value* defaultVal )
{
   Dictionary_Entry_Value *returnVal=NULL;
   Bool                    usedDefault = False;
   Stream*                 stream = Journal_Register( Info_Type, "Dictionary" );

   returnVal = Dictionary_GetDefault( dictionary, key, defaultVal );

   if( returnVal && returnVal->type == Dictionary_Entry_Value_Type_String ) { 
      Dictionary_Entry_Key rootDictKey = Dictionary_Entry_Value_AsString( returnVal ); 
      Dictionary*          rootDict    = dictionary; 

      /* Check if the number really is a string or not */ 
      if( Stg_StringIsNumeric( rootDictKey ) ) 
         return returnVal; 

      Journal_PrintfL(
         stream,
         2,
         "Key '%s' points to key '%s' in the root dictionary: ",
         key,
         rootDictKey ); 

      /* Get Value from dictionary */ 
      returnVal = Dictionary_Get( rootDict, rootDictKey ); 
      if( !returnVal && defaultVal ) { 
         returnVal = Dictionary_GetDefault( rootDict, rootDictKey, defaultVal ); 
         usedDefault = True; 
      } 

      /* Print Stuff */ 
      if( usedDefault ) { 
         Journal_PrintfL( stream, 2, "Using default value = " ); 
         if( Stream_IsPrintableLevel( stream, 2 ) )  
            Dictionary_Entry_Value_Print( returnVal, stream ); 
            Journal_PrintfL( stream, 2, "\n" ); 
         return returnVal; 
      } 
      else if( returnVal ) { 
         Journal_PrintfL( stream, 2, "Found - Value = " ); 
         if( Stream_IsPrintableLevel( stream, 2 ) )  
            Dictionary_Entry_Value_Print( returnVal, stream ); 
         Journal_PrintfL( stream, 2, "\n" ); 
      } 
      else  
         Journal_PrintfL( stream, 2, "Not found.\n" ); 
   } 
   return returnVal;
}

/* 
 * Will search for the entry in the dictionary and if it's a non-numeric 
 * string(a place holder), it'll try find the value of this place holder. 
 * Note: 
 *  1. will only parse one level of place holders, i.e. it's not a recursive function 
 *  2. by 'Scope' the search will only search in the scope of the passed dictionary, 
 *     it'll NOT check parent dictionaries 
 */
double Dictionary_GetDouble_WithScopeDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const double         defaultVal )
{
   return Dictionary_Entry_Value_AsDouble(
      _Dictionary_GetDouble_WithScopeDefault(
         dictionary, key, Dictionary_Entry_Value_FromDouble( defaultVal ) ) );
}

double Dictionary_GetDouble_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const double         defaultVal )
{
   return Dictionary_Entry_Value_AsDouble( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromDouble( defaultVal ) ) );
}

unsigned long Dictionary_GetUnsignedLong_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const unsigned long  defaultVal )
{
   return Dictionary_Entry_Value_AsUnsignedLong( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromUnsignedLong( defaultVal ) ) );
}

float Dictionary_GetFloat_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const float          defaultVal )
{
   return(float) Dictionary_Entry_Value_AsDouble( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromDouble( (double)defaultVal ) ) );
}

Bool Dictionary_GetBool_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const Bool           defaultVal )
{
   return Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromBool( defaultVal ) ) );
}

char* Dictionary_GetString_WithDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   const char* const    defaultVal )
{
   return Dictionary_Entry_Value_AsString( 
      Dictionary_GetDefault(
         dictionary, key, Dictionary_Entry_Value_FromString( defaultVal ) ) );
}

char* Dictionary_GetString_WithPrintfDefault(
   Dictionary*          dictionary,
   Dictionary_Entry_Key key,
   char*                format,
   ... )
{
   char*   returnString;
   char*   defaultVal;
   va_list ap;
   
   va_start( ap, format );

   /* Create Default String from format arguments */
   Stg_vasprintf( &defaultVal, format, ap );

   /* Read Dictionary */
   returnString = Dictionary_GetString_WithDefault( dictionary, key, defaultVal );

   /* Clean up */
   Memory_Free( defaultVal );
   va_end( ap );

   return returnString;
}

Bool Stream_RedirectFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename ) {
   Bool result;

   /* Check to make sure output path is emtpy */ 
   if( Stg_StringIsEmpty( prependedPath ) )
      result = Stream_RedirectFile( stream, filename );
   else {
      char* prependedFilename;

      Stg_asprintf( &prependedFilename, "%s/%s", prependedPath, filename );
      result = Stream_RedirectFile( stream, prependedFilename );
      Memory_Free( prependedFilename );
   }

   return result;
}

Bool Stream_AppendFile_WithPrependedPath( Stream* stream, char* prependedPath, char* filename ) {
   Bool result;

   /* Check to make sure output path is emtpy */ 
   if( Stg_StringIsEmpty( prependedPath ) )
      result = Stream_AppendFile( stream, filename );
   else {
      char* prependedFilename;

      Stg_asprintf( &prependedFilename, "%s/%s", prependedPath, filename );
      result = Stream_AppendFile( stream, prependedFilename );
      Memory_Free( prependedFilename );
   }

   return result;
}

void Journal_PrintString_WithLength( Stream* stream, char* string, int length ) {
   size_t stringLength = strlen( string ) ;
   char*  breakInMiddle = "...";
   size_t lengthOfBreak = strlen( breakInMiddle );

   if( length <= 0 )
      return;
   /* If length given is exeedingly short - then print initial */
   else if( lengthOfBreak >= length ) {
      Journal_Printf( stream, "%c", string[0] );
      Journal_Write( stream, breakInMiddle, sizeof( char ), length - 1 );
   }
   /* Shorten String if nessesary */
   else if( stringLength > length ) {
      int charsBeforeBreak = (int) ((float)length * 0.5) - 1;
      int charsAfterBreak  = length - charsBeforeBreak - lengthOfBreak;

      /* Print first half of string */
      Journal_Write( stream, string, sizeof( char ), charsBeforeBreak );

      /* Print '...' In middle */
      Journal_Printf( stream, breakInMiddle );

      /* Print end of string */
      Journal_Write(
         stream,
         &string[ stringLength - charsAfterBreak ],
         sizeof( char ),
         charsAfterBreak );

      return;
   }
   else {
      Index char_I;

      /* If string is shorter than allowed length - then add empty space */
      for( char_I = 0; char_I < length - stringLength; char_I++ )
         Journal_Printf( stream, " " );
      
      /* Print the complete string */
      Journal_Printf( stream, string );
   }
}


