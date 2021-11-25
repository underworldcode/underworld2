/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"

#include "types.h"
#include "shortcuts.h"
#include "EntryPoint.h"
#include "Hook.h"
#include "ClassHook.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <math.h>
#ifndef HUGE_VAL
   #define HUGE_VAL 1.0e99
#endif


/* Textual name of this class */
const Type EntryPoint_Type = "EntryPoint";

static const Type _EntryPoint_Run_Type = "EntryPoint->Run";

/* VIRTUAL FUNCTIONS */

/** allocate and initialise a new EntryPoint.
 * \param name textual name of the entry point (useful if its to be stored in a list).
 * \return the allocated entry point. */
EntryPoint* EntryPoint_New( const Name name, unsigned int castType ) {
   /* Variables set in this function */
   SizeT                      _sizeOfSelf = sizeof(EntryPoint);
   Type                              type = EntryPoint_Type;
   Stg_Class_DeleteFunction*      _delete = _EntryPoint_Delete;
   Stg_Class_PrintFunction*        _print = _EntryPoint_Print;
   Stg_Class_CopyFunction*          _copy = _EntryPoint_Copy;
   EntryPoint_GetRunFunction*     _getRun = _EntryPoint_GetRun;

   /* 
    * Variables that are set to ZERO are variables that will be set either by the 
    * current _New function or another parent _New function further up the hierachy.
    */

   /* default value NON_GLOBAL */
   AllocationType  nameAllocationType = NON_GLOBAL;

   return _EntryPoint_New( ENTRYPOINT_PASSARGS );
}

/** Initialise an existing entry point. See EntryPoint_New() for argument descriptions. */
void EntryPoint_Init( void* entryPoint, const Name name, unsigned int castType ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   /* General info */
   self->type = EntryPoint_Type;
   self->_sizeOfSelf = sizeof(EntryPoint);
   self->_deleteSelf = False;
   
   /* Virtual info */
   self->_delete = _EntryPoint_Delete;
   self->_print = _EntryPoint_Print;
   self->_copy = _EntryPoint_Copy;
   self->_getRun = _EntryPoint_GetRun;
   _Stg_Class_Init( (Stg_Class*)self );
   _Stg_Object_Init( (Stg_Object*)self, (Name) name, GLOBAL );
   
   /* EntryPoint info */
   _EntryPoint_Init( self, castType );
}

EntryPoint* _EntryPoint_New( ENTRYPOINT_DEFARGS ) {
   EntryPoint* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(EntryPoint) );
   /* 
    * The following terms are parameters that have been passed into this function
    * but are being set before being passed onto the parent.  
    * This means that any values of these parameters that are passed into this function
    * are not passed onto the parent function and so should be set to ZERO in any children of this class. 
    */
   nameAllocationType = GLOBAL;

   self = (EntryPoint*)_Stg_Object_New(  STG_OBJECT_PASSARGS  );
   
   /* General info */
   
   /* Virtual info */
   self->_getRun = _getRun;
   
   /* EntryPoint info */
   _EntryPoint_Init( self, castType );
   
   return self;
}

void _EntryPoint_Init( EntryPoint* self, unsigned int castType ){
   /* General and Virtual info should already be set */
   
   /* EntryPoint info */
   self->castType = castType;
   self->run = self->_getRun( self );
   
   /* Initialise the hooklist */
   self->hooks = Stg_ObjectList_New(); 
   self->alwaysFirstHook = NULL;
   self->alwaysLastHook = NULL;
}

void _EntryPoint_Delete( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   #if DEBUG
      assert( self );
   #endif   
   
   Stg_ObjectList_DeleteAllObjects( self->hooks );
   Stg_Class_Delete( self->hooks );
   
   /* Stg_Class_Delete parent */
   _Stg_Object_Delete( self );
}

