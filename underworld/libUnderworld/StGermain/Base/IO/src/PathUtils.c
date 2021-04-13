/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <mpi.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "StGermain/Base/Foundation/src/Foundation.h"
#include <libxml/parserInternals.h>

#include "types.h"
#include "shortcuts.h"
#include "Journal.h"
#include "File.h"
#include "Stream.h"

#include "PathUtils.h"

static const char* joiner = "/";

void FindFile( char* fullPath, const char* filename, const char* searchPaths ) {
   char* spaths;
   char* pos;
   FILE* f;
   
   assert( filename );
   
   fullPath[0] = 0;
   
   if( searchPaths && !(filename[0] == '/' || filename[0] == '~' || filename[0] == '.' ) ) {
      spaths = Memory_Alloc_Array_Unnamed(char,(strlen( searchPaths ) + 1));
      strcpy( spaths, searchPaths );
      pos = strtok( spaths, ":" );

      while( pos ) {
         PathJoin( fullPath, 2, pos, filename );
      
         f = fopen( fullPath, "r" );
         if( f ) {
            fclose( f );
            break;
         }
      
         fullPath[0] = 0;
         pos = strtok( NULL, ":" );
      }
      
      Memory_Free( spaths );
   }
   else {
      f = fopen( filename, "r" );
      if( f ) {
         fclose( f );
         strcpy( fullPath, filename );
      }
   }
}

Bool FindFileInPathList( char* fullPath, char* filename, char** searchPaths, Index searchPathsSize ) {
   xmlParserInputBufferPtr fileTester;
   Index                   ii;
   
   assert( filename );

   if( strcmp(filename, "Buffer")==0 )
      return False;

   if( searchPaths && searchPathsSize > 0 && !(filename[0] == '/' || filename[0] == '~' || filename[0] == '.') ) {
      for( ii = searchPathsSize; ii > 0; ii-- ) {
         sprintf( fullPath, "%s/%s", searchPaths[ii-1], filename );
         fileTester = xmlParserInputBufferCreateFilename( fullPath, XML_CHAR_ENCODING_NONE );
         if( fileTester ) {
            xmlFreeParserInputBuffer( fileTester );
            return True;
         }
      }
   }
   else {
      fileTester = xmlParserInputBufferCreateFilename( filename, XML_CHAR_ENCODING_NONE );
      if( fileTester ) {
         xmlFreeParserInputBuffer( fileTester );
         strcpy( fullPath, filename );
         return True;
      }
   }
   return False;
}

void PathJoin( char* path, unsigned count, ... ) {
   va_list  lst;
   char*    next;
   char*    pos;
   unsigned i;
   
   assert( count );

   pos = path;
   va_start( lst, count );
   for( i = 0; i < count - 1; i++ ) {
      next = va_arg( lst, char* );
      
      memcpy( pos, next, sizeof(char) * strlen( next ) );
      pos += strlen( next );
      
      if( strncmp( pos - strlen( joiner ), joiner, strlen( joiner ) ) ) {
         memcpy( pos, joiner, sizeof(char) * strlen( joiner ) );
         pos += strlen( joiner );
      }
   }

   next = va_arg( lst, char* );
   strcpy( pos, next );
   
   va_end( lst );
}

