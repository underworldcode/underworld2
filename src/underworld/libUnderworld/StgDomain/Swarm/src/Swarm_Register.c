/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <StGermain/libStGermain/src/StGermain.h>

#include <StgDomain/Geometry/src/Geometry.h>
#include <StgDomain/Shape/src/Shape.h>
#include <StgDomain/Mesh/src/Mesh.h>
#include <StgDomain/Utils/src/Utils.h>

#include "types.h"

#include "Swarm_Register.h"

#include "ParticleLayout.h"
#include "SwarmClass.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


/* Textual name of this class */
const Type Swarm_Register_Type = "Swarm_Register";
Swarm_Register* stgSwarm_Register = NULL;

Swarm_Register* _Swarm_Register_New(  SWARM_REGISTER_DEFARGS  )
{
	Swarm_Register* self = NULL;

	assert( _sizeOfSelf >= sizeof(Swarm_Register) );
	self = (Swarm_Register*) _Stg_Class_New(  STG_CLASS_PASSARGS  );
	
	return self;
}
	
Swarm_Register* Swarm_Register_New(  )
{
	/* Variables set in this function */
	SizeT                      _sizeOfSelf = sizeof(Swarm_Register);
	Type                              type = Swarm_Register_Type;
	Stg_Class_DeleteFunction*      _delete = _Swarm_Register_Delete;
	Stg_Class_PrintFunction*        _print = _Swarm_Register_Print;

	Swarm_Register* self = NULL;

	/* The following terms are parameters that have been passed into or defined in this function but are being set before being passed onto the parent */
	Stg_Class_CopyFunction*  _copy = NULL;

	self = _Swarm_Register_New(  SWARM_REGISTER_PASSARGS  );

	Swarm_Register_Init( self );

	if ( !stgSwarm_Register ) {
		stgSwarm_Register = self;
	}
	return self;
}

void _Swarm_Register_Init( Swarm_Register* self )
{
}
	
void Swarm_Register_Init( Swarm_Register* self )
{
	assert( self );

	self->swarmList = Stg_ObjectList_New( );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _Swarm_Register_Delete( void* swarmRegister )
{
	Swarm_Register* self = (Swarm_Register *) swarmRegister;

	assert( self );

	/** Actually, don't delete all swarms as they have already been deleted by the
	live component register : or else leave it up to the user */
	/* Stg_ObjectList_DeleteAllObjects( self->swarmList ); */

	Stg_Class_Delete( self->swarmList );
	if ( stgSwarm_Register == self ) {
		stgSwarm_Register = NULL;
	}
	_Stg_Class_Delete( self );
}

void _Swarm_Register_Print( void* swarmRegister, Stream* stream )
{
	Swarm_Register* self = (Swarm_Register *) swarmRegister;

	assert( self );
	
	/* General info */
	Journal_Printf( (void*) stream, "Swarm_Register (ptr): %p\n", self );
	
	/* Print parent class */
	_Stg_Class_Print( self, stream );
	
	Journal_Printf( (void*) stream, "swarmList (ptr): %p\n", self->swarmList );
	Stg_Class_Print( self->swarmList, stream );
}

/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

Index Swarm_Register_Add( Swarm_Register* self, void* swarm )
{
	assert( self );
	return Stg_ObjectList_Append( self->swarmList, swarm );
}

void Swarm_Register_RemoveIndex( Swarm_Register* self, unsigned int index )
{
   int swarm_i = 0;
	assert( self );
	/* The third argument controls if the Delete phase is run or not in this function
	 * KEEP = Don't run delete
	 * DELETE = Run delete
	 */
	_Stg_ObjectList_RemoveByIndex( self->swarmList, index, KEEP );

   /* decrement each swarms own index of where it is in swarm register,
   this is done because the function _Stg_ObjectList_RemoveByIndex() memmoves
   the block of memory in the list, with out altering the values within the memory */
   for( swarm_i = index ; swarm_i < self->swarmList->count ; swarm_i++ ) {
      ((Swarm*)self->swarmList->data[swarm_i])->swarmReg_I--;
   }
}
	
Swarm* Swarm_Register_Get( Swarm_Register* self, Name name )
{
	assert( self );
	
	return (Swarm*)Stg_ObjectList_Get( self->swarmList, (Name)name  );
}

Swarm* Swarm_Register_At( void* swarmRegister, Index index ) {
	Swarm_Register* self      = (Swarm_Register *) swarmRegister;
	assert( self );

	return (Swarm*) Stg_ObjectList_At( self->swarmList, index );
}
	
unsigned int Swarm_Register_GetCount( Swarm_Register* self )
{
	assert( self );

	return self->swarmList->count;
}

Index Swarm_Register_GetIndexFromPointer( Swarm_Register* self, void* ptr ) {
	Index i;

	for ( i = 0; i < self->swarmList->count; ++i ) {
		if ( (void*)self->swarmList->data[i] == ptr ) {
			return i;
		}
	}

	return (Index)-1;
}


Swarm_Register* Swarm_Register_GetSwarm_Register() {
	return stgSwarm_Register;
}


