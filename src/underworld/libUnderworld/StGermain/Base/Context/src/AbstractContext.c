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
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"
#include "StGermain/Base/Extensibility/src/Extensibility.h"

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
   Memory_Free( self->timeStamp );

   Stg_Class_Delete( self->variable_Register );

   /* Unload all dynamic plugins. */
   PluginsManager_UnloadAll( self->plugins );
   
   /* Stg_Class_Delete parent. */
   _Stg_Component_Delete( self );
}

void _AbstractContext_Print( void* abstractContext, Stream* stream ) {}

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
   self->experimentName = StG_Strdup( Dictionary_Entry_Value_AsString(
      Dictionary_GetDefault( self->dictionary, "experimentName", Dictionary_Entry_Value_FromString( "experiment" ) ) ) );
   self->outputPath = StG_Strdup( Dictionary_Entry_Value_AsString( 
      Dictionary_GetDefault( self->dictionary, "outputPath", Dictionary_Entry_Value_FromString( "./" ) ) ) );
   
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
   self->gracefulQuit = False;

   if( True == Dictionary_GetBool_WithDefault( self->dictionary, (Dictionary_Entry_Key)"visualOnly", False ) ) 
      self->gracefulQuit = True; /* Quit after visualise */

   /* TODO: does this need to be read from checkpoint file??? */
   self->currentTime = self->startTime;
   
   self->outputSlimmedXML = Dictionary_Entry_Value_AsBool(
      Dictionary_GetDefault( self->dictionary, "outputSlimmedXML", Dictionary_Entry_Value_FromBool( True ) ) );
   self->vis = Dictionary_Entry_Value_AsBool( 
      Dictionary_GetDefault( self->dictionary, "vis", Dictionary_Entry_Value_FromBool( True ) ) );

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

void _AbstractContext_Initialise( void* context, void* data ) {}

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

/* Context hooks ******************************************************************************************************************/
void _AbstractContext_Construct_Hook( void* _context, void* data ) {
}

void _AbstractContext_Execute_Hook( void* _context ) {}

void AbstractContext_Update(void* _context){}

void _AbstractContext_Step( void* _context, double dt ) {}
