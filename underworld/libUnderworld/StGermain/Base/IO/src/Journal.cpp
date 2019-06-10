/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifndef NO_UNISTD
#include <unistd.h>
#endif

#include <stdarg.h>
#include <stdexcept>
#include <mpi.h>
extern "C" {
#include "pcu/pcu.h"   /* We'll use pcu_assert in Firewall function */
#include "Base/Foundation/Foundation.h"

#include "types.h"
#include "shortcuts.h"
#include "Dictionary.h"
#include "Dictionary_Entry.h"
#include "Dictionary_Entry_Value.h"
#include "Journal.h"
#include "File.h"
#include "CFile.h"
#include "Stream.h"
#include "CStream.h"
#include "MPIStream.h"
#include "BinaryStream.h"
#include "StreamFormatter.h"
#include "LineFormatter.h"
#include "RankFormatter.h"
}

#include <sstream>
#include <string>

using namespace std;

Journal* stJournal = NULL;

const Type Journal_Type = "Journal";

/* String constants for default Journal typed streams. */
const Type Info_Type = "info";
const Type Debug_Type = "debug";
const Type Dump_Type = "dump";
const Type Error_Type = "error";

/* Depricated string constants. */
const Type InfoStream_Type = "info";
const Type DebugStream_Type = "debug";
const Type DumpStream_Type = "dump";
const Type ErrorStream_Type = "error";

/* Parsing constants for Dictionary reading. */
const char* JOURNAL_DELIMITER = ".";
const char* JOURNAL_KEY = "journal";
const char* JOURNAL_ENABLE_KEY = "journal";
const char* JOURNAL_ENABLE_BRANCH_KEY = "journal-enable-branch";
const char* JOURNAL_LEVEL_KEY = "journal-level";
const char* JOURNAL_LEVEL_BRANCH_KEY = "journal-level-branch";
const char* JOURNAL_FILE_KEY = "journal-file";
const char* JOURNAL_RANK_KEY = "journal-rank";
const char* JOURNAL_FLUSH_KEY = "journal-autoflush";
const char* JOURNAL_MPI_OFFSET_KEY = "journal-mpi-offset";

const char* JOURNAL_ON = "on";
const char* JOURNAL_OFF = "off";

static const int JOURNAL_START_SIZE = 16;   /**< Initial size of lists in Journal. */
static const int JOURNAL_DELTA_SIZE = 8;   /**< Number of elements to extend by for lists in Journal. */


/** Retrieves the given named stream. Dotted-decimal notation may be used to get sub streams.
 ** If the stream does not exists, a new stream is created from the parent stream.
 **/
Stream* Journal_GetNamedStream( Stream* typedStream, const Name name );

Journal* Journal_New() {
   Journal* self = Memory_Alloc( Journal, "Singleton" );
   
   self->_files = Stg_ObjectList_New2( JOURNAL_START_SIZE, JOURNAL_DELTA_SIZE );
   self->_typedStreams = Stg_ObjectList_New2( JOURNAL_START_SIZE, JOURNAL_DELTA_SIZE );
   self->enable = True;

   /** Initialise default files. */
   self->stdOut = CFile_New();
   self->stdOut->fileHandle = stdout;
   self->stdOut->_opened = True;
   
   self->stdErr = CFile_New();
   self->stdErr->fileHandle = stderr;
   self->stdErr->_opened = True;
   
   self->firewallProducesAssert = False;

   self->flushCount = 0;
   
   return self;
}

void Journal_Delete() {
   stJournal->stdOut->fileHandle = NULL;  // set this to null so that we do not fclose stdout
   Stg_Class_Delete( stJournal->stdOut );
   stJournal->stdOut = NULL;

   stJournal->stdErr->fileHandle = NULL;  // set this to null so that we do not fclose stderr
   Stg_Class_Delete( stJournal->stdErr );
   stJournal->stdErr = NULL;

   Journal_Purge();
   Stg_Class_Delete( stJournal->_typedStreams );
   stJournal->_typedStreams = NULL;
   Stg_Class_Delete( stJournal->_files );
   stJournal->_files = NULL;
   
   Memory_Free( stJournal );
   stJournal=NULL;
}

