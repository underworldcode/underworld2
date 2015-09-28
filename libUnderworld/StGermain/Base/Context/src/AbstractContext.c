/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include <mpi.h>  /* subsequent files need this */
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "units.h"
#include "types.h"
#include "shortcuts.h"
#include "Variable.h"
#include "Variable_Register.h"
#include "AbstractContext.h"
#include "ContextEntryPoint.h"
#include "DictionaryCheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(READ_HDF5) || defined(WRITE_HDF5)
#include <hdf5.h>
#endif

/* AbstractContext entry point names */
Type AbstractContext_EP_AssignFromXML =           "Context_Construct";
Type AbstractContext_EP_AssignFromXMLExtensions = "Context_ConstructExtensions";
Type AbstractContext_EP_Build =                   "Context_Build";
Type AbstractContext_EP_Initialise =              "Context_Initialise";
Type AbstractContext_EP_Execute =                 "Context_Execute";
Type AbstractContext_EP_Destroy =                 "Context_Destroy";
Type AbstractContext_EP_DestroyExtensions =       "Context_DestroyExtensions";

Type AbstractContext_EP_Dt =                      "Context_Dt";
Type AbstractContext_EP_Step =                    "Context_Step";
Type AbstractContext_EP_UpdateClass =             "Context_UpdateClass";
Type AbstractContext_EP_PreSolveClass =           "Context_PreSolveClass";
Type AbstractContext_EP_Solve =                   "Context_Solve";
Type AbstractContext_EP_PostSolvePreUpdate =      "Context_PostSolvePreUpdate";
Type AbstractContext_EP_PostSolvePreUpdateClass = "Context_PostSolvePreUpdateClass";
Type AbstractContext_EP_Sync =                    "Context_Sync";
Type AbstractContext_EP_FrequentOutput =          "Context_FrequentOutput";
Type AbstractContext_EP_Dump =                    "Context_Dump";
Type AbstractContext_EP_DumpClass =               "Context_DumpClass";
Type AbstractContext_EP_Save =                    "Context_Save";
Type AbstractContext_EP_SaveClass =               "Context_SaveClass";
Type AbstractContext_EP_DataSave =                "Context_DataSave";
Type AbstractContext_EP_DataSaveClass =           "Context_DataSaveClass";

/* Dictionary entry names */
const Type AbstractContext_Dict_Components = "components";

/* Class stuff ********************************************************************************************************************/

/* Textual name of this class */
const Type AbstractContext_Type = "Context";
const Type AbstractContext_Type_Verbose = "Context-verbose";

AbstractContext* _AbstractContext_New( ABSTRACTCONTEXT_DEFARGS ) {
   AbstractContext* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(AbstractContext) );
   self = (AbstractContext*)_Stg_Component_New( STG_COMPONENT_PASSARGS );
   
   /* General info */
   self->dictionary = dictionary;

   /* Virtual info */
   self->CF = 0; /* gets built in stgMain and passed in during the construct phase */
   self->_setDt = _setDt;
   self->startTime = startTime;
   self->stopTime = stopTime;
   self->communicator = communicator;
   
   MPI_Comm_rank( self->communicator, &self->rank );
   MPI_Comm_size( self->communicator, &self->nproc );

   self->info = Journal_Register( InfoStream_Type, (Name)AbstractContext_Type  );
   self->verbose = Journal_Register( InfoStream_Type, (Name)AbstractContext_Type_Verbose  );
   self->debug = Journal_Register( DebugStream_Type, (Name)AbstractContext_Type  );
   self->input_verbose_stream = Journal_Register( Error_Type, (Name)AbstractContext_Type  );
      
   return self;
}

