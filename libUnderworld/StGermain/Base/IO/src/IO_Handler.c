/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "IO_Handler.h"
#include "Journal.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef WINDOWS
   #include <direct.h>
   #define getCurrentDir _getcwd
#else
   #include <unistd.h>
   #define getCurrentDir getcwd
#endif

/* Textual name of this class */
const Type IO_Handler_Type = Type_Invalid;

/* Setting the default merge types centrally here */
const Dictionary_MergeType IO_Handler_DefaultMergeType = Dictionary_MergeType_Replace;
const Dictionary_MergeType IO_Handler_DefaultChildrenMergeType = Dictionary_MergeType_Append;

/* TODO: register with IO_Handler registry */
IO_Handler* _IO_Handler_New( IO_HANDLER_DEFARGS ) {
   IO_Handler* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(IO_Handler) );
   self = (IO_Handler*)_Stg_Class_New( STG_CLASS_PASSARGS );
   
   /* General info */
   
   /* Virtual functions */
   self->_readAllFromFile = _readAllFromFile;
   self->_readAllFromBuffer = _readAllFromBuffer;
   self->_writeAllToFile = _writeAllToFile;
   
   /* IO_Handler info */
   _IO_Handler_Init( self );
   
   return self;
}

void _IO_Handler_Init( IO_Handler* self ) {
   /* General and Virtual info should already be set */

   /* IO_Handler info */
   self->currDictionary = NULL;
   self->currSources = NULL;
   self->resource = NULL;
   self->currPath = NULL;
   self->schema = NULL;
   self->validate = 0;
}

/* delete the object's memory at this level */
void _IO_Handler_Delete( void* io_handler ) {
   IO_Handler* self = (IO_Handler*)io_handler;
   
   if( self->currPath )
      Memory_Free( self->currPath );

   if( self->resource )
      Memory_Free( self->resource );

   /* Stg_Class_Delete parent class */
   _Stg_Class_Delete( self );
}

/* print this level's info */
void _IO_Handler_Print( void* io_handler, Stream* stream ) {
   IO_Handler* self = (IO_Handler*)io_handler;
   
   /* General info */
   printf( "IO_Handler (ptr): %p\n", (void*)self );
   
   /* Print parent class */
   _Stg_Class_Print( self, stream );
   
   /* Virtual info */
   printf( "\t_readAllFromFile (func ptr): %p\n", (void*)self->_readAllFromFile );
   printf( "\t_writeAllToFile (func ptr): %p\n", (void*)self->_writeAllToFile );
   
   /* IO_Handler */
   printf( "\tcurrDictionary (ptr): %p\n", (void*)self->currDictionary );
   printf( "\tcurrSources (ptr): %p\n", (void*)self->currSources );

   if( self->resource ) 
      printf( "\tresource: %s\n", self->resource );
   else 
      printf( "\tresource: (null)\n" );
}

/* Read a dictionary entry of a given name from file */
Bool IO_Handler_ReadAllFromFile( void* io_handler, const char* filename, Dictionary* dictionary, Dictionary* sources ) {
   IO_Handler* self = (IO_Handler*)io_handler;
   Index       chr_I;
   char*       pwd;

   pwd = getCurrentDir( NULL, 0 );
   
   for( chr_I = strlen( filename ); chr_I > 0; chr_I--)
      if( filename[chr_I - 1] == '/' ) {
         self->currPath = Memory_Alloc_Array( char, (chr_I + 1), "IO_Handler->currPath" );
         strncpy( self->currPath, filename, chr_I );
         self->currPath[chr_I] = 0;
         break;
      }
      
   if( chr_I == 0 ) {
      Stg_asprintf( &self->currPath, "%s%s", pwd, "/" );
   }

   return self->_readAllFromFile( self, filename, dictionary, sources );
}

/* Read a dictionary entry of a given name from file */
Bool IO_Handler_ReadAllFromBuffer( void* io_handler, const char* buffer, Dictionary* dictionary, const char* tag ) {
   IO_Handler* self = (IO_Handler*)io_handler;

   return self->_readAllFromBuffer( self, buffer, dictionary, tag );
}

/* write a given dictionary entry to file */
Bool IO_Handler_WriteAllToFile(
   void*       io_handler,
   const char* filename,
   Dictionary* dictionary,
   Dictionary* sources )
{
   IO_Handler* self = (IO_Handler*)io_handler;
   return self->_writeAllToFile( self, filename, dictionary, sources );
}

