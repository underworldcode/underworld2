/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "Base/Foundation/Foundation.h"
#include "Base/IO/IO.h"
#include "Base/Container/Container.h"

#include "types.h"
#include "shortcuts.h"
#include "Stg_Component.h"
#include "Stg_ComponentFactory.h"
#include "LiveComponentRegister.h"
#include "CallGraph.h"

#include "Stg_ComponentRegister.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Textual name of this class */
const Type Stg_Component_Type = "Stg_Component";

Stg_Component* _Stg_Component_New( STG_COMPONENT_DEFARGS ) {
   Stg_Component* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(Stg_Component) );
   self = (Stg_Component*)_Stg_Object_New( STG_OBJECT_PASSARGS );
   
   /* General info */
   
   /* Virtual functions */
   self->_defaultConstructor = _defaultConstructor;
   self->_construct = _construct;
   self->_build = _build;
   self->_initialise =  _initialise;
   self->_execute = _execute;
   self->_destroy = _destroy;
   
   /* Stg_Component info */
   _Stg_Component_Init( self );
   
   return self;
}

void _Stg_Component_Init( Stg_Component* self ) {
   /* General and Virtual info should already be set */
   
   /* Stg_Component info */
   self->isConstructed = False;
   self->isBuilt = False;
   self->isInitialised = False;
   self->hasExecuted = False;
   self->isDestroyed = False;
   self->constructType = Stg_Object_AppendSuffix( self, "Construct" );
   self->buildType = Stg_Object_AppendSuffix( self, "Build" );
   self->initialiseType = Stg_Object_AppendSuffix( self, "Initialise" );
   self->executeType = Stg_Object_AppendSuffix( self, "Execute" );
   self->destroyType = Stg_Object_AppendSuffix( self, "Destroy" );
}

void _Stg_Component_Delete( void* component ) {
   Stg_Component* self = (Stg_Component*)component;
   
   /* just make sure the component is destroyed */
   Stg_Component_Destroy( self, NULL, False );
   
   /* lets remove from live component register if registered */
   if( LiveComponentRegister_Get( LiveComponentRegister_GetLiveComponentRegister(), self->name ) )
   {
      LiveComponentRegister_RemoveOneComponentsEntry( LiveComponentRegister_GetLiveComponentRegister(), self->name );
   }
   
   Memory_Free( self->destroyType );
   Memory_Free( self->executeType );
   Memory_Free( self->initialiseType );
   Memory_Free( self->buildType );
   Memory_Free( self->constructType );

   /* Stg_Class_Delete parent class */
   _Stg_Object_Delete( self );
}

void _Stg_Component_Print( void* component, Stream* stream ) {
   Stg_Component* self = (Stg_Component*)component;
   
   /* General info */
   Journal_Printf( (void*) stream, "Stg_Component (ptr): %p\n", self );
   
   /* Print parent class */
   _Stg_Object_Print( self, stream );
   
   /* Virtual info */
   Journal_Printf( (void*) stream, "\t_defaultConstructor (func ptr): %p\n", self->_defaultConstructor );
   Journal_Printf( (void*) stream, "\t_construct (func ptr): %p\n", self->_construct );
   
   Journal_Printf( (void*) stream, "\t_initialise (func ptr): %p\n", self->_initialise );
   Journal_Printf( (void*) stream, "\t_execute (func ptr): %p\n", self->_execute );
   Journal_Printf( (void*) stream, "\t_destroy (func ptr): %p\n", self->_destroy );
   
   /* Stg_Component */
   Journal_Printf( (void*) stream, "\tisConstructed: %s\n", self->isConstructed ? "True" : "False" );
   Journal_Printf( (void*) stream, "\tisBuilt: %s\n", self->isBuilt ? "True" : "False" );
   Journal_Printf( (void*) stream, "\tisInitialised: %s\n", self->isInitialised ? "True" : "False" );
   Journal_Printf( (void*) stream, "\thasExecuted: %s\n", self->hasExecuted ? "True" : "False" );
   Journal_Printf( (void*) stream, "\tisDestroyed: %s\n", self->isDestroyed ? "True" : "False" );

   Journal_Printf( (void*) stream, "\tconstructor function name: %s\n", self->constructType );
   Journal_Printf( (void*) stream, "\tbuild function name: %s\n", self->buildType );
   Journal_Printf( (void*) stream, "\tinitialise function name: %s\n", self->initialiseType );
   Journal_Printf( (void*) stream, "\texecute function name: %s\n", self->executeType );
   Journal_Printf( (void*) stream, "\tdestroy function name: %s\n", self->destroyType );
}

