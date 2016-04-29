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
#include "LiveComponentRegister.h"
#include "Stg_ComponentFactory.h"
#include "Stg_ComponentRegister.h"
#include "HierarchyTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* Textual name of this class */
const Type Stg_ComponentFactory_Type = "Stg_ComponentFactory";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/
/* Creation implementation */
Stg_ComponentFactory* _Stg_ComponentFactory_New(  STG_COMPONENTFACTORY_DEFARGS  )
{
   Stg_ComponentFactory *self = NULL;

   assert( _sizeOfSelf >= sizeof( Stg_ComponentFactory ) );
   self = (Stg_ComponentFactory*) _Stg_Class_New(  STG_CLASS_PASSARGS  );

   self->getDouble = getDouble;
   self->getInt = getInt;
   self->getUnsignedInt = getUnsignedInt;
   self->getBool = getBool;
   self->getString = getString;

   self->getRootDictDouble = getRootDictDouble;
   self->getRootDictInt = getRootDictInt;
   self->getRootDictUnsignedInt = getRootDictUnsignedInt;
   self->getRootDictBool = getRootDictBool;
   self->getRootDictString = getRootDictString;
   
   self->constructByName = constructByName;
   self->constructByKey = constructByKey;
   self->constructByNameWithKeyFallback  = constructByNameWithKeyFallback;
   self->constructByList = constructByList;

   self->rootDict = rootDict;
   self->componentDict = componentDict;
   self->infoStream = Journal_Register( InfoStream_Type, self->type );
   Stream_SetPrintingRank( self->infoStream, 0 );
   Stream_SetAutoFlush( self->infoStream, True );
      
   _Stg_ComponentFactory_Init( self );
   
   return self;
}
   
Stg_ComponentFactory* Stg_ComponentFactory_New( Dictionary* rootDict, Dictionary* componentDict ) {
   /* Variables set in this function */
   SizeT                                                                        _sizeOfSelf = sizeof( Stg_ComponentFactory );
   Type                                                                                type = Stg_ComponentFactory_Type;
   Stg_Class_DeleteFunction*                                                        _delete = _Stg_ComponentFactory_Delete;
   Stg_Class_PrintFunction*                                                          _print = _Stg_ComponentFactory_Print;
   Stg_ComponentFactory_GetDoubleFunc*                                            getDouble = _Stg_ComponentFactory_GetDouble;
   Stg_ComponentFactory_GetIntFunc*                                                  getInt = _Stg_ComponentFactory_GetInt;
   Stg_ComponentFactory_GetUnsignedIntFunc*                                  getUnsignedInt = _Stg_ComponentFactory_GetUnsignedInt;
   Stg_ComponentFactory_GetBoolFunc*                                                getBool = _Stg_ComponentFactory_GetBool;
   Stg_ComponentFactory_GetStringFunc*                                            getString = _Stg_ComponentFactory_GetString;
   Stg_ComponentFactory_GetRootDictDoubleFunc*                            getRootDictDouble = _Stg_ComponentFactory_GetRootDictDouble;
   Stg_ComponentFactory_GetRootDictIntFunc*                                  getRootDictInt = _Stg_ComponentFactory_GetRootDictInt;
   Stg_ComponentFactory_GetRootDictUnsignedIntFunc*                  getRootDictUnsignedInt = _Stg_ComponentFactory_GetRootDictUnsignedInt;
   Stg_ComponentFactory_GetRootDictBoolFunc*                                getRootDictBool = _Stg_ComponentFactory_GetRootDictBool;
   Stg_ComponentFactory_GetRootDictStringFunc*                            getRootDictString = _Stg_ComponentFactory_GetRootDictString;
   Stg_ComponentFactory_ConstructByNameFunc*                                constructByName = _Stg_ComponentFactory_ConstructByName;
   Stg_ComponentFactory_ConstructByKeyFunc*                                  constructByKey = _Stg_ComponentFactory_ConstructByKey;
   Stg_ComponentFactory_ConstructByNameWithKeyFallbackFunc*  constructByNameWithKeyFallback = _Stg_ComponentFactory_ConstructByNameWithKeyFallback;
   Stg_ComponentFactory_ConstructByListFunc*                                constructByList = _Stg_ComponentFactory_ConstructByList;

   Stg_ComponentFactory *self = NULL;

   assert( rootDict );
   /* The following terms are parameters that have been passed into or defined in this function but are being set before being passed onto the parent */
   Stg_Class_CopyFunction*  _copy = NULL;

   self = _Stg_ComponentFactory_New(  STG_COMPONENTFACTORY_PASSARGS  );

   return self;
}

void _Stg_ComponentFactory_Init( Stg_ComponentFactory *self )
{
   assert( self );
   
   self->LCRegister = LiveComponentRegister_New( );
}

void Stg_ComponentFactory_Init( Stg_ComponentFactory *self )
{
   assert( self );
   _Stg_ComponentFactory_Init( self );
}
   
/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Stg_ComponentFactory_Delete( void* cf )
{
   Stg_ComponentFactory *self = (Stg_ComponentFactory *) cf;

   assert( self );

   _Stg_Class_Delete( self );
}
   
