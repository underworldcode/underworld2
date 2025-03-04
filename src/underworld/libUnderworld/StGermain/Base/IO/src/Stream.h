/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_IO_Stream_h__
#define __StGermain_Base_IO_Stream_h__

#include <stdarg.h>

   /** Textual name for Stream class. */
   extern const Type Stream_Type;

   typedef SizeT (Stream_PrintfFunction) (Stream* object, char* fmt, va_list args);
   typedef SizeT (Stream_WriteFunction) (Stream* object, void* data, SizeT elem_size, SizeT num_elems);
   typedef Bool (Stream_DumpFunction) (Stream* object, void* data);
   typedef Bool (Stream_SetFileFunction) ( Stream* object, File* file );

   /** Indicates all ranks should print. */
   #define STREAM_ALL_RANKS (-1)

   /** \def __Stream See Stream */
   #define __Stream \
      __Stg_Object \
      \
      /* Virtual info */ \
      Stream_PrintfFunction*   _printf; \
      Stream_WriteFunction*    _write; \
      Stream_DumpFunction*     _dump; \
      Stream_SetFileFunction*  _setFile; \
      File_Constructor* defaultFileType; \
      \
      /* Stream info */ \
      Bool                     _enable; \
      JournalLevel             _level; \
      File*             _file; \
      int                      _printingRank; \
      Bool                     _autoFlush; \
      IndentFormatter*         _indent; \
      StreamFormatter**        _formatter; \
      Index                    _formatterCount; \
      Index                    _formatterSize; \
      char*                    _currentSource; \
      char*                    _currentFunction; \
      int                      _currentLine; \
      \
      Stream*                  _parent; \
      Stg_ObjectList*          _children; 
   struct Stream { __Stream };

   /** Constructor interface. */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define STREAM_DEFARGS \
      STG_CLASS_DEFARGS, \
      Name                    name, \
      Stream_PrintfFunction*  _printf, \
      Stream_WriteFunction*   _write, \
      Stream_DumpFunction*    _dump, \
      Stream_SetFileFunction* _setFile

   #define STREAM_PASSARGS \
      STG_CLASS_PASSARGS, \
      name, \
      _printf, \
      _write, \
      _dump, \
      _setFile

   Stream* _Stream_New( STREAM_DEFARGS );
      
   /** Init interface. */
   void _Stream_Init(
      Stream*                 self,
      Name                    name,
      Stream_PrintfFunction*  _printf,
      Stream_WriteFunction*   _write,
      Stream_DumpFunction*    _dump,
      Stream_SetFileFunction* _setFile );
   
   /** Stg_Class_Delete interface. */
   void _Stream_Delete( void* stream );

   /** Print interaface. */
   void _Stream_Print( void* stream, Stream* paramStream );
   
   /** Print concise info function */
   void Stream_PrintConcise( void* stream, Stream* paramStream );

   /** Copy interface. */   
   void* _Stream_Copy( void* stream, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
   
   /** Performs a printf() on the given stream. */
   SizeT Stream_Printf( Stream* stream, const char* const fmt, va_list args );

   /** Performs a write() on the given stream. */
   SizeT Stream_Write( Stream* stream, void* data, SizeT elem_size, SizeT num_elems );

   /** Performs a dump opertion on the given stream. */
   Bool Stream_Dump( Stream* stream, void* data );

   /** Opens a registers a file for this stream if not already opened and assigns it for output
       Defaults CFile for now. TODO for next io commit */
   Bool Stream_RedirectFile( Stream* stream, const char* const fileName );

   /** Opens a registers a file for all the stream if not already opened and assigns it for output
       Defaults CFile for now. TODO for next io commit */
   Bool Stream_RedirectAllToFile( const char* const fileName );

   /** Purges all redirected stream files */
   void Stream_PurgeAllRedirectedFiles( void );

   /** Opens a registers a file for this stream if not already opened and assigns it for output for whole branch */
   Bool Stream_RedirectFileBranch( Stream* stream, const char* const fileName );

   /** Opens and registers a file for this stream if not already opened, goes to end of file and assigns it for output
       Defaults CFile for now. TODO for next io commit */
   Bool Stream_AppendFile( Stream* stream, const char* const fileName );
   
   /** Opens a registers a file for this stream if not alreconst ady opened and assigns it for output for whole branch */
   Bool Stream_RedirectFileBranch( Stream* stream, const char* const fileName );

   /** Sets the file which the stream is directed to, returning True if successful.
    ** This function may fail if an unusable file type is given to the stream. */
   Bool Stream_SetFile( Stream* stream, File* file );

   /** Sets the file which the stream and its sub-streams is directed to, returning True if all was successful.
    ** This funciton may fail if an unusable file type is given to the stream. */
   Bool Stream_SetFileBranch( Stream* stream, File* file );

   /** returns a ptr to the stream's File */
   File* Stream_GetFile( void* stream );

   /** Returns True if this stream is valid to produce output. */
   Bool Stream_IsEnable( void* stream );

   /** Enables/disables a stream for printing. If enabling, this function performs a 'hard' enable, setting
    ** all of its parents to enable as well, ensuring that this stream will print. */
   void Stream_Enable( void* stream, Bool enable );

   /** Enables/disables a stream and its sub-streams for printing. */
   void Stream_EnableBranch( void* stream, Bool enable );

   /** Enables/disables a stream for printing. Will not affect parents or children streams. */
   void Stream_EnableSelfOnly( void* stream, Bool enable );

   /** Returns True if the argument level is less than or equal the the stream's level, meaning printing is possible
    ** with Journal_PrintfL() */
   Bool Stream_IsPrintableLevel( void* stream, JournalLevel level );

   /** Sets the printing level of a stream. */
   void Stream_SetLevel( void* stream, JournalLevel level );

   /** Sets the printing level of a stream and its sub-streams. */
   void Stream_SetLevelBranch( void* stream, JournalLevel level );

   /** Sets the printing rank for a stream. Only a process with the same rank value will perform printing, meaning
    ** only one rank will print from an output statement.
    **
    ** If rank is STREAM_ALL_RANKS, all ranks will print. This is the default behaviour.
    **/
   void Stream_SetPrintingRank( void* stream, int rank );

   /** Gets the printing rank for a stream. */
   int Stream_GetPrintingRank( void* stream );

   /** Sets the printing rank for a stream and sub-streams. */
   void Stream_SetPrintingRankBranch( void* stream, int rank );

   /** Sets whether a stream should flush after an output statement. */
   void Stream_SetAutoFlush( void* stream, Bool autoFlush );

   /** Gets the AutoFlush status of a stream */
   Bool Stream_GetAutoFlush( void* stream );

   /** Sets whether a stream and all of its sub-streams should flush after an output statement. */
   void Stream_SetAutoFlushBranch( void* stream, Bool autoFlush );

   /** Sets the number of indentations per line to begin printing for a stream. */
   void Stream_SetIndent( void* stream, Index indent );

   /** Increases the level of indentation by one for a stream. */
   void Stream_Indent( void* stream );

   /** Decreases the level of indentation by one for a stream. Will not go below zero. */
   void Stream_UnIndent( void* stream );

   /** Sets the level of indentation to zero for a stream. */
   void Stream_ZeroIndent( void* stream );

   /** Sets the number of indentations per line to begin printing for a stream and sub streams. */
   void Stream_SetIndentBranch( void* stream, Index indent );
   
   /** Increases the level of indentation by one for a stream and its sub streams. */
   void Stream_IndentBranch( void* stream );
   
   /** Decreases the level of indentation by one for a stream and its sub streams. Will not go below zero. */
   void Stream_UnIndentBranch( void* stream );
   
   /** Sets the level of indentation to zero for a stream and its sub streams. */
   void Stream_ZeroIndentBranch( void* stream );

   /** Adds a formatter to the stream for custom formatting. */
   void Stream_AddFormatter( void* stream, StreamFormatter* formatter );

   /** Remove any and all formatters currently applied to a stream */
   void Stream_ClearCustomFormatters( void* stream );
   
   /** Sets miscellaneous information to the stream.
    **
    ** @param currentSource The source file which output is currently being done.
    ** @param currentFunction The function which output is currently being done.
    ** @param line The current line which output is currently being done.
    **/
   void Stream_SetCurrentInfo(
      void* stream,
      const char* const currentSource,
      const char* const currentFunction,
      int line );
   
   /** Retrives a sub stream of the given name. If stream does not exist, a new sub stream is created and returned.
    ** Dotted-decimal notation can be used to retrive nested sub-streams. */
   Stream* Stream_RegisterChild( void* stream, Name name );

   /** Flushes a stream of its outputs. */
   Bool Stream_Flush( void* stream );

   /** Request to the stream that the file its using to be closed to save resources. */
   Bool Stream_CloseFile( void* stream );

   /* Closes, deregisters and deletes the File associated with this stream */
   void Stream_CloseAndFreeFile( void* stream ) ;

#endif /* __StGermain_Base_IO_Stream_h__ */

