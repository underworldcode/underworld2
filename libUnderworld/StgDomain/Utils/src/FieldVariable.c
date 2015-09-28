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

#include "types.h"
#include "FieldVariable.h"
#include "DomainContext.h"
#include "FieldVariable_Register.h"

#include <assert.h>
#include <string.h>

const Type FieldVariable_Type = "FieldVariable";

const char* InterpolationResultToStringMap[4] = { "OTHER_PROC", "LOCAL", "SHADOW", "OUTSIDE_GLOBAL" };

FieldVariable* FieldVariable_New( 
   Name                    name,
   DomainContext*          context,
   Index                   fieldComponentCount,
   Dimension_Index         dim,
   Bool                    isCheckpointedAndReloaded,
   MPI_Comm                communicator,
   FieldVariable_Register* fieldVariable_Register ) 
{
   FieldVariable* self = _FieldVariable_DefaultNew( name );

   self->isConstructed = True;

   _FieldVariable_Init(
      self,
      context,
      fieldComponentCount,
      dim,
      isCheckpointedAndReloaded,
      NULL,
      communicator,
      fieldVariable_Register,
      False );

   return self;
}

FieldVariable* _FieldVariable_DefaultNew( Name name ) {
   /* Variables set in this function */
   SizeT                                                      _sizeOfSelf = sizeof(FieldVariable);
   Type                                                              type = FieldVariable_Type;
   Stg_Class_DeleteFunction*                                      _delete = _FieldVariable_Delete;
   Stg_Class_PrintFunction*                                        _print = _FieldVariable_Print;
   Stg_Class_CopyFunction*                                          _copy = _FieldVariable_Copy;
   Stg_Component_DefaultConstructorFunction*          _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_FieldVariable_DefaultNew;
   Stg_Component_ConstructFunction*                            _construct = _FieldVariable_AssignFromXML;
   Stg_Component_BuildFunction*                                    _build = _FieldVariable_Build;
   Stg_Component_InitialiseFunction*                          _initialise = _FieldVariable_Initialise;
   Stg_Component_ExecuteFunction*                                _execute = _FieldVariable_Execute;
   Stg_Component_DestroyFunction*                                _destroy = _FieldVariable_Destroy;
   AllocationType                                      nameAllocationType = NON_GLOBAL;
   FieldVariable_InterpolateValueAtFunction*          _interpolateValueAt = NULL;
   FieldVariable_GetValueFunction*            _getMinGlobalFieldMagnitude = NULL;
   FieldVariable_GetValueFunction*            _getMaxGlobalFieldMagnitude = NULL;
   FieldVariable_CacheValuesFunction*    _cacheMinMaxGlobalFieldMagnitude = NULL;
   FieldVariable_GetCoordFunction*               _getMinAndMaxLocalCoords = NULL;
   FieldVariable_GetCoordFunction*              _getMinAndMaxGlobalCoords = NULL;

   return _FieldVariable_New( FIELDVARIABLE_PASSARGS );
}

FieldVariable* _FieldVariable_New( FIELDVARIABLE_DEFARGS ) {
   FieldVariable* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(FieldVariable) );
   self = (FieldVariable*)_Stg_Component_New( STG_COMPONENT_PASSARGS );
   
   /* Virtual functions */
   self->_interpolateValueAt              = _interpolateValueAt;
   self->_getMinGlobalFieldMagnitude      = _getMinGlobalFieldMagnitude;
   self->_getMaxGlobalFieldMagnitude      = _getMaxGlobalFieldMagnitude;
   self->_cacheMinMaxGlobalFieldMagnitude = _cacheMinMaxGlobalFieldMagnitude;
   self->_getMinAndMaxLocalCoords         = _getMinAndMaxLocalCoords;
   self->_getMinAndMaxGlobalCoords        = _getMinAndMaxGlobalCoords;

   /* General info */

   /* FieldVariable info */
   self->o_units = NULL;
   
   return self;
}

void _FieldVariable_Delete( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*) fieldVariable;

   _Stg_Component_Delete( self );
}