void _Stg_ComponentFactory_Print( void* cf, Stream* stream )
{
   Stg_ComponentFactory *self = (Stg_ComponentFactory*) cf;

   assert( self );
   
   /* General info */
   Journal_Printf( (void*) stream, "Stg_ComponentFactory (ptr): %p\n", self );
   
   /* Print parent class */
   _Stg_Class_Print( self, stream );
   
   Journal_Printf( (void*) stream, "LiveComponentRegister (ptr): %p\n", self->LCRegister );
   Stg_Class_Print( self->LCRegister, stream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

void Stg_ComponentFactory_SetComponentDictionary( Stg_ComponentFactory *self, Dictionary *dict )
{
   assert( self );
   assert( dict );

   self->componentDict = dict;
}

void Stg_ComponentFactory_CreateComponents( Stg_ComponentFactory *self ) {
   Dictionary_Entry*                      componentDictEntry           = NULL;
   Dictionary*                            currComponentDict            = NULL;
   Type                                   componentType                = NULL;
   Name                                   componentName                = NULL;
   Stg_Component_DefaultConstructorFunction*  componentConstructorFunction;
   Index                                  component_I;
   Stream*                                stream;
   
   assert( self );
   
   stream = self->infoStream;
   if( self->componentDict ){
      Stream_Indent( stream );

      /* add the contexts to the live component register first (so these get constructed/built/initialised first) */   
      for( component_I = 0; component_I < Dictionary_GetCount( self->componentDict ) ; component_I++ ){
         componentDictEntry = self->componentDict->entryPtr[ component_I ];

         currComponentDict  = Dictionary_Entry_Value_AsDictionary( componentDictEntry->value );
         componentType      = Dictionary_GetString( currComponentDict, "Type" );
         componentName      = componentDictEntry->key;

         if( strcmp( componentType, "DomainContext" ) && 
             strcmp( componentType, "FiniteElementContext" ) &&
             strcmp( componentType, "PICelleratorContext" ) &&
             strcmp( componentType, "UnderworldContext" ) )
            continue;

         if( LiveComponentRegister_Get( self->LCRegister, componentName ) != NULL ) {
            Journal_RPrintf( Journal_Register( Error_Type, self->type ),
               "Error in func %s: Trying to instantiate two components with the name of '%s'\n"
               "Each component's name must be unique.\n",
               __func__, componentName );
            exit(EXIT_FAILURE);
         }

         /* Print Message */
         /* Journal_Printf( stream, "Instantiating %s as %s\n", componentType, componentName ); */
         
         /* Get Default Constructor for this type */
         componentConstructorFunction = Stg_ComponentRegister_AssertGet( 
               Stg_ComponentRegister_Get_ComponentRegister(), componentType, "0" );

         /* Add to register */
         LiveComponentRegister_Add( self->LCRegister, (Stg_Component*)componentConstructorFunction( componentName ) );
      }

      /* now add the rest of the components */   
      for( component_I = 0; component_I < Dictionary_GetCount( self->componentDict ) ; component_I++ ){
         componentDictEntry = self->componentDict->entryPtr[ component_I ];

         currComponentDict  = Dictionary_Entry_Value_AsDictionary( componentDictEntry->value );
         componentType      = Dictionary_GetString( currComponentDict, "Type" );
         componentName      = componentDictEntry->key;

         if( !strcmp( componentType, "DomainContext" ) ||
             !strcmp( componentType, "FiniteElementContext" ) ||
             !strcmp( componentType, "PICelleratorContext" ) ||
             !strcmp( componentType, "UnderworldContext" ) )
            continue;

         if( LiveComponentRegister_Get( self->LCRegister, componentName ) != NULL ) {
            Journal_RPrintf( Journal_Register( Error_Type, self->type ),
               "Error in func %s: Trying to instantiate two components with the name of '%s'\n"
               "Each component's name must be unique.\n",
               __func__, componentName );
            exit(EXIT_FAILURE);
         }
         
         Journal_Firewall( strcmp( componentType, "" ), NULL, "In func %s: Component with name '%s' does not have a 'Type' specified.\n"
                                                                              "This is sometimes caused by incorrect or missing 'mergeType' resulting in clobbered input file components.\n"
                                                                              "You may need to add 'mergeType=\"merge\"' to this component. Please check your input file.", __func__, componentName);

         /* Print Message */
         /* Journal_Printf( stream, "Instantiating %s as %s\n", componentType, componentName ); */
         
         /* Get Default Constructor for this type */
         componentConstructorFunction = Stg_ComponentRegister_AssertGet( 
               Stg_ComponentRegister_Get_ComponentRegister(), componentType, "0" );

         /* Add to register */
         LiveComponentRegister_Add( self->LCRegister, (Stg_Component*)componentConstructorFunction( componentName ) );
      }

      Stream_UnIndent( stream );
   }
   else{
      Journal_Printf( stream, "No Stg_Component List found..!\n" );
   }
}

void Stg_ComponentFactory_ConstructComponents( Stg_ComponentFactory* self, void* data ) {
   Stg_Component*                         component                    = NULL;
   Index                                  component_I;
   Stream*                                stream;
   
   assert( self );
   
   stream = self->infoStream;

   if( self->componentDict ){
      Journal_Printf( stream, "\nConstructing Stg_Components from the live-component register\n\n" );
      Stream_Indent( stream );
   
      for( component_I = 0; component_I < LiveComponentRegister_GetCount( self->LCRegister ); component_I++ ){
         /* Grab component from register */
         component = LiveComponentRegister_At( self->LCRegister, component_I );
         if( component && !component->isConstructed ){
            Stg_Component_AssignFromXML( component, self, data, True );
         }
      }
      Stream_UnIndent( stream );
   }
   else{
      Journal_Printf( stream, "No Stg_ComponentList found..!\n" );
   }
}

void Stg_ComponentFactory_BuildComponents( Stg_ComponentFactory* self, void* data ) {
   Stg_Component*                         component                    = NULL;
   Index                                  component_I;
   Stream*                                stream;
   
   assert( self );
   
   stream = self->infoStream;

   if( self->componentDict ){
      Journal_Printf( stream, "\nBuilding Stg_Components from the live-component register\n\n" );
      Stream_Indent( stream );
   
      for( component_I = 0; component_I < LiveComponentRegister_GetCount( self->LCRegister ); component_I++ ){
         /* Grab component from register */
         component = LiveComponentRegister_At( self->LCRegister, component_I );
         if( component && !component->isBuilt ){
            Stg_Component_Build( component, data, True );
         }
      }
      Stream_UnIndent( stream );
   }
   else{
      Journal_Printf( stream, "No Stg_ComponentList found..!\n" );
   }
}

void Stg_ComponentFactory_InitialiseComponents( Stg_ComponentFactory* self, void* data ) {
   Stg_Component*                         component                    = NULL;
   Index                                  component_I;
   Stream*                                stream;
   
   assert( self );
   
   stream = self->infoStream;

   if( self->componentDict ){
      Journal_Printf( stream, "\nInitialising Stg_Components from the live-component register\n\n" );
      Stream_Indent( stream );
   
      for( component_I = 0; component_I < LiveComponentRegister_GetCount( self->LCRegister ); component_I++ ){
         /* Grab component from register */
         component = LiveComponentRegister_At( self->LCRegister, component_I );
         if( component && !component->isInitialised ){
            Stg_Component_Initialise( component, data, True );
         }
      }
      Stream_UnIndent( stream );
   }
   else{
      Journal_Printf( stream, "No Stg_ComponentList found..!\n" );
   }
}

void Stg_ComponentFactory_ExecuteComponents( Stg_ComponentFactory* self, void* data ) {
   Stg_Component*                         component                    = NULL;
   Index                                  component_I;
   Stream*                                stream;
   
   assert( self );
   
   stream = self->infoStream;

   if( self->componentDict ){
      Journal_Printf( stream, "\nExecuting Stg_Components from the live-component register\n\n" );
      Stream_Indent( stream );
   
      for( component_I = 0; component_I < LiveComponentRegister_GetCount( self->LCRegister ); component_I++ ){
         /* Grab component from register */
         component = LiveComponentRegister_At( self->LCRegister, component_I );
         if( component && !component->hasExecuted ){
            Stg_Component_Execute( component, data, True );
         }
      }
      Stream_UnIndent( stream );
   }
   else{
      Journal_Printf( stream, "No Stg_ComponentList found..!\n" );
   }
}

void Stg_ComponentFactory_DestroyComponents( Stg_ComponentFactory* self, void* data ) {
   Stg_Component*                         component                    = NULL;
   Index                                  component_I;
   Stream*                                stream;
   
   assert( self );
   
   stream = self->infoStream;

   if( self->componentDict ){
      Journal_Printf( stream, "\nDestroying Stg_Components from the live-component register\n\n" );
      Stream_Indent( stream );
   
      for( component_I = 0; component_I < LiveComponentRegister_GetCount( self->LCRegister ); component_I++ ){
         /* Grab component from register */
         component = LiveComponentRegister_At( self->LCRegister, component_I );
         if( component && !component->isDestroyed ){
            Stg_Component_Destroy( component, data, True );
         }
      }
      Stream_UnIndent( stream );
   }
   else{
      Journal_Printf( stream, "No Stg_ComponentList found..!\n" );
   }
}

Dictionary_Entry_Value* _Stg_ComponentFactory_GetDictionaryValue( void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) {
   Stg_ComponentFactory*       self              = (Stg_ComponentFactory*) cf;
   Dictionary*             componentDict     = NULL;
   Dictionary*             thisComponentDict = NULL;
   Dictionary_Entry_Value* returnVal;
   Bool                    usedDefault       = False;
   Stream*                 errorStream       = Journal_Register( Error_Type, Stg_Component_Type );
   Stream*                 stream            = self->infoStream;

   Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_ComponentFactory is NULL.\n", __func__ );

   Journal_PrintfL( stream, 2, "Getting parameter '%s': ", key );

   /* Get this Stg_Component's Dictionary */
   componentDict = self->componentDict;
   Journal_Firewall( componentDict != NULL, errorStream, 
         "In func %s: Stg_Component Factory's dictionary is NULL.\n", __func__ );
   thisComponentDict = Dictionary_GetDictionary( componentDict, componentName );
   if( thisComponentDict == NULL )
      return defaultVal;

   /* Get Value from dictionary */
   returnVal = Dictionary_Get( thisComponentDict, key );
   if ( !returnVal && defaultVal ) {
      returnVal = Dictionary_GetDefault( thisComponentDict, key, defaultVal );
      usedDefault = True;
   }

   /* Print Stuff */
   if ( usedDefault ) {
      Journal_PrintfL( stream, 2, "Using default value = " );
      if ( Stream_IsPrintableLevel( stream, 2 ) ) 
         Dictionary_Entry_Value_Print( returnVal, stream );
      Journal_PrintfL( stream, 2, "\n" );

      return returnVal;
   }
   else if ( returnVal ) {
      Journal_PrintfL( stream, 2, "Found - Value = " );
      if ( Stream_IsPrintableLevel( stream, 2 ) ) 
         Dictionary_Entry_Value_Print( returnVal, stream );
      Journal_PrintfL( stream, 2, "\n" );
   }
   else 
      Journal_PrintfL( stream, 2, "Not found.\n" );

   return returnVal;
}

Dictionary_Entry_Value* _Stg_ComponentFactory_GetNumericalValue( void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) {
   Stg_ComponentFactory*       self              = (Stg_ComponentFactory*) cf;
   Dictionary_Entry_Value* returnVal;
   Bool                    usedDefault       = False;
   Stream*                 errorStream       = Journal_Register( Error_Type, Stg_Component_Type );
   Stream*                 stream            = self->infoStream;

   returnVal = _Stg_ComponentFactory_GetDictionaryValue( self, componentName, key, defaultVal );

   /* Check to see whether the type is a string -
    * if it is then assume that this is a dictionary key linking to the root dictionary */
   if ( returnVal ) {
      Dictionary_Entry_Key rootDictKey = Dictionary_Entry_Value_AsString( returnVal );
      Dictionary*          rootDict    = self->rootDict;

      /* Check if the number really is a string or not */
      if ( Stg_StringIsNumeric( rootDictKey ) )
         return returnVal;
      
      Journal_PrintfL( stream, 2, "Key '%s' points to key '%s' in the root dictionary: ", key, rootDictKey );

      Journal_Firewall( rootDict != NULL, errorStream, "Root Dictionary NULL in component factory.\n" );

      /* Get Value from dictionary */
      returnVal = Dictionary_Get( rootDict, rootDictKey );
      if ( !returnVal && defaultVal ) {
         returnVal = Dictionary_GetDefault( rootDict, rootDictKey, defaultVal );
         usedDefault = True;
      }

      /* Print Stuff */
      if ( usedDefault ) {
         Journal_PrintfL( stream, 2, "Using default value = " );
         if ( Stream_IsPrintableLevel( stream, 2 ) ) 
            Dictionary_Entry_Value_Print( returnVal, stream );
         Journal_PrintfL( stream, 2, "\n" );
         return returnVal;
      }
      else if ( returnVal ) {
         Journal_PrintfL( stream, 2, "Found - Value = " );
         if ( Stream_IsPrintableLevel( stream, 2 ) ) 
            Dictionary_Entry_Value_Print( returnVal, stream );
         Journal_PrintfL( stream, 2, "\n" );
      }
      else 
         Journal_PrintfL( stream, 2, "Not found.\n" );
   }

   return returnVal;
}

double Stg_ComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double defaultVal ) {
        Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
        return self->getDouble( cf, componentName, key, defaultVal );
}
double _Stg_ComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double defaultVal ) {
   return Dictionary_Entry_Value_AsDouble( 
         _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, 
            Dictionary_Entry_Value_FromDouble( defaultVal )));
}
Bool Stg_ComponentFactory_TryDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double* value ) {
   Dictionary_Entry_Value* dict_value = _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, NULL);
   if (!dict_value) return False;
   *value = Dictionary_Entry_Value_AsDouble(dict_value);
   return True;
}

