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
#include "Base/Automation/Automation.h"
#include "Base/Extensibility/Extensibility.h"

#include "types.h"
#include "shortcuts.h"
#include "ContextEntryPoint.h"
#include "AbstractContext.h"
#include "Variable.h"
#include "Variable_Register.h"

#define NO_IMPORT_ARRAY
#define PY_ARRAY_UNIQUE_SYMBOL stg_ARRAY_API
#include <numpy/arrayobject.h>

#if NPY_API_VERSION < 0x00000007
#define NPY_ARRAY_WRITEABLE  NPY_WRITEABLE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

/* Textual name of this class */
const Type StgVariable_Type = "Variable";


/*----------------------------------------------------------------------------------------------------------------------------------
** Constructors
*/

StgVariable* StgVariable_New(
	Name								name,
	AbstractContext*				context,
	Index								dataCount,
	SizeT*							dataOffsets,
	StgVariable_DataType*			dataTypes,
	Index*							dataTypeCounts,
	Name*								dataNames,
	SizeT*							structSizePtr,
	Index*							arraySizePtr,
	StgVariable_ArraySizeFunc*		arraySizeFunc,
	void**							arrayPtrPtr,
	Variable_Register*			vr )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(StgVariable);
	Type                                                      type = StgVariable_Type;
	Stg_Class_DeleteFunction*                              _delete = _StgVariable_Delete;
	Stg_Class_PrintFunction*                                _print = _StgVariable_Print;
	Stg_Class_CopyFunction*                                  _copy = _StgVariable_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_StgVariable_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _StgVariable_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _StgVariable_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _StgVariable_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _StgVariable_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _StgVariable_Destroy;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	StgVariable* self = _StgVariable_New(  VARIABLE_PASSARGS  );

	self->isConstructed = True;
	_StgVariable_Init( self, context, dataCount, dataOffsets, dataTypes, dataTypeCounts, dataNames, structSizePtr, arraySizePtr, arraySizeFunc, arrayPtrPtr, False, vr );
	
	return self;
}

StgVariable* _StgVariable_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(StgVariable);
	Type                                                      type = StgVariable_Type;
	Stg_Class_DeleteFunction*                              _delete = _StgVariable_Delete;
	Stg_Class_PrintFunction*                                _print = _StgVariable_Print;
	Stg_Class_CopyFunction*                                  _copy = _StgVariable_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_StgVariable_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _StgVariable_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _StgVariable_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _StgVariable_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _StgVariable_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _StgVariable_Destroy;
	Index                                                dataCount = 0;
	SizeT*                                             dataOffsets = NULL;
	StgVariable_DataType*                                   dataTypes = NULL;
	Index*                                          dataTypeCounts = NULL;
	Name*                                                dataNames = NULL;
	SizeT*                                           structSizePtr = NULL;
	Index*                                            arraySizePtr = NULL;
	StgVariable_ArraySizeFunc*                          arraySizeFunc = NULL;
	void**                                             arrayPtrPtr = NULL;
	Variable_Register*                                          vr = NULL;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	StgVariable* self = _StgVariable_New(  VARIABLE_PASSARGS  );
	
	return self;
}

StgVariable* StgVariable_NewScalar( 
	Name							name,
	AbstractContext*			context,
	StgVariable_DataType			dataType,
	Index*						arraySizePtr,
	StgVariable_ArraySizeFunc*	arraySizeFunc,
	void**						arrayPtrPtr,
	Variable_Register*		vr )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(StgVariable);
	Type                                                      type = StgVariable_Type;
	Stg_Class_DeleteFunction*                              _delete = _StgVariable_Delete;
	Stg_Class_PrintFunction*                                _print = _StgVariable_Print;
	Stg_Class_CopyFunction*                                  _copy = _StgVariable_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_StgVariable_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _StgVariable_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _StgVariable_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _StgVariable_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _StgVariable_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _StgVariable_Destroy;
	Index                                                dataCount = 1;
	Name*                                                dataNames = 0;
	SizeT*                                           structSizePtr = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	StgVariable*			    self;
	SizeT				    dataOffsets[] = { 0 };
	StgVariable_DataType	    dataTypes[] = { dataType };		/* Init value later */
	Index					dataTypeCounts[] = { 1 };
	
	dataTypes[0] = dataType;
	
	self = _StgVariable_New(  VARIABLE_PASSARGS  );

	self->isConstructed = True;
	_StgVariable_Init( self, context, dataCount, dataOffsets, dataTypes, dataTypeCounts, dataNames, structSizePtr, arraySizePtr, arraySizeFunc, arrayPtrPtr, False, vr );

	return self;
}

StgVariable* StgVariable_NewVector( 
	Name							name,
	AbstractContext*			context,
	StgVariable_DataType			dataType,
	Index							dataTypeCount,
	Index*						arraySizePtr,
	StgVariable_ArraySizeFunc*	arraySizeFunc,
	void**						arrayPtrPtr,
	Variable_Register*		vr,
	... 						/* vector component names */ )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(StgVariable);
	Type                                                      type = StgVariable_Type;
	Stg_Class_DeleteFunction*                              _delete = _StgVariable_Delete;
	Stg_Class_PrintFunction*                                _print = _StgVariable_Print;
	Stg_Class_CopyFunction*                                  _copy = _StgVariable_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_StgVariable_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _StgVariable_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _StgVariable_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _StgVariable_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _StgVariable_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _StgVariable_Destroy;
	Index                                                dataCount = 1;
	SizeT*                                           structSizePtr = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	StgVariable*			self;
	SizeT					dataOffsets[] = { 0 };
	StgVariable_DataType	dataTypes[] = { dataType }; /* Init later... */
	Index					dataTypeCounts[] = { 0 }; /* Init later... */
	Name*					dataNames;
	Index					vector_I;
	va_list				ap;

	dataTypes[0] = dataType;
	dataTypeCounts[0] = dataTypeCount;

	dataNames = Memory_Alloc_Array( Name, dataTypeCount, "dataNames" );

	va_start( ap, vr );
	for( vector_I = 0; vector_I < dataTypeCount; vector_I++ ) {
		dataNames[vector_I] = va_arg( ap, Name );
	}
	va_end( ap );
	
	self = _StgVariable_New(  VARIABLE_PASSARGS  );

	self->isConstructed = True;
	_StgVariable_Init( self, context, dataCount, dataOffsets, dataTypes, dataTypeCounts, dataNames, structSizePtr, arraySizePtr, arraySizeFunc, arrayPtrPtr, False, vr );

	Memory_Free( dataNames );

	return self;
}

