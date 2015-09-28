/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>

#include "pcu/pcu.h"
#include "StGermain/Base/Foundation/Foundation.h"
#include "StGermain/Base/IO/IO.h"
#include "StGermain/Base/Container/Container.h"
#include "StGermain/Base/Automation/Automation.h"
#include "ComponentCopySuite.h"

/****************************************************************************/

/*
 * A
 * |
 * +------>C
 * |       |
 * +-->B<--+
 */

struct Stg_ComponentA;
struct Stg_ComponentB;
struct Stg_ComponentC;

typedef struct Stg_ComponentA Stg_ComponentA;
typedef struct Stg_ComponentB Stg_ComponentB;
typedef struct Stg_ComponentC Stg_ComponentC;

Type Stg_ComponentA_Type = "Stg_ComponentA";
Type Stg_ComponentB_Type = "Stg_ComponentB";
Type Stg_ComponentC_Type = "Stg_ComponentC";

#define __Stg_ComponentA \
	__Stg_Component \
	Stg_ComponentB* b; \
	Stg_ComponentC* c;
struct Stg_ComponentA { __Stg_ComponentA };

void* Stg_ComponentA_NewDefault( Name name );
void Stg_ComponentA_Delete( void* _class );
void Stg_ComponentA_Print( void* _class, Stream* s );
void* Stg_ComponentA_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void Stg_ComponentA_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data );
void Stg_ComponentA_Build( void* component, void* data );
void Stg_ComponentA_Initialise( void* component, void* data );
void Stg_ComponentA_Execute( void* component, void* data );
void Stg_ComponentA_Destroy( void* component, void* data );

void* Stg_ComponentA_NewDefault( Name name )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Stg_ComponentA);
	Type                                                      type = "Stg_ComponentA";
	Stg_Class_DeleteFunction*                              _delete = Stg_ComponentA_Delete;
	Stg_Class_PrintFunction*                                _print = Stg_ComponentA_Print;
	Stg_Class_CopyFunction*                                  _copy = Stg_ComponentA_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = Stg_ComponentA_NewDefault;
	Stg_Component_ConstructFunction*                    _construct = Stg_ComponentA_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = Stg_ComponentA_Build;
	Stg_Component_InitialiseFunction*                  _initialise = Stg_ComponentA_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = Stg_ComponentA_Execute;
	Stg_Component_DestroyFunction*                        _destroy = Stg_ComponentA_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	return _Stg_Component_New(  STG_COMPONENT_PASSARGS  );
}
void Stg_ComponentA_Delete( void* _class ) {
	Stg_ComponentA* self = (Stg_ComponentA*)_class;
	
	_Stg_Component_Delete( self );
}
void Stg_ComponentA_Print( void* _class, Stream* s ) {
	Stg_ComponentA* self = (Stg_ComponentA*)_class;

	Journal_Printf(
		s,
		"%s %s,\n", self->name, self->type );
	Stg_Class_Print( self->b, s );
	Stg_Class_Print( self->c, s );
}
void* Stg_ComponentA_Copy( void* _class, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stg_ComponentA* self = (Stg_ComponentA*)_class;
	Stg_ComponentA* newCopy;
	
	newCopy = (Stg_ComponentA*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newCopy );
	
	newCopy->b = (Stg_ComponentB*)Stg_Class_Copy( self->b, dest, deep, nameExt, ptrMap );
	newCopy->c = (Stg_ComponentC*)Stg_Class_Copy( self->c, dest, deep, nameExt, ptrMap );
	
	return newCopy;
}
void Stg_ComponentA_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	Stg_ComponentA* self = (Stg_ComponentA*)component;

	self->b =  Stg_ComponentFactory_ConstructByName( cf, (Name)"b", Stg_ComponentB, True, data   ) ;
	self->c =  Stg_ComponentFactory_ConstructByName( cf, (Name)"c", Stg_ComponentC, True, data   ) ;
}
void Stg_ComponentA_Build( void* component, void* data ) {
	Stg_ComponentA* self = (Stg_ComponentA*)component;
	
	Stg_Component_Build( self->b, data, False );
	Stg_Component_Build( self->c, data, False );
}
void Stg_ComponentA_Initialise( void* component, void* data ) {

}
void Stg_ComponentA_Execute( void* component, void* data ) {

}
void Stg_ComponentA_Destroy( void* component, void* data ) {
}

#define __Stg_ComponentB \
	__Stg_Component
struct Stg_ComponentB { __Stg_ComponentB };

