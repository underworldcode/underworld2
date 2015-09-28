/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <StGermain/StGermain.h>

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>
#include <StgDomain/Utils/Utils.h>

#include "types.h"

#include "Swarm_Register.h"

#include "ParticleLayout.h"
#include "SwarmClass.h"
#include "SwarmDump.h"

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


void Swarm_Register_SaveAllRegisteredSwarms( Swarm_Register* self, void* context ) {
	Index                     swarmCount = self->swarmList->count;
	Swarm**                   swarmList = NULL;
	Index                     swarm_I;
	Index                     swarmsToDumpCount = 0;
	SwarmDump*                swarmDumper;
	Swarm*                    swarm;
	Stream*                   info = Journal_Register( Info_Type, (Name)self->type );

	if ( swarmCount == 0  ) {
		return;
	}	
	
	Journal_Printf( info, "In %s(): about to save the swarms to disk:\n", __func__ );

	swarmList = Memory_Alloc_Array_Unnamed( Swarm*, swarmCount ); 

	for ( swarm_I=0 ; swarm_I < swarmCount; swarm_I++ ) {
		swarm = Swarm_Register_At( self, swarm_I );
		if ( swarm->isSwarmTypeToCheckPointAndReload == True ) {
			swarmList[swarmsToDumpCount++] = swarm;
		}
	}
	
	if ( swarmsToDumpCount == 0 ) {
		Journal_Printf( info, "found 0 swarms that need to be checkpointed -> nothing to do\n",
			swarmsToDumpCount );
	}
	else {
		Journal_Printf( info, "\t(found %u swarms that need to be checkpointed)\n",
			swarmsToDumpCount );

		/* Create new SwarmDump object to check point the swarms
		 * We're not passing in the 'context' as the second argument because we don't need the swarm dumper to 
		 * add a hook on the Save entrypoint - contexts can manually execute this function */ 
		swarmDumper = SwarmDump_New( "swarmDumper", NULL, swarmList, swarmsToDumpCount, True, True, False );
		/* Passing the context through allows the swarmDumper to check requested strings etc */
		SwarmDump_Execute( swarmDumper, context );
		Stg_Class_Delete( swarmDumper );
	}

	Memory_Free( swarmList );
	Journal_Printf( info, "%s: saving of swarms completed.\n", __func__ );
}



