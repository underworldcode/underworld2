/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/StGermain.h>

#include <StgDomain/Geometry/Geometry.h>
#include <StgDomain/Shape/Shape.h>
#include <StgDomain/Mesh/Mesh.h>
#include <StgDomain/Utils/Utils.h>

#include "types.h"
#include "SwarmClass.h"
#include "SwarmVariable_Register.h"
#include "SwarmVariable.h"

#include <assert.h>
#include <string.h>

const Type SwarmVariable_Type = "SwarmVariable";

SwarmVariable* SwarmVariable_New(		
   Name					name,
   AbstractContext*	context,
   Swarm*				swarm,
   Variable*			variable,
   Index			      dofCount,
   Bool              addToSwarmParticleExtension)
{
	SwarmVariable* self = (SwarmVariable*) _SwarmVariable_DefaultNew( name );

	self->isConstructed = True;
	_SwarmVariable_Init( self, context, swarm, variable, dofCount, False, addToSwarmParticleExtension );

	return self;
}

void* _SwarmVariable_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                                 _sizeOfSelf = sizeof(SwarmVariable);
	Type                                                         type = SwarmVariable_Type;
	Stg_Class_DeleteFunction*                                 _delete = _SwarmVariable_Delete;
	Stg_Class_PrintFunction*                                   _print = _SwarmVariable_Print;
	Stg_Class_CopyFunction*                                     _copy = _SwarmVariable_Copy;
	Stg_Component_DefaultConstructorFunction*     _defaultConstructor = _SwarmVariable_DefaultNew;
	Stg_Component_ConstructFunction*                       _construct = _SwarmVariable_AssignFromXML;
	Stg_Component_BuildFunction*                               _build = _SwarmVariable_Build;
	Stg_Component_InitialiseFunction*                     _initialise = _SwarmVariable_Initialise;
	Stg_Component_ExecuteFunction*                           _execute = _SwarmVariable_Execute;
	Stg_Component_DestroyFunction*                           _destroy = _SwarmVariable_Destroy;
	AllocationType                                 nameAllocationType = NON_GLOBAL;
	SwarmVariable_ValueAtFunction*                           _valueAt = NULL;
	SwarmVariable_GetGlobalValueFunction*      _getMinGlobalMagnitude = _SwarmVariable_GetMinGlobalMagnitude;
	SwarmVariable_GetGlobalValueFunction*      _getMaxGlobalMagnitude = _SwarmVariable_GetMaxGlobalMagnitude;

	return (void*) _SwarmVariable_New(  SWARMVARIABLE_PASSARGS  );
}

SwarmVariable* _SwarmVariable_New(  SWARMVARIABLE_DEFARGS  ) {
	SwarmVariable* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(SwarmVariable) );
	self = (SwarmVariable*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* Virtual functions */
	self->_valueAt						= _valueAt;
	self->_getMinGlobalMagnitude	= _getMinGlobalMagnitude;
	self->_getMaxGlobalMagnitude	= _getMaxGlobalMagnitude;

	return self;
}