/* Based on Robert Turnbull's ExpandEnvironmentVariables function */
/* assumes string is malloced and from a non-tokenised source */
char* ExpandEnvironmentVariables( char* string ) {
   char* result; /* copy of input argument */
   
   char* variableBegin; /* ptr to where variable starts */
   char* variableEnd; /* ptr to position after end of variable text */
   char* nameBegin; /* ptr to where name of variable starts */
   char* nameEnd; /* ptr to position of the last character in variable name */
   char* endChars = NULL; /* list of characters which indicate an end of variable */

   char* current; /* current position in string */
   char* endOfString; /* pointer to end of string */

   char envName[256]; /* name of environment variable */
   char* envValue; /* the value of the environment variable */

   int length; /* current length of result string */
   int nameLength; /* length of the variable name */

   int charsToWrite; /* number of characters to write to result */
   int charsWritten; /* number of characters written to the result */

   Bool hasBraces; /* whether the env variable is using a brace in its name */

   length = strlen( string );
   charsWritten = 0;
   
   result = Memory_Alloc_Array( char, length + 1, "EnvTemp" );

   current = string;
   endOfString = string + length;

   variableBegin = strchr( current, '$' );
   if( variableBegin == NULL ) {
      Memory_Free( result );
      return string;
   }

   while( variableBegin != NULL ) {
      charsToWrite = variableBegin - current;
      strncpy( result + charsWritten, current, charsToWrite );
      charsWritten += charsToWrite;
      
      hasBraces = True;
      switch ( *(variableBegin + 1) ) {
         case '{':
            nameBegin = variableBegin + 2;
            endChars = "}";
            break;
         case '(':
            nameBegin = variableBegin + 2;
            endChars = ")";
            break;
         case '\0':
            nameBegin = variableBegin + 1;
            break;
         default:
            nameBegin = variableBegin + 1;
            endChars = "/.;, \\"; 
            hasBraces = False;
      }

      if( nameBegin >= endOfString ) {
         Journal_Printf(
            Journal_Register( Debug_Type, "PathUtils" ),
            "Warning - Environment variable begins at the end of string: %s\n",
            string );
         Memory_Free( result );
         return string;
      }

      variableEnd = strpbrk( nameBegin, endChars );

      if( variableEnd == NULL ) {
         if( hasBraces ) {
            Journal_Printf(
               Journal_Register( Debug_Type, "PathUtils" ),
               "Warning - Mismatched %s for environment variable in value: %s\n",
               endChars,
               string );
            Memory_Free( result );
            return string;
         }
         else {
            variableEnd = endOfString;
         }
      }
      if( variableBegin == variableEnd ) {
         Journal_Printf(
            Journal_Register( Debug_Type, "PathUtils" ),
            "Warning - Empty name for environment variable: %s\n",
            string );
      }

      if( hasBraces ) {
         nameEnd = variableEnd;
         variableEnd++;
      }
      else {
         nameEnd = variableEnd;
      }
      
      nameLength = nameEnd - nameBegin;

      strncpy( envName, nameBegin, nameLength );
      *(envName + nameLength) = '\0';

      envValue = getenv( envName );

      if( envValue == NULL ) {
         envValue = "";
      }

      if( strlen( envValue ) > ( variableEnd - variableBegin ) ) {
         length += strlen( envValue ) - ( variableEnd - variableBegin );
         result = Memory_Realloc_Array( result, char, length + 1 );
      }


      charsToWrite = strlen( envValue );
      strncpy( result + charsWritten, envValue, charsToWrite );
      charsWritten += charsToWrite;

      current = variableEnd;
      variableBegin = strchr( current, '$' );
   }

   charsToWrite = endOfString - current;
   strncpy( result + charsWritten, current, charsToWrite );
   charsWritten += charsToWrite;

   *(result + charsWritten) = '\0';
   
   /* write the result back to the argument */
   /*
   if ( strlen( string ) < strlen( result ) ) {
      string = Memory_Realloc_Array( string, char, strlen( result ) + 1 );
   }
   strcpy( string, result );
   Memory_Free( result );


   return string;
   */
   return result;
}

char* ParentDirectory( const char* path ) {
   char* result;
   int i, length;

   assert( path );
   
   length = strlen( path );
   
   if( length > 0 ) {
      int endIndex = length;
      
      if( path[length - 1] == '/' ) {
         endIndex--;
      }
      
      for( i = endIndex - 1; i >= 0; --i ) {
         if ( path[i] == '/' ) {
            break;
         }
      }
      if( i < 0  ) {
         return NULL;
      }
      result = Memory_Alloc_Array_Unnamed( char, i + 1 );
      strncpy( result, path, i );
      result[i] = '\0';

      return result;
   }
   return NULL;
}

Bool Stg_CreateDirectory( const char* path ) {
   assert( path );
   
   if( ! Stg_DirectoryExists( path ) ) {
      char* parent;
      int ret;

      parent = ParentDirectory( path );
      if( parent != NULL ) {
         if ( Stg_CreateDirectory( parent ) == False ) {
            Memory_Free( parent );
            return False;
         }
         Memory_Free( parent );
      }

      #ifdef NO_SYS_STAT
      ret = mkdir( path);
      #else
      ret = mkdir( path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH );
      #endif

      return (Bool)(ret == 0);
   }

   return True;
}

Bool Stg_FileExists( const char* path ) {
   struct stat info;
   assert( path );
   if( stat( path, &info ) < 0 ) {
      return False;
   }
   return (Bool)(info.st_mode & S_IFREG);
}

Bool Stg_DirectoryExists( const char* path ) {
   struct stat info;
   assert( path );
   if( stat( path, &info ) < 0 ) {
      return False;
   }
   return (Bool)(info.st_mode & S_IFDIR);
}