void _EntryPoint_Print( void* entryPoint, Stream* stream ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   #if DEBUG
      assert( self );
      assert( stream );
   #endif   
   
   /* General info */
   Journal_Printf( (void*) stream, "EntryPoint (ptr) (%p):\n", self );
   
   /* Virtual info */
   Journal_Printf( (void*) stream, "\t_getRun (ptr): %p\n", self->_getRun );
   
   /* EntryPoint info */
   Journal_Printf( (void*) stream, "\tname: %s\n", self->name );
   Journal_Printf( (void*) stream, "\tcastType: %u\n", self->castType );
   Journal_Printf( (void*) stream, "\trun (ptr): %p\n", self->run );
   Journal_Printf( (void*) stream, "\thooks: \n" );
   Stg_Class_Print( self->hooks, stream );
   Journal_Printf( (void*) stream, "\n" );
   
   /* Print parent */
   _Stg_Class_Print( self, stream );
}

void* _EntryPoint_Copy( void* entryPoint, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   EntryPoint* newEntryPoint;
   PtrMap*     map = ptrMap;
   Bool        ownMap = False;
   
   if( !map ) {
      map = PtrMap_New( 10 );
      ownMap = True;
   }
   
   newEntryPoint = (EntryPoint*)_Stg_Object_Copy( self, dest, deep, nameExt, map );
   
   /* Virtual methods */
   newEntryPoint->_getRun = self->_getRun;
   
   newEntryPoint->castType = self->castType;
   newEntryPoint->run = self->run;
   
   if( deep ) {
      newEntryPoint->hooks = (HookList*)Stg_Class_Copy( self->hooks, NULL, deep, nameExt, map );
      
      if( newEntryPoint->hooks ) {
         if( self->alwaysFirstHook ) {
            if( nameExt ) {
               Name tmpName;
               
               tmpName = Memory_Alloc_Array(
                  char,
                  strlen( Stg_Object_GetName( self->alwaysFirstHook ) ) + strlen( nameExt ) + 1,
                  "tmpName" );

               strcpy( tmpName, Stg_Object_GetName( self->alwaysFirstHook ) );
               strcpy( &tmpName[strlen( Stg_Object_GetName( self->alwaysFirstHook ) )], nameExt );
               
               newEntryPoint->alwaysFirstHook = (Hook*)Stg_ObjectList_Get( newEntryPoint->hooks, tmpName );
               Memory_Free( tmpName );
            }
            else {
               newEntryPoint->alwaysFirstHook = (Hook*)Stg_ObjectList_Get(
                  newEntryPoint->hooks,
                  Stg_Object_GetName( self->alwaysFirstHook ) );
            }
         }
         else {
            self->alwaysFirstHook = NULL;
         }
         
         if( self->alwaysLastHook ) {
            if( nameExt ) {
               Name tmpName;
               
               tmpName = Memory_Alloc_Array(
                  char,
                  strlen( Stg_Object_GetName( self->alwaysLastHook ) ) + strlen( nameExt ) + 1,
                  "tmpName" );

               strcpy( tmpName, Stg_Object_GetName( self->alwaysLastHook ) );
               strcpy( &tmpName[strlen( Stg_Object_GetName( self->alwaysLastHook ) )], nameExt );
               
               newEntryPoint->alwaysLastHook = (Hook*)Stg_ObjectList_Get( newEntryPoint->hooks, tmpName );
               Memory_Free( tmpName );
            }
            else {
               newEntryPoint->alwaysLastHook = (Hook*)Stg_ObjectList_Get(
                  newEntryPoint->hooks,
                  Stg_Object_GetName( self->alwaysLastHook ) );
            }
         }
         else {
            self->alwaysLastHook = NULL;
         }
      }
      else {
         self->alwaysFirstHook = NULL;
         self->alwaysLastHook = NULL;
      }
   }
   else {
      newEntryPoint->hooks = self->hooks;
      newEntryPoint->alwaysFirstHook = self->alwaysFirstHook;
      newEntryPoint->alwaysLastHook = self->alwaysLastHook;
   }
   
   if( ownMap ) {
      Stg_Class_Delete( map );
   }
   
   return (void*)newEntryPoint;
}

Func_Ptr EntryPoint_GetRun( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   return self->_getRun( self );
}