void Stg_ComponentFactory_GetRequiredDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double* value ) {
   Dictionary_Entry_Value* dict_value = _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, NULL);
   if (!dict_value) {
      Journal_Firewall( NULL, NULL, "Input error for component '%s': Can't find the input key '%s'\n", componentName, key);
   }
   *value = Dictionary_Entry_Value_AsDouble(dict_value);
}

int Stg_ComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, int defaultVal ) {
   Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
   return self->getInt( cf, componentName, key, defaultVal );
}
int _Stg_ComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, int defaultVal ) {
   return Dictionary_Entry_Value_AsInt( 
         _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, 
            Dictionary_Entry_Value_FromInt( defaultVal ) ) );
}   
Bool Stg_ComponentFactory_TryInt( void* cf, Name componentName, Dictionary_Entry_Key key, int* value ) {
   Dictionary_Entry_Value* dict_value = _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, NULL);
   if (!dict_value) return False;
   *value = Dictionary_Entry_Value_AsInt(dict_value);
   return True;
}

unsigned int Stg_ComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, unsigned int defaultVal ) {
   Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
   return self->getUnsignedInt( cf, componentName, key, defaultVal );
}
unsigned int _Stg_ComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, unsigned int defaultVal ) {
   unsigned int retVal;
   retVal = Dictionary_Entry_Value_AsUnsignedInt( 
         _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, 
            Dictionary_Entry_Value_FromUnsignedInt( defaultVal )));