void _FieldVariable_Print( void* _fieldVariable, Stream* stream ) {
   FieldVariable* self = (FieldVariable*) _fieldVariable;

   Journal_Printf( stream, "FieldVariable - '%s'\n", self->name );
   Stream_Indent( stream );
   _Stg_Component_Print( self, stream );

   Journal_PrintPointer( stream, self->_interpolateValueAt );
   Journal_PrintPointer( stream, self->_getMinGlobalFieldMagnitude );
   Journal_PrintPointer( stream, self->_getMaxGlobalFieldMagnitude );
   Journal_PrintPointer( stream, self->_cacheMinMaxGlobalFieldMagnitude );
   Journal_PrintPointer( stream, self->_getMinAndMaxLocalCoords );
   Journal_PrintPointer( stream, self->_getMinAndMaxGlobalCoords );

   Journal_PrintValue( stream, self->fieldComponentCount );
   Journal_PrintValue( stream, self->dim );
   Journal_PrintBool( stream, self->isCheckpointedAndReloaded);
   #ifdef LAM_MPI
      Journal_PrintPointer( stream, self->communicator );
   #elif  defined( OPEN_MPI )
      Journal_PrintPointer( stream, self->communicator );
   #else
      Journal_PrintValue( stream, self->communicator );
   #endif
   Journal_PrintPointer( stream, self->fieldVariable_Register );
   Stream_UnIndent( stream );
}

void _FieldVariable_Init( 
   FieldVariable*          self, 
   DomainContext*          context,
   Index                   fieldComponentCount, 
   Dimension_Index         dim,
   Bool                    isCheckpointedAndReloaded,
   char*                   o_units,
   MPI_Comm                communicator, 
   FieldVariable_Register* fV_Register,
   Bool                    useCacheMaxMin ) {
   /* Add ourselves to the register for later retrieval by clients */
   
   self->context                   = context;
   self->fieldComponentCount       = fieldComponentCount;
   self->dim                       = dim;
   self->communicator              = communicator;
   self->fieldVariable_Register    = fV_Register;
   self->isCheckpointedAndReloaded = isCheckpointedAndReloaded;
   self->useCacheMaxMin            = useCacheMaxMin;

   if( self != NULL && fV_Register != NULL ) {   
      /* Prevent the same field from being added more than once */
      if( NamedObject_Register_GetIndex( fV_Register, self->name ) == -1 )
         FieldVariable_Register_Add( fV_Register, self );
   }   

   self->extensionMgr = ExtensionManager_New_OfExistingObject( self->name, self );
   self->cachedTimestep = -1;
}

void* _FieldVariable_Copy( void* fieldVariable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;
   FieldVariable* newFieldVariable;
   PtrMap*        map = ptrMap;
   Bool           ownMap = False;
   
   if( !map ) {
      map = PtrMap_New( 10 );
      ownMap = True;
   }
   
   newFieldVariable = _Stg_Component_Copy( self, dest, deep, nameExt, map );
   newFieldVariable->_interpolateValueAt       = self->_interpolateValueAt;
   newFieldVariable->_getMinAndMaxLocalCoords  = self->_getMinAndMaxLocalCoords;
   newFieldVariable->_getMinAndMaxGlobalCoords = self->_getMinAndMaxGlobalCoords;
   newFieldVariable->fieldComponentCount       = self->fieldComponentCount;
   newFieldVariable->dim                       = self->dim;
   newFieldVariable->isCheckpointedAndReloaded = self->isCheckpointedAndReloaded;
   newFieldVariable->communicator              = self->communicator;
   newFieldVariable->fieldVariable_Register    = self->fieldVariable_Register;
   newFieldVariable->extensionMgr              = Stg_Class_Copy( self->extensionMgr, NULL, deep, nameExt, map );
   
   if( ownMap ) {
      Stg_Class_Delete( map );
   }
            
   return (void*)newFieldVariable;
}