void _AbstractContext_Init( AbstractContext* self ) {
   char buf[80];

#ifdef READ_HDF5
   /* 
    * Disable HDF5 error reporting, as verbosity can be excessive, and some 
    * errors are expected and need not be reported.
    */
   #if H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8
   H5Eset_auto(NULL, NULL);
   #else
   H5Eset_auto2(H5E_DEFAULT, NULL, NULL);
   #endif
#endif
   
   /* General and Virtual info should already be set. */
   Journal_Enable_TypedStream( DebugStream_Type, False );
   Journal_Enable_TypedStream( DumpStream_Type, False );

   if( self->rank == 0 ) {
      Journal_Printf( self->debug, "In: %s: self->communicator: %u, self->nproc: %u, self->rank %u\n", 
         __func__, self->communicator, self->nproc, self->rank );
   }
   sprintf( buf, "journal.info.%s", AbstractContext_Type_Verbose );

   if( !Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)buf ) ) 
      Journal_Enable_NamedStream( InfoStream_Type, AbstractContext_Type_Verbose, False );
   
   /* Turn off the journal warning debug stream by default: even if debug is enabled in general. */
   if( !Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"journal.debug.DictionaryWarning" ) ) 
      Journal_Enable_NamedStream( DebugStream_Type, "DictionaryWarning", False );
   
   /* Set up the registers and managers */
   self->variable_Register = Variable_Register_New();
   self->extensionMgr = ExtensionManager_New_OfExistingObject( self->type, self );
   ExtensionManager_Register_Add( extensionMgr_Register, self->extensionMgr );
   self->plugins = PluginsManager_New();
   
   /* Build the entryPoint table */
   self->entryPoint_Register = EntryPoint_Register_New(); 

   /* 
    * For the construct EP, override the run function such that the context/ptrToContext
    * remain in sync in the loop. 
    */
   self->constructK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_AssignFromXML, EntryPoint_2VoidPtr_CastType ) );
   AbstractContext_GetEntryPoint( self, AbstractContext_EP_AssignFromXML )->_getRun = _AbstractContext_Construct_EP_GetRun;
   AbstractContext_GetEntryPoint( self, AbstractContext_EP_AssignFromXML )->run = EntryPoint_GetRun( AbstractContext_GetEntryPoint( self, AbstractContext_EP_AssignFromXML ) );

   self->constructExtensionsK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_AssignFromXMLExtensions, EntryPoint_VoidPtr_CastType ) );
   self->buildK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Build, EntryPoint_VoidPtr_CastType ) );
   self->initialiseK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Initialise, EntryPoint_VoidPtr_CastType ) );
   self->executeK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Execute, EntryPoint_VoidPtr_CastType ) );
   self->destroyK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Destroy, EntryPoint_VoidPtr_CastType ) );
   self->destroyExtensionsK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_DestroyExtensions, EntryPoint_VoidPtr_CastType ) );
   
   self->dtK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Dt, ContextEntryPoint_Dt_CastType ) );
   self->stepK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Step, ContextEntryPoint_Step_CastType ) );
   self->updateClassK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_UpdateClass, EntryPoint_Class_VoidPtr_CastType ) );
   self->preSolveClassK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_PreSolveClass, EntryPoint_Class_VoidPtr_CastType ) );
   self->solveK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Solve, EntryPoint_VoidPtr_CastType ) );
   self->postSolveK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_PostSolvePreUpdate, EntryPoint_VoidPtr_CastType ) );
   self->postSolveClassK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_PostSolvePreUpdateClass, EntryPoint_Class_VoidPtr_CastType ) );
   self->syncK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Sync, EntryPoint_VoidPtr_CastType ) );
   self->frequentOutputK =   Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_FrequentOutput, EntryPoint_VoidPtr_CastType ) );
   self->dumpK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Dump, EntryPoint_VoidPtr_CastType ) );
   self->dumpClassK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_DumpClass, EntryPoint_Class_VoidPtr_CastType ) );
   self->saveK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_Save, EntryPoint_VoidPtr_CastType ) );
   self->saveClassK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_SaveClass, EntryPoint_Class_VoidPtr_CastType ) );
   self->dataSaveK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_DataSave, EntryPoint_VoidPtr_CastType ) );
   self->dataSaveClassK = Context_AddEntryPoint( self, ContextEntryPoint_New( AbstractContext_EP_DataSaveClass, EntryPoint_Class_VoidPtr_CastType ) );
   
   /* Add initial hooks. */

   /* 
    * Don't need now Stg_ComponentFactory_ConstructComponents, 13Nov09 JG,
    * plan to rejig the context post upcoming release.
    */
   EntryPoint_Append(
      AbstractContext_GetEntryPoint( self, AbstractContext_EP_AssignFromXML ), "_AbstractContext_Construct_Hook",
      (Func_Ptr)_AbstractContext_Construct_Hook, AbstractContext_Type );

   EntryPoint_Append( 
      AbstractContext_GetEntryPoint( self, AbstractContext_EP_Execute ), "_AbstractContext_Execute_Hook", 
      (Func_Ptr)_AbstractContext_Execute_Hook, AbstractContext_Type );

   EntryPoint_Append( 
      AbstractContext_GetEntryPoint( self, AbstractContext_EP_Step ), "_AbstractContext_Step", 
      (Func_Ptr)_AbstractContext_Step, AbstractContext_Type );

   EntryPoint_Append( 
      AbstractContext_GetEntryPoint( self, AbstractContext_EP_Save ), "_AbstractContext_SaveTimeInfo", 
      (Func_Ptr)_AbstractContext_SaveTimeInfo, AbstractContext_Type );

   EntryPoint_Prepend_AlwaysFirst( 
      AbstractContext_GetEntryPoint( self, AbstractContext_EP_Save ), "_AbstractContext_CreateCheckpointDirectory", 
      (Func_Ptr)_AbstractContext_CreateCheckpointDirectory, AbstractContext_Type );

   EntryPoint_Prepend_AlwaysFirst( 
      AbstractContext_GetEntryPoint( self, AbstractContext_EP_DataSave ), "_AbstractContext_CreateCheckpointDirectory", 
      (Func_Ptr)_AbstractContext_CreateCheckpointDirectory, AbstractContext_Type );
}

void _AbstractContext_Delete( void* abstractContext ) {
   AbstractContext* self = (AbstractContext*)abstractContext;

   Stg_Class_Delete( self->entryPoint_Register );

   /* Remove the self->extensionMgr of this context from the extensionMgr_Register. */
   ExtensionManager_Register_Remove( extensionMgr_Register, self->extensionMgr );
   Stg_Class_Delete( self->extensionMgr );
   Stg_Class_Delete( self->dictionary );

   Memory_Free( self->experimentName );
   Memory_Free( self->outputPath );
   Memory_Free( self->checkpointReadPath );
   Memory_Free( self->checkpointWritePath );
   Memory_Free( self->timeStamp );

   Stg_Class_Delete( self->variable_Register );

   /* Unload all dynamic plugins. */
   PluginsManager_UnloadAll( self->plugins );
   
   /* Stg_Class_Delete parent. */
   _Stg_Component_Delete( self );
}