Func_Ptr _EntryPoint_GetRun( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   
   switch( self->castType ) {
      case EntryPoint_0_CastType:
         return (void*) _EntryPoint_Run;
      
      case EntryPoint_VoidPtr_CastType:
         return (void*) _EntryPoint_Run_VoidPtr;
      
      case EntryPoint_2VoidPtr_CastType:
         return (void*) _EntryPoint_Run_2VoidPtr;
      
      case EntryPoint_3VoidPtr_CastType:
         return (void*) _EntryPoint_Run_3VoidPtr;
      
      case EntryPoint_Minimum_VoidPtr_CastType:
         return (void*) _EntryPoint_Run_Minimum_VoidPtr;
      
      case EntryPoint_Maximum_VoidPtr_CastType:
         return (void*) _EntryPoint_Run_Maximum_VoidPtr;
      
      case EntryPoint_Class_0_CastType:
         return (void*) _EntryPoint_Run_Class;
      
      case EntryPoint_Class_VoidPtr_CastType:
         return (void*) _EntryPoint_Run_Class_VoidPtr;
      
      case EntryPoint_Class_Minimum_VoidPtr_CastType:
         return (void*) _EntryPoint_Run_Class_Minimum_VoidPtr;
      
      case EntryPoint_Class_Maximum_VoidPtr_CastType:
         return (void*) _EntryPoint_Run_Class_Maximum_VoidPtr;
      
      default:
         assert( 0 /* castType unknown */ );
         /* Decendant versions of this function should call the parent's getrun here instead */
   }
   return 0;
}