StgVariable* StgVariable_NewVector2( 
	Name							name,
	AbstractContext*			context,
	StgVariable_DataType			dataType,
	Index							dataTypeCount,
	Index*						arraySizePtr,
	StgVariable_ArraySizeFunc*	arraySizeFunc,
	void**						arrayPtrPtr,
	Variable_Register*		vr,
	char**						dataNames )
{
	/* Variables set in this function */
	SizeT                                              _sizeOfSelf = sizeof(StgVariable);
	Type                                                      type = StgVariable_Type;
	Stg_Class_DeleteFunction*                              _delete = _StgVariable_Delete;
	Stg_Class_PrintFunction*                                _print = _StgVariable_Print;
	Stg_Class_CopyFunction*                                  _copy = _StgVariable_Copy;
	Stg_Component_DefaultConstructorFunction*  _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_StgVariable_DefaultNew;
	Stg_Component_ConstructFunction*                    _construct = _StgVariable_AssignFromXML;
	Stg_Component_BuildFunction*                            _build = _StgVariable_Build;
	Stg_Component_InitialiseFunction*                  _initialise = _StgVariable_Initialise;
	Stg_Component_ExecuteFunction*                        _execute = _StgVariable_Execute;
	Stg_Component_DestroyFunction*                        _destroy = _StgVariable_Destroy;
	Index                                                dataCount = 1;
	SizeT*                                           structSizePtr = 0;

	/* Variables that are set to ZERO are variables that will be set either by the current _New function or another parent _New function further up the hierachy */
	AllocationType  nameAllocationType = NON_GLOBAL /* default value NON_GLOBAL */;

	StgVariable*			self;
	SizeT					dataOffsets[] = { 0 };
	StgVariable_DataType	dataTypes[] = { dataType };
	Index					dataTypeCounts[] = { 0 };

	dataTypes[0] = dataType;
	dataTypeCounts[0] = dataTypeCount;

	self = _StgVariable_New(  VARIABLE_PASSARGS  );

	self->isConstructed = True;
	_StgVariable_Init( self, context, dataCount, dataOffsets, dataTypes, dataTypeCounts, dataNames, structSizePtr, arraySizePtr, arraySizeFunc, arrayPtrPtr, False, vr );

	return self;
}