void _AbstractContext_Print( void* abstractContext, Stream* stream ) {
   AbstractContext* self = (AbstractContext*)abstractContext;
   
   /* General info */
   Journal_Printf( (void*) stream, "AbstractContext (ptr)(%p):\n", self );
   Journal_Printf( (void*) stream, "\tdictionary (ptr): %p\n", self->dictionary );
   
   /* Virtual info */
   Journal_Printf( (void*) stream, "\t_setDt (ptr): %p\n", self->_setDt );
   
   /* AbstractContext info */
   Journal_Printf( (void*) stream, "\tcommunicator: %i\n", self->communicator );
   Journal_Printf( (void*) stream, "\tstartTime: %g\n", self->startTime );
   Journal_Printf( (void*) stream, "\tstopTime: %g\n", self->stopTime );
   Journal_Printf( (void*) stream, "\tcurrentTime: %g\n", self->currentTime );
   Journal_Printf( (void*) stream, "\ttimeStep: %u\n", self->timeStep );
   Journal_Printf( (void*) stream, "\ttimeStepSinceJobRestart: %u\n", self->timeStepSinceJobRestart );
   Journal_Printf( (void*) stream, "\tmaxTimeSteps: %u\n", self->maxTimeSteps );
   Journal_Printf( (void*) stream, "\tfinalTimeStep: %u\n", self->finalTimeStep );
   Journal_Printf( (void*) stream, "\toutputEvery: %u\n", self->frequentOutputEvery );
   Journal_Printf( (void*) stream, "\tdumpEvery: %u\n", self->dumpEvery );
   Journal_Printf( (void*) stream, "\tcheckpointEvery: %u\n", self->checkpointEvery );
   Journal_Printf( (void*) stream, "\tsaveDataEvery: %u\n", self->saveDataEvery );
   Journal_Printf( (void*) stream, "\tcheckpointAtTimeInc: %g\n", self->checkpointAtTimeInc );

   if( self->outputPath ) 
      Journal_Printf( (void*) stream, "\toutputPath: %s\n", self->outputPath );
   else 
      Journal_Printf( (void*) stream, "\toutputPath: (null)\n" );

   if( self->checkpointReadPath ) 
      Journal_Printf( (void*) stream, "\tcheckpointReadPath: %s\n", self->checkpointReadPath );
   else 
      Journal_Printf( (void*) stream, "\tcheckpointReadPath: (null)\n" );

   if( self->checkpointWritePath ) 
      Journal_Printf( (void*) stream, "\tcheckpointWritePath: %s\n", self->checkpointWritePath );
   else 
      Journal_Printf( (void*) stream, "\tcheckpointWritePath: (null)\n" );

   Journal_Printf( stream, "\tloadFromCheckPoint: %u\n", self->loadFromCheckPoint );
   Journal_Printf( stream, "\tloadFieldsFromCheckpoint: %u\n", self->loadFieldsFromCheckpoint );
   Journal_Printf( stream, "\tloadSwarmsFromCheckpoint: %u\n", self->loadSwarmsFromCheckpoint );   
   Journal_Printf( stream, "\toutputSlimmedXML: %u\n", self->outputSlimmedXML );   
   Journal_Printf( stream, "\trestartTimestep: %u\n", self->restartTimestep );
   Journal_Printf( stream, "\tcheckPointPrefixString: %s\n", self->checkPointPrefixString );
   
   Stg_Class_Print( self->entryPoint_Register, stream );
   
   Journal_Printf( (void*) stream, "\tconstructK: %u\n", self->constructK );
   Journal_Printf( (void*) stream, "\tconstructExtensionsK: %u\n", self->constructExtensionsK );
   Journal_Printf( (void*) stream, "\tbuildK: %u\n", self->buildK );
   Journal_Printf( (void*) stream, "\tinitialiseK: %u\n", self->initialiseK );
   Journal_Printf( (void*) stream, "\texecuteK: %u\n", self->executeK );
   Journal_Printf( (void*) stream, "\tdestroyK: %u\n", self->destroyK );
   Journal_Printf( (void*) stream, "\tdestroyExtensionsK: %u\n", self->destroyExtensionsK );
   
   Journal_Printf( (void*) stream, "\tdt: %u\n", self->dtK );
   Journal_Printf( (void*) stream, "\tstepK: %u\n", self->stepK );
   Journal_Printf( (void*) stream, "\tpreSolveClassK: %u\n", self->preSolveClassK );
   Journal_Printf( (void*) stream, "\tsolveK: %u\n", self->solveK );
   Journal_Printf( (void*) stream, "\tsyncK: %u\n", self->syncK );
   
   /* Print parent */
   _Stg_Class_Print( self, stream );
}

/* Construct EP override stuff ****************************************************************************************************/
Func_Ptr _AbstractContext_Construct_EP_GetRun( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   
   switch( self->castType ) {
      case EntryPoint_2VoidPtr_CastType:
         return (void*) _AbstractContext_Construct_EP_Run;
      
      default:
         return _EntryPoint_GetRun( self );
   }
   return 0;
}