void* _Stg_Component_Copy( void* component, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
   Stg_Component* self = (Stg_Component*)component;
   Stg_Component* newComponent;
   
   newComponent = (Stg_Component*) _Stg_Object_Copy( component, dest, deep, nameExt, ptrMap );
   
   /* Virtual methods */
   newComponent->_defaultConstructor = self->_defaultConstructor;
   newComponent->_construct = self->_construct;
   newComponent->_build = self->_build;
   newComponent->_initialise = self->_initialise;
   newComponent->_execute = self->_execute;
   newComponent->_destroy = self->_destroy;
   
   newComponent->isConstructed = self->isConstructed;
   newComponent->isBuilt = self->isBuilt;
   newComponent->isInitialised = self->isInitialised;
   newComponent->hasExecuted = self->hasExecuted;
   newComponent->isDestroyed = self->isDestroyed;
   
   newComponent->constructType = StG_Strdup( self->constructType );
   newComponent->buildType = StG_Strdup( self->buildType );
   newComponent->initialiseType = StG_Strdup( self->initialiseType );
   newComponent->executeType = StG_Strdup( self->executeType );
   newComponent->destroyType = StG_Strdup( self->destroyType );

   if ( LiveComponentRegister_GetLiveComponentRegister() ) {
      LiveComponentRegister_Add( LiveComponentRegister_GetLiveComponentRegister(), newComponent );
   }

   return newComponent;
}

void Stg_Component_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data, Bool force ) {
   Stg_Component* self = (Stg_Component*)component;

   assert( self );
   
   if( force || !self->isConstructed ) {
      
      Stream* stream = Journal_Register( Info_Type, "Stg_ComponentFactory" );

      /* Journal_RPrintf( stream, "Constructing %s..\n", self->name ); */
      Stream_Indent( stream );
      
      #ifdef USE_PROFILE
         Stg_CallGraph_Push( stgCallGraph, self->_construct, self->constructType );
      #endif
      
      if ( !self->isConstructed ) {
         Stg_Component_SetupStreamFromDictionary( 
            self, 
            Dictionary_GetDictionary( cf->componentDict, self->name ) );
      }
      
      self->isConstructed = True;
      self->_construct( self, cf, data );
      
      #ifdef USE_PROFILE
         Stg_CallGraph_Pop( stgCallGraph );
      #endif
      
      Stream_UnIndent( stream );
   }
}

void Stg_Component_Build( void* component, void* data, Bool force ) {
   Stg_Component* self = (Stg_Component*)component;
   
   if( self && (force || !self->isBuilt) ) {
      #ifdef USE_PROFILE
         Stg_CallGraph_Push( stgCallGraph, self->_build, self->buildType );
      #endif
      
      self->isBuilt = True;
      self->_build( self, data );
      
      #ifdef USE_PROFILE
         Stg_CallGraph_Pop( stgCallGraph );
      #endif
   }
}

void Stg_Component_Initialise( void* component, void* data, Bool force ) {
   Stg_Component* self = (Stg_Component*)component;

   if( self && (force || !self->isInitialised) ) {
      #ifdef USE_PROFILE
         Stg_CallGraph_Push( stgCallGraph, self->_initialise, self->initialiseType );
      #endif
      
      self->isInitialised = True;
      self->_initialise( self, data );
      
      
      #ifdef USE_PROFILE
         Stg_CallGraph_Pop( stgCallGraph );
      #endif
   }
}