StgVariable* _StgVariable_New(  VARIABLE_DEFARGS  ) {
	StgVariable* self;
	
	/* Allocate memory */
	assert( _sizeOfSelf >= sizeof(StgVariable) );
	/* The following terms are parameters that have been passed into this function but are being set before being passed onto the parent */
	/* This means that any values of these parameters that are passed into this function are not passed onto the parent function
	   and so should be set to ZERO in any children of this class. */
	nameAllocationType = NON_GLOBAL;

	self = (StgVariable*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* General info */
	
	/* Virtual info */
	self->_build = _build;
	self->_initialise = _initialise;
	
	/* Variable info */
	self->vr = NULL;
	return self;
}

void _StgVariable_Init(
	StgVariable*					self, 
	AbstractContext*			context,
	Index							dataCount,
	SizeT*						dataOffsets,
	StgVariable_DataType*		dataTypes,
	Index*						dataTypeCounts,
	Name*							dataNames,
	SizeT*						structSizePtr,
	Index*						arraySizePtr,
	StgVariable_ArraySizeFunc*	arraySizeFunc,
	void**						arrayPtrPtr,
	Bool							allocateSelf,
	Variable_Register*		vr )
{
	/*Stream* errorStream = Journal_Register( Error_Type, self->type );*/

	/* General and Virtual info should already be set */
	
	/* Variable info */
	self->context = context;
	self->allocateSelf  = allocateSelf;
	self->offsetCount = dataCount;
	self->structSizePtr = structSizePtr;
	self->arraySizePtr = arraySizePtr;
	self->arraySizeFunc = arraySizeFunc;
	self->arrayPtrPtr = arrayPtrPtr;
	self->parent = NULL;

	/* Checks */
	/*Journal_Firewall( (self->arraySizePtr || self->arraySizeFunc) ,
		errorStream, "Error: in %s(), for Variable %s - either arraySizePtr or arraySizeFunc "
			"passed in must be non-NULL.\n", __func__, self->name );*/

	/* Use of this class has increased... can't assume the info arrays are on persistant memory... copy by default. They will
	   be deleted. */
	Journal_Firewall( dataOffsets ? True : False, Journal_Register( Error_Type, StgVariable_Type ), "dataOffsets is null\n" );
	self->offsets = Memory_Alloc_Array( SizeT, self->offsetCount, "Variable->offsets" );
	memcpy( self->offsets, dataOffsets, sizeof(SizeT) * self->offsetCount );
	
	Journal_Firewall( dataTypes ? True : False, Journal_Register( Error_Type, StgVariable_Type ), "dataTypes is null\n" );
	self->dataTypes = Memory_Alloc_Array( StgVariable_DataType, self->offsetCount, "Variable->dataTypes" );
	memcpy( self->dataTypes, dataTypes, sizeof(StgVariable_DataType) * self->offsetCount );
	
	Journal_Firewall( dataTypeCounts ? True : False , Journal_Register( Error_Type, StgVariable_Type ), "dataTypeCounts is null\n" );
	self->dataTypeCounts = Memory_Alloc_Array( Index, self->offsetCount, "Variable->dataTypeCounts" );
	memcpy( self->dataTypeCounts, dataTypeCounts, sizeof(Index) * self->offsetCount );
	
	/* These get set at the build phase (when we assume the variable's data memory has been allocated) */
	self->structSize = 0;
	self->arrayPtr = 0;
	self->dataSizes = 0;
	
	if ( dataNames && self->offsetCount == 1 && self->dataTypeCounts[0] > 1 ) {
		/* Vector case */
		self->subVariablesCount = self->dataTypeCounts[0];
	}
	else {
		/* Scalar or Complex case */
		self->subVariablesCount = self->offsetCount;
	}
	self->components = Memory_Alloc_Array( StgVariable*, self->subVariablesCount, "Variable->components" );
	memset( self->components, 0, sizeof(StgVariable*) * self->subVariablesCount );
	
	/* If the variable register is provided, add this and component variable(s) to the register */
	if( vr ) {
		self->vr = vr;
		Variable_Register_Add( vr, self );
	}

    /* If we have component names, create the associated variables. Don't do if there is only one component. */
    if( dataNames && self->offsetCount > 1 ) {
        Index component_I;
        
        for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
            if( dataNames[component_I] ) {
                SizeT					componentOffsets[] = { 0 };
                StgVariable_DataType	componentTypes[] = { self->dataTypes[component_I] };
                Index					componentTypeCounts[] = { 0 };

                componentOffsets[0] = self->offsets[component_I];
                componentTypes[0] = self->dataTypes[component_I];
                componentTypeCounts[0] = self->dataTypeCounts[component_I];
                
                /* Assumption: components are scalar or vector, but cannot be complex */
                self->components[component_I] = StgVariable_New( 
                    dataNames[component_I], 
                    self->context,
                    1, 
                    componentOffsets, 
                    componentTypes, 
                    componentTypeCounts,
                    0,
                    self->structSizePtr,
                    self->arraySizePtr,
                    self->arraySizeFunc,
                    self->arrayPtrPtr,
                    vr );
                self->components[component_I]->parent = self;
            }
        }
    }
    /* Else if we have vector-component names, create the associated variables. Do only if non-complex and a vector. */
    else if( dataNames && self->offsetCount == 1 && self->dataTypeCounts[0] > 1 ) {
        Index vector_I;

        for( vector_I = 0; vector_I < self->dataTypeCounts[0]; vector_I++ ) {
            if( dataNames[vector_I] ) {
                /* Unfortunately we cannot call any of our fancy macros here as the array is not resolved
                 * yet. As a consequence we have to manually work out the vector's indecis offsets. Ouch
                 * only from a code-maintenance point of view. */
                SizeT					componentOffsets[] = { 0 }; /* Init later... */
                StgVariable_DataType	componentTypes[] = { self->dataTypes[0] }; /* Init later... */
                Index					componentTypeCounts[] = { 1 };
            
                componentOffsets[0] = (ArithPointer)self->offsets[0] + StgVariable_SizeOfDataType( self->dataTypes[0] )*vector_I;
                componentTypes[0] = self->dataTypes[0];

                /* Assumption: vector-components are scalar, but cannot be complex */
                self->components[vector_I] = StgVariable_New( 
                    dataNames[vector_I],
                    self->context,
                    1, 
                    componentOffsets, 
                    componentTypes, 
                    componentTypeCounts,
                    0,
                    self->structSizePtr,
                    self->arraySizePtr,
                    self->arraySizeFunc,
                    self->arrayPtrPtr,
                    vr );
                self->components[vector_I]->parent = self;
            }
        }
    }
}


/*----------------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/

void _StgVariable_Delete( void* variable ) {
	StgVariable* self = (StgVariable*)variable;

	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _StgVariable_Print( void* variable, Stream* stream ) {
	StgVariable*	self = (StgVariable*)variable;
	Index		component_I;
	
	/* Print parent */
	_Stg_Component_Print( self, stream );
	
	/* General info */
	Journal_Printf( stream, "Variable (ptr): (%p)\n", self );
	
	/* Virtual info */
	
	/* Variable info */
	Journal_Printf( stream, "\toffsetCount: %u\n", self->offsetCount );
	
	Journal_Printf( stream, "\toffsets (ptr): %p\n", self->offsets );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\toffsets[%u]: %lu\n", component_I, self->offsets[component_I] );
	}
	
	Journal_Printf( stream, "\tdataTypes (ptr): %p\n", self->dataTypes );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\tdataTypes[%u]: %lu\n", component_I, self->dataTypes[component_I] );
	}
	
	Journal_Printf( stream, "\tdataTypeCounts (ptr): %p\n", self->dataTypeCounts );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\tdataTypeCounts[%u]: %lu\n", component_I, self->dataTypeCounts[component_I] );
	}

	Journal_Printf( stream, "\tdataSizes (ptr): %p\n", self->dataSizes );
	for( component_I = 0; component_I < self->offsetCount && self->dataSizes; component_I++ ) {
		Journal_Printf( stream, "\t\tdataSizes[%u]: %lu\n", component_I, self->dataSizes[component_I] );
	}

	Journal_Printf( stream, "\tcomponents (ptr): %p\n", self->components );
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		Journal_Printf( stream, "\t\tcomponents[%u] (ptr): %p\n", component_I, self->components[component_I] );
		if( self->components[component_I] ) {
			Journal_Printf( stream, "\t\tcomponents[%u]: %s\n", component_I, self->components[component_I]->name );
		}
	}

	Journal_Printf( stream, "\tstructSize: %lu\n", self->structSize );
	Journal_Printf( stream, "\tstructSizePtr (ptr): %p\n", self->structSizePtr );

	Journal_Printf( stream, "\tarrayPtr (ptr): %p\n", self->arrayPtr );
	Journal_Printf( stream, "\tarrayPtrPtr (ptr): %p\n", self->arrayPtrPtr );

	Journal_Printf( stream, "\tarraySize: %lu\n", _StgVariable_GetNewArraySize( self ) );
	Journal_Printf( stream, "\tarraySizePtr (ptr): %p\n", self->arraySizePtr );
	Journal_Printf( stream, "\tarraySizeFunc (ptr): %p\n", self->arraySizeFunc );

	Journal_Printf( stream, "\tallocateSelf = %s\n", self->allocateSelf ? "True" : "False" );
}