void _SwarmVariable_Init( void* swarmVariable, 
                          AbstractContext* context, 
                          Swarm* swarm, 
                          Variable* variable, 
                          Index dofCount,
                          Bool useCacheMaxMin,
                          Bool addToSwarmParticleExtension )
{
	SwarmVariable* self = (SwarmVariable*) swarmVariable;
   int ii;

   self->context                     = context;
   self->swarm                       = swarm;
   self->variable                    = variable;
   self->dofCount                    = dofCount;
   self->swarmVariable_Register      = swarm->swarmVariable_Register;
   self->dim                         = swarm->dim;
   self->useCacheMaxMin              = useCacheMaxMin;
   self->addToSwarmParticleExtension = addToSwarmParticleExtension;
	
   if( variable ){  //OK: This seems dodgy but if condition removed causes DirectorSuite tests to fail...
      Swarm_AddVariable( swarm, self );
      /* also ensure variable will not allocate itself, as swarm will handle this */
      self->variable->allocateSelf  = False;
      self->variable->structSizePtr = &(self->swarm->particleExtensionMgr->finalSize);
      self->variable->arraySizePtr  = &(self->swarm->particleLocalCount);
      self->variable->arrayPtrPtr   = (void**)&(self->swarm->particles);
   }
   if ( self->swarmVariable_Register != NULL )	
      SwarmVariable_Register_Add( self->swarmVariable_Register, self );
   if ( self->addToSwarmParticleExtension && variable ){
      SizeT variableSize;
      /** first need to calculate required size */
      switch(variable->dataTypes[0])
      {
      case Variable_DataType_Char:
         variableSize = sizeof(char);
         break;
      case Variable_DataType_Short:
         variableSize = sizeof(short);
         break;
      case Variable_DataType_Int:
         variableSize = sizeof(int);
         break;
      case Variable_DataType_Float:
         variableSize = sizeof(float);
         break;
      case Variable_DataType_Double:
         variableSize = sizeof(double);
         break;
      case Variable_DataType_Pointer:
         variableSize = sizeof(void*);
         break;
      }
      /** get current particle extension size, as this will be offset where new extension will start from */
      /** and variable needs to points its offset there */
      variable->offsets[0] = ExtensionManager_GetFinalSize( swarm->particleExtensionMgr );
      /** now add extension to swarm particle extension manager */
      ExtensionManager_Add( swarm->particleExtensionMgr, (Name)variable->name, variableSize*variable->dataTypeCounts[0] );
      /** also ensure swarmvariable dofcount corresponds to variable dofcount */
      self->dofCount = variable->dataTypeCounts[0];
      if(variable->subVariablesCount > 1){
         for(ii=0; ii<variable->subVariablesCount; ii++){
            variable->components[ii]->allocateSelf  = False;
            variable->components[ii]->structSizePtr = &(self->swarm->particleExtensionMgr->finalSize);
            variable->components[ii]->arraySizePtr  = &(self->swarm->particleLocalCount);
            variable->components[ii]->arrayPtrPtr   = (void**)&(self->swarm->particles);
            variable->components[ii]->offsets[0]    = variable->offsets[0] + ii*variableSize;
         }
      }
   }
   self->cachedTimestep = -1;
}

void _SwarmVariable_Delete( void* swarmVariable ) {
	SwarmVariable* self = (SwarmVariable*) swarmVariable;

	_Stg_Component_Delete( self );
}

void _SwarmVariable_Print( void* _swarmVariable, Stream* stream ) {
	SwarmVariable* self = (SwarmVariable*) _swarmVariable;

	Journal_Printf( stream, "SwarmVariable - '%s'\n", self->name );
	Stream_Indent( stream );
	_Stg_Component_Print( self, stream );

	Journal_PrintPointer( stream, self->_valueAt );
	Journal_PrintPointer( stream, self->_getMinGlobalMagnitude );
	Journal_PrintPointer( stream, self->_getMaxGlobalMagnitude );

	Journal_Printf( stream, "Swarm - '%s'\n", self->swarm->name );
	if ( self->variable != NULL )
		Journal_Printf( stream, "Variable - '%s'\n", self->variable->name );

	Journal_PrintValue( stream, self->dofCount );
	Journal_PrintPointer( stream, self->swarmVariable_Register );
	Stream_UnIndent( stream );
}