void _EntryPoint_Run( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      ((EntryPoint_0_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)();
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

void _EntryPoint_ProfileFunction( char ep_name[], char hk_name[], double time ) {
#if ENABLE_STGERMAIN_LOG > 1
   // log every processor timing
   stg_profile_EntryPoint( ep_name, hk_name, time );
#else
   // log every processor timing
   int rank;
   MPI_Comm_rank( MPI_COMM_WORLD, &rank );
   if( rank == 0 )
      stg_profile_EntryPoint( ep_name, hk_name, time );
#endif
}


void _EntryPoint_Run_VoidPtr( void* entryPoint, void* data0 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
   double       wallTime;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      wallTime = MPI_Wtime();

      ((EntryPoint_VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0 );
      
#ifdef ENABLE_STGERMAIN_LOG
      _EntryPoint_ProfileFunction( self->name, self->hooks->data[hookIndex]->name, MPI_Wtime() - wallTime );
#endif
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

void _EntryPoint_Run_2VoidPtr( void* entryPoint, void* data0, void* data1 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
        double wallTime;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_2VoidPtr, self->name );
   #endif

   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      wallTime = MPI_Wtime();
      
      ((EntryPoint_2VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0, data1 );
   
#ifdef ENABLE_STGERMAIN_LOG
      _EntryPoint_ProfileFunction( self->name, self->hooks->data[hookIndex]->name, MPI_Wtime() - wallTime );
#endif
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

void _EntryPoint_Run_3VoidPtr( void* entryPoint, void* data0, void* data1, void* data2 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
   double wallTime; 
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_3VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0 ; hookIndex < self->hooks->count; hookIndex++ ) {
      wallTime = MPI_Wtime();

      ((EntryPoint_3VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0, data1, data2 );
      
#ifdef ENABLE_STGERMAIN_LOG
      _EntryPoint_ProfileFunction( self->name, self->hooks->data[hookIndex]->name, MPI_Wtime() - wallTime );
#endif
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

double _EntryPoint_Run_Minimum_VoidPtr( void* entryPoint, void* data0 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
   double      minValue = HUGE_VAL;
   double      value;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_Minimum_VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      value = ((EntryPoint_Minimum_VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0 );
      /* Get Minimum out of currently stored value and one from this hook */
      if ( minValue > value )
         minValue = value;
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
   return minValue;
}

double _EntryPoint_Run_Maximum_VoidPtr( void* entryPoint, void* data0 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
   double      maxValue = -HUGE_VAL;
   double      value;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_Maximum_VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      value = ((EntryPoint_Maximum_VoidPtr_Cast*)((Hook*)self->hooks->data[hookIndex])->funcPtr)( data0 );
      /* Get Maximum out of currently stored value and one from this hook */
      if ( maxValue < value )
         maxValue = value;
   }

   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
   return maxValue;
}

double _EntryPoint_Run_Class_Minimum_VoidPtr( void* entryPoint, void* data0 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   ClassHook*  hook;
   Hook_Index  hookIndex;
   double      minValue = HUGE_VAL;
   double      value;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_Class_Minimum_VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      hook = (ClassHook*) self->hooks->data[hookIndex];
      value = ((EntryPoint_Class_Minimum_VoidPtr_Cast*)(hook->funcPtr))( hook->reference, data0 );

      /* Get Minimum out of currently stored value and one from this hook */
      if ( minValue > value )
         minValue = value;
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
   return minValue;
}

double _EntryPoint_Run_Class_Maximum_VoidPtr( void* entryPoint, void* data0 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   ClassHook*  hook;
   Hook_Index  hookIndex;
   double      maxValue = -HUGE_VAL;
   double      value;
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_Class_Maximum_VoidPtr, self->name );
   #endif
   
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      hook = (ClassHook*) self->hooks->data[hookIndex];
      value = ((EntryPoint_Class_Maximum_VoidPtr_Cast*)(hook->funcPtr))( hook->reference, data0 );
      
      /* Get Maximum out of currently stored value and one from this hook */
      if ( maxValue < value )
         maxValue = value;
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
   return maxValue;
}

void _EntryPoint_Run_Class( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   ClassHook*  hook;
   Hook_Index  hookIndex;

   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_Class, self->name );
   #endif
   
   for ( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      hook = (ClassHook*)self->hooks->data[hookIndex];
      ((EntryPoint_Class_0_Cast*)(hook->funcPtr))( hook->reference );
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

void _EntryPoint_Run_Class_VoidPtr( void* entryPoint, void* data0 ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   ClassHook*  hook;
   Hook_Index  hookIndex;

   #ifdef USE_PROFILE
      Stg_CallGraph_Push( stgCallGraph, _EntryPoint_Run_Class_VoidPtr, self->name );
   #endif
   
   for ( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      hook = (ClassHook*)self->hooks->data[hookIndex];
      ((EntryPoint_Class_VoidPtr_Cast*)(hook->funcPtr))( hook->reference, data0 );
   }
   
   #ifdef USE_PROFILE
      Stg_CallGraph_Pop( stgCallGraph );
   #endif
}

/* Functions for Adding hooks */
void _EntryPoint_PrependHook( void* entryPoint, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   
   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( NULL == self->alwaysFirstHook ) {
      Stg_ObjectList_Prepend( self->hooks, hook );
   }
   else {
      Stg_ObjectList_InsertAfter( self->hooks, self->alwaysFirstHook->name, hook );
   }   
}

void _EntryPoint_PrependHook_AlwaysFirst( void* entryPoint, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   
   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( self->alwaysFirstHook ) {
      Stream* error = Journal_Register( ErrorStream_Type, EntryPoint_Type );
      Journal_Printf( error, "Error: tried to prepend a new hook \"%s\" to entry point "
         "%s to always come first, but hook \"%s\" already specified to always be first.\n",
         hook->name, self->name, self->alwaysFirstHook->name ); 
      assert( 0 );
   }

   Stg_ObjectList_Prepend( self->hooks, hook );
   self->alwaysFirstHook = hook;
}

void _EntryPoint_AppendHook( void* entryPoint, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( NULL == self->alwaysLastHook ) {
      Stg_ObjectList_Append( self->hooks, hook );
   }
   else {
      Stg_ObjectList_InsertBefore( self->hooks, self->alwaysLastHook->name, hook );
   }   
}

void _EntryPoint_AppendHook_AlwaysLast( void* entryPoint, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   
   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( self->alwaysLastHook ) {
      Stream* error = Journal_Register( ErrorStream_Type, EntryPoint_Type );
      Journal_Printf( error, "Error: tried to append a new hook \"%s\" to entry point "
         "%s to always come last, but hook \"%s\" already specified to always be last.\n",
         hook->name, self->name, self->alwaysLastHook->name ); 
      assert( 0 );
   }

   Stg_ObjectList_Append( self->hooks, hook );
   self->alwaysLastHook = hook;
}

void _EntryPoint_InsertHookBefore( void* entryPoint, Name hookToInsertBefore, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( self->alwaysFirstHook && ( 0 == strcmp( hookToInsertBefore, self->alwaysFirstHook->name ) ) ) {
      Stream* error = Journal_Register( ErrorStream_Type, EntryPoint_Type );
      Journal_Printf( error, "Error: tried to insert a new hook \"%s\" to entry point "
         "%s before a hook specified to always be first, \"%s\".\n",
         hook->name, self->name, self->alwaysFirstHook->name ); 
      assert( 0 );
   }

   Stg_ObjectList_InsertBefore( self->hooks, hookToInsertBefore, hook );
}

void _EntryPoint_InsertHookAfter( void* entryPoint, Name hookToInsertAfter, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( self->alwaysLastHook && ( 0 == strcmp( hookToInsertAfter, self->alwaysLastHook->name ) ) ) {
      Stream* error = Journal_Register( ErrorStream_Type, EntryPoint_Type );
      Journal_Printf( error, "Error: tried to insert a new hook \"%s\" to entry point "
         "%s after a hook specified to always be last, \"%s\".\n",
         hook->name, self->name, self->alwaysLastHook->name ); 
      assert( 0 );
   }
   
   Stg_ObjectList_InsertAfter( self->hooks, hookToInsertAfter, hook );
}

void _EntryPoint_ReplaceAllHook( void* entryPoint, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   /* Purge the EP first. */
   EntryPoint_Purge( self );
   Stg_ObjectList_ReplaceAll( self->hooks, DELETE, hook );
}

void _EntryPoint_ReplaceHook( void* entryPoint, Name hookToReplace, Hook* hook ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if ( self->alwaysFirstHook && ( 0 == strcmp( hookToReplace, self->alwaysFirstHook->name ) ) ) {
      self->alwaysFirstHook = NULL;
   }
   else if ( self->alwaysLastHook && ( 0 == strcmp( hookToReplace, self->alwaysLastHook->name ) ) ) {
      self->alwaysLastHook = NULL;
   }

   Stg_ObjectList_Replace( self->hooks, hookToReplace, DELETE, hook );
}

/* PUBLIC FUNCTIONS */
void EntryPoint_PrintConcise( void* entryPoint, Stream* stream ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook_Index  hookIndex;
   Hook_Ptr    hook;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   Journal_Printf( stream, "\tEP: %s\n", self->name );
   for( hookIndex = 0; hookIndex < self->hooks->count; hookIndex++ ) {
      hook = (Hook*) self->hooks->data[hookIndex];
      Journal_Printf( stream, "\t\tH: \"%s\"", hook->name );
      if ( 0 != strcmp( "", hook->addedBy ) ) {
         Journal_Printf( stream, " (%s)", hook->addedBy );
      }
      Journal_Printf( stream, "\n" );
   }
}

void EntryPoint_Prepend(
   void*    entryPoint,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr , addedBy );

   _EntryPoint_PrependHook( self, hook );
}

void EntryPoint_Prepend_AlwaysFirst(
   void*        entryPoint,
   Name         name,
   Func_Ptr     funcPtr,
   Hook_AddedBy addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr, addedBy );

   _EntryPoint_PrependHook_AlwaysFirst( self, hook );
}

void EntryPoint_Append(
   void*        entryPoint,
   Name         name,
   Func_Ptr     funcPtr,
   Hook_AddedBy addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr, addedBy );

   _EntryPoint_AppendHook( self, hook );
}

void EntryPoint_Append_AlwaysLast(
   void*        entryPoint,
   Name         name,
   Func_Ptr     funcPtr,
   Hook_AddedBy addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr, addedBy );

   _EntryPoint_AppendHook_AlwaysLast( self, hook );
}

void EntryPoint_InsertBefore(
   void*    entryPoint,
   Name     hookToInsertBefore,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr , addedBy );

   _EntryPoint_InsertHookBefore( self, hookToInsertBefore, hook );
}

void EntryPoint_InsertAfter(
   void*    entryPoint,
   Name     hookToInsertAfter,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr , addedBy );

   _EntryPoint_InsertHookAfter( self, hookToInsertAfter, hook );
}

void EntryPoint_ReplaceAll(
   void*    entryPoint,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = Hook_New( name, funcPtr , addedBy );

   _EntryPoint_ReplaceAllHook( self, hook );
}

void EntryPoint_Replace(
   void*    entryPoint,
   Name     hookToReplace,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook*       hook = Hook_New( name, funcPtr , addedBy );

   _EntryPoint_ReplaceHook( self, hookToReplace, hook );
}

void EntryPoint_PrependClassHook(
   void*    entryPoint,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy,
   void*    reference ) {
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook*       hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_PrependHook( self, hook );
}
   
void EntryPoint_PrependClassHook_AlwaysFirst( 
   void*    entryPoint, 
   Name     name, 
   Func_Ptr funcPtr, 
   char*    addedBy, 
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_PrependHook_AlwaysFirst( self, hook );
}
   
void EntryPoint_AppendClassHook(
   void*    entryPoint,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy,
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_AppendHook( self, hook );
}
   
void EntryPoint_AppendClassHook_AlwaysLast( 
   void*    entryPoint, 
   Name     name, 
   Func_Ptr funcPtr, 
   char*    addedBy, 
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_AppendHook_AlwaysLast( self, hook );
}

void EntryPoint_InsertClassHookBefore( 
   void*    entryPoint, 
   Name     hookToInsertBefore, 
   Name     name, 
   Func_Ptr funcPtr, 
   char*    addedBy, 
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_InsertHookBefore( self, hookToInsertBefore, hook );

}
   
void EntryPoint_InsertClassHookAfter( 
   void*    entryPoint, 
   Name     hookToInsertAfter, 
   Name     name, 
   Func_Ptr funcPtr, 
   char*    addedBy, 
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_InsertHookAfter( self, hookToInsertAfter, hook );
}

void EntryPoint_ReplaceClassHook( 
   void*    entryPoint, 
   Name     hookToReplace, 
   Name     name, 
   Func_Ptr funcPtr, 
   char*    addedBy, 
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook* hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_ReplaceHook( self, hookToReplace, hook );
}
   
void EntryPoint_ReplaceAllClassHook(
   void*    entryPoint,
   Name     name,
   Func_Ptr funcPtr,
   char*    addedBy,
   void*    reference )
{
   EntryPoint* self = (EntryPoint*)entryPoint;
   Hook*       hook = (Hook*)ClassHook_New( name, funcPtr, addedBy, reference );

   _EntryPoint_ReplaceAllHook( self, hook );
}

void EntryPoint_Remove( void* entryPoint, Name name ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   Stg_ObjectList_Remove( self->hooks, name, DELETE );
}

void EntryPoint_Purge( void* entryPoint ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   /* 
    * Stg_Class_Delete the existing hook information structures.
    * Note: this won't try and delete the func ptrs themselves 
    */
   Stg_ObjectList_DeleteAllObjects( self->hooks );
   self->hooks->count = 0;
   self->alwaysFirstHook = NULL;
   self->alwaysLastHook = NULL;
}

void EntryPoint_WarnIfNoHooks( void* entryPoint, const char* parentFunction ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if (0 == self->hooks->count ) {
      Name epName = self->name;
      Stream* stream = Journal_Register (Debug_Type, self->name );
      Journal_Printf( (void*) stream, "Warning: No hooks defined for E.P. \"%s\". Calling parent function "
         "%s() usually implies at least one %s hook defined.\n",
         epName, parentFunction, epName );
   }
}

void EntryPoint_ErrorIfNoHooks( void* entryPoint, const char* parentFunction ) {
   EntryPoint* self = (EntryPoint*)entryPoint;

   Journal_Firewall( 
      self != NULL, 
      Journal_Register( Error_Type, EntryPoint_Type ),
      "Entry Point is NULL in %s\n", __func__ );

   if (0 == self->hooks->count ) {
      Name epName = self->name;
      Stream* stream = Journal_Register (Error_Type, self->name );
      Journal_Firewall( 0, (void*) stream, "Error: No hooks defined for E.P. \"%s\". Calling parent function "
         "%s() requires at least one %s hook defined.\n",
         epName, parentFunction, epName );
   }
}