/*     TODO : Possible memory leak if defaultVal not added to the dictionary */
   return retVal;
}   
Bool Stg_ComponentFactory_TryUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, unsigned int* value ) {
   Dictionary_Entry_Value* dict_value = _Stg_ComponentFactory_GetNumericalValue( cf, componentName, key, NULL);
   if (!dict_value) return False;
   *value = Dictionary_Entry_Value_AsUnsignedInt(dict_value);
   return True;
}

Bool Stg_ComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, Bool defaultVal ) {
   Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
   return self->getBool( cf, componentName, key, defaultVal );
}
Bool _Stg_ComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, Bool defaultVal ) {
   return Dictionary_Entry_Value_AsBool( 
         _Stg_ComponentFactory_GetDictionaryValue( cf, componentName, key, 
            Dictionary_Entry_Value_FromBool( defaultVal ) ) );
}   
Bool Stg_ComponentFactory_TryBool( void* cf, Name componentName, Dictionary_Entry_Key key, Bool* value ) {
   Dictionary_Entry_Value* dict_value = _Stg_ComponentFactory_GetDictionaryValue( cf, componentName, key, NULL);
   if (!dict_value) return False;
   *value = Dictionary_Entry_Value_AsBool(dict_value);
   return True;
}

char* Stg_ComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) {
   Stg_ComponentFactory* self = (Stg_ComponentFactory*)cf;
   /* return a copy to this string.  this is safer, but will create potential memory leaks at times, but should only be minor */
   return StG_Strdup( self->getString( cf, componentName, key, defaultVal ) );
}
char* _Stg_ComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) {
   return Dictionary_Entry_Value_AsString( 
         _Stg_ComponentFactory_GetDictionaryValue( cf, componentName, key, 
            Dictionary_Entry_Value_FromString( defaultVal ) ) );
}