void _AbstractContext_Construct_EP_Run( void* entryPoint, void* data0, void* data1 ) {
   EntryPoint*      self = (EntryPoint*)entryPoint;
   Hook_Index       hookIndex;
   AbstractContext* context = (AbstractContext*)data0;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_2VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      ((EntryPoint_2VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( context, NULL );
      
      self = KeyHandle( context, context->constructK );
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

/* Component stuff ****************************************************************************************************************/
void _AbstractContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data ) {
   AbstractContext*        self = (AbstractContext*)context;
   Dictionary_Entry_Value* dictEntryVal = NULL;
   double                  startTime, stopTime;

   Journal_Printf( self->debug, "In: %s\n", __func__ );

   self->dictionary = Stg_Class_Copy( cf->rootDict, NULL, True, NULL, NULL );

   /*
    * The following just pauses at this point to allow time to attach a debugger.
    * Useful for mpi debugging.
    */
   sleep( Dictionary_Entry_Value_AsUnsignedInt(
                                               Dictionary_GetDefault( self->dictionary, "pauseToAttachDebugger", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) ) );
   
   /* Check if we have been provided a constant to multiply our calculated dt values by. */
   self->dtFactor = Dictionary_GetDouble_WithDefault( self->dictionary, (Dictionary_Entry_Key)"timestepFactor", 1.0 );

   /* Main input parameters. */
   self->frequentOutputEvery = Dictionary_Entry_Value_AsUnsignedInt( 
      Dictionary_GetDefault( self->dictionary, "outputEvery", Dictionary_Entry_Value_FromUnsignedInt( 1 ) ) );
   self->stepDump = Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault( self->dictionary, "stepDump", Dictionary_Entry_Value_FromBool( False ) ) );
   self->dumpEvery = Dictionary_Entry_Value_AsUnsignedInt( 
      Dictionary_GetDefault( self->dictionary, "dumpEvery", Dictionary_Entry_Value_FromUnsignedInt( 10 ) ) );
   self->checkpointEvery = Dictionary_Entry_Value_AsUnsignedInt( 
      Dictionary_GetDefault( self->dictionary, "checkpointEvery", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );
   self->saveDataEvery = Dictionary_Entry_Value_AsUnsignedInt( 
      Dictionary_GetDefault( self->dictionary, "saveDataEvery", Dictionary_Entry_Value_FromUnsignedInt( 0 ) ) );
   self->checkpointAtTimeInc = Dictionary_Entry_Value_AsDouble( 
      Dictionary_GetDefault( self->dictionary, "checkpointAtTimeInc", Dictionary_Entry_Value_FromDouble( 0 ) ) );
   self->nextCheckpointTime = self->checkpointAtTimeInc;
   self->experimentName = StG_Strdup( Dictionary_Entry_Value_AsString( 
      Dictionary_GetDefault( self->dictionary, "experimentName", Dictionary_Entry_Value_FromString( "experiment" ) ) ) );
   self->outputPath = StG_Strdup( Dictionary_Entry_Value_AsString( 
      Dictionary_GetDefault( self->dictionary, "outputPath", Dictionary_Entry_Value_FromString( "./" ) ) ) );
   
   if( Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"checkpointReadPath" ) ) {
      self->checkpointReadPath = StG_Strdup(
         Dictionary_Entry_Value_AsString( Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"checkpointReadPath" ) ) );
   }
   else 
      self->checkpointReadPath = StG_Strdup( self->outputPath );

   if( Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"checkpointWritePath" ) ) {
      self->checkpointWritePath = StG_Strdup(
         Dictionary_Entry_Value_AsString( Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"checkpointWritePath" ) ) );
   }
   else 
      self->checkpointWritePath = StG_Strdup( self->outputPath );

   self->checkpointAppendStep = Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault( self->dictionary, "checkpointAppendStep", Dictionary_Entry_Value_FromBool( False ) ) );
   self->interpolateRestart = Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault( self->dictionary, "interpolateRestart", Dictionary_Entry_Value_FromBool( False ) ) );

   if( self->rank == 0 ) {
      if( ! Stg_DirectoryExists( self->outputPath ) ) {
         Bool ret;

         if( Stg_FileExists( self->outputPath ) ) 
            Journal_Firewall( 0, self->info, "outputPath '%s' is a file an not a directory! Exiting...\n", self->outputPath );
         
         Journal_Printf( self->info, "outputPath '%s' does not exist, attempting to create...\n", self->outputPath );
         ret = Stg_CreateDirectory( self->outputPath );
         Journal_Firewall( ret, self->info, "Unable to create non-existing outputPath to '%s'\n", self->outputPath );
         /* else */
         Journal_Printf( self->info, "outputPath '%s' successfully created!\n", self->outputPath );
      }
      if( ! Stg_DirectoryExists( self->checkpointWritePath ) ) {
         Bool ret;

         if( Stg_FileExists( self->checkpointWritePath ) ) 
            Journal_Firewall( 0, self->info, "checkpointWritePath '%s' is a file an not a directory! Exiting...\n", self->checkpointWritePath );
         
         Journal_Printf( self->info, "checkpointWritePath '%s' does not exist, attempting to create...\n", self->checkpointWritePath );
         ret = Stg_CreateDirectory( self->checkpointWritePath );
         Journal_Firewall( ret, self->info, "Unable to create non-existing checkpointWritePath to '%s'\n", self->checkpointWritePath );
         /* else */
         Journal_Printf( self->info, "checkpointWritePath '%s' successfully created!\n", self->checkpointWritePath );
      }
   }

   /* 
    * Note: These try for deprecated keys "start", "end" and "stop" as well as new ones "startTime"
    * and "stopTime" - Main.PatrickSunter - 4 November 2004.
    */
   startTime = stopTime = 0;
   dictEntryVal = Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"start" );

   if( NULL == dictEntryVal ) {
      dictEntryVal = Dictionary_GetDefault( self->dictionary, "startTime",
         Dictionary_Entry_Value_FromDouble( startTime ) );
   }

   self->startTime = Dictionary_Entry_Value_AsDouble( dictEntryVal );
   dictEntryVal = Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"end" );

   if( NULL == dictEntryVal ) {
      dictEntryVal = Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"stop" );
      if( NULL == dictEntryVal ) {
         dictEntryVal = Dictionary_GetDefault( self->dictionary, "stopTime",
            Dictionary_Entry_Value_FromDouble( stopTime ) );
      }
   } 
   self->stopTime = Dictionary_Entry_Value_AsDouble( dictEntryVal );

   /* 
    * MaxTimeSteps of -2 means no maximum applied 
    * We default to -3 to signify that no maxTimeSteps was provided.
    */

   /* 
    * Note: These try for deprecated key "maxLoops" as well as new one "maxTimeSteps" -
    * Main.PatrickSunter - 4 November 2004.
    */
   dictEntryVal = Dictionary_Get( self->dictionary, (Dictionary_Entry_Key)"maxLoops" );

   if( NULL == dictEntryVal ) 
      dictEntryVal = Dictionary_GetDefault( self->dictionary, "maxTimeSteps", Dictionary_Entry_Value_FromInt( -3 ) );

   self->maxTimeSteps = Dictionary_Entry_Value_AsInt( dictEntryVal );
   self->finalTimeStep = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "finalTimeStep", 0 );
   self->gracefulQuit = False;

   if( True == Dictionary_GetBool_WithDefault( self->dictionary, (Dictionary_Entry_Key)"visualOnly", False ) ) 
      self->gracefulQuit = True; /* Quit after visualise */

   /* TODO: does this need to be read from checkpoint file??? */
   self->currentTime = self->startTime;
   
   /* Read in the checkpointing info. */
   self->restartTimestep = Dictionary_GetUnsignedInt_WithDefault( self->dictionary, "restartTimestep", -1 );
   self->checkPointPrefixString = StG_Strdup(Dictionary_GetString_WithDefault( self->dictionary, "checkPointPrefixString", "" ));
   self->outputSlimmedXML = Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault( self->dictionary, "outputSlimmedXML", Dictionary_Entry_Value_FromBool( True ) ) );
   self->vis = Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault( self->dictionary, "vis", Dictionary_Entry_Value_FromBool( True ) ) );

   if( self->restartTimestep >= 0 ) {
      self->loadFromCheckPoint = True;
      self->loadFieldsFromCheckpoint = Dictionary_Entry_Value_AsBool( 
         Dictionary_GetDefault( self->dictionary, "loadFieldsFromCheckpoint", Dictionary_Entry_Value_FromBool( True ) ) );
      self->loadSwarmsFromCheckpoint = Dictionary_Entry_Value_AsBool( 
         Dictionary_GetDefault( self->dictionary, "loadSwarmsFromCheckpoint", Dictionary_Entry_Value_FromBool( True ) ) );
      self->timeStep = self->restartTimestep;
   }
   else {
      self->loadFromCheckPoint = False;
      self->loadFieldsFromCheckpoint = False;
      self->loadSwarmsFromCheckpoint = False;
   }

   /* 
    * This defines all the entryPoints, eg, self->constructK, etc.
    * So it must go before we start KeyCall.
    */
   _AbstractContext_Init( self );

   /* Construct entry point. */
   KeyCall( self, self->constructK, EntryPoint_2VoidPtr_CallCast* )( KeyHandle( self, self->constructK ), self, self );

   /* Load the plugins desired by this context (dictionary). */
   ModulesManager_Load( self->plugins, self->dictionary, self->name );

   self->CF = cf;

   /* Extensions are the last thing we want to do. */
   KeyCall( self, self->constructExtensionsK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->constructExtensionsK), self );
   
   if( True == Dictionary_GetBool_WithDefault( self->dictionary, (Dictionary_Entry_Key)"showJournalStatus", False ) ) 
      Journal_PrintConcise( );   
}

