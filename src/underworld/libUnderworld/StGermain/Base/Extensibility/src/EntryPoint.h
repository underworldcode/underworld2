/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Extensibility_EntryPoint_h__
#define __StGermain_Base_Extensibility_EntryPoint_h__
   
   /* Templates of virtual functions */
   typedef Func_Ptr (EntryPoint_GetRunFunction) ( void* self );
   
   /* Templates for default entry point type */
   typedef void   (EntryPoint_0_Cast)                         ();
   typedef void   (EntryPoint_0_CallCast)                     ( void* entryPoint );
   typedef void   (EntryPoint_VoidPtr_Cast)                   ( void* data0 );
   typedef void   (EntryPoint_VoidPtr_CallCast)               ( void* entryPoint, void* data0 );
   typedef void   (EntryPoint_2VoidPtr_Cast)                  ( void* data0, void* data1 );
   typedef void   (EntryPoint_2VoidPtr_CallCast)              ( void* entryPoint, void* data0, void* data1 );
   typedef void   (EntryPoint_3VoidPtr_Cast)                  ( void* data0, void* data1, void* data2 );
   typedef void   (EntryPoint_3VoidPtr_CallCast)              ( void* entryPoint, void* data0, void* data1, void* data2 );
   typedef double (EntryPoint_Minimum_VoidPtr_Cast)           ( void* data0 );
   typedef double (EntryPoint_Minimum_VoidPtr_CallCast)       ( void* entryPoint, void* data0 );
   typedef double (EntryPoint_Maximum_VoidPtr_Cast)           ( void* data0 );
   typedef double (EntryPoint_Maximum_VoidPtr_CallCast)       ( void* entryPoint, void* data0 );
   typedef double (EntryPoint_Class_Minimum_VoidPtr_Cast)     ( void* reference, void* data0 );
   typedef double (EntryPoint_Class_Minimum_VoidPtr_CallCast) ( void* entryPoint, void* reference, void* data0 );
   typedef double (EntryPoint_Class_Maximum_VoidPtr_Cast)     ( void* reference, void* data0 );
   typedef double (EntryPoint_Class_Maximum_VoidPtr_CallCast) ( void* entryPoint, void* reference, void* data0 );
   typedef void   (EntryPoint_Class_0_Cast)                   ( void* reference );
   typedef void   (EntryPoint_Class_0_CallCast)               ( void* entryPoint );
   typedef void   (EntryPoint_Class_VoidPtr_Cast)             ( void* reference, void* data0 );
   typedef void   (EntryPoint_Class_VoidPtr_CallCast)         ( void* entryPoint, void* data0 );

   #define EntryPoint_0_CastType                     0
   #define EntryPoint_VoidPtr_CastType               (EntryPoint_0_CastType+1)
   #define EntryPoint_2VoidPtr_CastType              (EntryPoint_VoidPtr_CastType+1)
   #define EntryPoint_3VoidPtr_CastType              (EntryPoint_2VoidPtr_CastType+1)
   #define EntryPoint_Minimum_VoidPtr_CastType       (EntryPoint_3VoidPtr_CastType+1)
   #define EntryPoint_Maximum_VoidPtr_CastType       (EntryPoint_Minimum_VoidPtr_CastType+1)
   #define EntryPoint_Class_0_CastType               (EntryPoint_Maximum_VoidPtr_CastType+1)
   #define EntryPoint_Class_VoidPtr_CastType         (EntryPoint_Class_0_CastType+1)
   #define EntryPoint_Class_Minimum_VoidPtr_CastType (EntryPoint_Class_VoidPtr_CastType+1)
   #define EntryPoint_Class_Maximum_VoidPtr_CastType (EntryPoint_Class_Minimum_VoidPtr_CastType+1)
   #define EntryPoint_CastType_MAX                   (EntryPoint_Class_Maximum_VoidPtr_CastType+1)
   
   /** Textual name of this class */
   extern const Type EntryPoint_Type;
   
   /** EntryPoint info */
   #define __EntryPoint \
      /* General info */ \
      __Stg_Object \
      \
      /* Virtual info */ \
      EntryPoint_GetRunFunction* _getRun; \
      \
      /* EntryPoint info */ \
      unsigned int               castType; \
      void*                      run; \
      HookList*                  hooks; \
      Hook*                      alwaysFirstHook; \
      Hook*                      alwaysLastHook;

   struct EntryPoint { __EntryPoint };
   
   /** Create a new EntryPoint */
   EntryPoint* EntryPoint_New( const Name name, unsigned int castType );
   
   /** Initialise an existing EntryPoint */
   void EntryPoint_Init( void* entryPoint, const Name name, unsigned int castType );
   
   /** Creation implementation */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define ENTRYPOINT_DEFARGS \
      STG_OBJECT_DEFARGS, \
      EntryPoint_GetRunFunction* _getRun, \
      unsigned int               castType

   #define ENTRYPOINT_PASSARGS \
      STG_OBJECT_PASSARGS, \
      _getRun,  \
      castType

   EntryPoint* _EntryPoint_New( ENTRYPOINT_DEFARGS );
   
   /** Member Initialisation implementation */
   void _EntryPoint_Init( EntryPoint* self, unsigned int castType );
   
   /** Stg_Class_Delete() implementation */
   void _EntryPoint_Delete( void* entryPoint );
   
   /** Stg_Class_Print() implementation */
   void _EntryPoint_Print( void* entryPoint, Stream* stream );
   
   /* Copy */
   #define EntryPoint_Copy( self ) \
      (EntryPoint*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
   #define EntryPoint_DeepCopy( self ) \
      (EntryPoint*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
   
   void* _EntryPoint_Copy( void* entryPoint, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
   
   /* VIRTUAL FUNCTIONS */

   /* Obtain the pointer the function that will run this entry point (i.e. the function that will call the many functions this
    *  entry point */
   Func_Ptr EntryPoint_GetRun( void* entryPoint );
   
   /** Default GetRun implementation */
   Func_Ptr _EntryPoint_GetRun( void* entryPoint );
   
   /** Default entry point run... no arguements passed */
   void _EntryPoint_Run( void* entryPoint );
   
   /** Default entry point run... one void* arguement passed */
   void _EntryPoint_Run_VoidPtr( void* entryPoint, void* data0 );
   
   /** Default entry point run funcs... many void* arguments passed */
   void _EntryPoint_Run_2VoidPtr( void* entryPoint, void* data0, void* data1 );
   void _EntryPoint_Run_3VoidPtr( void* entryPoint, void* data0, void* data1, void* data2 ) ;

   /** Entry point run functions which calculates and returns the max or min of what each of the hooks returned */
   double _EntryPoint_Run_Minimum_VoidPtr( void* entryPoint, void* data0 ) ;
   double _EntryPoint_Run_Maximum_VoidPtr( void* entryPoint, void* data0 ) ;
   double _EntryPoint_Run_Class_Minimum_VoidPtr( void* entryPoint, void* data0 ) ;
   double _EntryPoint_Run_Class_Maximum_VoidPtr( void* entryPoint, void* data0 ) ;

   /** Runs a plain ClassHook */
   void _EntryPoint_Run_Class( void* entryPoint );
   void _EntryPoint_Run_Class_VoidPtr( void* entryPoint, void* data0 );

   
   /* Functions for Adding hooks */
   void _EntryPoint_PrependHook( void* entryPoint, Hook* hook );
   void _EntryPoint_PrependHook_AlwaysFirst( void* entryPoint, Hook* hook );
   void _EntryPoint_AppendHook( void* entryPoint, Hook* hook );
   void _EntryPoint_AppendHook_AlwaysLast( void* entryPoint, Hook* hook );
   void _EntryPoint_InsertHookBefore( void* entryPoint, Name hookToInsertBefore, Hook* hook );
   void _EntryPoint_InsertHookAfter( void* entryPoint, Name hookToInsertAfter, Hook* hook );
   void _EntryPoint_ReplaceAllHook( void* entryPoint, Hook* hook );
   void _EntryPoint_ReplaceHook( void* entryPoint, Name hookToReplace, Hook* hook );
   void _EntryPoint_ProfileFunction( char ep_name[], char hk_name[], double time );
   
   /* PUBLIC FUNCTIONS */
   
   /* Print concise function:- print information of interest when not debugging */
   void EntryPoint_PrintConcise( void* entryPoint, Stream* stream );
   
   /** Prepend a new Hook function at the start of an entry point's list of hooks to run. */
   void EntryPoint_Prepend( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /** Prepend a new Hook function at the start of an entry point's list of hooks to run, specifying it
   should always remain the first hook. */
   void EntryPoint_Prepend_AlwaysFirst( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /** Add a new Func_Ptr to an entry point, at the end of the current list of hooks. If there is a hook
   already specified to always come last, the new hook will be appended to come just before it. */
   void EntryPoint_Append( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /** Add a new Func_Ptr to an entry point, at the end of the list of hooks - and make
   sure the entry point is always kept at the end. If this is called twice on
   the same entry point, an assert results. */
   void EntryPoint_Append_AlwaysLast( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );

   /** Add a new Hook function to an entry point, just before the specified hookToInsertBefore in the list of hooks.
    * If the specified hook to insert before doesn't exist, then asserts with an error. */
   void EntryPoint_InsertBefore( void* entryPoint, Name hookToInsertBefore, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /** Add a new Hook function to an entry point, just after the specified hookToInsertAfter in the list of hooks.
    * If the specified hook to insert after doesn't exist, then asserts with an error. */
   void EntryPoint_InsertAfter( void* entryPoint, Name hookToInsertAfter, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /** Replace a specific named hook with a new hook function.
    * If the specified hook to replace doesn't exist, then asserts with an error. */
   void EntryPoint_Replace( void* entryPoint, Name hookToReplace, Name name, Func_Ptr funcPtr, char* addedBy );
   
   /** Replace all existing hooks of an entry point with a new hook. */
   void EntryPoint_ReplaceAll( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy );


   /** Prepend a new Hook function at the start of an entry point's list of hooks to run. */
   void EntryPoint_PrependClassHook( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
   
   /** Prepend a new Hook function at the start of an entry point's list of hooks to run, specifying it
   should always remain the first hook. */
   void EntryPoint_PrependClassHook_AlwaysFirst( 
      void*    entryPoint, 
      Name     name, 
      Func_Ptr funcPtr, 
      char*    addedBy, 
      void*    reference );
   
   /** Add a new Func_Ptr to an entry point, at the end of the current list of hooks. If there is a hook
   already specified to always come last, the new hook will be appended to come just before it. */
   void EntryPoint_AppendClassHook( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );
   
   /** Add a new Func_Ptr to an entry point, at the end of the list of hooks - and make
   sure the entry point is always kept at the end. If this is called twice on
   the same entry point, an assert results. */
   void EntryPoint_AppendClassHook_AlwaysLast( 
      void*    entryPoint, 
      Name     name, 
      Func_Ptr funcPtr, 
      char*    addedBy, 
      void*    reference );

   /** Add a new Hook function to an entry point, just before the specified hookToInsertBefore in the list of hooks.
    * If the specified hook to insert before doesn't exist, then asserts with an error. */
   void EntryPoint_InsertClassHookBefore( 
      void*    entryPoint, 
      Name     hookToInsertBefore, 
      Name     name, 
      Func_Ptr funcPtr, 
      char*    addedBy, 
      void*    reference );
   
   /** Add a new Hook function to an entry point, just after the specified hookToInsertAfter in the list of hooks.
    * If the specified hook to insert after doesn't exist, then asserts with an error. */
   void EntryPoint_InsertClassHookAfter( 
      void*    entryPoint, 
      Name     hookToInsertAfter, 
      Name     name, 
      Func_Ptr funcPtr, 
      char*    addedBy, 
      void*    reference );
   
   /** Replace a specific named hook with a new hook function.
    * If the specified hook to replace doesn't exist, then asserts with an error. */
   void EntryPoint_ReplaceClassHook( 
      void*    entryPoint, 
      Name     hookToReplace, 
      Name     name, 
      Func_Ptr funcPtr, 
      char*    addedBy, 
      void*    reference );
   
   /** Replace all existing hooks of an entry point with a new hook. */
   void EntryPoint_ReplaceAllClassHook( void* entryPoint, Name name, Func_Ptr funcPtr, char* addedBy, void* reference );

   /** Removes a hook from an entry point
    * If the specified hook to remove doesn't exist, then asserts with an error. */
   void EntryPoint_Remove( void* entryPoint, Name name ) ;   

   /** "Purge" the entry point of all existing hooks, leaving it blank. */
   void EntryPoint_Purge( void* entryPoint );
   
   void EntryPoint_WarnIfNoHooks( void* entryPoint, const char* parentFunction );

   void EntryPoint_ErrorIfNoHooks( void* entryPoint, const char* parentFunction );
   
#endif /* __StGermain_Base_Extensibility_EntryPoint_h__ */