double Stg_ComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   return self->getRootDictDouble( self, key, defaultVal );
}
double _Stg_ComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   
   Journal_PrintfL( self->infoStream, 2, "Getting double from root dictionary with key '%s' and default value '%g'\n",
         key, defaultVal );

   assert( self->rootDict );
   return Dictionary_GetDouble_WithDefault( self->rootDict, key, defaultVal );
}
int Stg_ComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   return self->getRootDictInt( self, key, defaultVal );
}
int _Stg_ComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   
   Journal_PrintfL( self->infoStream, 2, "Getting int from root dictionary with key '%s' and default value '%d'\n",
         key, defaultVal );

   assert( self->rootDict );
   return Dictionary_GetInt_WithDefault( self->rootDict, key, defaultVal );
}   
unsigned int Stg_ComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   return self->getRootDictUnsignedInt( self, key, defaultVal );
}
unsigned int _Stg_ComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   
   Journal_PrintfL( self->infoStream,2, "Getting unsigned int from root dictionary with key '%s' and default value '%u'\n",
         key, defaultVal );

   assert( self->rootDict );
   return Dictionary_GetUnsignedInt_WithDefault( self->rootDict, key, defaultVal );
}   
Bool Stg_ComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   return self->getRootDictBool( self, key, defaultVal );
}
Bool _Stg_ComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   
   Journal_PrintfL( self->infoStream, 2, "Getting Bool from root dictionary with key '%s' and default value '%s'\n",
         key, defaultVal ? "True" : "False" );

   assert( self->rootDict );
   return Dictionary_GetBool_WithDefault( self->rootDict, key, defaultVal );
}   
char* Stg_ComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* const defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;
   return self->getRootDictString( self, key, defaultVal );
}
char* _Stg_ComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* const defaultVal ) {
   Stg_ComponentFactory*    self              = (Stg_ComponentFactory*)cf;

   Journal_PrintfL( self->infoStream, 2, "Getting string from root dictionary with key '%s' and default value '%s'\n",
         key, defaultVal );

   assert( self->rootDict );
   return Dictionary_GetString_WithDefault( self->rootDict, key, defaultVal );
}


Stg_Component* _Stg_ComponentFactory_ConstructByName( void* cf, Name componentName, Type type, Bool isEssential, void* data ) {
   Stg_ComponentFactory*   self              = (Stg_ComponentFactory*)cf;
   Stg_Component*      component;
   Stream*         stream            = self->infoStream;

   Journal_PrintfL( stream, 2, "Looking for %sessential component '%s': ", (isEssential ? "" : "non-"), componentName );

   component = LiveComponentRegister_Get( self->LCRegister, componentName );

   /* Checking */
   if (component) {
      Journal_PrintfL( stream, 2, "Found.\n" );

      if ( !component->isConstructed ) {
         /* Journal_Printf( stream, "%s has not been constructed yet. Constructing now.\n", componentName ); */
         Stream_Indent( stream );
         Stg_Component_AssignFromXML( component, self, data, True );
         Stream_UnIndent( stream );
      }

      Stg_Class_CheckType( component, type );

   }
   else {
      Name         redirect;

      /* If we can find the component's name in the root dictionary, use that value instead. */
      if( self->rootDict ) {
         redirect = Dictionary_GetString_WithDefault( self->rootDict, componentName, "" );
         if( strcmp( redirect, "" ) ) {
            componentName = redirect;
            return self->constructByName( self, componentName, type, isEssential, data );
         }
      }

      Journal_PrintfL( stream, 2, "Not found.\n" );

      if ( isEssential ) {
         Stream* errorStream = Journal_Register( Error_Type, self->type );

         Journal_Printf( errorStream, "In func %s: Cannot find essential component '%s'.\n", __func__, componentName );

         Journal_Printf( errorStream, "Could you have meant one of these?\n" );

         Stream_Indent( errorStream );
         LiveComponentRegister_PrintSimilar( self->LCRegister, componentName, errorStream, 5 );
         Journal_Firewall( 0, errorStream, "" );
      }
   }
   
   return component;
}