void Journal_Purge() {
   unsigned i;
   
   Stg_ObjectList_DeleteAllObjects( stJournal->_typedStreams );

   for( i = 0; i < stJournal->_files->count; ++i ) {
      if( !File_Close( (File*) stJournal->_files->data[i] ) ) {
         /* TODO: warning can't close */
      }
   }
   Stg_ObjectList_DeleteAllObjects( stJournal->_files );
}

void Journal_ReadFromDictionary( Dictionary* dictionary ) {
   Dictionary_Index index;
   Stream*          stream = NULL; /* The stream which the operation will be applied to. */
   char*            keyCopy;   
   char*            typedToken;
   char*            namedToken;
   char*            operation;
   
   /* Iterate through the whole dictionary, checking for journal related commands */
   for( index = 0; index < dictionary->count; ++index ) {
      /* Check to see if it is a journal operation by searching for the JOURNAL_KEY. */
      if( ( 0 == strncasecmp( dictionary->entryPtr[index]->key, JOURNAL_KEY, strlen(JOURNAL_KEY) ) ) ) {

         /* Make copy to tokenise so original contents will not be destroyed */
         keyCopy = StG_Strdup( dictionary->entryPtr[index]->key );
         
         /* The first token determines the operation to perform. */
         operation = strtok( keyCopy, JOURNAL_DELIMITER );
         
         typedToken = strtok( NULL, JOURNAL_DELIMITER );
         namedToken = strtok( NULL, "" );
            
         if( strcmp( operation, JOURNAL_ENABLE_KEY ) == 0 ) {
            /* Enable function */
            Bool enable = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
            
            if( typedToken == NULL ) {
               /* no more tokens, set the entire journal */
               stJournal->enable = enable;
            }
            else {
               if( namedToken == NULL ) {
                  /* no more tokens, set the entire type */
                  Journal_Enable_TypedStream( typedToken, enable );
               }
               else {
                  Journal_Enable_NamedStream( typedToken, namedToken, enable );
               }
            }
         } /* END Enable function */
         if( strcmp( operation, JOURNAL_ENABLE_BRANCH_KEY ) == 0 ) {
            Stream* branchStream;
            /* Enable function */
            Bool enable = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
      
            assert( typedToken );
            if( namedToken == NULL ) {
               branchStream = Journal_GetTypedStream( typedToken );
            }
            else {
               branchStream = Journal_Register( typedToken, namedToken );
            }
            Stream_EnableBranch( branchStream, enable );
            
         } /* END Enable function */
         else if( strcmp( operation, JOURNAL_LEVEL_KEY ) == 0 ) {
            /* Set level function */
            JournalLevel level;
            
            level = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );
            
            if( typedToken == NULL ) {
               printf( "Warning - Can only change level for typed and named streams.\n" );
            }
            else {
               if( namedToken == NULL ) {
                  stream = Journal_GetTypedStream( typedToken );
               }
               else {
                  stream = Journal_Register( typedToken, namedToken );
               }
               Stream_SetLevel( stream, level );
            }
         } /* END SetLevel function */
         else if( strcmp( operation, JOURNAL_LEVEL_BRANCH_KEY ) == 0 ) {
            /* Set level function */
            JournalLevel level;
            
            level = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );
            
            if( typedToken == NULL ) {
               printf( "Warning - Can only change level for typed and named streams.\n" );
            }
            else {
               if( namedToken == NULL ) {
                  stream = Journal_GetTypedStream( typedToken );
               }
               else {
                  stream = Journal_Register( typedToken, namedToken );
               }
               Stream_SetLevelBranch( stream, level );
            }
         } /* END SetLevelBranch function */
         else if( strcmp( operation, JOURNAL_FILE_KEY ) == 0 ) {
            /* Set file destination function */
            File* file = NULL;
            char* fileName = Dictionary_Entry_Value_AsString( dictionary->entryPtr[index]->value );
            
            if( typedToken == NULL ) {
               printf( "Warning - Can only change files for typed and named streams.\n" );
            }
            else {
               if( namedToken == NULL ) {
                  /* setting file for typed stream */
                  stream = Journal_GetTypedStream( typedToken );
               }
               else {
                  stream = Journal_Register( typedToken, namedToken );
               }
               
               file = Journal_GetFile( fileName );
               if( file == NULL ) {
                  file = stream->defaultFileType();
                  if( File_Write( file, fileName ) ) {
                     Journal_RegisterFile( file );
                  }
                  else {
                     /* error/warning */
                  }
               }
               if( !Stream_SetFile( stream, file ) ) {
                  /* file is not valid */
               }
            }
         } /* END File function */
         else if( strcmp( operation, JOURNAL_RANK_KEY ) == 0 ) {
            int rank;

            rank = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );

            if( typedToken == NULL ) {
               printf( "Warning - Can only set printing rank for typed and named streams.\n" );
            }
            else {
               if( namedToken == NULL ) {
                  stream = Journal_GetTypedStream( typedToken );
               }
               else {
                  stream = Journal_Register( typedToken, namedToken );
               }
               Stream_SetPrintingRank( stream, rank );
            }
         } /* END Printing Rank funciton. */
         else if( strcmp( operation, JOURNAL_FLUSH_KEY ) == 0 ) {
            Bool flush = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );

            if( typedToken == NULL ) {
               printf( "Warning - Can only set auto-flush for typed and named streams.\n" );
            }
            else {
               if( namedToken == NULL ) {
                  stream = Journal_GetTypedStream( typedToken );
               }
               else {
                  stream = Journal_Register( typedToken, namedToken );
               }
               Stream_SetAutoFlush( stream, flush );
            }
         }
         else if( strcmp( operation, JOURNAL_MPI_OFFSET_KEY ) == 0 ) {
            MPI_Offset offset = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );

            if( typedToken == NULL ) {
               printf( "Warning - Can only set mpi-offset for typed and named streams.\n" );
            }   
            else {
               if( namedToken == NULL ) {
                  stream = Journal_GetTypedStream( typedToken );
               }
               else {
                  stream = Journal_Register( typedToken, namedToken );
               }

               if( ! MPIStream_SetOffset( stream, offset, MPI_COMM_WORLD ) ) {
                  printf( "Warning - Failed to set mpi-offset for %s", typedToken );
                  if( namedToken ) {
                     printf( ".%s", namedToken );
                  }
                  printf( "\n" );
               }   
            }
         }   
         Memory_Free( keyCopy );
      }   
   }
}