void Stg_Component_Execute( void* component, void* data, Bool force ) {
   Stg_Component* self = (Stg_Component*)component;
   if( self && (force || !self->hasExecuted) ) {
      #ifdef USE_PROFILE
         Stg_CallGraph_Push( stgCallGraph, self->_execute, self->executeType );
      #endif
      
      self->_execute( self, data );
      self->hasExecuted = True;
      
      #ifdef USE_PROFILE
         Stg_CallGraph_Pop( stgCallGraph );
      #endif
   }
}

void Stg_Component_Destroy( void* component, void* data, Bool force ) {
   Stg_Component* self = (Stg_Component*)component;

   if( self && !self->isDestroyed && !self->locked) {
      #ifdef USE_PROFILE
         Stg_CallGraph_Push( stgCallGraph, self->_destroy, self->destroyType );
      #endif
      
      self->isDestroyed = True;

      if( self->_destroy )
         self->_destroy( self, data );

      #ifdef USE_PROFILE
         Stg_CallGraph_Pop( stgCallGraph );
      #endif
      
   }
}

Bool Stg_Component_IsConstructed( void* component ) {
   Stg_Component* self = (Stg_Component*)component;
   
   return self->isConstructed;
}

Bool Stg_Component_IsBuilt( void* component ) {
   Stg_Component* self = (Stg_Component*)component;
   
   return self->isBuilt;
}

Bool Stg_Component_IsInitialised( void* component ) {
   Stg_Component* self = (Stg_Component*)component;
   
   return self->isInitialised;
}

Bool Stg_Component_HasExecuted( void* component ) {
   Stg_Component* self = (Stg_Component*)component;
   
   return self->hasExecuted;
}

Bool Stg_Component_IsDestroyed( void* component ) {
   Stg_Component* self = (Stg_Component*)component;
   
   return self->isDestroyed;
}


void Stg_Component_SetupStreamFromDictionary( void* component, Dictionary* dictionary ) {
   Stg_Component* self = (Stg_Component*)component;
   Index          index;
   Bool           valid;
   char*          keyCopy;
   char*          operation;
   char*          streamType;

   Stream* stream;

   if ( dictionary == NULL ) {
      return;
   }
   
   /* Iterate through the whole dictionary, checking for journal related commands */
   for ( index = 0; index < dictionary->count; ++index ) {
      /* Check to see if it is a journal operation by searching for the JOURNAL_KEY. */
      if ( (strncasecmp( dictionary->entryPtr[index]->key, JOURNAL_KEY, strlen(JOURNAL_KEY))) == 0 ) {
         
         keyCopy = StG_Strdup( dictionary->entryPtr[index]->key );

         operation = strtok( keyCopy, JOURNAL_DELIMITER );
         streamType = strtok( NULL, JOURNAL_DELIMITER );

         if ( streamType == NULL ) {
            streamType = Info_Type;
         }

         stream = Journal_MyStream( streamType, self );

         valid = True;
         if ( strcmp( operation, JOURNAL_ENABLE_KEY ) == 0 ) {
            Bool enable = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
            Stream_Enable( stream, enable );
         }
         else if ( strcmp( operation, JOURNAL_ENABLE_BRANCH_KEY ) == 0 ) {
            Bool enable = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
            Stream_EnableBranch( stream, enable );
         }
         else if ( strcmp( operation, JOURNAL_LEVEL_KEY ) == 0 ) {
            JournalLevel level = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );
            Stream_SetLevelBranch( stream, level );
         }
         else if ( strcmp( operation, JOURNAL_RANK_KEY ) == 0 ) {
            int rank = Dictionary_Entry_Value_AsUnsignedInt( dictionary->entryPtr[index]->value );
            Stream_SetPrintingRank( stream, rank );
         }
         else if ( strcmp( operation, JOURNAL_FLUSH_KEY ) == 0 ) {
            Bool flush = Dictionary_Entry_Value_AsBool( dictionary->entryPtr[index]->value );
            Stream_SetAutoFlush( stream, flush );
         }
         else {
            valid = False;
         }

         if ( !valid ) {
            Journal_Printf(
               Journal_Register( Info_Type, "Journal" ),
               "Warning - unknown journal operation %s for component %s\n",
               dictionary->entryPtr[index]->key,
               self->name );
         }
         Memory_Free( keyCopy );
      }
   }
}