Stg_Component* _Stg_ComponentFactory_ConstructByKey( 
      void*         cf, 
      Name         parentComponentName, 
      Dictionary_Entry_Key   componentKey,
      Type         type, 
      Bool          isEssential,
      void*          data ) 
{
   Stg_ComponentFactory*   self              = (Stg_ComponentFactory*)cf;
   Dictionary*      thisComponentDict = NULL;
   Dictionary*      componentDict     = NULL;
   Name         componentName, redirect;
   Dictionary_Entry_Value*   componentEntryVal;
   Stream*         errorStream       = Journal_Register( Error_Type, self->type );

   Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_Component is NULL.\n", __func__ );

   /* Get this Stg_Component's Dictionary */
   componentDict = self->componentDict;
   Journal_Firewall( componentDict != NULL, errorStream, 
         "In func %s: Stg_Component Factory's dictionary is NULL.\n", __func__ );
   thisComponentDict = Dictionary_GetDictionary( componentDict, parentComponentName );

   /* Get Dependency's Name */
   componentEntryVal = Dictionary_Get( thisComponentDict, componentKey );
   if ( componentEntryVal == NULL ) {
      Journal_Firewall( !isEssential, errorStream,
            "Stg_Component '%s' cannot find essential component with key '%s'.\n", parentComponentName, componentKey );
      Journal_PrintfL( self->infoStream, 2, "Stg_Component '%s' cannot find non-essential component with key '%s'.\n", parentComponentName, componentKey );
      return NULL;
   }
      
   componentName = Dictionary_Entry_Value_AsString( componentEntryVal );

   /* If we can find the component's name in the root dictionary, use that value instead. */
   if( self->rootDict ) {
      redirect = Dictionary_GetString_WithDefault( self->rootDict, componentName, "" );
      if( strcmp( redirect, "" ) )
         componentName = redirect;
   }

   return self->constructByName( self, componentName, type, isEssential, data );
}

Stg_Component* _Stg_ComponentFactory_PluginConstructByKey( 
   void*                  cf, 
   void*                  codelet, 
   Dictionary_Entry_Key   componentKey,
   Type                  type, 
   Bool                  isEssential,
   void*                  data ) 
{
   Stg_ComponentFactory*   self = (Stg_ComponentFactory*)cf;
   Stg_Component*            plugin = (Stg_Component*)codelet;
   Dictionary*               thisPluginDict = NULL;
   Dictionary*               pluginDict = (Dictionary*)Dictionary_Get( self->rootDict, "plugins" );
   Name                     componentName, redirect, pluginType;
   Dictionary_Entry_Value*   componentEntryVal;
   Index                     pluginIndex;
   Stream*                  errorStream = Journal_Register( Error_Type, self->type );

   Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_Component is NULL.\n", __func__ );

   /* Get this plugins Dictionary */
   for( pluginIndex = 0; pluginIndex < Dictionary_Entry_Value_GetCount( (Dictionary_Entry_Value*)pluginDict ); pluginIndex++ ) {
      thisPluginDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Entry_Value_GetElement( (Dictionary_Entry_Value*)pluginDict, pluginIndex ) );
      pluginType = StG_Strdup( Dictionary_GetString( thisPluginDict, "Type" ) );

      if( !strcmp( plugin->type, pluginType ) ){
         Memory_Free( pluginType );
         break;
      }
      Memory_Free( pluginType );
   }
   
   /* Get Dependency's Name */
   componentEntryVal = Dictionary_Get( thisPluginDict, componentKey );
   if ( componentEntryVal == NULL ) {
      Journal_Firewall( !isEssential, errorStream,
            "plugin '%s' cannot find essential component with key '%s'.\n", plugin->type, componentKey );
      Journal_PrintfL( self->infoStream, 2, "plugin '%s' cannot find non-essential component with key '%s'.\n", plugin->type, componentKey );
      return NULL;
   }
      
   componentName = Dictionary_Entry_Value_AsString( componentEntryVal );

   /* If we can find the component's name in the root dictionary, use that value instead. */
   if( self->rootDict ) {
      redirect = Dictionary_GetString_WithDefault( self->rootDict, componentName, "" );
      if( strcmp( redirect, "" ) )
         componentName = redirect;
   }

   return self->constructByName( self, componentName, type, isEssential, data );
}