void Journal_RegisterTypedStream( Stream* typedStream ) {
   /* check exists and update */
   Stg_ObjectList_Append( stJournal->_typedStreams, typedStream );
}

Stream* Journal_GetTypedStream( const Type type ) {
   Stream* typedStream = NULL;

   typedStream = (Stream*)Stg_ObjectList_Get( stJournal->_typedStreams, (Name)type );   
   if( typedStream == NULL ) {
      /* Make a default stream for this name. */
      typedStream = CStream_New( type );
      Stream_Enable( typedStream, True );
      Stream_SetLevel( typedStream, 1 );
      Stream_SetFile( typedStream, stJournal->stdOut );

      Journal_RegisterTypedStream( typedStream );
   }
   return typedStream;
}

Stream* Journal_GetNamedStream( Stream* typedStream, const Name name ) {
   Stream* currentStream ;
   char*   nameCopy;
   char*   token;
   
   currentStream = typedStream;
   nameCopy = StG_Strdup( name );
   
   
   token = strtok( nameCopy, JOURNAL_DELIMITER );
   while( token != NULL ) {
      currentStream = Stream_RegisterChild( currentStream, token );
      token = strtok( NULL, JOURNAL_DELIMITER );
   }
   
   Memory_Free( nameCopy );
   
   if( currentStream == typedStream ) {
      return NULL;
   }
   
   return currentStream;
}

Stream* Journal_Register( const Type type, const Name name ) {
   Stream* typedStream = NULL;
   Stream* namedStream = NULL;

   typedStream = Journal_GetTypedStream( type );
   namedStream = Journal_GetNamedStream( typedStream, name );   
   
   return namedStream;
}

Stream* Journal_Register2( const Type streamType, const Type componentType, const Name componentName ) {
   Stream* componentStream;
   Stream* instanceStream;
   componentStream = Journal_Register( streamType, componentType );
   instanceStream = Stream_RegisterChild( componentStream, componentName );

   return instanceStream;
}

File* Journal_GetFile( const Name fileName ) {
   return (File*) Stg_ObjectList_Get( stJournal->_files, fileName );
}