void* _StgVariable_Copy( void* variable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap ) {
	StgVariable*	self = (StgVariable*)variable;
	StgVariable*	newVariable;
	PtrMap*		map = ptrMap;
	
	newVariable = (StgVariable*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	PtrMap_Append( map, self, newVariable );
	
	/* virtual methods */
	newVariable->offsetCount = self->offsetCount;
	newVariable->structSize = self->structSize;
	newVariable->arraySizePtr = self->arraySizePtr;
	newVariable->arraySizeFunc = self->arraySizeFunc;
	
	newVariable->allocateSelf = self->allocateSelf; /* This may change depending on whether arrayPtr is found in map */
	newVariable->subVariablesCount = self->subVariablesCount;

	/* single valued members */
	
	if( deep ) {
		if( (newVariable->offsets = (SizeT*)PtrMap_Find( map, self->offsets )) == NULL && self->offsets != NULL ) {
			newVariable->offsets = Memory_Alloc_Array( SizeT, newVariable->offsetCount, "Variable->offsets" );
			memcpy( newVariable->offsets, self->offsets, sizeof(SizeT) * newVariable->offsetCount );
			PtrMap_Append( map, self->offsets, newVariable->offsets );
		}
		
		if( (newVariable->dataTypes = (StgVariable_DataType*)PtrMap_Find( map, self->dataTypes )) == NULL && self->offsets != NULL ) {
			newVariable->dataTypes = Memory_Alloc_Array( StgVariable_DataType, newVariable->offsetCount, "Variable->dataTypes" );
			memcpy( newVariable->dataTypes, self->dataTypes, sizeof(StgVariable_DataType) * newVariable->offsetCount );
			PtrMap_Append( map, self->dataTypes, newVariable->dataTypes );
		}
		
		if( (newVariable->dataTypeCounts = (Index*)PtrMap_Find( map, self->dataTypeCounts )) == NULL && self->dataTypeCounts != NULL ) {
			newVariable->dataTypeCounts = Memory_Alloc_Array( Index, newVariable->offsetCount, "Variable->dataTypeCounts" );
			memcpy( newVariable->dataTypeCounts, self->dataTypeCounts, sizeof(Index) * newVariable->offsetCount );
			PtrMap_Append( map, self->dataTypeCounts, newVariable->dataTypeCounts );
		}
		
		if ( self->structSizePtr != NULL ) {
			if( (newVariable->structSizePtr = (SizeT*)PtrMap_Find( map, self->structSizePtr )) == NULL ) {
				newVariable->structSizePtr = Memory_Alloc_Array( 
					SizeT, 
					1, 
					"Variable->structSizePtr" );
				memcpy( newVariable->structSizePtr, self->structSizePtr, sizeof(SizeT) );
				PtrMap_Append( map, self->structSizePtr, newVariable->structSizePtr );
			}
		}
		else {
			newVariable->structSizePtr = NULL;
		}
		
		if ( self->arraySizePtr != NULL ) {
			if( (newVariable->arraySizePtr = (Index*)PtrMap_Find( map, self->arraySizePtr )) == NULL ) {
				newVariable->arraySizePtr = Memory_Alloc_Array(
					Index,
					1,
					"Variable->arraySizePtr" );
				memcpy( newVariable->arraySizePtr, self->arraySizePtr, sizeof(Index) );
				PtrMap_Append( map, self->arraySizePtr, newVariable->arraySizePtr );
			}
		}
		else {
			newVariable->arraySizeFunc = NULL;
		}
		if ( self->arraySizeFunc != NULL ) {
			if( (newVariable->arraySizeFunc = (StgVariable_ArraySizeFunc*)PtrMap_Find( map, self->arraySizeFunc )) == NULL ) {
				newVariable->arraySizeFunc = Memory_Alloc_Array(
					StgVariable_ArraySizeFunc,
					1,
					"Variable->arraySizeFunc" );
				memcpy( newVariable->arraySizeFunc, self->arraySizeFunc, sizeof(Index) );
				PtrMap_Append( map, self->arraySizeFunc, newVariable->arraySizeFunc );
			}
		}
		else {
			newVariable->arraySizeFunc = NULL;
		}
		
		if( (newVariable->dataSizes = (SizeT*)PtrMap_Find( map, self->dataSizes )) == NULL && self->dataSizes != NULL ) {
			newVariable->dataSizes = Memory_Alloc_Array( SizeT, newVariable->offsetCount, "Variable->dataSizes" );
			memcpy( newVariable->dataSizes, self->dataSizes, sizeof(SizeT) * newVariable->offsetCount );
			PtrMap_Append( map, self->dataSizes, newVariable->dataSizes );
		}

		if( (newVariable->arrayPtrPtr = (void**)PtrMap_Find( map, self->arrayPtrPtr )) == NULL && self->arrayPtrPtr != NULL ) {
			if( (newVariable->arrayPtr = PtrMap_Find( map, self->arrayPtr )) == NULL && self->arrayPtr != NULL ) {
				Index memoryToAllocSize = _StgVariable_GetNewArraySize( self ) * self->structSize;
				newVariable->arrayPtr = Memory_Alloc_Bytes( 
					memoryToAllocSize,
					StgVariable_Type,
					"SelfAllocedArray" );
				memcpy( newVariable->arrayPtr, self->arrayPtr, memoryToAllocSize );
				PtrMap_Append( map, self->arrayPtr, newVariable->arrayPtr );

				newVariable->arrayPtrPtr = &(newVariable->arrayPtr);
				newVariable->allocateSelf = True;
			}
		}
		else {
			newVariable->arrayPtr = *newVariable->arrayPtrPtr;
			memcpy( newVariable->arrayPtr, self->arrayPtr, _StgVariable_GetNewArraySize(self) * self->structSize );
		}
		
		if( (newVariable->components = (StgVariable**)PtrMap_Find( map, self->components )) == NULL && self->components != NULL ) {
			Index	comp_I;
			
			if( newVariable->offsetCount == 1 && newVariable->dataTypeCounts[0] > 1 ) {
				/* Vector case */
				newVariable->components = Memory_Alloc_Array( 
					StgVariable*, 
					newVariable->dataTypeCounts[0], 
					"Variable->components" );
				memset( newVariable->components, 0, sizeof(StgVariable*) * newVariable->dataTypeCounts[0] );
				
				for( comp_I = 0; comp_I < newVariable->dataTypeCounts[0]; comp_I++ ) {
					newVariable->components[comp_I] = (StgVariable*)Stg_Class_Copy( 
						self->components[comp_I], 
						NULL, 
						deep, 
						nameExt, 
						map );
				}
			}
			else {
				/* Scalar or Complex case */
				newVariable->components = Memory_Alloc_Array( 
					StgVariable*, 
					newVariable->offsetCount, 
					"Variable->components" );
				memset( newVariable->components, 0, sizeof(StgVariable*) * newVariable->offsetCount );
				
				for( comp_I = 0; comp_I < newVariable->offsetCount; comp_I++ ) {
					if ( self->components[comp_I] ) {
						newVariable->components[comp_I] = (StgVariable*)Stg_Class_Copy( 
							self->components[comp_I], 
							NULL, 
							deep, 
							nameExt, 
							map );
					}
				}
			}
			
			PtrMap_Append( map, self->components, newVariable->components );
		}

		if ( self->vr ) {
			newVariable->vr = (Variable_Register*)Stg_Class_Copy( self->vr, NULL, deep, nameExt, map );
		}
	}
	else {
		fprintf( stderr, "Warning: please don't shallow copy variables yet...or ever!\n" );
	}
	
	return (void*)newVariable;
}


void _StgVariable_Build( void* variable, void* data ) {
	StgVariable*	self = (StgVariable*)variable;
	Index 		component_I;
	Index 		subVariable_I;

	if( self->parent ) {
		/* We need to build the parent first, as it may initialise data structures needed 
	 	 * by this Variable.
 	   */	
		Stg_Component_Build( self->parent, NULL, False );
	}
	
	/* Obtain the actual array size, and array pointer */
	Journal_Firewall( 
		( self->arraySizeFunc || self->arraySizePtr )? True : False, 
		Journal_Register( Error_Type, StgVariable_Type ), 
		"arraySizePtr && arraySizeFunc is null\n" );
	self->arraySize = _StgVariable_GetNewArraySize( self );


	/* Work out the actual data sizes from the data types */
	if( self->dataSizes ) {
		Memory_Free( self->dataSizes );
	}
	self->dataSizes = Memory_Alloc_Array( SizeT, self->offsetCount, "Variable->dataSizes" );
	for( component_I = 0; component_I < self->offsetCount; component_I++ )
        self->dataSizes[component_I] = StgVariable_SizeOfDataType( self->dataTypes[component_I] )*self->dataTypeCounts[component_I];
	
	/* Non-complex variables: Scalar & Vector can workout struct size for themselves.
	 * Now that we know all the data sizes, work out the actual struct size (which may need the data sizes) */
	if( self->offsetCount == 1 && !self->structSizePtr ) {
		/* Vector and Scalars which are not apart of a Vector can work out their own size */
		if ( self->structSize == 0 ) {
			self->structSize = self->dataSizes[0];
		}

		/* If this is a vector, set its sub variables to the same struct size */
		if ( self->dataTypeCounts[0] > 1 ) {
			for ( subVariable_I = 0; subVariable_I < self->dataTypeCounts[0]; ++subVariable_I ) {
				if ( self->components[subVariable_I] ) {
					self->components[subVariable_I]->structSize = self->structSize;
				}
				else
					/* break here because if one subVariable isn't allocated - then the rest shouldn't be */
					break;
			}
		}
	}
	else {
		Journal_Firewall( 
			self->structSizePtr ? True: False, 
			Journal_Register( Error_Type, StgVariable_Type ), 
			"structSizePtr is null\n" );
		self->structSize = *self->structSizePtr;
	}
	
	if (self->allocateSelf) {
		Index memoryToAllocSize = _StgVariable_GetNewArraySize( self ) * self->structSize;
		Index subVariable_I;

		Journal_Firewall( self->offsetCount == 1, 
				Journal_Register( Error_Type, StgVariable_Type ), 
				"Variable '%s' has 'offsetCount = %u', which should be 1 for self allocation.\n", 
				self->name, self->offsetCount );

		self->arrayPtr = Memory_Alloc_Bytes( memoryToAllocSize, StgVariable_Type, "SelfAllocedArray" );
		memset( self->arrayPtr, 0, memoryToAllocSize );
		
		/* If this is a vector then set arrayPtr to be the same for each element of the vector */
		if ( self->dataTypeCounts[0] > 1 ) {
			for ( subVariable_I = 0 ; subVariable_I < self->dataTypeCounts[0] ; subVariable_I++ ) {
				if ( self->components[subVariable_I] ) {
					self->components[ subVariable_I ]->arrayPtr = self->arrayPtr;
					self->components[ subVariable_I ]->arrayPtrPtr = &self->arrayPtr;
				}
				else {
					/* break here because if one subVariable isn't allocated - then the rest shouldn't be */
					break;
				}
			}
		}
	}
	else {
		Journal_Firewall( 
			self->arrayPtrPtr ? True : False, 
			Journal_Register( Error_Type, StgVariable_Type ), 
			"Variable '%s' has NULL arrayPtrPtr\n", self->name );
		self->arrayPtr = *self->arrayPtrPtr;
	}

	/* Lastly, build all components. It is reasonable if the user calls Build on "velocity", then
	 * "velocity-x" etc are automatically build also. */	
	for ( subVariable_I = 0; subVariable_I < self->subVariablesCount; subVariable_I++ ) {
		if ( self->components[subVariable_I] ) {
			/*Components now call Build on the parent */
			Stg_Component_Build( self->components[subVariable_I], data, False );
		}
	}
}


void _StgVariable_Initialise( void* variable, void* data ) {
	StgVariable*	self = (StgVariable*)variable;
	Index 		subVariable_I;

	for ( subVariable_I = 0; subVariable_I < self->subVariablesCount; subVariable_I++ ) {
		if ( self->components[subVariable_I] ) {
			Stg_Component_Initialise( self->components[subVariable_I], data, False );
		}
	}
}


void _StgVariable_Execute( void* variable, void* data ) {
}

void _StgVariable_AssignFromXML( void* variable, Stg_ComponentFactory* cf, void* data ) {
	StgVariable*			self = (StgVariable*) variable;
	SizeT					dataOffsets[] = { 0 };
	StgVariable_DataType	dataTypes[] = { StgVariable_DataType_Size };		/* Init value later */
	Index					dataTypeCounts[] = { 1 };
	Dictionary*			componentDict = NULL;
	Dictionary*			thisComponentDict = NULL;
	Name					dataTypeName = NULL;
	Name					rankName = NULL;
	Name					countName = NULL;
	unsigned int*		count = NULL;
	void*					variableRegister = NULL;
	Name*					names = NULL;
	Stream*				error = Journal_Register( Error_Type, self->type );
	AbstractContext*	context;	
	
	assert( self );

	context = Stg_ComponentFactory_ConstructByKey( cf, self->name, "Context", AbstractContext, False, data );
	if( !context )
		context = Stg_ComponentFactory_ConstructByName( cf, "context", AbstractContext, False, data );

	componentDict = cf->componentDict;
	assert( componentDict );
	thisComponentDict = Dictionary_GetDictionary( componentDict, self->name );
	assert( thisComponentDict );
	
	/* Grab Registers */
	variableRegister = context ? context->variable_Register : NULL;
	
	Stg_ComponentFactory_ConstructByKey( cf, self->name, "Dependency", Stg_Component, False, data );
			
	/* Get Pointer to number of elements in array */
	countName = Dictionary_GetString( thisComponentDict, "Count" );
	
	/* Get Type of Variable */
	dataTypeName = Dictionary_GetString( thisComponentDict, "DataType" );
	if ( !strcasecmp( dataTypeName, "Double" ) )
		dataTypes[0] = StgVariable_DataType_Double;
	else if ( !strcasecmp( dataTypeName, "Float" ) )
		dataTypes[0] = StgVariable_DataType_Float;
	else if ( !strcasecmp( dataTypeName, "Int" ) )
		dataTypes[0] = StgVariable_DataType_Int;
	else if ( !strcasecmp( dataTypeName, "Long" ) )
		dataTypes[0] = StgVariable_DataType_Long;
	else if ( !strcasecmp( dataTypeName, "Char" ) )
		dataTypes[0] = StgVariable_DataType_Char;
	else if ( !strcasecmp( dataTypeName, "Short" ) )
		dataTypes[0] = StgVariable_DataType_Short;
	else 
		Journal_Firewall( False, error, "Variable '%s' cannot understand data type '%s'\n", self->name, dataTypeName );

	/* Get Rank of Variable - i.e. Scalar or Vector */
	rankName = Dictionary_GetString( thisComponentDict, "Rank" );
	if( !strcasecmp( rankName, "Scalar" ) ){
		dataTypeCounts[0] = 1;
	}
	else if ( !strcasecmp( rankName, "Vector" ) ){
		Dictionary_Entry_Value* list;
		Index                   nameCount = 0;

		/* Get Names from list */
		if (( list = Dictionary_Get( thisComponentDict, "names" ) )) {
			Index entry_I;

			nameCount = Dictionary_Entry_Value_GetCount( list );
			names = Memory_Alloc_Array( Name, nameCount, "Variable Names" );

			for ( entry_I = 0 ; entry_I < nameCount ; entry_I++ )
				names[ entry_I ] = Dictionary_Entry_Value_AsString( Dictionary_Entry_Value_GetElement(list, entry_I ) );
		}
		dataTypeCounts[0] = Stg_ComponentFactory_GetUnsignedInt( cf, self->name, "VectorComponentCount", nameCount );

		Journal_Firewall( nameCount >= dataTypeCounts[0], error, "Variable '%s' has too few names in list for %d vector components.\n", self->name, dataTypeCounts[0] );
	}
	else
		Journal_Firewall( False, error, "Variable '%s' cannot understand rank '%s'\n", self->name, rankName );

	_StgVariable_Init( 
		self, 
		context,
		1, 
		dataOffsets,
		dataTypes,
		dataTypeCounts, 
		names, 
		0, 
		count, 
		NULL,	/* Note: don't support arraySize being calculated from a Func Ptr through
		Construct() Yet - PatrickSunter, 29 Jun 2007 */ 
		(void**)&self->arrayPtr,
		True,
		(Variable_Register*)variableRegister );

	/* Clean Up */
	if (names)
		Memory_Free(names);
}
	
void _StgVariable_Destroy( void* variable, void* data ) {
	StgVariable* self = (StgVariable*)variable;
   int ii;

   for( ii = 0 ; ii < self->subVariablesCount ; ii++ ) {
      Stg_Component_Destroy( self->components[ii], data, False );
   }
	
    Memory_Free( self->dataSizes ); self->dataSizes = NULL;

	if (self->allocateSelf) {
		Memory_Free( self->arrayPtr ); self->arrayPtr = NULL;
	}
	
	Memory_Free( self->dataTypeCounts ); self->dataTypeCounts = NULL;
	Memory_Free( self->dataTypes ); self->dataTypes = NULL;
	Memory_Free( self->offsets ); self->offsets = NULL;
	Memory_Free( self->components ); self->components = NULL;
}

Index _StgVariable_GetNewArraySize( StgVariable* self ) {
	Index arraySize = 0;

	/* By default, we will try the simple ptr approach. */
	if ( NULL != self->arraySizePtr ) {
		arraySize = (*self->arraySizePtr);
	}
	else {
		/* If that was NULL, we expect the arraySizeFunc to be set */

		/* Because of the way sub-variables are constructed, they will use their parent's
		 * arraySizePtr function (eg in the case of "vx", we want it to use the parent
		 * Velocity MeshVariable's function to calculate the
		 * size of the mesh.
		 */ 
		if ( NULL != self->parent ) {
			arraySize = (*self->arraySizeFunc)( self->parent );
		}
		else {
			arraySize = (*self->arraySizeFunc)( self );
		}
	}

	return arraySize;
}

/*--------------------------------------------------------------------------------------------------------------------------
** Public Functions
*/

void StgVariable_SetValue( void* variable, Index array_I, void* value ) {
	StgVariable*		self = (StgVariable*)variable;
	ArithPointer*	vPtr;
	Index				component_I;
	
	vPtr = (ArithPointer*)value;
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		memcpy( _StgVariable_GetPtr( self, array_I, component_I, 0 ), (void*)vPtr, self->dataSizes[component_I] );
		
		vPtr += self->dataSizes[component_I];
	}
}