void* _SwarmVariable_Copy( void* swarmVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	SwarmVariable*	newSwarmVariable;
	PtrMap*			map = ptrMap;
	Bool				ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newSwarmVariable = _Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	newSwarmVariable->_valueAt						= self->_valueAt;
	newSwarmVariable->_getMinGlobalMagnitude	= self->_getMinGlobalMagnitude;
	newSwarmVariable->_getMaxGlobalMagnitude	= self->_getMaxGlobalMagnitude;

	newSwarmVariable->swarm							= self->swarm;
	newSwarmVariable->variable						= self->variable;
	newSwarmVariable->dofCount						= self->dofCount;
	newSwarmVariable->swarmVariable_Register	= self->swarmVariable_Register;

	if( ownMap ) {
		Stg_Class_Delete( map );
	}

	return (void*)newSwarmVariable;
}

void _SwarmVariable_AssignFromXML( void* swarmVariable, Stg_ComponentFactory* cf, void* data ) {
	SwarmVariable*		self = (SwarmVariable*)swarmVariable;
	Swarm*				swarm;
	Variable*			variable;
	Index					dofCount;
	AbstractContext*	context;
   Bool              useCacheMaxMin, addToSwarmParticleExtension;

	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Context", AbstractContext, False, data );
	if( !context  )
		context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", AbstractContext, False, data  );

	swarm = Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Swarm", Swarm, True, data  );
	variable =  Stg_ComponentFactory_ConstructByKey( cf, self->name, (Dictionary_Entry_Key)"Variable", Variable, False, data  );
	dofCount = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dofCount", 1  );
	useCacheMaxMin = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"useCacheMaxMin", False  );
   addToSwarmParticleExtension = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"AddToSwarmParticleExtension", True  );

	_SwarmVariable_Init( self, context, swarm, variable, dofCount, useCacheMaxMin, addToSwarmParticleExtension);
}

void _SwarmVariable_Build( void* swarmVariable, void* data ) {
	SwarmVariable* self = (SwarmVariable*)swarmVariable;

	if ( self->variable )
		Stg_Component_Build( self->variable, data, False );
}

void _SwarmVariable_Initialise( void* swarmVariable, void* data ) {
	SwarmVariable* self = (SwarmVariable*)swarmVariable;

	if ( self->variable ) {
		Variable_Update( self->variable );
		Stg_Component_Initialise( self->variable, data, False );
	}
	switch( self->variable->dataTypes[0] ) {
		case Variable_DataType_Double:
			self->_valueAt = _SwarmVariable_ValueAtDouble;
			break;
		case Variable_DataType_Int:
			self->_valueAt = _SwarmVariable_ValueAtInt;
			break;
		case Variable_DataType_Float:
			self->_valueAt = _SwarmVariable_ValueAtFloat;
			break;
		case Variable_DataType_Char:
			self->_valueAt = _SwarmVariable_ValueAtChar;
			break;
		case Variable_DataType_Short:
			self->_valueAt = _SwarmVariable_ValueAtShort;
			break;
		default:
			assert(0);
	}

}

void _SwarmVariable_Execute( void* swarmVariable, void* data ) {
}

void _SwarmVariable_Destroy( void* swarmVariable, void* data ) {
	SwarmVariable* self = (SwarmVariable*)swarmVariable;
   
   Stg_Component_Destroy( self->swarm, data, False );
   if( self->variable )
		Stg_Component_Destroy(self->variable, data, False);
}

double SwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	return self->_getMinGlobalMagnitude( self );
}

double SwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	return self->_getMaxGlobalMagnitude( self );
}

/*** Default Implementations ***/
	
void _SwarmVariable_ValueAtDouble( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	double*			dataPtr;

	dataPtr = Variable_GetPtrDouble( self->variable, lParticle_I );
	memcpy( value, dataPtr, sizeof(double) * self->dofCount );
}

void _SwarmVariable_ValueAtInt( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Variable*		variable = self->variable;
   Dof_Index		dofCount = self->dofCount;
	Dof_Index		dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtInt( variable, lParticle_I, dof_I );
	}
}

void _SwarmVariable_ValueAtFloat( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Variable*		variable = self->variable;
	Dof_Index		dofCount = self->dofCount;
	Dof_Index		dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtFloat( variable, lParticle_I, dof_I );
	}
}

