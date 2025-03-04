/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Automation_Stg_Component_h__
#define __StGermain_Base_Automation_Stg_Component_h__
   
   /* Templates of virtual functions */
   typedef void* (Stg_Component_DefaultConstructorFunction) ( Name name );
   typedef void (Stg_Component_ConstructFunction) ( void* component, Stg_ComponentFactory* cf, void* data );
   typedef void (Stg_Component_BuildFunction) ( void* component, void* data );
   typedef void (Stg_Component_InitialiseFunction) ( void* component, void* data );
   typedef void (Stg_Component_ExecuteFunction) ( void* component, void* data );
   typedef void (Stg_Component_DestroyFunction) ( void* component, void* data );

   /* Textual name of this class */
   extern const Type Stg_Component_Type;
   
   
   /* Stg_Component information */
   #define __Stg_Component \
      /* General info */ \
      __Stg_Object \
      \
      /* Virtual info */ \
      Stg_Component_DefaultConstructorFunction* _defaultConstructor; \
      Stg_Component_ConstructFunction*          _construct; \
      Stg_Component_BuildFunction*              _build; \
      Stg_Component_InitialiseFunction*         _initialise; \
      Stg_Component_ExecuteFunction*            _execute; \
      Stg_Component_DestroyFunction*            _destroy; \
      \
      /* Stg_Component info */ \
      Bool                                      isConstructed; \
      Bool                                      isBuilt; \
      Bool                                      isInitialised; \
      Bool                                      hasExecuted; \
      Bool                                      isDestroyed; \
      Type                                      constructType; \
      Type                                      buildType; \
      Type                                      initialiseType; \
      Type                                      executeType; \
      Type                                      destroyType;

   struct Stg_Component { __Stg_Component };
   
   /* No Stg_Component_New or Stg_Component_Init as this is an abstract class */
   
   /* Creation implementation */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define STG_COMPONENT_DEFARGS \
      STG_OBJECT_DEFARGS, \
      Stg_Component_DefaultConstructorFunction* _defaultConstructor, \
      Stg_Component_ConstructFunction*          _construct, \
      Stg_Component_BuildFunction*              _build, \
      Stg_Component_InitialiseFunction*         _initialise, \
      Stg_Component_ExecuteFunction*            _execute, \
      Stg_Component_DestroyFunction*            _destroy

   #define STG_COMPONENT_PASSARGS \
      STG_OBJECT_PASSARGS, \
      _defaultConstructor, \
      _construct, \
      _build, \
      _initialise, \
      _execute, \
      _destroy           

   Stg_Component* _Stg_Component_New( STG_COMPONENT_DEFARGS );
   
   /* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   /* Initialisation implementation */
   void _Stg_Component_Init( Stg_Component* self );
   
   /* Stg_Class_Delete boundary condition layout implementation */
   void _Stg_Component_Delete( void* component );
   
   /* Print boundary condition layout implementation */
   void _Stg_Component_Print( void* component, Stream* stream );
   
   void* _Stg_Component_Copy( void* component, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
   
   /* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   /* Copy the component */
   #define Stg_Component_Copy( self ) \
      (Stg_Component*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
   #define Stg_Component_DeepCopy(self) \
      (Stg_Component*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
   
   /** Contruct the component. Configure/setup the component. */
   void Stg_Component_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data, Bool force );
   
   /** Build the component: Take the configuration and instantiate the component (do all main mallocs, etc). */
   void Stg_Component_Build( void* component, void* data, Bool force );
   
   /** Initialise the component: Place in initial values. After this point the component is ready to execute. */
   void Stg_Component_Initialise( void* component, void* data, Bool force );
   
   /** Execute the component: Perform its task. */
   void Stg_Component_Execute( void* component, void* data, Bool force );
   
   /** Destroy the component: All resources used in the other phases are released. */
   void Stg_Component_Destroy( void* component, void* data, Bool force );
   
   /** Is the component constructed? (i.e. its configration/setup performed) */
   Bool Stg_Component_IsConstructed( void* component );
   
   /** Is the component built? (i.e. instantiated) */
   Bool Stg_Component_IsBuilt( void* component );
   
   /** Is the component initialised? (i.e. all initial values set) */
   Bool Stg_Component_IsInitialised( void* component );
   
   /** Has the component executed? */
   Bool Stg_Component_HasExecuted( void* component );
   
   /** Is the component destroyed? */
   Bool Stg_Component_IsDestroyed( void* component );

   void Stg_Component_SetupStreamFromDictionary( void* component, Dictionary* dictionary );

   
#endif /* __StGermain_Base_Automation_Stg_Component_h__ */