void StgVariable_GetValue( void* variable, Index array_I, void* value ) {
	StgVariable*	self = (StgVariable*)variable;
	ArithPointer*	vPtr;
	Index		component_I;
	
	vPtr = (ArithPointer*)value;
	for( component_I = 0; component_I < self->offsetCount; component_I++ ) {
		memcpy( (void*)vPtr, _StgVariable_GetPtr( self, array_I, component_I, 0 ), self->dataSizes[component_I] );
		
		vPtr += self->dataSizes[component_I];
	}
}

void StgVariable_Update( void* variable ) {
	StgVariable* self = (StgVariable*)variable;

	/* array may have resized, assign local properties again */
	self->arrayPtr = *self->arrayPtrPtr;
	self->arraySize = _StgVariable_GetNewArraySize( self );
	
	if( !( self->offsetCount == 1 && !self->structSizePtr ) ) {
		/* For non-scalar or non-vector variables, the targets may have been extended. */
		self->structSize = *self->structSizePtr;
	}
}

void StgVariable_SetValueDoubleAll( void* variable, double value ) {
	StgVariable* self       = (StgVariable*)  variable;
	Index     arraySize  = self->arraySize;
	Index     array_I;

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) 
		StgVariable_SetValueDouble( self, array_I, value );
}
		