void Journal_RegisterFile( File* newFile ) {
   if( newFile == NULL )
   {
      /* Warning */
      return;
   }
   Stg_ObjectList_Append( stJournal->_files, newFile );
}

void Journal_DeregisterFile( File* file ) {
   if( file == NULL ) {
      return;
   }
   Stg_ObjectList_Remove( stJournal->_files, file->name, KEEP );
}

void Journal_Enable_TypedStream( const Type type, Bool enable ) {
   Stream* typedStream = Journal_GetTypedStream( type );
   Stream_Enable( typedStream, enable );
}

void Journal_Enable_AllTypedStream( Bool enable ) {
   Journal_Enable_TypedStream( Info_Type, enable );
   Journal_Enable_TypedStream( Debug_Type, enable );
   Journal_Enable_TypedStream( Dump_Type, enable );
   Journal_Enable_TypedStream( Error_Type, enable );
}

void Journal_Enable_NamedStream( const Type type, const Name name, Bool enable ) {
   Stream* stream = Journal_Register( type, name );
   Stream_Enable( stream, enable );
}

void Journal_PrintConcise() {
   Stream* stream;
   int     i;
   
   if( stJournal == NULL ) {
      return;
   }

   stream = Journal_Register( Info_Type, "Journal" );

   Journal_Printf( stream, "\nSummary of Journal and Stream Status:\n" );
   Journal_Printf( stream, "Journal Enabled: %s\n", stJournal->enable ? "True" : "False" );
   
   Journal_Printf(
      stream,
      "Explanation of the following stream status printout:\n"
      "The streams are organised hierarchically, as trees, according to the\n"
      "purpose of the stream - the key stream types \"info\", \"debug\" and \"error\"\n"
      "are at the top, with all other streams as their children.\n"
      "We also display the status of each stream: the \"Enabled\" parameter, which can\n"
      "be individually set from input files/command line, determines whether output sent\n"
      "to a given stream will print or not. The \"Level\" parameter determines the level\n"
      "of detail that it will output - 1 is the default, 4 means very verbose.\n");

   Journal_Printf( stream, "Typed Streams:\n" );

   for( i = 0; i < stJournal->_typedStreams->count; ++i ) {
      Stream_PrintConcise( stJournal->_typedStreams->data[i], stream );
   }
   Journal_Printf( stream, "\n" );
}

int Journal_Printf( void* _stream, const char* fmt, ... ) {
   int     result;
   Stream* stream = (Stream*)_stream;
   
   va_list ap;

   if( !stJournal || !stJournal->enable || !Stream_IsEnable( stream ) ) {
      return 0;
   }

   va_start( ap, fmt );
    
   result = Stream_Printf( stream, fmt, ap );
   
   va_end(ap);
   
   return result;
}

int Journal_PrintfL( void* _stream, JournalLevel level, const char* fmt, ... ) {
   int     result;
   Stream* stream = (Stream*)_stream;
   
   va_list ap;

   if( !stJournal->enable ||
      !Stream_IsEnable( stream ) ||
      !Stream_IsPrintableLevel( stream, level ) ) {
      return 0;
   }

   va_start( ap, fmt );
    
   result = Stream_Printf( stream, fmt, ap );
   
   va_end(ap);
   return result;
}
/*
int Journal_Firewall_Func( int expression, char* expressionText, 
   const char* file, const char* func, int line, void* _stream, char* fmt, ... )
*/
int Journal_Firewall_Trigger( int expression, void* _stream, char* fmt, ... ) {
   
   // Set up error strings
   stringstream ss;
   ss << "Error encountered. Full restart recommended as exception safety not guaranteed. Error message:\n";

   va_list ap;
   char* fullerrorstring;
   va_start( ap, fmt );

   // copy the string into buffer
   Stg_vasprintf( &fullerrorstring, fmt, ap ) ;
   ss << fullerrorstring;
   Memory_Free(fullerrorstring);

   if( stJournal->firewallProducesAssert && !stJournal->firewallProtected ) {
      /* Use pcu_assert, so that StGermain PCU tests can check that a Firewall
       * is correctly produced. */ 
      pcu_assert( expression );
   }
   else if( stJournal->firewallProducesAssert && stJournal->firewallProtected ) {
      pcu_rollback( expression );
   }
   else {
     throw std::runtime_error( ss.str() );
   }
   return 0;

}