void _AbstractContext_Build( void* context, void* data ) {
   AbstractContext* self = (AbstractContext*)context;
   Bool             isBuilt;
   
   Journal_Printf( self->debug, "In: %s\n", __func__ );

   #ifdef DEBUG
      Context_WarnIfNoHooks( self, self->buildK, __func__  );
   #endif
   
   /* 
    * Pre-mark the phase as complete as a default hook will attempt to build all live components
    * (including this again).
    */
   isBuilt = self->isBuilt;
   self->isBuilt = True;

   /* 
    * Construct the list of plugins. do this in the build phase se that we know that any
    * components required by the plugins have already been constructed.
    */
   if( self->plugins->codelets->count )
      ModulesManager_ConstructModules( self->plugins, self->CF, data );

   KeyCall( self, self->buildK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->buildK), self );
   self->isBuilt = isBuilt;
}

void _AbstractContext_Initialise( void* context, void* data ) {
   AbstractContext* self = (AbstractContext*)context;
   Bool             isInitialised;
   
   Journal_Printf( self->debug, "In: %s\n", __func__ );

   #ifdef DEBUG
      AbstractContext_WarnIfNoHooks( self, self->initialiseK, __func__ );
   #endif

   /* 
    * Pre-mark the phase as complete as a default hook will attempt to initialise all live components
    * (including this again).
    */
   isInitialised = self->isInitialised;
   self->isInitialised = True;
   KeyCall( self, self->initialiseK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->initialiseK), self );
   self->isInitialised = isInitialised;

   /* lets go ahead and setup these bits/pieces now */
   /* Print the EP and hooks for the run. */
   if( self->rank == 0 ) { 
      Stream* stream=Journal_Register( InfoStream_Type, (Name)"EP info"  );
      Bool fileOpened = False;
      if( self->loadFromCheckPoint == False ) {
         /* Always overwrite the file if starting a new run. */
         fileOpened = Stream_RedirectFile_WithPrependedPath( stream, self->outputPath, "EP.info" );
      }
      else {
         /* Just append to the file if doing a restart from checkpoint. */
         fileOpened = Stream_AppendFile_WithPrependedPath( stream, self->outputPath, "EP.info" );
      }
      Journal_Firewall(
         fileOpened, Journal_Register( Error_Type, (Name)CURR_MODULE_NAME  ), 
         "Could not open file %s/%s. Possibly directory %s does not exist or is not writable.\n"
         "Check 'outputPath' in input file.\n", self->outputPath, "EP.info", self->outputPath );

      Context_PrintConcise( self, stream );
   }

   if( self->maxTimeSteps == -3 ) { /* if no maxTimeSteps was provided, */
      if( self->finalTimeStep )
         self->maxTimeSteps = -2;   /* set the maxTimeSteps to -2 (ie, infinite) if finalTimeStep was provided, */
      else if( self->stopTime )
         self->maxTimeSteps = -2;   /* or if a stopTime was provided. */
      else self->maxTimeSteps = 0;  /* else set maxTimeSteps to zero, so that we only solve given the initial condition */
   }
   if( self->maxTimeSteps == -1 ) 
      Journal_RPrintf( self->info, "Initialise and quit\n" );
   if( self->maxTimeSteps == 0 ) 
      Journal_RPrintf( self->info, "Initialise, execute, and quit\n" );
   if( self->maxTimeSteps == 1 ) 
      Journal_RPrintf( self->info, "Run until simulation has stepped forward %u timeStep\n", self->maxTimeSteps );
   if( self->maxTimeSteps > 1 ) 
      Journal_RPrintf( self->info, "Run until simulation has stepped forward %u timeSteps\n", self->maxTimeSteps );
   if( self->finalTimeStep ) {
      if( self->maxTimeSteps >= 0 ) 
         Journal_RPrintf( self->info, "or " );
      else 
         Journal_RPrintf( self->info, "Run " );
      Journal_Printf( self->info, "until absolute time step %u reached\n", self->stopTime );
   }
   
   if( self->stopTime ) {
      if((self->maxTimeSteps >= 0) || self->finalTimeStep ) 
         Journal_RPrintf( self->info, "or " );
      else 
         Journal_RPrintf( self->info, "Run " );
      Journal_RPrintf( self->info, "until simulation time passes %g.\n", self->stopTime );
   }

   /* 
    * Set timeStep to 0 if not restarting, so that incrementing timestep below affects both
    * regular and restart mode -- PatrickSunter - 18 June 2006.
    */
   self->timeStepSinceJobRestart = 0;
   self->needUpdate = False;
   if( False == self->loadFromCheckPoint ) {
      self->timeStep = 0;
      self->currentTime = self->startTime;
      self->_setDt( self, 0 );

      if( self->maxTimeSteps == -1 ) {
         self->gracefulQuit = True;
      }
   } else {
      double dtLoadedFromFile = 0;
      /* 
       * Note: when checkpointing time info, we called AbstractContext_Dt( self )
       * at the end of the step we were restarting from, which should be equivalent to the
       * call here - and that calculation may be dependent on the solver info for that step,
       * so we need to reload it here.
       * Note also that in effect, we have already complete the restart timestep.  
       * so we load it, then update everything in preparation for the next solve etc.
       */ 
      _AbstractContext_LoadTimeInfoFromCheckPoint( (void*)self, self->restartTimestep, &dtLoadedFromFile );
      self->_setDt( self, dtLoadedFromFile );

      self->nextCheckpointTime += self->currentTime;

      if( self->maxTimeSteps == -1 ){
         self->gracefulQuit = True;
      } else if ( self->maxTimeSteps == 0 ){
      	 self->needUpdate = False;
      } else {
         /* ok, we have the solve already for the reloaded timestep, so lets update to prepare for next solve */
         self->needUpdate = True;
      }
   }


}

void _AbstractContext_Execute( void* context, void* data ) {
   AbstractContext* self = (AbstractContext*)context;
   Bool             hasExecuted;
   
   Journal_Printf( self->debug, "In: %s\n", __func__ );

   #ifdef DEBUG
      AbstractContext_WarnIfNoHooks( self, self->executeK, __func__ );
   #endif
   
   /* 
    * Pre-mark the phase as complete as a default hook will attempt to initialise all live components
    * (including this again).
    */
   hasExecuted = self->hasExecuted;
   self->hasExecuted = True;
   KeyCall( self, self->executeK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->executeK), self );
   self->hasExecuted = hasExecuted;
}

void _AbstractContext_Destroy( void* context, void* data ) {
   AbstractContext* self = (AbstractContext*)context;

   Journal_Printf( self->debug, "In: %s\n", __func__ );

   /* 
    * Pre-mark the phase as complete as a default hook will attempt to initialise all live components
    * (including this again).
    */
   PluginsManager_RemoveAllFromComponentRegister( self->plugins ); 

   KeyCall( self, self->destroyExtensionsK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->destroyExtensionsK), self );
   KeyCall( self, self->destroyK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->destroyK), self );

}