void* Stg_ComponentB_NewDefault( Name name );
void Stg_ComponentB_Delete( void* _class );
void Stg_ComponentB_Print( void* _class, Stream* s );
void* Stg_ComponentB_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void Stg_ComponentB_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data );
void Stg_ComponentB_Build( void* component, void* data );
void Stg_ComponentB_Initialise( void* component, void* data );
void Stg_ComponentB_Execute( void* component, void* data );
void Stg_ComponentB_Destroy( void* component, void* data );

void* Stg_ComponentB_NewDefault( Name name )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Stg_ComponentB);
	Type                                                      type = "Stg_ComponentB";
	Stg_Class_DeleteFunction*                              _delete = Stg_ComponentB_Delete;
	Stg_Class_PrintFunction*                                _print = Stg_ComponentB_Print;
	Stg_Class_CopyFunction*                                  _copy = Stg_ComponentB_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = Stg_ComponentB_NewDefault;
	Stg_Component_ConstructFunction*                    _construct = Stg_ComponentB_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = Stg_ComponentB_Build;
	Stg_Component_InitialiseFunction*                  _initialise = Stg_ComponentB_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = Stg_ComponentB_Execute;
	Stg_Component_DestroyFunction*                        _destroy = Stg_ComponentB_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	return _Stg_Component_New(  STG_COMPONENT_PASSARGS  );
}
void Stg_ComponentB_Delete( void* _class ) {
	Stg_ComponentB* self = (Stg_ComponentB*)_class;
	
	_Stg_Component_Delete( self );
}
void Stg_ComponentB_Print( void* _class, Stream* s ) {
	Stg_ComponentB* self = (Stg_ComponentB*)_class;

	Journal_Printf(
		s,
		"%s %s,\n", self->name, self->type );
}
void* Stg_ComponentB_Copy( void* _class, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stg_ComponentB* self = (Stg_ComponentB*)_class;
	Stg_ComponentB* newCopy;
	
	newCopy = (Stg_ComponentB*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newCopy );
	
	return newCopy;
}
void Stg_ComponentB_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
}
void Stg_ComponentB_Build( void* component, void* data ) {
}
void Stg_ComponentB_Initialise( void* component, void* data ) {
}
void Stg_ComponentB_Execute( void* component, void* data ) {
}
void Stg_ComponentB_Destroy( void* component, void* data ) {
}


#define __Stg_ComponentC \
	__Stg_Component \
	Stg_ComponentB* b;
struct Stg_ComponentC { __Stg_ComponentC };

void* Stg_ComponentC_NewDefault( Name name );
void Stg_ComponentC_Delete( void* _class );
void Stg_ComponentC_Print( void* _class, Stream* s );
void* Stg_ComponentC_Copy( void* self, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
void Stg_ComponentC_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data );
void Stg_ComponentC_Build( void* component, void* data );
void Stg_ComponentC_Initialise( void* component, void* data );
void Stg_ComponentC_Execute( void* component, void* data );
void Stg_ComponentC_Destroy( void* component, void* data );

void* Stg_ComponentC_NewDefault( Name name )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Stg_ComponentC);
	Type                                                      type = "Stg_ComponentC";
	Stg_Class_DeleteFunction*                              _delete = Stg_ComponentC_Delete;
	Stg_Class_PrintFunction*                                _print = Stg_ComponentC_Print;
	Stg_Class_CopyFunction*                                  _copy = Stg_ComponentC_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = Stg_ComponentC_NewDefault;
	Stg_Component_ConstructFunction*                    _construct = Stg_ComponentC_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = Stg_ComponentC_Build;
	Stg_Component_InitialiseFunction*                  _initialise = Stg_ComponentC_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = Stg_ComponentC_Execute;
	Stg_Component_DestroyFunction*                        _destroy = Stg_ComponentC_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;

	return _Stg_Component_New(  STG_COMPONENT_PASSARGS  );
}
void Stg_ComponentC_Delete( void* _class ) {
	Stg_ComponentC* self = (Stg_ComponentC*)_class;
	
	_Stg_Component_Delete( self );
}
void Stg_ComponentC_Print( void* _class, Stream* s ) {
	Stg_ComponentC* self = (Stg_ComponentC*)_class;

	Journal_Printf(
		s,
		"%s %s,\n", self->name, self->type );
	Stg_Class_Print( self->b, s );
}
void* Stg_ComponentC_Copy( void* _class, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	Stg_ComponentC* self = (Stg_ComponentC*)_class;
	Stg_ComponentC* newCopy;
	
	newCopy = (Stg_ComponentC*)_Stg_Component_Copy( self, dest, deep, nameExt, ptrMap );
	PtrMap_Append( ptrMap, self, newCopy );

	newCopy->b = (Stg_ComponentB*)Stg_Class_Copy( self->b, dest, deep, nameExt, ptrMap );

	return newCopy;
}
void Stg_ComponentC_AssignFromXML( void* component, Stg_ComponentFactory* cf, void* data ) {
	Stg_ComponentC* self = (Stg_ComponentC*)component;

	self->b =  Stg_ComponentFactory_ConstructByName( cf, (Name)"b", Stg_ComponentB, True, data   ) ;
}
void Stg_ComponentC_Build( void* component, void* data ) {
	Stg_ComponentC* self = (Stg_ComponentC*)component;
	
	Stg_Component_Build( self->b, data, False );
}
void Stg_ComponentC_Initialise( void* component, void* data ) {

}
void Stg_ComponentC_Execute( void* component, void* data ) {

}
void Stg_ComponentC_Destroy( void* component, void* data ) {

}