/* Add a new dictionary value depending on the parent type, or replace the existing value if found.
 * if parent is NULL, add as a plain entry. If parent is a list, add as an element to that
 * list. If parent is a struct, add as a member of that struct. */
Dictionary_Entry_Value* IO_Handler_DictSetAddValue( 
   void*                   io_handler, 
   Dictionary_Entry_Key    name,
   char*                   newValue, 
   char                    newDictValType,
   Dictionary_Entry_Value* parent, 
   Dictionary_MergeType    mergeType )
{
   IO_Handler*             self = (IO_Handler*)io_handler;
   Dictionary_Entry_Value* newDictVal = NULL;
   #if DEBUG
      assert( self );
   #endif
   
   /* If parent == NULL... then this is being added to no list or struct, but the dictionary itself */
   if( NULL == parent ) {
      if( NULL == name ) {
         fprintf( stdout, "Warning- while parsing file %s: entry that isn't a list element "
            "given with no name. Not adding this parameter to the dictionary.\n",
            self->resource );
         return NULL;
      }
      
      newDictVal = Dictionary_Entry_Value_FromStringTo( newValue, newDictValType );
      newDictVal = Dictionary_AddMerge( self->currDictionary, name, newDictVal, mergeType );
   }
   else if( parent->type == Dictionary_Entry_Value_Type_List ) {
      if( NULL != name ) {
         fprintf( stdout, "Warning - while parsing file %s: entry inside a list given with a name. Adding the "
            "entry to the list, but ignoring the name.\n", self->resource );
      }   
      
      newDictVal = Dictionary_Entry_Value_FromStringTo( newValue, newDictValType );
      Dictionary_Entry_Value_AddElement( parent, newDictVal );
   }
   else if( parent->type == Dictionary_Entry_Value_Type_Struct ) {
   /*   Dictionary_Entry_Value* member = NULL;      */
      newDictVal = Dictionary_Entry_Value_FromStringTo( newValue, newDictValType );
      
      if( NULL == name ) {
         fprintf( stdout, "Warning - while parsing file %s: entry that isn't a list element given with no name. "
            "Not adding it to the dictionary.\n", self->resource );
         return NULL;
      }
      
      newDictVal = Dictionary_AddMerge( parent->as.typeStruct, name, newDictVal, mergeType );
      /* This code overrides the merge code, disabling... 
      if( (member = Dictionary_Entry_Value_GetMember( parent, name ) ) )
         Dictionary_Entry_Value_SetMember( parent, name, newDictVal );
      else 
         Dictionary_Entry_Value_AddMember( parent, name, newDictVal );
      */
   }
   else {
      fprintf( stdout, "Warning - while parsing file %s: IO_Handler_SetAdd passed a parent not of list "
         "or struct type. Ignoring.\n", self->resource );
   }
   
   return newDictVal;
} /* IO_Handler_SetAdd */

/* Add a new dictionary value depending on the parent type, or replace the existing value if found.
 * if parent is NULL, add as a plain entry. If parent is a list, add as an element to that
 * list. If parent is a struct, add as a member of that struct. */