void StgVariable_SaveToFileAsDoubles( void* variable, char* filename ) {
	StgVariable* self       = (StgVariable*)  variable;
	Index     arraySize  = self->arraySize;
	Index     array_I;
	double    value;
	FILE*     outputFile;
	Index     comp_I;

	outputFile = fopen( filename, "w" );

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		if ( *self->dataTypeCounts > 1 ) {
			for ( comp_I = 0; comp_I < *self->dataTypeCounts; comp_I++ ) {	
				value = StgVariable_GetValueAtDouble( self, array_I, comp_I );
				fwrite( &value, sizeof(double), 1, outputFile );
			}
		}
		else {
			value = StgVariable_GetValueDouble( self, array_I );
			fwrite( &value, sizeof(double), 1, outputFile );
		}	
	
	}	

	fclose( outputFile );
}


void StgVariable_ReadFromFileAsDoubles( void* variable, char* filename ) {
	StgVariable* self       = (StgVariable*)  variable;
	Index     arraySize  = self->arraySize;
	Index     array_I;
	double    value;
	FILE*     inputFile;
	Index     comp_I;

	inputFile = fopen( filename, "r" );

	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		if ( *self->dataTypeCounts > 1 ) {
			for ( comp_I = 0; comp_I < *self->dataTypeCounts; comp_I++ ) {	
				fread( &value, sizeof(double), 1, inputFile );
				StgVariable_SetValueAtDouble( self, array_I, comp_I, value );
			}
		}
		else {
			fread( &value, sizeof(double), 1, inputFile );
			StgVariable_SetValueDouble( self, array_I, value );
		}
	}	

	fclose( inputFile );
}