void _SwarmVariable_ValueAtChar( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Variable*		variable = self->variable;
	Dof_Index		dofCount = self->dofCount;
	Dof_Index		dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtChar( variable, lParticle_I, dof_I );
	}
}

void _SwarmVariable_ValueAtShort( void* swarmVariable, Particle_Index lParticle_I, double* value ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Variable*		variable = self->variable;
	Dof_Index		dofCount = self->dofCount;
	Dof_Index		dof_I;

	for ( dof_I = 0 ; dof_I < dofCount ; dof_I++ ) {
		value[ dof_I ] = (double) Variable_GetValueAtShort( variable, lParticle_I, dof_I );
	}
}

void SwarmVariable_CacheMinMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
   int timestep = 0;

   if (self->context) timestep = self->context->timeStep;
   if (self->cachedTimestep != timestep || self->useCacheMaxMin == False )
   {
      self->cachedTimestep = timestep;

      SwarmVariable_GetMinMaxGlobalMagnitude(swarmVariable );

      /* override values with results of derived class functions if provided */
      if (self->_getMinGlobalMagnitude != _SwarmVariable_GetMinGlobalMagnitude)
         self->magnitudeMin = self->_getMinGlobalMagnitude(swarmVariable);
      if (self->_getMaxGlobalMagnitude != _SwarmVariable_GetMaxGlobalMagnitude)
         self->magnitudeMax = self->_getMaxGlobalMagnitude(swarmVariable);
   } 
}

void SwarmVariable_GetMinMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	double*			value;
	Swarm*			swarm = self->swarm;
	Particle_Index	particleLocalCount = swarm->particleLocalCount;
	Particle_Index	lParticle_I;
	double			localMax = -HUGE_VAL, localMin = HUGE_VAL;
	Index				dofCount = self->dofCount;
	double			magnitude;

   value = Memory_Alloc_Array( double, dofCount, "value" );

   /* Search through all local particles and find smallest & largest value of variable */
   for ( lParticle_I = 0 ; lParticle_I < particleLocalCount ; lParticle_I++ ) {
      SwarmVariable_ValueAt( self, lParticle_I, value );

      if ( dofCount == 1 )
         magnitude = value[0];
      else 
         assert(0); /*TODO */

      if ( localMax < magnitude )
         localMax = magnitude;
      if ( localMin > magnitude )
         localMin = magnitude;
   }

   Memory_Free( value );
   /** Find upper and lower bounds on all processors */
   (void)MPI_Allreduce( &localMax, &self->magnitudeMax, dofCount, MPI_DOUBLE, MPI_MAX, swarm->comm );
   (void)MPI_Allreduce( &localMin, &self->magnitudeMin, dofCount, MPI_DOUBLE, MPI_MIN, swarm->comm );
}

double _SwarmVariable_GetMinGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
   SwarmVariable_CacheMinMaxGlobalMagnitude(swarmVariable);
   return self->magnitudeMin;
}

double _SwarmVariable_GetMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
   SwarmVariable_CacheMinMaxGlobalMagnitude(swarmVariable);
   return self->magnitudeMax;
}

double SwarmVariable_GetCachedMinGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Bool originalFlag = self->useCacheMaxMin;

	self->useCacheMaxMin = True;
   SwarmVariable_CacheMinMaxGlobalMagnitude( swarmVariable );
   self->useCacheMaxMin = originalFlag;
   return self->magnitudeMin;
}

double SwarmVariable_GetCachedMaxGlobalMagnitude( void* swarmVariable ) {
	SwarmVariable*	self = (SwarmVariable*)swarmVariable;
	Bool originalFlag = self->useCacheMaxMin;

	self->useCacheMaxMin = True;
   SwarmVariable_CacheMinMaxGlobalMagnitude( swarmVariable );
   self->useCacheMaxMin = originalFlag;
   return self->magnitudeMax;
}