/****************************************************************************/

typedef struct {
   Dictionary*    dict;
} ComponentCopySuiteData;


void ComponentCopySuite_Setup( ComponentCopySuiteData* data ) {
   data->dict = Dictionary_New();
}


void ComponentCopySuite_Teardown( ComponentCopySuiteData* data ) {
   Stg_Class_Delete( data->dict );
   LiveComponentRegister_DestroyAll( LiveComponentRegister_GetLiveComponentRegister() );
   Stg_Class_Delete( LiveComponentRegister_Get(LiveComponentRegister_GetLiveComponentRegister(), "a" ));
   Stg_Class_Delete( LiveComponentRegister_Get(LiveComponentRegister_GetLiveComponentRegister(), "b" ));
   Stg_Class_Delete( LiveComponentRegister_Get(LiveComponentRegister_GetLiveComponentRegister(), "c" ));
   LiveComponentRegister_Delete();
}
   

void ComponentCopySuite_TestCopy( ComponentCopySuiteData* data ) {
   Stg_ComponentA* a=NULL;
   Stg_ComponentA* aCopy=NULL;
   Dictionary* componentsDictionary=NULL;
   Dictionary* componentDict=NULL;

   Stg_ComponentFactory* cf;

   Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Stg_ComponentA_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)Stg_ComponentA_NewDefault );

   Stg_ComponentRegister_Add( 
      Stg_ComponentRegister_Get_ComponentRegister( ), 
      Stg_ComponentB_Type,
      "0",
      (Stg_Component_DefaultConstructorFunction*)Stg_ComponentB_NewDefault );

   Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Stg_ComponentC_Type, (Name)"0", (Stg_Component_DefaultConstructorFunction*)Stg_ComponentC_NewDefault );

   /* Creating a dictionary of components */
   componentsDictionary = Dictionary_New();
   componentDict = Dictionary_New( );
   Dictionary_AddFromString( componentDict, "Type", "Stg_ComponentA" );
   Dictionary_AddFromDictionary( componentsDictionary, "a", componentDict );
   componentDict = Dictionary_New();
   Dictionary_AddFromString( componentDict, "Type", "Stg_ComponentB" );
   Dictionary_AddFromDictionary( componentsDictionary, "b", componentDict );
   componentDict = Dictionary_New();
   Dictionary_AddFromString( componentDict, "Type", "Stg_ComponentC" );
   Dictionary_AddFromDictionary( componentsDictionary, "c", componentDict );

   Dictionary_AddFromDictionary( data->dict, "components", componentsDictionary );

   cf = Stg_ComponentFactory_New( data->dict, componentsDictionary );
   Stream_Enable( cf->infoStream, False );

   Stg_ComponentFactory_CreateComponents( cf );
   Stg_ComponentFactory_ConstructComponents( cf, 0 /* dummy */ );

   a = (Stg_ComponentA*)LiveComponentRegister_Get( cf->LCRegister, (Name)"a" );
   
   /* "Stg_Components creation" */
   pcu_check_true(
      a != NULL &&
      a->b != NULL &&
      a->c != NULL &&
      a->b == a->c->b  );

   aCopy = (Stg_ComponentA*)Stg_Class_Copy( a, NULL, True, "_dup", NULL );

   /*   "Can we copy the components and subcomponents correctly" */
   pcu_check_true(
      aCopy != NULL &&
      aCopy->b != NULL &&
      aCopy->c != NULL &&
      aCopy->b == aCopy->c->b );
}


void ComponentCopySuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, ComponentCopySuiteData );
   pcu_suite_setFixtures( suite, ComponentCopySuite_Setup, ComponentCopySuite_Teardown );
   pcu_suite_addTest( suite, ComponentCopySuite_TestCopy );
}