SizeT Journal_Write( void* _stream, void* data, SizeT elem_size, SizeT num_elems ) {
   Stream* stream = (Stream*)_stream;

   return Stream_Write( stream, data, elem_size, num_elems );
}

Bool Journal_Dump( void* _stream, void* data ) {
   Stream* stream = (Stream*)_stream;

   return Stream_Dump( stream, data );
}

/* Only rank 0 will print to stream */
int Journal_RPrintf( void* _stream, const char* const fmt, ... ) {
   Stream* stream = (Stream*)_stream;
   int     result, init_stream_rank;
   va_list ap;

   init_stream_rank = Stream_GetPrintingRank( stream );
   Stream_SetPrintingRank( stream, 0 );

   va_start( ap, fmt );

   if( !stJournal->enable || !Stream_IsEnable( stream ) ) {
      return 0;
   }

   result = Stream_Printf( stream, fmt, ap );

   va_end(ap);

   Stream_SetPrintingRank( stream, init_stream_rank );
   return result;
}

/* Only rank 0 will print to stream */
int Journal_RPrintfL( void* _stream, JournalLevel level, const char* const fmt, ... ) {
   Stream* stream = (Stream*)_stream;
   int     result, init_stream_rank;
   va_list ap;

   init_stream_rank = Stream_GetPrintingRank( stream );
   Stream_SetPrintingRank( stream, 0 );

   va_start( ap, fmt );

   if( !stJournal->enable ||
      !Stream_IsEnable( stream ) ||
      !Stream_IsPrintableLevel( stream, level ) ) {
      return 0;
   }

   result = Stream_Printf( stream, fmt, ap );

   va_end(ap);

   Stream_SetPrintingRank( stream, init_stream_rank );
   return result;
}

void Journal_SetupDefaultTypedStreams() {
   Stream* typedStream;

   /* info */
   typedStream = CStream_New( Info_Type );
   Stream_Enable( typedStream, True );
   Stream_SetLevel( typedStream, 1 );
   Stream_SetFile( typedStream, stJournal->stdOut );
   Stream_SetAutoFlush( typedStream, True );
   Journal_RegisterTypedStream( typedStream );
   /* Default when registering a Journal stream is to have it print from Rank 0 only) */
   Stream_SetPrintingRank( typedStream, 0);

   /* debug */
   typedStream = CStream_New( Debug_Type );
   Stream_Enable( typedStream, False );
   Stream_SetLevel( typedStream, 1 );
   Stream_SetFile( typedStream, stJournal->stdOut );
   Stream_SetAutoFlush( typedStream, True );
   Stream_AddFormatter( typedStream, RankFormatter_New() );
   Journal_RegisterTypedStream( typedStream );
   /* Default when registering a Journal stream is to have it print from Rank 0 only) */
   Stream_SetPrintingRank( typedStream, 0);
   
   /* dump */
   typedStream = CStream_New( Dump_Type );
   Stream_Enable( typedStream, False );
   Stream_SetLevel( typedStream, 1 );
   Stream_SetFile( typedStream, stJournal->stdOut );
   Journal_RegisterTypedStream( typedStream );
   
   /* error */
   typedStream = CStream_New( Error_Type );
   Stream_Enable( typedStream, True );
   Stream_SetLevel( typedStream, 1 );
   Stream_SetFile( typedStream, stJournal->stdErr );
   Stream_SetAutoFlush( typedStream, True );
   Stream_AddFormatter( typedStream, RankFormatter_New() );
   Journal_RegisterTypedStream( typedStream );
   
   /* mpi stream */
   typedStream = MPIStream_New( MPIStream_Type );
   Stream_Enable( typedStream, True );
   Stream_SetLevel( typedStream, 1 );
   Journal_RegisterTypedStream( typedStream );
   /* MPI Streams need to print from all ranks */

   /* binary stream */
   typedStream = BinaryStream_New( BinaryStream_Type );
   Stream_Enable( typedStream, True );
   Stream_SetLevel( typedStream, 1 );
   Journal_RegisterTypedStream( typedStream );
}