void _FieldVariable_AssignFromXML( void* fieldVariable, Stg_ComponentFactory* cf, void* data ) {
   FieldVariable*          self = (FieldVariable*)fieldVariable;
   FieldVariable_Register* fV_Register=NULL;
   Dimension_Index         dim;
   Index                   fieldComponentCount;
   Bool                    isCheckpointedAndReloaded, isCheckpointedAndReloaded2;
   Dictionary_Entry_Value* feVarsList = NULL;
   char*                   o_units = NULL;
   DomainContext*          context = NULL;
   Bool                    useCacheMaxMin;

   context = Stg_ComponentFactory_ConstructByKey(
      cf,
      self->name,
      (Dictionary_Entry_Key)"Context",
      DomainContext,
      False,
      data );

   if( !context )
      context = Stg_ComponentFactory_ConstructByName( cf, (Name)"context", DomainContext, False, data );
   
   if (context) {
      fV_Register = context->fieldVariable_Register;
      assert( fV_Register );
   }

   dim = Stg_ComponentFactory_GetRootDictUnsignedInt( cf, (Dictionary_Entry_Key)"dim", 0 );
   /* allow this to be overwritten by the component dub dict */
   dim = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, (Dictionary_Entry_Key)"dim", dim );

   fieldComponentCount = Stg_ComponentFactory_GetUnsignedInt(
      cf,
      self->name,
      (Dictionary_Entry_Key)"fieldComponentCount",
      0 );

   useCacheMaxMin = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"useCacheMaxMin", False );
   
   o_units = Stg_ComponentFactory_GetString( cf, self->name, (Dictionary_Entry_Key)"outputUnits", NULL );

   /* 
    * Decide whether this FieldVariable will be checkpointed & reloaded, based on the dictionary list 
    * "fieldVariableToCheckpoint". NB may want to put this in the XML component definintion of a
    * FieldVariable itself, but for now prefer list so it can be centrally set.
    * -- Pat, Jules, Kath - 29 November 2006 
    */

   isCheckpointedAndReloaded2 = Stg_ComponentFactory_GetBool( cf, self->name, (Dictionary_Entry_Key)"isCheckpointedAndReloaded", False );

   /* Case insensitive search */
   feVarsList = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)"fieldVariablesToCheckpoint" );
   if( NULL == feVarsList ) {
      feVarsList = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)"FieldVariablesToCheckpoint" );
   }

   if( feVarsList != NULL ) {
      Index                   listLength = Dictionary_Entry_Value_GetCount( feVarsList );
      Index                   var_I = 0;
      Dictionary_Entry_Value* feVarDictValue = NULL;
      char*                   fieldVariableName;
   
      /* if the 'fieldVariablesToCheckpoint' list is empty (but exists)
       *    checkpoint every field
       * else selectively checkpoint based on list entries */
      if( listLength == 0 ) { 
         isCheckpointedAndReloaded = True; 
      }
      else {
         isCheckpointedAndReloaded = False;
         for ( var_I = 0; var_I < listLength; var_I++  ) {
            feVarDictValue = Dictionary_Entry_Value_GetElement( feVarsList, var_I );
            fieldVariableName = Dictionary_Entry_Value_AsString( feVarDictValue ); 
            if ( 0 == strcmp( self->name, fieldVariableName ) ) {
               isCheckpointedAndReloaded = True;
               break;
            }
         }
      }
   }
   else {
      /* If there's no special list, just checkpoint/reload everything. */
      isCheckpointedAndReloaded = True;
   }

   isCheckpointedAndReloaded = (isCheckpointedAndReloaded || isCheckpointedAndReloaded2 );

   feVarsList = NULL;
   /* also include check to see if this fevariable should be saved for analysis purposes */ 
   feVarsList = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)"fieldVariablesToSave" );

   if( NULL == feVarsList ) {
      feVarsList = Dictionary_Get( cf->rootDict, (Dictionary_Entry_Key)"FieldVariablesToSave" );
   }
   if( feVarsList != NULL ) {
      Index                   listLength = Dictionary_Entry_Value_GetCount( feVarsList );
      Index                   var_I = 0;
      Dictionary_Entry_Value* feVarDictValue = NULL;
      char*                   fieldVariableName;
   
      for( var_I = 0; var_I < listLength; var_I++ ) {
         feVarDictValue = Dictionary_Entry_Value_GetElement( feVarsList, var_I );
         fieldVariableName = Dictionary_Entry_Value_AsString( feVarDictValue ); 

         if( 0 == strcmp( self->name, fieldVariableName ) ) {
            self->isSavedData = True;
            break;
         }
      }
   }
   _FieldVariable_Init(
      self,
      context,
      fieldComponentCount,
      dim,
      isCheckpointedAndReloaded,
      o_units,
      MPI_COMM_WORLD,
      fV_Register,
      useCacheMaxMin );

}

