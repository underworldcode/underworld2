/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#ifndef __StGermain_Base_Automation_Stg_ComponentFactory_h__
#define __StGermain_Base_Automation_Stg_ComponentFactory_h__

   extern const Type Stg_ComponentFactory_Type;
   
   typedef double (Stg_ComponentFactory_GetDoubleFunc) (
      void*                cf, 
      Name                 componentName, 
      Dictionary_Entry_Key key, 
      const double         defaultVal );

   typedef int (Stg_ComponentFactory_GetIntFunc) ( 
      void*                cf, 
      Name                 componentName, 
      Dictionary_Entry_Key key, 
      const int            defaultVal );

   typedef unsigned int (Stg_ComponentFactory_GetUnsignedIntFunc) (
      void*                cf, 
      Name                 componentName, 
      Dictionary_Entry_Key key, 
      const unsigned int   defaultVal);

   typedef Bool (Stg_ComponentFactory_GetBoolFunc) ( 
      void*                cf, 
      Name                 componentName, 
      Dictionary_Entry_Key key, 
      const Bool           defaultVal );

   typedef char* (Stg_ComponentFactory_GetStringFunc) ( 
      void*                cf, 
      Name                 componentName, 
      Dictionary_Entry_Key key, 
      const char*          defaultVal ) ;

   typedef double (Stg_ComponentFactory_GetRootDictDoubleFunc) (
      void*                cf, 
      Dictionary_Entry_Key key, 
      const double         defaultVal );

   typedef int (Stg_ComponentFactory_GetRootDictIntFunc) ( 
      void*                cf, 
      Dictionary_Entry_Key key, 
      const int            defaultVal );

   typedef unsigned int (Stg_ComponentFactory_GetRootDictUnsignedIntFunc) (
      void*                cf, 
      Dictionary_Entry_Key key, 
      const unsigned int   defaultVal);

   typedef Bool (Stg_ComponentFactory_GetRootDictBoolFunc) ( 
      void*                cf, 
      Dictionary_Entry_Key key, 
      const Bool           defaultVal );

   typedef char* (Stg_ComponentFactory_GetRootDictStringFunc) ( 
      void*                cf, 
      Dictionary_Entry_Key key, 
      const char*          defaultVal ) ;

   typedef Stg_Component* (Stg_ComponentFactory_ConstructByNameFunc) ( 
      void*                cf, 
      Name                 componentName, 
      Type                 type, 
      Bool                 isEssential,
      void*                data );

   typedef Stg_Component* (Stg_ComponentFactory_ConstructByKeyFunc) (
      void*                cf, 
      Name                 parentComponentName, 
      Dictionary_Entry_Key componentKey, 
      Type                 type, 
      Bool                 isEssential,
      void*                data );

   typedef Stg_Component* (Stg_ComponentFactory_ConstructByNameWithKeyFallbackFunc) ( 
      void*                cf, 
      Name                 parentComponentName, 
      Name                 componentTrialName, 
      Dictionary_Entry_Key componentTrialKey, 
      Type                 type, 
      Bool                 isEssential,
      void*                data );

   typedef Stg_Component** (Stg_ComponentFactory_ConstructByListFunc) ( 
      void*                cf, 
      Name                 parentComponentName, 
      Name                 listName, 
      unsigned int         maxComponents,
      Type                 type, 
      Bool                 isEssential,
      unsigned int*        componentCount,
      void*                data );

   #define __Stg_ComponentFactory \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      Stg_ComponentFactory_GetDoubleFunc*                      getDouble; \
      Stg_ComponentFactory_GetIntFunc*                         getInt; \
      Stg_ComponentFactory_GetUnsignedIntFunc*                 getUnsignedInt; \
      Stg_ComponentFactory_GetBoolFunc*                        getBool; \
      Stg_ComponentFactory_GetStringFunc*                      getString; \
      Stg_ComponentFactory_GetRootDictDoubleFunc*              getRootDictDouble; \
      Stg_ComponentFactory_GetRootDictIntFunc*                 getRootDictInt; \
      Stg_ComponentFactory_GetRootDictUnsignedIntFunc*         getRootDictUnsignedInt; \
      Stg_ComponentFactory_GetRootDictBoolFunc*                getRootDictBool; \
      Stg_ComponentFactory_GetRootDictStringFunc*              getRootDictString; \
      Stg_ComponentFactory_ConstructByNameFunc*                constructByName; \
      Stg_ComponentFactory_ConstructByKeyFunc*                 constructByKey; \
      Stg_ComponentFactory_ConstructByNameWithKeyFallbackFunc* constructByNameWithKeyFallback; \
      Stg_ComponentFactory_ConstructByListFunc*                constructByList; \
      \
      /* Class info */ \
      Dictionary*                                              rootDict; \
      Dictionary*                                              componentDict; \
      LiveComponentRegister*                                   LCRegister;    \
      Stream*                                                  infoStream;    
      
   struct Stg_ComponentFactory { __Stg_ComponentFactory };
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Constructor
   */

   /* Creation implementation */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define STG_COMPONENTFACTORY_DEFARGS \
      STG_CLASS_DEFARGS, \
      Stg_ComponentFactory_GetDoubleFunc*                                           getDouble, \
      Stg_ComponentFactory_GetIntFunc*                                                 getInt, \
      Stg_ComponentFactory_GetUnsignedIntFunc*                                 getUnsignedInt, \
      Stg_ComponentFactory_GetBoolFunc*                                               getBool, \
      Stg_ComponentFactory_GetStringFunc*                                           getString, \
      Stg_ComponentFactory_GetRootDictDoubleFunc*                           getRootDictDouble, \
      Stg_ComponentFactory_GetRootDictIntFunc*                                 getRootDictInt, \
      Stg_ComponentFactory_GetRootDictUnsignedIntFunc*                 getRootDictUnsignedInt, \
      Stg_ComponentFactory_GetRootDictBoolFunc*                               getRootDictBool, \
      Stg_ComponentFactory_GetRootDictStringFunc*                           getRootDictString, \
      Stg_ComponentFactory_ConstructByNameFunc*                               constructByName, \
      Stg_ComponentFactory_ConstructByKeyFunc*                                 constructByKey, \
      Stg_ComponentFactory_ConstructByNameWithKeyFallbackFunc* constructByNameWithKeyFallback, \
      Stg_ComponentFactory_ConstructByListFunc*                               constructByList, \
      Dictionary*                                                                    rootDict, \
      Dictionary*                                                               componentDict
   
   #define STG_COMPONENTFACTORY_PASSARGS \
      STG_CLASS_PASSARGS, \
      getDouble, \
      getInt, \
      getUnsignedInt, \
      getBool, \
      getString, \
      getRootDictDouble, \
      getRootDictInt, \
      getRootDictUnsignedInt, \
      getRootDictBool, \
      getRootDictString, \
      constructByName, \
      constructByKey, \
      constructByNameWithKeyFallback, \
      constructByList, \
      rootDict, \
      componentDict                 
   
   Stg_ComponentFactory* _Stg_ComponentFactory_New( STG_COMPONENTFACTORY_DEFARGS );
   
   Stg_ComponentFactory* Stg_ComponentFactory_New( Dictionary* rootDict, Dictionary* componentDict );

   void _Stg_ComponentFactory_Init( Stg_ComponentFactory *self );
   
   void Stg_ComponentFactory_Init( Stg_ComponentFactory *self );

   /*-----------------------------------------------------------------------------------------------------------------
   ** Virtual functions
   */

   void _Stg_ComponentFactory_Delete( void* Stg_ComponentFactory );
   
   void _Stg_ComponentFactory_Print( void* Stg_ComponentFactory, Stream* stream );
   
   /*-----------------------------------------------------------------------------------------------------------------
   ** Functions
   */
   
   void Stg_ComponentFactory_SetComponentDictionary( Stg_ComponentFactory *self, Dictionary *dict );

   void Stg_ComponentFactory_CreateComponents( Stg_ComponentFactory *self );
   
   void Stg_ComponentFactory_ConstructComponents( Stg_ComponentFactory* self, void* data );

   void Stg_ComponentFactory_BuildComponents( Stg_ComponentFactory* self, void* data );

   void Stg_ComponentFactory_InitialiseComponents( Stg_ComponentFactory* self, void* data );

   void Stg_ComponentFactory_ExecuteComponents( Stg_ComponentFactory* self, void* data );

   void Stg_ComponentFactory_DestroyComponents( Stg_ComponentFactory* self, void* data );
   
   Dictionary_Entry_Value* _Stg_ComponentFactory_GetDictionaryValue( 
      void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) ;

   /** Function to get *numerical* values from component's dictionary
    * if the key in the sub-dictionary gives a string then it assumes 
    * that this is a key that points to the root dictionary */
   Dictionary_Entry_Value* _Stg_ComponentFactory_GetNumericalValue( void* cf, Name componentName, Dictionary_Entry_Key key, Dictionary_Entry_Value* defaultVal ) ;

   double Stg_ComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, const double defaultVal ) ;
   double _Stg_ComponentFactory_GetDouble( void* cf, Name componentName, Dictionary_Entry_Key key, const double defaultVal ) ;
   int Stg_ComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, const int defaultVal ) ;
   int _Stg_ComponentFactory_GetInt( void* cf, Name componentName, Dictionary_Entry_Key key, const int defaultVal ) ;
   unsigned int Stg_ComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, const unsigned int defaultVal);
   unsigned int _Stg_ComponentFactory_GetUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, const unsigned int defaultVal);
   Bool Stg_ComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, const Bool defaultVal ) ;
   Bool _Stg_ComponentFactory_GetBool( void* cf, Name componentName, Dictionary_Entry_Key key, const Bool defaultVal ) ;
   char* Stg_ComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) ;
   char* _Stg_ComponentFactory_GetString( void* cf, Name componentName, Dictionary_Entry_Key key, const char* const defaultVal ) ;

   /* Functions for getting optional values instead using defaults, return False if not found */
   Bool Stg_ComponentFactory_TryDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double* value );
   Bool Stg_ComponentFactory_TryInt( void* cf, Name componentName, Dictionary_Entry_Key key, int* value );
   Bool Stg_ComponentFactory_TryUnsignedInt( void* cf, Name componentName, Dictionary_Entry_Key key, unsigned int* value );
   Bool Stg_ComponentFactory_TryBool( void* cf, Name componentName, Dictionary_Entry_Key key, Bool* value );

   void Stg_ComponentFactory_GetRequiredDouble( void* cf, Name componentName, Dictionary_Entry_Key key, double* value );

   double Stg_ComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) ;
   double _Stg_ComponentFactory_GetRootDictDouble( void* cf, Dictionary_Entry_Key key, const double defaultVal ) ;
   int Stg_ComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) ;
   int _Stg_ComponentFactory_GetRootDictInt( void* cf, Dictionary_Entry_Key key, const int defaultVal ) ;
   unsigned int Stg_ComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal);
   unsigned int _Stg_ComponentFactory_GetRootDictUnsignedInt( void* cf, Dictionary_Entry_Key key, const unsigned int defaultVal);
   Bool Stg_ComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) ;
   Bool _Stg_ComponentFactory_GetRootDictBool( void* cf, Dictionary_Entry_Key key, const Bool defaultVal ) ;
   char* Stg_ComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* defaultVal ) ;
   char* _Stg_ComponentFactory_GetRootDictString( void* cf, Dictionary_Entry_Key key, const char* defaultVal ) ;

   #define Stg_ComponentFactory_ConstructByName( cf, componentName, type, isEssential, data ) \
      (type*) ( (Stg_ComponentFactory*) cf )->constructByName( cf, componentName, type ## _Type, isEssential, data )
   Stg_Component* _Stg_ComponentFactory_ConstructByName( void* cf, Name componentName, Type type, Bool isEssential, void* d );
   
   #define Stg_ComponentFactory_ConstructByKey( cf, parentComponentName, componentKey, type, isEssential, data ) \
      (type*) ( (Stg_ComponentFactory*) cf )->constructByKey( \
         cf, parentComponentName, componentKey, type ## _Type, isEssential, data )
   Stg_Component* _Stg_ComponentFactory_ConstructByKey( 
      void*                cf, 
      Name                 parentComponentName, 
      Dictionary_Entry_Key componentKey, 
      Type                 type, 
      Bool                 isEssential,
      void*                data );

   #define Stg_ComponentFactory_PluginConstructByKey( cf, codelet, componentKey, type, isEssential, data ) \
      (type*) _Stg_ComponentFactory_PluginConstructByKey( \
         cf, codelet, componentKey, type ## _Type, isEssential, data )
   Stg_Component* _Stg_ComponentFactory_PluginConstructByKey( 
      void*                cf, 
      void*                codelet, 
      Dictionary_Entry_Key componentKey, 
      Type                 type, 
      Bool                 isEssential,
      void*                data );

   double Stg_ComponentFactory_PluginGetUnsignedInt( void* cf, void *codelet, Dictionary_Entry_Key key, double defaultVal );

   double Stg_ComponentFactory_PluginGetDouble( void* cf, void *codelet, Dictionary_Entry_Key key, double defaultVal );

   int Stg_ComponentFactory_PluginGetInt( void* cf, void *codelet, Dictionary_Entry_Key key, int defaultVal );

   Bool Stg_ComponentFactory_PluginGetBool( void* cf, void *codelet, Dictionary_Entry_Key key, Bool defaultVal );

   char* Stg_ComponentFactory_PluginGetString( void* cf, void* codelet, Dictionary_Entry_Key key, const char* const defaultVal );

   #define Stg_ComponentFactory_ConstructByNameWithKeyFallback( \
      cf, parentComponentName, componentTrialName, componentFallbackKey, type, isEssential, data ) \
      (type*) ( (Stg_ComponentFactory*) cf )->constructByNameWithKeyFallback( \
         cf, parentComponentName, componentTrialName, componentFallbackKey, type ## _Type, isEssential, data )
   Stg_Component* _Stg_ComponentFactory_ConstructByNameWithKeyFallback( 
      void*                cf, 
      Name                 parentComponentName, 
      Name                 componentTrialName, 
      Dictionary_Entry_Key componentTrialKey, 
      Type                 type, 
      Bool                 isEssential,
      void*                data );

   #define Stg_ComponentFactory_ConstructByList( \
      cf, parentComponentName, listName, maxComponents, type, isEssential, componentCount, data ) \
      (type**) ( (Stg_ComponentFactory*) cf )->constructByList( \
         cf, parentComponentName, listName, maxComponents, type ## _Type, isEssential, componentCount, data )

   Stg_Component** _Stg_ComponentFactory_ConstructByList( 
      void*         cf, 
      Name          parentComponentName, 
      Name          listName, 
      unsigned int  maxComponents,
      Type          type,
      Bool          isEssential,
      unsigned int* componentCount,
      void*         data );

   #define Stg_ComponentFactory_Unlimited ((unsigned int) -1)

#endif /* __StGermain_Base_Automation_Stg_ComponentFactory_h__ */