Dictionary_Entry_Value* _Stg_ComponentFactory_PluginGetDictionaryValue( void* cf, void *codelet, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) {
   Stg_ComponentFactory*       self              = (Stg_ComponentFactory*) cf;
   Stg_Component*             plugin          = (Stg_Component*)codelet;
   Dictionary*                thisPluginDict = NULL;
   Dictionary*                pluginDict     = (Dictionary*)Dictionary_Get( self->rootDict, "plugins" );
   Name                      pluginType;
   Index      pluginIndex;
   Dictionary_Entry_Value* returnVal;
   Bool                    usedDefault       = False;
   Stream*                 errorStream       = Journal_Register( Error_Type, Stg_Component_Type );
   Stream*                 stream            = self->infoStream;

   Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_ComponentFactory is NULL.\n", __func__ );

   Journal_PrintfL( stream, 2, "Getting parameter '%s': ", key );

   Journal_Firewall( pluginDict != NULL, errorStream, 
         "In func %s: Stg_Component Factory's dictionary is NULL.\n", __func__ );

   /* Get this plugins Dictionary */
   for( pluginIndex = 0; pluginIndex < Dictionary_Entry_Value_GetCount( (Dictionary_Entry_Value*)pluginDict ); pluginIndex++ ) {
      thisPluginDict = Dictionary_Entry_Value_AsDictionary( Dictionary_Entry_Value_GetElement( (Dictionary_Entry_Value*)pluginDict, pluginIndex ) );
      pluginType = StG_Strdup( Dictionary_GetString( thisPluginDict, "Type" ) );
      if( !strcmp( plugin->type, pluginType ) ){
         Memory_Free( pluginType );
         break;
      }
                Memory_Free( pluginType );
   }

   /* Get this Stg_Component's Dictionary */
   Journal_Firewall( thisPluginDict != NULL, errorStream,
         "In func %s: Can't find sub-dictionary for component '%s'.\n", __func__, plugin->name );

   /* Get Value from dictionary */
   returnVal = Dictionary_Get( thisPluginDict, key );
   if ( !returnVal && defaultVal ) {
      returnVal = Dictionary_GetDefault( thisPluginDict, key, defaultVal );
      usedDefault = True;
   }

   /* Print Stuff */
   if ( usedDefault ) {
      Journal_PrintfL( stream, 2, "Using default value = " );
      if ( Stream_IsPrintableLevel( stream, 2 ) ) 
         Dictionary_Entry_Value_Print( returnVal, stream );
      Journal_PrintfL( stream, 2, "\n" );

      return returnVal;
   }
   else if ( returnVal ) {
      Journal_PrintfL( stream, 2, "Found - Value = " );
      if ( Stream_IsPrintableLevel( stream, 2 ) ) 
         Dictionary_Entry_Value_Print( returnVal, stream );
      Journal_PrintfL( stream, 2, "\n" );
   }
   else 
      Journal_PrintfL( stream, 2, "Not found.\n" );

   return returnVal;
}

Dictionary_Entry_Value* _Stg_ComponentFactory_PluginGetNumericalValue( void* cf, void *codelet, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) {
   Stg_ComponentFactory*    self           = (Stg_ComponentFactory*)cf;
   Dictionary_Entry_Value* returnVal;
   Bool                    usedDefault       = False;
   Stream*                 stream            = self->infoStream;
   Stream*         errorStream       = Journal_Register( Error_Type, self->type );

   Journal_Firewall( self != NULL, errorStream, "In func %s: Stg_Component is NULL.\n", __func__ );

   returnVal = _Stg_ComponentFactory_PluginGetDictionaryValue( self, codelet, key, defaultVal );

   /* Check to see whether the type is a string -
    * if it is then assume that this is a dictionary key linking to the root dictionary */
   if ( returnVal ) {
      Dictionary_Entry_Key rootDictKey = Dictionary_Entry_Value_AsString( returnVal );
      Dictionary*          rootDict    = self->rootDict;

      /* Check if the number really is a string or not */
      if ( Stg_StringIsNumeric( rootDictKey ) )
         return returnVal;
      
      Journal_PrintfL( stream, 2, "Key '%s' points to key '%s' in the root dictionary: ", key, rootDictKey );

      Journal_Firewall( rootDict != NULL, errorStream, "Root Dictionary NULL in component factory.\n" );

      /* Get Value from dictionary */
      returnVal = Dictionary_Get( rootDict, rootDictKey );
      if ( !returnVal && defaultVal ) {
         returnVal = Dictionary_GetDefault( rootDict, rootDictKey, defaultVal );
         usedDefault = True;
      }

      /* Print Stuff */
      if ( usedDefault ) {
         Journal_PrintfL( stream, 2, "Using default value = " );
         if ( Stream_IsPrintableLevel( stream, 2 ) ) 
            Dictionary_Entry_Value_Print( returnVal, stream );
         Journal_PrintfL( stream, 2, "\n" );
         return returnVal;
      }
      else if ( returnVal ) {
         Journal_PrintfL( stream, 2, "Found - Value = " );
         if ( Stream_IsPrintableLevel( stream, 2 ) ) 
            Dictionary_Entry_Value_Print( returnVal, stream );
         Journal_PrintfL( stream, 2, "\n" );
      }
      else 
         Journal_PrintfL( stream, 2, "Not found.\n" );
   }

   return returnVal;
}


double Stg_ComponentFactory_PluginGetUnsignedInt( void* cf, void *codelet, Dictionary_Entry_Key key, double defaultVal ) {
   return Dictionary_Entry_Value_AsDouble( 
                _Stg_ComponentFactory_PluginGetNumericalValue( cf, codelet, key, 
                                                               Dictionary_Entry_Value_FromUnsignedInt( defaultVal )));
}

