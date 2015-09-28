/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __StGermain_Base_Context_AbstractContext_h__
#define __StGermain_Base_Context_AbstractContext_h__
   
   /* Templates of virtual functions */
   typedef void (AbstractContext_SetDt) ( void* self, double dt );
   
   /* Context_CallInfo info */
   #define __StGermain_Base_Context_CallInfo \
      void* functionPointer; \
      void* entryPoint;

   struct Context_CallInfo { __StGermain_Base_Context_CallInfo };
   
   typedef enum CheckpointFileFormat {
      CHECKPOINT_FORMAT_ASCII,   /* Default Stg Ascii text format */
      CHECKPOINT_FORMAT_HDF5      /* Format using HDF5 library */
   } CheckpointFileFormat;
   
   /* AbstractContext entry point names */
   extern Type AbstractContext_EP_AssignFromXML;
   extern Type AbstractContext_EP_AssignFromXMLExtensions;
   extern Type AbstractContext_EP_Build;
   extern Type AbstractContext_EP_Initialise;
   extern Type AbstractContext_EP_Execute;
   extern Type AbstractContext_EP_Destroy;
   extern Type AbstractContext_EP_DestroyExtensions;
   extern Type AbstractContext_EP_Dt;
   extern Type AbstractContext_EP_Step;
   extern Type AbstractContext_EP_UpdateClass;
   extern Type AbstractContext_EP_PreSolveClass;
   extern Type AbstractContext_EP_Solve;
   extern Type AbstractContext_EP_PostSolvePreUpdate;
   extern Type AbstractContext_EP_PostSolvePreUpdateClass;
   extern Type AbstractContext_EP_Sync;
   extern Type AbstractContext_EP_FrequentOutput;
   extern Type AbstractContext_EP_Dump;
   extern Type AbstractContext_EP_DumpClass;
   extern Type AbstractContext_EP_Save;
   extern Type AbstractContext_EP_SaveClass;
   extern Type AbstractContext_EP_DataSave;
   extern Type AbstractContext_EP_DataSaveClass;
   
   /* Textual name of this class */
   extern const Type AbstractContext_Type;
   extern const Type AbstractContext_Type_Verbose; /* Use for a particular info stream */
   
   /* AbstractContext info */
   #define __AbstractContext \
      /* General info */ \
      __Stg_Component \
      Dictionary*            dictionary; \
      \
      /* Virtual info */ \
      AbstractContext_SetDt* _setDt; \
      \
      /* AbstractContext info */ \
      MPI_Comm               communicator; \
      int                    rank; \
      int                    nproc; \
      /* Start time for the simulation. */ \
      double                 startTime; \
      /* Stop time for the simulation. Note that if this is 0, the sim will keep running unless a 
      max loops criterion is met. */ \
      double                 stopTime; \
      double                 currentTime; \
      unsigned int           timeStep; \
      double                 dt; \
      double                 dtFactor; \
      /* This additional timestep is necessary for checkpoint restart runs, so it can be compared against
         maxTimeSteps (which is now relative to job restart).*/ \
      unsigned int           timeStepSinceJobRestart; \
      /* Maximum number of time steps to run for. If set to 0, then this will  be ignored. */ \
      int                    maxTimeSteps; \
      /* Final Time Step: last time step to run till, no matter if maxTimeSteps still has some
         left in a checkpoint restart run. If 0 (the default), not active. */ \
      unsigned int           finalTimeStep; \
      Bool                   gracefulQuit; \
      unsigned int           frequentOutputEvery; \
      Bool                   stepDump; \
      unsigned int           dumpEvery; \
      unsigned int           checkpointEvery; \
      unsigned int           saveDataEvery; \
      double                 checkpointAtTimeInc; \
      double                 nextCheckpointTime; \
      Name                   experimentName; \
      char*                  outputPath; \
      char*                  checkpointReadPath; \
      char*                  checkpointWritePath; \
      /* user set bool to determine whether checkpoint (or data) files should be placed in a per timestep directory */ \
      Bool                   checkpointAppendStep; \
      /* user set bool to determine whether checkpoint restarts should interpolate to new 
         resolution (where resolution is different from checkpoints) */ \
      Bool                   interpolateRestart; \
      Bool                   loadFromCheckPoint; \
      Bool                   loadFieldsFromCheckpoint; \
      Bool                   loadSwarmsFromCheckpoint; \
      Bool                   outputSlimmedXML; \
      Bool                   vis; \
      /* flattened XML output can be disabled if desired (default True) */ \
      int                    restartTimestep; \
      char*                  checkPointPrefixString; \
      Stream*                info; \
      Stream*                verbose; /* general error stream */ \
      Stream*                debug; \
      Stream*                input_verbose_stream; /* intended for xml file warnings */ \
      \
      /* These are stored keys to entrypoints in the table, used for O(1) lookup (i.e. speed) */ \
      /* Contexts "are" Components implemented by entrypoints... there's an entry point per component phase */ \
      EntryPoint_Index       constructK; \
      EntryPoint_Index       constructExtensionsK; \
      EntryPoint_Index       buildK; \
      EntryPoint_Index       initialiseK; \
      EntryPoint_Index       executeK; \
      EntryPoint_Index       destroyK; \
      EntryPoint_Index       destroyExtensionsK; \
      \
      /* The following are not really part of an abstract context, and will one day be refactored somewhere else */ \
      EntryPoint_Index       dtK; \
      EntryPoint_Index       stepK; \
      EntryPoint_Index       updateClassK; \
      EntryPoint_Index       preSolveClassK; \
      EntryPoint_Index       solveK; \
      EntryPoint_Index       postSolveK; \
      EntryPoint_Index       postSolveClassK; \
      EntryPoint_Index       syncK; \
      EntryPoint_Index       frequentOutputK; \
      EntryPoint_Index       dumpK; \
      EntryPoint_Index       dumpClassK; \
      EntryPoint_Index       saveK; \
      EntryPoint_Index       saveClassK; \
      EntryPoint_Index       dataSaveK; \
      EntryPoint_Index       dataSaveClassK; \
      \
      Variable_Register*     variable_Register; \
      EntryPoint_Register*   entryPoint_Register; \
      ExtensionManager*      extensionMgr; \
      Stg_ComponentFactory*  CF; \
      PluginsManager*        plugins; \
      char*                  timeStamp; \
      Bool                   needUpdate;

   struct AbstractContext { __AbstractContext };
      
   /* Class stuff ************************************************************************************************************/
   
   /* No "AbstractContext_New" and "AbstractContext_Init" as this is an abstract class */
   
   /* Creation implementation / Virtual constructor */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define ABSTRACTCONTEXT_DEFARGS \
      STG_COMPONENT_DEFARGS, \
      AbstractContext_SetDt* _setDt, \
      double                 startTime, \
      double                 stopTime, \
      MPI_Comm               communicator, \
      Dictionary*            dictionary

   #define ABSTRACTCONTEXT_PASSARGS \
      STG_COMPONENT_PASSARGS, \
      _setDt, \
      startTime, \
      stopTime, \
      communicator, \
      dictionary  

   AbstractContext* _AbstractContext_New( ABSTRACTCONTEXT_DEFARGS ); 

   /* Initialisation implementation */
   void _AbstractContext_Init( AbstractContext* self );
   
   /* Stg_Class_Delete implementation */
   void _AbstractContext_Delete( void* abstractContext );
   
   /* Print implementation */
   void _AbstractContext_Print( void* abstractContext, Stream* stream );
   
   /* Component stuff ********************************************************************************************************/
   
   /* Construct the context ... connect and validate component connections, and initialise non-bulk internal values */
   void _AbstractContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data );
   
   /* Build the context ... allocates memory (builds arrays) */
   void _AbstractContext_Build( void* context, void* data );
   
   /* Initialise the context ... initialises memory (fills arrays with initial values) */
   void _AbstractContext_Initialise( void* context, void* data );
   
   /* Run the context ... perform the task this component is supposed to perform */
   void _AbstractContext_Execute( void* context, void* data );
   
   /* Destroy the context ... clean up / un build */
   void _AbstractContext_Destroy( void* context, void* data );
   
   /* Context public methods *************************************************************************************************/
   
   /* Print entry points utility */
   void AbstractContext_PrintConcise( void* abstractContext, Stream* stream );
   
   EntryPoint_Index AbstractContext_AddEntryPoint( void* abstractContext, void* entryPoint );
   
   EntryPoint* AbstractContext_GetEntryPoint( void* abstractContext, const Name entryPointName ); 
   
   /* Runs the AbstractContext_EP_FrequentOutput Entry Point */
   void AbstractContext_FrequentOutput( void* context ) ;
   
   /* Runs the AbstractContext_EP_Dump and AbstractContext_EP_DumpClass Entry Points */
   void AbstractContext_Dump( void* context ) ;
   
   /* Runs the AbstractContext_EP_Save Entry Point */
   void AbstractContext_Save( void* context ) ;

   /* Run an entry point... resolving from name (slower) */
   #define AbstractContext_Call( self, name, cast, handle )   ((cast)_AbstractContext_Call( self, name, &handle ))
   Func_Ptr _AbstractContext_Call( void* abstractContext, Name name, void** handle );
   
   /* Run an entry point... resolving using key (slower) */
   #define AbstractContext_KeyHandle( self, key ) \
      EntryPoint_Register_At( (self)->entryPoint_Register, key )
   #define AbstractContext_KeyCall( self, key, cast ) \
      ((cast)(EntryPoint_Register_At( (self)->entryPoint_Register, key )->run))
   
   /* Obtain the Dt */
   double AbstractContext_Dt( void* context );
   
   /* Step the solver */
   void AbstractContext_Step( void* context, double dt );
   
   /* function to warn if no hooks to an entrypoint defined */
   void AbstractContext_WarnIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller );
   
   /* function to error if no hooks to an entrypoint defined */
   void AbstractContext_ErrorIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller );
   
   Bool AbstractContext_CheckPointExists( void* context, Index timeStep );

   /* Works out the prefix string to use for reading checkpoint files (input path + C.P. prefix)
    * Note this allocates a string which the user needs to free when done. */ 
   char* Context_GetCheckPointReadPrefixString( void* context );
   char* Context_GetCheckPointWritePrefixStringTopLevel( void* context );

   /* Works out the prefix string to use for writing checkpoint files (input path + C.P. prefix)
    * Note this allocates a string which the user needs to free when done. */ 
   char* Context_GetCheckPointWritePrefixString( void* context );
   
   /* Context private methods ************************************************************************************************/
   
   /* Default construction hook, and overrides for the EP to handle the context/ptrToContext synchronisation */
   void _AbstractContext_Construct_Hook( void* _context, void* data );

   Func_Ptr _AbstractContext_Construct_EP_GetRun( void* entryPoint );

   void _AbstractContext_Construct_EP_Run( void* entryPoint, void* data0, void* data1 );
   
   /* Default construction hook */
   void _AbstractContext_Execute_Hook( void* _context );
   
   /* Step the solver implementation */
   void _AbstractContext_Step( void* _context, double dt );
   
   /* update configuration in preperation for next solve */
   void AbstractContext_Update(void* _context);

   void _AbstractContext_LoadTimeInfoFromCheckPoint( void* _context, Index timeStep, double* dtLoadedFromFile );

   void _AbstractContext_SaveTimeInfo( void* _context );

   void _AbstractContext_CreateCheckpointDirectory( void* _context );

   void _AbstractContext_AllOutput( void* _context );



#endif /* __StGermain_Base_Context_AbstractContext_h__ */