void _FieldVariable_Build( void* fieldVariable, void* data ) {
}

void _FieldVariable_Initialise( void* fieldVariable, void* data ) {
}

void _FieldVariable_Execute( void* fieldVariable, void* data ) {
}

void _FieldVariable_Destroy( void* fieldVariable, void* data ) {
   FieldVariable* self = (FieldVariable*) fieldVariable;

   if( self->o_units ) {
      Memory_Free( self->o_units ); 
      self->o_units=NULL;
   }

   if( self->extensionMgr ) {
      Stg_Class_Delete( self->extensionMgr );
   }
}

InterpolationResult FieldVariable_InterpolateValueAt( void* fieldVariable, Coord coord, double* value ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   return self->_interpolateValueAt( self, coord, value );
}

double FieldVariable_GetMinGlobalFieldMagnitude( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   FieldVariable_CacheMinMaxGlobalFieldMagnitude( fieldVariable );
   return self->magnitudeMin;
}

double FieldVariable_GetMaxGlobalFieldMagnitude( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   FieldVariable_CacheMinMaxGlobalFieldMagnitude( fieldVariable );
   return self->magnitudeMax;
}

double FieldVariable_GetCachedMinGlobalFieldMagnitude( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;
   Bool           originalFlag = self->useCacheMaxMin;

   self->useCacheMaxMin = True;
   FieldVariable_CacheMinMaxGlobalFieldMagnitude( fieldVariable );
   self->useCacheMaxMin = originalFlag;
   return self->magnitudeMin;
}

double FieldVariable_GetCachedMaxGlobalFieldMagnitude( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;
   Bool           originalFlag = self->useCacheMaxMin;

   self->useCacheMaxMin = True;
   FieldVariable_CacheMinMaxGlobalFieldMagnitude( fieldVariable );
   self->useCacheMaxMin = originalFlag;
   return self->magnitudeMax;
}

void FieldVariable_CacheMinMaxGlobalFieldMagnitude( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;
   int            timestep = 0;

   if( self->context )
      timestep = self->context->timeStep;

   if( self->cachedTimestep != timestep || self->useCacheMaxMin == False ) {
      self->_cacheMinMaxGlobalFieldMagnitude( self );
      self->cachedTimestep = timestep;
   } 
}

void FieldVariable_GetMinAndMaxLocalCoords( void* fieldVariable, Coord min, Coord max ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   self->_getMinAndMaxLocalCoords( self, min, max );
}

void FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord min, Coord max ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   self->_getMinAndMaxGlobalCoords( self, min, max );
}

void _FieldVariable_GetMinAndMaxGlobalCoords( void* fieldVariable, Coord globalMin, Coord globalMax ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;
   Coord          localMin, localMax;

   self->_getMinAndMaxLocalCoords( self, localMin, localMax );

   MPI_Allreduce( localMin, globalMin, self->dim, MPI_DOUBLE, MPI_MIN, self->communicator );
   MPI_Allreduce( localMax, globalMax, self->dim, MPI_DOUBLE, MPI_MAX, self->communicator );
}

void FieldVariable_SetUnits( void* fieldVariable, char* o_units ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   /* Override any previously defined o_units */
   if(self->o_units) { 
      Memory_Free( self->o_units ); 
      self->o_units = NULL;
   }

   self->o_units = StG_Strdup( o_units );
}

char* FieldVariable_GetUnits( void* fieldVariable ) {
   FieldVariable* self = (FieldVariable*)fieldVariable;

   return self->o_units;
}
