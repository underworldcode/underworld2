/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**   A system which manages all output systems of a program.
**
** <b>Assumptions:</b>
**   None
**
** <b>Comments:</b>
**   The StGermain Journal contains 4 default typed streams: Info, Debug, Dump and Error. 
**   
**   By default, the streams use CStream for outputing and display to standard displays.
**
** <b>Description:</b>
**   The Journal class is a singleton that stores all information about available streams ("typed streams") and currently
**   active stream instances ("named streams"). The user is able to create and access named streams of any type at a whim
**   using the Journal singleton.
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Base_IO_Journal_h__
#define __StGermain_Base_IO_Journal_h__

#ifdef __cplusplus
extern "C" {
#endif


   /** Textual name for Journal class. */
   extern const Type Journal_Type;
   
   /** The Journal singleton. */
   extern Journal* stJournal;

   extern const Type Info_Type; /**< Use this constant to retrieve the Info stream. */
   extern const Type Debug_Type; /**< Use this constant to retrieve the Debug stream. */
   extern const Type Dump_Type; /**< Use this constant to retrieve the Dump stream. */
   extern const Type Error_Type; /**< Use this constant to retrieve the Error stream. */

   extern const Type InfoStream_Type; /**< \deprecated Info stream. */
   extern const Type DebugStream_Type; /**< \deprecated Debug stream. */
   extern const Type DumpStream_Type; /**< \deprecated Dump stream. */
   extern const Type ErrorStream_Type; /**< \deprecated Error stream. */

   extern const char* JOURNAL_DELIMITER;
   extern const char* JOURNAL_KEY;
   extern const char* JOURNAL_ENABLE_KEY;
   extern const char* JOURNAL_ENABLE_BRANCH_KEY;
   extern const char* JOURNAL_LEVEL_KEY;
   extern const char* JOURNAL_LEVEL_BRANCH_KEY;
   extern const char* JOURNAL_FILE_KEY;
   extern const char* JOURNAL_RANK_KEY;
   extern const char* JOURNAL_FLUSH_KEY;
   extern const char* JOURNAL_MPI_OFFSET_KEY;

   extern const char* JOURNAL_ON;
   extern const char* JOURNAL_OFF;

   /** \def __Journal See Journal. */
   #define __Journal \
      Bool            enable; \
      File*    stdOut; /**< Default file for stdout. */ \
      File*    stdErr; /**< Default file for stderr. */ \
      Stg_ObjectList* _files; \
      Stg_ObjectList* _typedStreams; \
      Bool            firewallProducesAssert; /**< Can be overruled by compile flag NDEBUG even if True */ \
      Bool            firewallProtected; /** This should be set to True by the pcu driver */ \
      Index           flushCount;

   struct Journal { __Journal };

   /** Creates a Journal instance. */
   Journal* Journal_New();

   /** Deallocates the Journal singleton. */
   void Journal_Delete( );

   /** Purge all the streams */
   void Journal_Purge();

   /** Reads entries from the the given dictionary object to create/configure streams in Journal. */
   void Journal_ReadFromDictionary( Dictionary* dictionary );


   /** Adds the given stream as a typed stream to the journal. The name of the stream should be set to a unique value.
    ** If an existing typed stream in the system has the same name, the argument will be rejected and deallocated. 
    **/
   void Journal_RegisterTypedStream( Stream* typedStream );

   /** Returns a typed stream registered in the Journal. If the stream does not exists, NULL is returned. */
   Stream* Journal_GetTypedStream( const Type type );
   
   /** Retrives a named stream from the system of the the given type and name.
    ** If the instance does not exist, a new stream is created as a duplicate of the typed stream.
    **
    ** It is possible to provide a dotted-decimal name to retrive a sub stream.
    **/
   Stream* Journal_Register( const Type streamType, Name name );   
   
   /** Designed for components to have their own stream.
    ** If the instance does not exist, a new stream is created as a duplicate of the typed stream.
    **
    ** @param componentType The type of the component
    ** @param componentName The name of the component
    **/
   Stream* Journal_Register2( const Type streamType, const Type componentType, Name componentName );   

   /** Shortcut for objects and components to retrive their own stream */
   #define Journal_MyStream( streamType, component ) \
      Journal_Register2( (streamType), (component)->type, (component)->name )

   /** Returns a registered file from the system. If the file does not exist, NULL is returned. */
   File* Journal_GetFile( Name fileName );

   /** Registers a file with the Journal system. If the argument is NULL, no operation is performed. */
   void Journal_RegisterFile( File* newFile );
   
   /** Deregisters a file with the Journal system, deleting the reference from the object list.
    ** If the argument is NULL, no operation is performed. */
   void Journal_DeregisterFile( File* file ); 

   /** Enables/Disables the specified typed stream. */
   void Journal_Enable_TypedStream( const Type type, Bool enable );

   /** Enables/Disables all defined typed streams */
   void Journal_Enable_AllTypedStream( Bool enable );

   /** Enables/Disables the given named stream.
    **
    ** If enabling, this will enable all parent streams to ensure output is possible.
    ** Dotted-decimal notation is available to access sub streams. 
    **/
   void Journal_Enable_NamedStream( const Type type, const Name name, Bool enable );

   /** Prints a summary of the status of each stream in Journal */
   void Journal_PrintConcise();

   /** Performs a printf() with the given stream. */
   int Journal_Printf( void* _stream, const char* fmt, ... );

   /** Performs a printf() with the given stream and level of printing.
    **
    ** Output is only produced if the stream's level of printing is equal or greater.
    **/
   int Journal_PrintfL( void* _stream, JournalLevel level, const char* fmt, ... );
   
   #ifdef DEBUG
      /** Performs a Journal_Printf() only if DEBUG is defined. */
      #define Journal_DPrintf   Journal_Printf
   #else
      /* Removes from compilation using if (0)
       * Compilers won't compile if (0) code even without optimisation options
       */
      #define Journal_DPrintf if (0) Journal_Printf
   #endif

   #ifdef DEBUG
      /** Performs a Journal_PrintfL() only if DEBUG is defined. */
      #define Journal_DPrintfL Journal_PrintfL
   #else
      /* Removes from compilation using if (0)
       * Compilers won't compile if (0) code even without optimisation options
       */
      #define Journal_DPrintfL if (0) Journal_PrintfL
   #endif

   /** If expression is zero, this function will display a printf to the stream, and exit the program.
    **
    ** If NDEBUG is defined, a graceful exit is performed, otherwise an assert() is called to produce
    ** a core file.
    **
    ** Not to be called on its own. Please use Journal_Firewall() macro.
    **/
   /* 
   int Journal_Firewall_Func( int expression, char* expressionText, 
      const char* file, const char* func, int line, void* stream, char* fmt, ... );
   */
   
   /** Macro for calling Journal_Firewall()
    **
    ** Call this function as if the prototype was:
    **    int Journal_Firewall( int expression, void* stream, char* fmt, ... );
    **
    ** The 'char* fmt' argument is passed as a part of __VA_ARGS__. This is intentional as
    ** old cc compilers (such as cc on grendel) must have at least 1 argument. C99 compliant
    ** compilers can have 0 arguments as it will consume the last comma ',' to prevent a syntax error.
    **/
   /*
   #define Journal_Firewall( expression, stream, ... ) \
      Journal_Firewall_Func( expression, #expression, __FILE__, __func__, __LINE__, stream, ##__VA_ARGS__ )
   */
   
   /* Temporary revert back to old protoype to avoid compilation issues. Need to fix. */
   #define Journal_Firewall( expression, stream, ... ) \
                             expression ? 0 : Journal_Firewall_Trigger( 0, stream, ##__VA_ARGS__ )
   int Journal_Firewall_Trigger( int expression, void* stream, char* fmt, ... );
   
   #ifdef DEBUG
      /** Performs a Journal_Firewall() only if DEBUG is defined. */
      #define Journal_DFirewall Journal_Firewall
   #else
      /* Removes from compilation using if (0)
       * Compilers won't compile if (0) code even without optimisation options
       */
      #define Journal_DFirewall if (0) Journal_Firewall
   #endif

   /** Performs a write() to the given stream.
    **
    ** @param data A contiguous block of data to write, generally a 1D array.
    ** @param elem_size The size of each element in the block.
    ** @param num_elems The number of elements in this array.
    **/
   SizeT Journal_Write( void* stream, void* data, SizeT elem_size, SizeT num_elems );

   /** Performs a dump depending on the implemenation of the given stream. */
   Bool Journal_Dump( void* stream, void* data );

   int Journal_RPrintf( void* _stream, const char* const fmt, ... );
   int Journal_RPrintfL( void* _stream, JournalLevel level, const char* const fmt, ... );

   /** Set up the default type streams expected throughout StGermain */
   void Journal_SetupDefaultTypedStreams();

#ifdef __cplusplus
}
#endif
   
#endif /* __StGermain_Base_IO_Journal_h__ */
