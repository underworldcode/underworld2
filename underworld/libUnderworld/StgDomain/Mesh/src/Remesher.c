/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <mpi.h>

#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>
#include <StgDomain/Mesh/src/Mesh.h>

#include "types.h"
#include "Remesher.h"


/* Textual name of this class */
const Type Remesher_Type = "Remesher";

/* Constructors */

Remesher* _Remesher_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(Remesher);
	Type                                                      type = Remesher_Type;
	Stg_Class_DeleteFunction*                              _delete = _Remesher_Delete;
	Stg_Class_PrintFunction*                                _print = _Remesher_Print;
	Stg_Class_CopyFunction*                                  _copy = NULL;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (void*(*)(Name))_Remesher_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _Remesher_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _Remesher_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _Remesher_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _Remesher_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _Remesher_Destroy;
	AllocationType                              nameAllocationType = NON_GLOBAL;
	Remesher_RemeshFunc*                                remeshFunc = NULL;

   return _Remesher_New(  REMESHER_PASSARGS  );
}

Remesher* _Remesher_New(  REMESHER_DEFARGS  ) {
   Remesher* self;

   /* Allocate memory. */
   self = (Remesher*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );

   /* Virtual functions. */
   self->remeshFunc = remeshFunc;

   return self;
}

void _Remesher_Init( void* remesher, AbstractContext* context, Mesh* mesh ) {
   Remesher*	self = (Remesher*)remesher;

   /* Remesher info */
	self->context = context;
   self->mesh = mesh;
}

/* Virtual functions */

void _Remesher_Delete( void* remesher ) {
   Remesher*	self = (Remesher*)remesher;

   /* Delete the class itself */

   /* Delete parent */
   _Stg_Component_Delete( self );
}

void _Remesher_Print( void* remesher, Stream* stream ) {
   Remesher*	self = (Remesher*)remesher;
   Stream*		myStream;
	
   /* Set the Journal for printing informations */
   myStream = Journal_Register( InfoStream_Type, (Name)"RemesherStream"  );

   /* Print parent */
   _Stg_Component_Print( self, stream );

   /* General info */
   Journal_Printf( myStream, "Remesher (ptr): (%p)\n", self );

   /* Virtual info */

   /* Remesher info */
}

void _Remesher_AssignFromXML( void* remesher, Stg_ComponentFactory* cf, void* data ) {
   Remesher*			self = (Remesher*)remesher;
	AbstractContext*	context;
	Mesh*					mesh;

   assert( self );
   assert( cf );
   assert( cf->componentDict );

   context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
   if( !context  )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, True, data  );

   mesh = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"mesh", Mesh, True, data  );

	_Remesher_Init ( self, context, mesh );
}

void _Remesher_Build( void* remesher, void* data ) {
   Remesher*	self = (Remesher*)remesher;

   assert( self );
   assert( self->mesh );

   /*Stg_Component_Build( self->remeshFunc, data, False );*/
   Stg_Component_Build( self->mesh, data, False );   
}

void _Remesher_Initialise( void* remesher, void* data ) {
   Remesher*	self = (Remesher*)remesher;

   assert( self );

   /*Stg_Component_Initialise( self->remeshFunc, data, False );*/
   Stg_Component_Initialise( self->mesh, data, False );   
}

void _Remesher_Execute( void* remesher, void* data ) {
   Remesher*	self = (Remesher*)remesher;


   assert( self );
}

void _Remesher_Destroy( void* remesher, void* data ) {
   Remesher*	self = (Remesher*)remesher;

   assert( self );

   Stg_Component_Destroy( self->remeshFunc, data, False );   
}

/* Public Functions */