/* Context public stuff ***********************************************************************************************************/
void AbstractContext_PrintConcise( void* abstractContext, Stream* stream ) {
   AbstractContext* self = (AbstractContext*)abstractContext;
   EntryPoint_Index entryPointIndex;

   Journal_Printf( stream, "A listing of all entrypoints (EP) and associated hooks (H) on the " );
   Journal_Printf( stream, "Context: %s\n", self->type );

   for( entryPointIndex = 0; entryPointIndex < self->entryPoint_Register->count; entryPointIndex++ ) 
      EntryPoint_PrintConcise( EntryPoint_Register_At( self->entryPoint_Register, entryPointIndex ), stream );
}

EntryPoint_Index AbstractContext_AddEntryPoint( void* abstractContext, void* entryPoint ) {
   AbstractContext* self = (AbstractContext*)abstractContext;

   return EntryPoint_Register_Add( self->entryPoint_Register, entryPoint );
}

EntryPoint* AbstractContext_GetEntryPoint( void* abstractContext, const Name entryPointName ) {
   AbstractContext* self = (AbstractContext*)abstractContext;
   EntryPoint_Index ep_I;

   /* Find the entry point. */
   ep_I = EntryPoint_Register_GetHandle( self->entryPoint_Register, entryPointName );
   if( ep_I == (unsigned)-1 ) 
      return 0;
   else 
      return EntryPoint_Register_At( self->entryPoint_Register, ep_I );
}

Func_Ptr _AbstractContext_Call( void* abstractContext, Name entryPointName, void** epPtr ) {
   AbstractContext* self = (AbstractContext*)abstractContext;
   EntryPoint_Index ep_I;
   
   /* Find the entry point. */
   ep_I = EntryPoint_Register_GetHandle( self->entryPoint_Register, entryPointName );
   if( ep_I == (unsigned)-1 ) 
      *epPtr = 0;
   else 
      *epPtr = EntryPoint_Register_At( self->entryPoint_Register, ep_I );
   
   /* Run the entry point. */
   if( *epPtr != 0 ) 
      return ((EntryPoint*) (*epPtr))->run;
   return 0;
}

double AbstractContext_Dt( void* context ) {
   AbstractContext* self = (AbstractContext*)context;

   AbstractContext_ErrorIfNoHooks( self, self->dtK, __func__ );
   return KeyCall( self, self->dtK, ContextEntryPoint_Dt_CallCast* )( KeyHandle(self,self->dtK), self );
}

void AbstractContext_Step( void* context, double dt ) {
   AbstractContext* self = (AbstractContext*)context;
   #if DEBUG
      AbstractContext_WarnIfNoHooks( self, self->stepK, __func__ );
   #endif
   KeyCall( self, self->stepK, ContextEntryPoint_Step_CallCast* )( KeyHandle(self,self->stepK), self, dt );
}

void AbstractContext_WarnIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller ) {
   AbstractContext* self = (AbstractContext*)context;
   EntryPoint_WarnIfNoHooks( EntryPoint_Register_At( self->entryPoint_Register, epIndex ), caller );
}

void AbstractContext_ErrorIfNoHooks( void* context, EntryPoint_Index epIndex, const char* caller ) {
   AbstractContext* self = (AbstractContext*)context;
   EntryPoint_ErrorIfNoHooks( EntryPoint_Register_At( self->entryPoint_Register, epIndex ), caller );
}

void AbstractContext_FrequentOutput( void* context ) {
   AbstractContext* self = (AbstractContext*)context;

   KeyCall( self, self->frequentOutputK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->frequentOutputK), self );
}

void AbstractContext_Dump( void* context ) {
   AbstractContext* self = (AbstractContext*)context;

   KeyCall( self, self->dumpK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->dumpK), self );
   KeyCall( self, self->dumpClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->dumpClassK), self );
}

void AbstractContext_Save( void* context ) {
   AbstractContext* self = (AbstractContext*)context;

   KeyCall( self, self->saveK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->saveK), self );
   KeyCall( self, self->saveClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->saveClassK), self );
}

void AbstractContext_DataSave( void* context ) {
   AbstractContext* self = (AbstractContext*)context;

   KeyCall( self, self->dataSaveK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->dataSaveK), self );
   KeyCall( self, self->dataSaveClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->dataSaveClassK), self );
}

/* Context hooks ******************************************************************************************************************/
void _AbstractContext_Construct_Hook( void* _context, void* data ) {
}

void _AbstractContext_Execute_Hook( void* _context ) {
   AbstractContext* self = (AbstractContext*)_context;

   /* if set to gracefulQuit, first check any outputting requirements */
   if( self->gracefulQuit )
      _AbstractContext_AllOutput( _context );

   while( !self->gracefulQuit ) {
      if( self->needUpdate ) AbstractContext_Update(self);
      AbstractContext_Step( self, self->dt );

      if( ( self->maxTimeSteps >= -1 ) && self->timeStepSinceJobRestart == self->maxTimeSteps )
         break;

      if( self->finalTimeStep && ( self->timeStep >= self->finalTimeStep ) )
         break;

      if( self->stopTime && ( self->currentTime >= self->stopTime ) )
         break; 

      #ifdef ENABLE_STGERMAIN_LOG
      #if ENABLE_STGERMAIN_LOG > 1
      // log every processor timing
      stg_log_printf( "========================= Done step %d =========================\n\n", self->timeStep );      
      #else
      // log every processor timing
      if( self->rank == 0 )
         stg_log_printf( "========================= Done step %d =========================\n\n", self->timeStep );      
      #endif
      #endif

   }
}

void AbstractContext_Update(void* _context){
   AbstractContext* self = (AbstractContext*)_context;
   /* Now we update everything for next timesteps configuration. */
   self->timeStep++;
   self->timeStepSinceJobRestart++;
   self->currentTime += AbstractContext_Dt( self );
   KeyCall( self, self->updateClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->updateClassK), self );
   KeyCall( self, self->syncK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->syncK), self );

}