/** Returns \f$ \frac{|| variable - comparison ||} { || variable ||} \f$, 
  * where \f$ ||x|| \f$ indicates the Euclidean Norm which is the square root of the sum of the squares for each component in x
  * i.e.  \f$ ||x|| = \sqrt{\sum_i x_i^2} \f$ */
double StgVariable_ValueCompare( void* variable, void* _comparison ) {
	StgVariable* self            = (StgVariable*)  variable;
	StgVariable* comparison      = (StgVariable*) _comparison;
	Index     arraySize       = self->arraySize;
	Index     dataTypeCounts  = *self->dataTypeCounts;
	Index     array_I;
	Index     component_I;
	double    differenceValue;
	double    selfValue;
	double    normDifference  = 0.0;
	double    normVariable    = 0.0;
	Stream*   error           = Journal_Register( Error_Type, self->type );

	/* Simple Error Checking */
	Journal_Firewall( arraySize == comparison->arraySize, error, 
			"In func %s: Array sizes different for variables '%s' and '%s'\n", __func__, self->name, comparison->name );
	Journal_Firewall( dataTypeCounts == *comparison->dataTypeCounts, error, 
			"In func %s: Data Type Counts different for variables '%s' and '%s'\n",  __func__, self->name,comparison->name);

	for ( component_I = 0 ; component_I < self->offsetCount ; component_I++ ) {
		Journal_Firewall( self->dataTypes[component_I] == StgVariable_DataType_Double, error, 
				"In func %s: Stg_Component %d of variable '%s' has different type to StgVariable_DataType_Double.\n", 
				 __func__, component_I, self->name );
		Journal_Firewall( comparison->dataTypes[component_I] == StgVariable_DataType_Double, error, 
				"In func %s: Stg_Component %d of variable '%s' has different type to StgVariable_DataType_Double.\n", 
				 __func__, component_I, comparison->name );
	}

	/* Calculate Euclidean Norms */
	for ( array_I = 0 ; array_I < arraySize ; array_I++ ) {
		for ( component_I = 0 ; component_I < dataTypeCounts ; component_I++ ) {
			selfValue       = StgVariable_GetValueAtDouble( self, array_I, component_I );
			differenceValue = selfValue - StgVariable_GetValueAtDouble( comparison, array_I, component_I );

			normDifference += differenceValue * differenceValue;
			normVariable   += selfValue * selfValue;
		}
	}

	normDifference = sqrt( normDifference );
	normVariable   = sqrt( normVariable );

	return normDifference/normVariable;
}