double Stg_ComponentFactory_PluginGetDouble( void* cf, void *codelet, Dictionary_Entry_Key key, double defaultVal ) {
   return Dictionary_Entry_Value_AsDouble( 
                _Stg_ComponentFactory_PluginGetNumericalValue( cf, codelet, key, 
                                                               Dictionary_Entry_Value_FromDouble( defaultVal )));
}

int Stg_ComponentFactory_PluginGetInt( void* cf, void *codelet, Dictionary_Entry_Key key, int defaultVal ) {
   return Dictionary_Entry_Value_AsInt( 
                _Stg_ComponentFactory_PluginGetNumericalValue( cf, codelet, key, 
                                                               Dictionary_Entry_Value_FromInt( defaultVal )));
}

Bool Stg_ComponentFactory_PluginGetBool( void* cf, void *codelet, Dictionary_Entry_Key key, Bool defaultVal ) {
        return Dictionary_Entry_Value_AsBool( 
                _Stg_ComponentFactory_PluginGetDictionaryValue( cf, codelet, key,
                                                                Dictionary_Entry_Value_FromBool( defaultVal ) ) );
}

char* Stg_ComponentFactory_PluginGetString( void* cf, void* codelet, Dictionary_Entry_Key key, const char* const defaultVal ) {
   return Dictionary_Entry_Value_AsString( 
            _Stg_ComponentFactory_PluginGetDictionaryValue( cf, codelet, key,
                                                            Dictionary_Entry_Value_FromString( defaultVal ) ) );
}

Stg_Component* _Stg_ComponentFactory_ConstructByNameWithKeyFallback( 
   void*         cf, 
   Name          parentComponentName, 
   Name                 componentTrialName, 
   Dictionary_Entry_Key fallbackKey, 
   Type         type, 
   Bool         isEssential,
   void*         data ) 
{
   Stg_ComponentFactory*   self              = (Stg_ComponentFactory*)cf;
   Stg_Component*      component;
   Stream*         stream            = self->infoStream;

   Journal_PrintfL( stream, 2, "First Trying to find component by name '%s': ", componentTrialName );
   component = LiveComponentRegister_Get( self->LCRegister, componentTrialName );
   
   if (component) {
      Journal_PrintfL( stream, 2, "Found.\n" );

      if ( !component->isConstructed ) {
         Journal_Printf( stream, "%s has not been constructed yet. Constructing now.\n", componentTrialName );
         Stream_Indent( stream );
         Stg_Component_AssignFromXML( component, self, data, True );
         Stream_UnIndent( stream );
      }
   }
   else {
      Journal_PrintfL( stream, 2, "Not found.\n" );
      Journal_PrintfL( stream, 2, "Fallback - Trying to find component by key '%s'.\n", fallbackKey );

      component = self->constructByKey( self, parentComponentName, fallbackKey, type, isEssential, data );
   }
      
   return component;
}

Stg_Component** _Stg_ComponentFactory_ConstructByList( 
   void*         cf, 
   Name          parentComponentName, 
   Name          listName, 
   unsigned int  maxComponents,
   Type          type,
   Bool          isEssential,
   unsigned int* componentCount,
   void*         data )
{
   Stg_ComponentFactory*   self = (Stg_ComponentFactory*)cf;
   Dictionary*             thisComponentDict = NULL;
   Dictionary*             componentDict = NULL;
   Name                    componentName;
   Dictionary_Entry_Value* list;
   Stg_Component**         componentList;
   Index                   component_I;
   Stream*                 errorStream = Journal_Register( Error_Type, self->type );

   Journal_Firewall(
      self != NULL,
      errorStream,
      "In func %s: Stg_Component is NULL.\n",
      __func__ );

   /* Get this Stg_Component's Dictionary */
   componentDict = self->componentDict;

   Journal_Firewall(
      componentDict != NULL,
      errorStream, 
      "In func %s: Stg_Component Factory's dictionary is NULL.\n",
      __func__ );

   thisComponentDict = Dictionary_GetDictionary( componentDict, parentComponentName );

   Journal_Firewall(
      thisComponentDict != NULL,
      errorStream,
      "In func %s: Can't find sub-dictionary for component '%s'.\n",
      __func__,
      parentComponentName );
   
   Journal_PrintfL( self->infoStream, 2, "Looking for list '%s': ", listName );
   if( ( list = Dictionary_Get( thisComponentDict, listName ) ) ) {
      Journal_PrintfL( self->infoStream, 2, "Found.\n" );

      *componentCount = MIN( Dictionary_Entry_Value_GetCount( list ), maxComponents );
      componentList = Memory_Alloc_Array( Stg_Component*, *componentCount, "componentList" );
   
      Stream_Indent( self->infoStream );

      for( component_I = 0 ; component_I < *componentCount ; component_I++ ) {
         componentName = Dictionary_Entry_Value_AsString(
            Dictionary_Entry_Value_GetElement( list, component_I ) );

         componentList[ component_I ] = self->constructByName( self, componentName, type, True, data );
      }
      Stream_UnIndent( self->infoStream );
   }
   else {
      Journal_PrintfL( self->infoStream, 2, "Not Found.\n" );

      Journal_Firewall(
         !isEssential,
         errorStream,
         "Stg_Component '%s' cannot find list '%s'.\n", 
         parentComponentName,
         listName );

      *componentCount = 0;
      componentList   = NULL;
   }
   return componentList;
}