void _AbstractContext_Step( void* _context, double dt ) {
   AbstractContext* self = (AbstractContext*)_context;
   
   /* 
    * This will make it clear where the timestep starts when several procs
    * running. Figure this 1 synchronisation is ok since we are likely to
    * have just synchronised while calculating timestep anyway. 
    */
   MPI_Barrier( self->communicator );
   Journal_DPrintf( self->debug, "In: %s\n", __func__ );
   if( self->maxTimeSteps != 0)
      Journal_RPrintf( self->info, "TimeStep = %d, Start time = %.6g, prev timeStep dt = %.6g\n", self->timeStep, self->currentTime, dt );

   if( self->loadFromCheckPoint ) 
      Journal_RPrintf( self->info, "TimeStep since job restart = %d\n", self->timeStepSinceJobRestart );

   #ifdef DEBUG
      Context_WarnIfNoHooks( self, self->solveK, __func__ );   
   #endif

   KeyCall( self, self->preSolveClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->preSolveClassK), self );
   KeyCall( self, self->solveK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->solveK), self );
   KeyCall( self, self->postSolveK, EntryPoint_VoidPtr_CallCast* )( KeyHandle(self,self->postSolveK), self );
   KeyCall( self, self->postSolveClassK, EntryPoint_Class_VoidPtr_CallCast* )( KeyHandle(self,self->postSolveClassK), self );

   /* solve done, lets set the update flag in anticipation of next solve */
   self->needUpdate = True;

   _AbstractContext_AllOutput( _context );

}

void _AbstractContext_AllOutput( void* _context ){
   AbstractContext* self = (AbstractContext*)_context;
   if( self->frequentOutputEvery ) {
      if( self->timeStep % self->frequentOutputEvery == 0 )
         AbstractContext_FrequentOutput( self );
   }
   
   if( self->dumpEvery && !self->stepDump ) {
      if( self->timeStep % self->dumpEvery == 0 )
         AbstractContext_Dump( self );
   }
   
   if( self->checkpointEvery ) {
      if( self->timeStep % self->checkpointEvery == 0 )
         AbstractContext_Save( self );
   }   

   if( self->saveDataEvery ) {
      if( self->timeStep % self->saveDataEvery == 0 )
         AbstractContext_DataSave( self );
   }   

   if( self->checkpointAtTimeInc ) {
      if( self->currentTime >= self->nextCheckpointTime){
         AbstractContext_Save( self );
         self->nextCheckpointTime += self->checkpointAtTimeInc; 
      }
   }   
}

void _AbstractContext_LoadTimeInfoFromCheckPoint( void* _context, Index timeStep, double* dtLoadedFromFile ) {
   AbstractContext* self = (AbstractContext*)_context;
   char*            timeInfoFileName = NULL;
   char*            timeInfoFileNamePart = NULL;
   FILE*            timeInfoFile;      
   Stream*          errorStr = Journal_Register( Error_Type, (Name)self->type  );
#ifdef READ_HDF5
   hid_t            file, fileSpace, fileData;
#endif

   timeInfoFileNamePart = Context_GetCheckPointReadPrefixString( self );
#ifdef WRITE_HDF5
   timeInfoFile = NULL;
   Stg_asprintf( &timeInfoFileName, "%stimeInfo.%.5u.h5", timeInfoFileNamePart, self->restartTimestep );
    
   /* Open the file and data set. */
   file = H5Fopen( timeInfoFileName, H5F_ACC_RDONLY, H5P_DEFAULT );
   Journal_Firewall( file >= 0, 
      errorStr, "\n\nError- in %s(), Couldn't find checkpoint time info file with "
      "filename \"%s\" - aborting.\n", __func__, timeInfoFileName );
            
   /* Read currentTime from file. */
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
   fileData = H5Dopen( file, "/currentTime" );
   #else
   fileData = H5Dopen( file, "/currentTime", H5P_DEFAULT );
   #endif
   fileSpace = H5Dget_space( fileData );
      
   H5Dread( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &self->currentTime );
      
   H5Sclose( fileSpace );
   H5Dclose( fileData );
   
   /* Read Dt from file. */
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
   fileData = H5Dopen( file, "/Dt" );
   #else
   fileData = H5Dopen( file, "/Dt", H5P_DEFAULT );
   #endif
   fileSpace = H5Dget_space( fileData );
      
   H5Dread( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, dtLoadedFromFile );
      
   H5Sclose( fileSpace );
   H5Dclose( fileData );
   H5Fclose( file );
#else   
   Stg_asprintf( &timeInfoFileName, "%stimeInfo.%.5u.dat", timeInfoFileNamePart, self->restartTimestep );
    
   timeInfoFile = fopen( timeInfoFileName, "r" );
   Journal_Firewall( NULL != timeInfoFile, errorStr, "Error- in %s(), Couldn't find checkpoint time info file with "
      "filename \"%s\" (HD5 not enabled) - aborting.\n", __func__, timeInfoFileName );

   /* Set currentTime and Dt loaded from file. */
   fscanf( timeInfoFile, "%lg", &self->currentTime );
   fscanf( timeInfoFile, "%lg", dtLoadedFromFile );
   fclose( timeInfoFile );
#endif
   
   Memory_Free( timeInfoFileName );
   Memory_Free( timeInfoFileNamePart );
}