/** Checks whether || variable - comparison || / || variable || < tolerance */
Bool StgVariable_ValueCompareWithinTolerance( void* variable, void* comparison, double tolerance ) {
	return (Bool)( StgVariable_ValueCompare( variable, comparison ) < tolerance );
}

PyObject* StgVariable_getAsNumpyArray( void* variable ){
	StgVariable* self = (StgVariable*) variable;

    Journal_Firewall(   self->offsetCount==1,
                        NULL,
                        "Error: Numpy array generation from Variable class objects is only currently possible with simply (ie non-struct) variables." );

    StgVariable_Update( variable );
    npy_intp dims[2] = { self->arraySize, self->dataTypeCounts[0] };
    
    int numtype=-1;
    if(         self->dataTypes[0] == StgVariable_DataType_Char ){
        numtype = NPY_BYTE;
    } else if ( self->dataTypes[0] == StgVariable_DataType_Short ){
        numtype = NPY_SHORT;
    } else if ( self->dataTypes[0] == StgVariable_DataType_Int ){
        numtype = NPY_INT;
	} else if ( self->dataTypes[0] == StgVariable_DataType_Long ){
		numtype = NPY_LONG;
    } else if ( self->dataTypes[0] == StgVariable_DataType_Float ){
        numtype = NPY_FLOAT;
    } else if ( self->dataTypes[0] == StgVariable_DataType_Double ){
        numtype = NPY_DOUBLE;
    } else
        Journal_Firewall(   NULL,
                            NULL,
                            "Error: Numpy array generation from this variable datatype not supported."  );

    int sizeitem = StgVariable_SizeOfDataType(self->dataTypes[0]);
    npy_intp strides[2] = { self->structSize, sizeitem };
    
    PyObject* pyobj = PyArray_New(&PyArray_Type, 2, dims, numtype, strides, (void*)( (ArithPointer) self->arrayPtr + self->offsets[0]), sizeitem, NPY_ARRAY_WRITEABLE, NULL);
 
    return pyobj;

}

size_t StgVariable_SizeOfDataType( StgVariable_DataType dataType ){
    switch ( dataType ) {
        case StgVariable_DataType_Char:
            return sizeof( char );
        case StgVariable_DataType_Short:
            return sizeof( short );
        case StgVariable_DataType_Int:
            return sizeof( int );
		case StgVariable_DataType_Long:
			return sizeof( long );
		case StgVariable_DataType_Float:
			return sizeof( float );
        case StgVariable_DataType_Double:
            return sizeof( double );
        case StgVariable_DataType_Pointer:
            return sizeof( void* );
        default:
            Journal_Firewall( 
                0,
                Journal_Register( Error_Type, StgVariable_Type ),
                "dataType is not recognised." );
            break;
    }
}