Dictionary_Entry_Value* IO_Handler_DictSetAddValueWithSource( 
   void*                   io_handler, 
   Dictionary_Entry_Key    name,
   char*                   newValue, 
   char                    newDictValType,
   Dictionary_Entry_Units  newUnits,
   Dictionary_Entry_Value* parent, 
   Dictionary_MergeType    mergeType,
   Dictionary_Entry_Source source )
{
   IO_Handler* self = (IO_Handler*)io_handler;
   Dictionary_Entry_Value* newDictVal = NULL;
   #if DEBUG
      assert( self );
   #endif
   
   /* If parent == NULL... then this is being added to no list or struct, but the dictionary itself */
   if( NULL == parent ) {
      if( NULL == name ) {
         fprintf( stdout, "Warning- while parsing file %s: entry that isn't a list element "
            "given with no name. Not adding this parameter to the dictionary.\n",
            self->resource );
         return NULL;
      }
      
      newDictVal = Dictionary_Entry_Value_FromStringTo( newValue, newDictValType );
      newDictVal = Dictionary_AddMergeWithSource( self->currDictionary, name, 
         newDictVal, newUnits, mergeType, source );
   }
   else if( parent->type == Dictionary_Entry_Value_Type_List ) {
      if( NULL != name ) {
         fprintf( stdout, "Warning - while parsing file %s: entry inside a list given with a name. Adding the "
            "entry to the list, but ignoring the name.\n", self->resource );
      }   
      newDictVal = Dictionary_Entry_Value_FromStringTo( newValue, newDictValType );
      Dictionary_Entry_Value_AddElementWithSource( parent, newDictVal, source );
   }
   else if( parent->type == Dictionary_Entry_Value_Type_Struct ) {
      /* Dictionary_Entry_Value* member = NULL;      */
      newDictVal = Dictionary_Entry_Value_FromStringTo( newValue, newDictValType );
      
      if( NULL == name ) {
         fprintf( stdout, "Warning - while parsing file %s: entry that isn't a list element given with no name. "
            "Not adding it to the dictionary.\n", self->resource );
         return NULL;
      }
      
      newDictVal = Dictionary_AddMergeWithSource( parent->as.typeStruct, name, 
         newDictVal, newUnits, mergeType, source );

      /* This code overrides the merge code, disabling...
      if( (member = Dictionary_Entry_Value_GetMember( parent, name ) ) )
         Dictionary_Entry_Value_SetMember( parent, name, newDictVal );
      else 
         Dictionary_Entry_Value_AddMember( parent, name, newDictVal );
      */
   }
   else {
      fprintf( stdout, "Warning - while parsing file %s: IO_Handler_SetAdd passed a parent not of list "
         "or struct type. Ignoring.\n", self->resource );
   }
   
   return newDictVal;
} 

Index IO_Handler_ReadAllFilesFromCommandLine(
   void*       ioHandler,
   int         argc,
   char*       argv[],
   Dictionary* dictionary,
   Dictionary* sources )
{
   IO_Handler* self = (IO_Handler*)ioHandler;
   Stream*     errorStream  = Journal_Register( Error_Type, CURR_MODULE_NAME );
   Index       arg_I;
   char*       filename;
   char*       extension;
   Bool        result;
   Index       filesRead = 0;

   /* Loop over all the arguments from command line */
   for( arg_I = 1; arg_I < argc; arg_I++ ) {
      filename = argv[ arg_I ];

      /* Find extension of potential filename by finding the pointer to the last dot in the string */
      extension = strrchr( filename, '.' );

      /* Check if there was a '.' in the filename at all - if not, then bail */
      if( extension == NULL )
         continue;

      /* Check if file has a ".xml" extension - if not, then bail */
      if( strcasecmp( extension, ".xml" ) != 0 )
         continue;
   
      /* Read file */
      result = IO_Handler_ReadAllFromFile( self, filename, dictionary, sources );

      Journal_Firewall( result, errorStream, 
         "Error: %s could not read input file %s. Exiting.", argv[0], filename );

      filesRead++;      
   }

   return filesRead;
}

void _get_XML(void* ioHandler, char* argv[], int argc) {
   IO_Handler* self = (IO_Handler*)ioHandler;
   Index       arg_I;
   int         schema = 0;
   char*       name;
   char*       option;
   
   for ( arg_I = 1 ; arg_I < argc; arg_I++) {
      name = argv[ arg_I ];
      option = strrchr( name, '-' );
      if( schema == 1 ) { 
         if( self->schema==NULL ) {
            self->schema = (char*)malloc( strlen( argv[ arg_I ] ) +1 );
            strcpy( self->schema, argv[ arg_I ] );
            schema = 0;
         }
      }
      if( option != NULL ) {
         if( strcasecmp( option, "-schema" ) == 0 )
            schema = 1;
         if( strcasecmp( option, "-dontXMLValidate" ) == 0 )
            (self->validate) = 0;
         if( strcasecmp( option, "-XMLValidate" ) == 0 )
            (self->validate) = 1;
      }
   }
}

Index IO_Handler_ReadAllFromCommandLine( void* ioHandler, int argc, char* argv[], Dictionary* dictionary, Dictionary* sources ) {
   Index filesRead = 0;

   _get_XML( ioHandler, argv, argc );
   filesRead = IO_Handler_ReadAllFilesFromCommandLine( ioHandler, argc, argv, dictionary, sources );
   Dictionary_ReadAllParamFromCommandLine( dictionary, argc, argv );

   return filesRead;
}