void _AbstractContext_SaveTimeInfo( void* _context ) {
   AbstractContext* self = (AbstractContext*)_context;   
   FILE*            timeInfoFile = NULL;
   char*            timeInfoFileName = NULL;
   char*            timeInfoFileNamePart = NULL;
   Stream*          errorStr = Journal_Register( Error_Type, (Name)self->type  );
#ifdef WRITE_HDF5
   hid_t            file, fileSpace, fileData, props;
   hsize_t          count;
   double           Dt;
#endif 

   /* Only the master process needs to write this file. */
   if( self->rank == 0 ) {
      timeInfoFileNamePart = Context_GetCheckPointWritePrefixString( self );
#ifdef WRITE_HDF5
      timeInfoFile = NULL;
      Stg_asprintf( &timeInfoFileName, "%stimeInfo.%.5u.h5", timeInfoFileNamePart, self->timeStep );
   
      /* Create parallel file property list. */
      props = H5Pcreate( H5P_FILE_ACCESS );
   
      /* Open the HDF5 output file. */
      file = H5Fcreate( timeInfoFileName, H5F_ACC_TRUNC, H5P_DEFAULT, props );
      Journal_Firewall( 
         file >= 0, errorStr, "Error in %s for %s '%s' - Cannot create file %s.\n", 
         __func__, self->type, self->name, timeInfoFileName );
      
      H5Pclose( props );

      /* Dump currentTime. */
      count = 1;
      fileSpace = H5Screate_simple( 1, &count, NULL );         
      #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/currentTime", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate( file, "/currentTime", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif
         
      props = H5Pcreate( H5P_DATASET_XFER );
      H5Dwrite( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, props, &(self->currentTime) );
      H5Pclose( props );
      H5Dclose( fileData );
      H5Sclose( fileSpace );
   
      /* Dump Dt. */
      fileSpace = H5Screate_simple( 1, &count, NULL );         
      #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/Dt", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate( file, "/Dt", H5T_NATIVE_DOUBLE, fileSpace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif
         
      props = H5Pcreate( H5P_DATASET_XFER );
      Dt = AbstractContext_Dt( self );
      H5Dwrite( fileData, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, props, &Dt );
      H5Pclose( props );
      H5Dclose( fileData );
      H5Sclose( fileSpace );
   
      /* Dump nproc. */
      fileSpace = H5Screate_simple( 1, &count, NULL );         
      #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dcreate_vers == 1
      fileData = H5Dcreate( file, "/nproc", H5T_NATIVE_INT, fileSpace, H5P_DEFAULT );
      #else
      fileData = H5Dcreate( file, "/nproc", H5T_NATIVE_INT, fileSpace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
      #endif
         
      props = H5Pcreate( H5P_DATASET_XFER );
      H5Dwrite( fileData, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, props, &(self->nproc) );
      H5Pclose( props );
      H5Dclose( fileData );
      H5Sclose( fileSpace );
      H5Fclose( file );
#else   
      Stg_asprintf( &timeInfoFileName, "%stimeInfo.%.5u.dat", timeInfoFileNamePart, self->timeStep );
    
      timeInfoFile = fopen( timeInfoFileName, "w" );

      if( False == timeInfoFile ) {
         Journal_Printf( errorStr, "Error- in %s(), Couldn't create checkpoint time info file with "
         "filename \"%s\" - aborting.\n", __func__, timeInfoFileName );
         exit(EXIT_FAILURE);
      }

      /* set currentTime and Dt loaded from file */
      fprintf( timeInfoFile, "%lg ", self->currentTime );
      fprintf( timeInfoFile, "%lg\n", AbstractContext_Dt( self ) );
      fclose( timeInfoFile );
#endif
   
      Memory_Free( timeInfoFileName );
      Memory_Free( timeInfoFileNamePart );
   } 
}

Bool AbstractContext_CheckPointExists( void* context, Index timeStep ) {
   AbstractContext* self = (AbstractContext*)context;
   char*            timeInfoFileName = NULL;
   char*            timeInfoFileNamePart = NULL;   
   struct stat      statInfo;
   int              statResult;

   timeInfoFileNamePart = Context_GetCheckPointWritePrefixString( self );
#ifdef WRITE_HDF5
   Stg_asprintf( &timeInfoFileName, "%stimeInfo.%.5u.h5", timeInfoFileNamePart, timeStep );
#else   
   Stg_asprintf( &timeInfoFileName, "%stimeInfo.%.5u.dat", timeInfoFileNamePart, timeStep );
#endif   
   statResult = stat( timeInfoFileName, &statInfo );

   Memory_Free( timeInfoFileName );
   Memory_Free( timeInfoFileNamePart );

   if( 0 == statResult ) 
      return True;
   else 
      return False;
}

char* Context_GetCheckPointReadPrefixString( void* context ) {
   AbstractContext* self = (AbstractContext*)context;
   char*            readPathString = NULL;

   if( self->checkpointAppendStep ) {
      if( strlen(self->checkPointPrefixString) > 0 ) 
         Stg_asprintf( &readPathString, "%s/data.%.5u/%s.", self->checkpointReadPath, self->restartTimestep, self->checkPointPrefixString );
      else 
         Stg_asprintf( &readPathString, "%s/data.%.5u/", self->checkpointReadPath, self->restartTimestep );
   }
   else {
      if( strlen(self->checkPointPrefixString) > 0 ) 
         Stg_asprintf( &readPathString, "%s/%s.", self->checkpointReadPath, self->checkPointPrefixString );
      else 
         Stg_asprintf( &readPathString, "%s/", self->checkpointReadPath );
   }
   return readPathString;
}

char* Context_GetCheckPointWritePrefixString( void* context ) {
   AbstractContext* self = (AbstractContext*)context;
   char*            writePathString = NULL;

   if( self->checkpointAppendStep ) {
      if( strlen(self->checkPointPrefixString) > 0 ) 
         Stg_asprintf( &writePathString, "%s/data.%.5u/%s.", self->checkpointWritePath, self->timeStep, self->checkPointPrefixString );
      else 
         Stg_asprintf( &writePathString, "%s/data.%.5u/", self->checkpointWritePath, self->timeStep );
   }
   else {
      if( strlen(self->checkPointPrefixString) > 0 ) 
         Stg_asprintf( &writePathString, "%s/%s.", self->checkpointWritePath, self->checkPointPrefixString );
      else 
         Stg_asprintf( &writePathString, "%s/", self->checkpointWritePath );
   }
   return writePathString;
}

char* Context_GetCheckPointWritePrefixStringTopLevel( void* context ) {
   AbstractContext* self = (AbstractContext*)context;
   char*            writePathString = NULL;

   if( strlen(self->checkPointPrefixString) > 0 ) 
      Stg_asprintf( &writePathString, "%s/%s.", self->checkpointWritePath, self->checkPointPrefixString );
   else 
      Stg_asprintf( &writePathString, "%s/", self->checkpointWritePath );

   return writePathString;
}

void _AbstractContext_CreateCheckpointDirectory( void* _context ) {
   AbstractContext* self = (AbstractContext*)_context;

   /*
    * If we are creating individual directories for each checkpoint timestep,
    * first create the directory if it doesn't exist. 
    */
   if( self->checkpointAppendStep ) {
      /* Only the master process creates the directory */      
      if( self->rank == 0 ) {
         char* writePathString = NULL;
         Stg_asprintf( &writePathString, "%s/data.%.5u/", self->checkpointWritePath, self->timeStep );
         if( ! Stg_DirectoryExists( writePathString ) ) {
            Bool ret;
            if( Stg_FileExists( writePathString ) )
               Journal_Firewall( 0, self->info, 
                  "checkpoint outputPath '%s' is a file an not a directory! Exiting...\n", self->outputPath );
            ret = Stg_CreateDirectory( writePathString );
            Journal_Firewall( ret, self->info,
               "Unable to create non-existing outputPath to '%s'\n", self->outputPath );
         }
         /* Other processes may proceed now that required directory has been created. */
         MPI_Barrier( self->communicator );
         Memory_Free( writePathString );
         writePathString = NULL;
      }
      else
         /* Barrier to stop other processes continuing until required directory has been created. */
         MPI_Barrier( self->communicator );
   }
}
