/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdarg.h>
#include "StGermain/Base/Foundation/src/Foundation.h"
#include "StGermain/Base/IO/src/IO.h"
#include "StGermain/Base/Container/src/Container.h"
#include "StGermain/Base/Automation/src/Automation.h"

#include "types.h"
#include "Variable.h"
#include "Variable_Register.h"
#include "ConditionFunction.h"
#include "ConditionFunction_Register.h"
#include "VariableCondition.h"

#include <string.h>
#include <assert.h>

/** Textual name of this class */
const Type VariableCondition_Type = "VariableCondition";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

VariableCondition* _VariableCondition_New(  VARIABLECONDITION_DEFARGS  ) {
	VariableCondition*	self;
	
	/* Allocate memory/General info */
	assert( _sizeOfSelf >= sizeof(VariableCondition) );
	self = (VariableCondition*)_Stg_Component_New(  STG_COMPONENT_PASSARGS  );
	
	/* Virtual info */
	self->_buildSelf = _buildSelf;
	self->_printConcise = _printConcise;
	self->_readDictionary = _readDictionary;
	self->_getSet = _getSet;
	self->_getVariableCount = _getVariableCount;
	self->_getVariableIndex = _getVariableIndex;
	self->_getValueIndex = _getValueIndex;
	self->_getValueCount = _getValueCount;
	self->_getValue = _getValue;
	self->_apply = _apply;
	self->_isCondition = _isCondition;
	self->_set = NULL;
	self->indexCount = 0;
	self->indexTbl = NULL;
	self->vcVarCountTbl = NULL;
	self->vcTbl = NULL;
	self->valueCount = 0;
	self->valueTbl = NULL;
	self->mapping = NULL;

	/* Stg_Class info */
	
	return self;
}


void _VariableCondition_Init(
	void*									variableCondition, 
	AbstractContext*					context,
	Variable_Register*				variable_Register, 
	ConditionFunction_Register*	conFunc_Register,
	Dictionary*							dictionary )
{
	VariableCondition*	self = (VariableCondition*)variableCondition;
	
	self->context = context;
	self->variable_Register = variable_Register;
	self->conFunc_Register = conFunc_Register;
	self->dictionary = dictionary;
}

/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _VariableCondition_Delete(void* variableCondition) {
	VariableCondition* self = (VariableCondition*)variableCondition;
	
	/* Stg_Class_Delete parent */
	_Stg_Component_Delete( self );
}


void _VariableCondition_Print(void* variableCondition) {
	VariableCondition*					self = (VariableCondition*)variableCondition;
	VariableCondition_StgVariableIndex	vcVar_I;
	VariableCondition_ValueIndex		val_I;
	Index										i;
	
	/* Set the Journal for printing informations */
	Stream* variableConditionStream = Journal_Register( InfoStream_Type,	"VariableConditionStream");
	
	/* General info */
	Journal_Printf( variableConditionStream, "VariableCondition (ptr): %p\n", self);
	
	/* Print parent */
	_Stg_Component_Print( self, variableConditionStream );
	
	/* Virtual info */
	Journal_Printf( variableConditionStream, "\t_getSet (func ptr): %p\n", self->_getSet);
	Journal_Printf( variableConditionStream, "\t_getVariableCount (func ptr): %p\n", self->_getVariableCount);
	Journal_Printf( variableConditionStream, "\t_getVariableIndex (func ptr): %p\n", self->_getVariableIndex);
	Journal_Printf( variableConditionStream, "\t_getValueIndex (func ptr): %p\n", self->_getValueIndex);
	Journal_Printf( variableConditionStream, "\t_getValueCount (func ptr): %p\n", self->_getValueCount);
	Journal_Printf( variableConditionStream, "\t_getValue (func ptr): %p\n", self->_getValue);
	
	/* Stg_Class info */
	Journal_Printf( variableConditionStream, "\tvariable_Register (ptr): %p\n", self->variable_Register);
	Journal_Printf( variableConditionStream, "\tconFunc_Register (ptr): %p\n", self->conFunc_Register);
	Journal_Printf( variableConditionStream, "\t_set (ptr): %p\n", self->_set);
	Journal_Printf( variableConditionStream, "\tindexCount: %u\n", self->indexCount);
	Journal_Printf( variableConditionStream, "\tindexTbl (ptr): %p\n", self->indexTbl);

	if (self->indexTbl)
		for (i = 0; i < self->indexCount; i++)
			Journal_Printf( variableConditionStream, "\t\tindexTbl[%u]: %u\n", i, self->indexTbl[i]);
	Journal_Printf( variableConditionStream, "\tvcVarCountTbl (ptr): %p\n", self->vcVarCountTbl);
	if (self->vcVarCountTbl)
		for (i = 0; i < self->indexCount; i++)
			Journal_Printf( variableConditionStream, "\t\tvcVarCountTbl[%u]: %u\n", i, self->vcVarCountTbl[i]);
	Journal_Printf( variableConditionStream, "\tvcTbl (ptr): %p\n", self->vcTbl);
	if (self->vcTbl)
		for (i = 0; i < self->indexCount; i++)
			for (vcVar_I = 0; vcVar_I < self->vcVarCountTbl[i]; vcVar_I++)
			{
				Journal_Printf( variableConditionStream, "\t\tvcTbl[%u][%u]:\n", i, vcVar_I);
				Journal_Printf( variableConditionStream, "\t\t\tvarIndex: %u\n", self->vcTbl[i][vcVar_I].varIndex);
				Journal_Printf( variableConditionStream, "\t\t\tvalIndex: %u\n", self->vcTbl[i][vcVar_I].valIndex);
			}
	Journal_Printf( variableConditionStream, "\tvalueCount: %u\n", self->valueCount);
	Journal_Printf( variableConditionStream, "\tvalueTbl (ptr): %p\n", self->valueTbl);
	if( self->valueTbl ) {
		for (val_I = 0; val_I < self->valueCount; val_I++)
		{
			Journal_Printf( variableConditionStream, "\t\tvalueTbl[%u]:\n", val_I);
			switch (self->valueTbl[val_I].type)
			{
				case VC_ValueType_Double:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_Double\n" );
					Journal_Printf( variableConditionStream, "\t\t\tasDouble: %g\n", self->valueTbl[val_I].as.typeDouble );
					break;
					
				case VC_ValueType_Int:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_Int\n" );
					Journal_Printf( variableConditionStream, "\t\t\tasInt: %i\n", self->valueTbl[val_I].as.typeInt );
					break;
					
				case VC_ValueType_Short:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_Short\n" );
					Journal_Printf( variableConditionStream, "\t\t\tasShort: %i\n", self->valueTbl[val_I].as.typeShort );
					break;
					
				case VC_ValueType_Char:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_Char\n");
					Journal_Printf( variableConditionStream, "\t\t\tasChar: %c\n", self->valueTbl[val_I].as.typeChar );
					break;
					
				case VC_ValueType_Ptr:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_Ptr\n");
					Journal_Printf( variableConditionStream, "\t\t\tasPtr: %g\n", self->valueTbl[val_I].as.typePtr );
					break;
					
				case VC_ValueType_DoubleArray:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_DoubleArray\n");
					Journal_Printf( variableConditionStream, "\t\t\tarraySize: %u\n", self->valueTbl[val_I].as.typeArray.size);
					Journal_Printf( variableConditionStream, "\t\t\tasDoubleArray (ptr): %p\n", self->valueTbl[val_I].as.typeArray.array);
					if (self->valueTbl[val_I].as.typeArray.array)
						for (i = 0; i < self->valueTbl[val_I].as.typeArray.size; i++)
							Journal_Printf( variableConditionStream, "\t\t\t\tasDoubleArray[%u]: %g\n", i,
								self->valueTbl[val_I].as.typeArray.array[i]);
					break;
					
				case VC_ValueType_CFIndex:
					Journal_Printf( variableConditionStream, "\t\t\ttype: VC_ValueType_CFIndex\n");
					Journal_Printf( variableConditionStream, "\t\t\tasCFIndex: %u\n", self->valueTbl[val_I].as.typeCFIndex);
					break;
			}
		}
	}
}


void* _VariableCondition_Copy( void* variableCondition, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
	VariableCondition*	self = (VariableCondition*)variableCondition;
	VariableCondition*	newVariableCondition;
	PtrMap*			map = ptrMap;
	Bool			ownMap = False;
	
	if( !map ) {
		map = PtrMap_New( 10 );
		ownMap = True;
	}
	
	newVariableCondition = (VariableCondition*)_Stg_Component_Copy( self, dest, deep, nameExt, map );
	
	/* Virtual methods */
	newVariableCondition->_buildSelf = self->_buildSelf;
	newVariableCondition->_printConcise = self->_printConcise;
	newVariableCondition->_readDictionary = self->_readDictionary;
	newVariableCondition->_getSet = self->_getSet;
	newVariableCondition->_getVariableCount = self->_getVariableCount;
	newVariableCondition->_getVariableIndex = self->_getVariableIndex;
	newVariableCondition->_getValueIndex = self->_getValueIndex;
	newVariableCondition->_getValueCount = self->_getValueCount;
	newVariableCondition->_getValue = self->_getValue;
	
	newVariableCondition->variable_Register = self->variable_Register;
	newVariableCondition->conFunc_Register = self->conFunc_Register;
	newVariableCondition->dictionary = self->dictionary;
	newVariableCondition->indexCount = self->indexCount;
	newVariableCondition->valueCount = self->valueCount;
	
	if( deep ) {
		newVariableCondition->_set = (IndexSet*)Stg_Class_Copy( self->_set, NULL, deep, nameExt, map );
		
		if( (newVariableCondition->indexTbl = (Index*)PtrMap_Find( map, self->indexTbl )) == NULL && self->indexTbl ) {
			newVariableCondition->indexTbl = (Index*)Memory_Alloc_Array( Index, newVariableCondition->indexCount, "VariableCondition->indexTbl" );
			memcpy( newVariableCondition->indexTbl, self->indexTbl, sizeof(Index) * newVariableCondition->indexCount );
			PtrMap_Append( map, newVariableCondition->indexTbl, self->indexTbl );
		}
		
		if( (newVariableCondition->vcVarCountTbl = (VariableCondition_StgVariableIndex*)PtrMap_Find( map, self->vcVarCountTbl )) == NULL && self->vcVarCountTbl ) {
			newVariableCondition->vcVarCountTbl = Memory_Alloc_Array( VariableCondition_StgVariableIndex, newVariableCondition->indexCount, "VC->vcVarCountTbl" );
			memcpy( newVariableCondition->vcVarCountTbl, self->vcVarCountTbl, sizeof(VariableCondition_StgVariableIndex) * newVariableCondition->indexCount );
			PtrMap_Append( map, newVariableCondition->vcVarCountTbl, self->vcVarCountTbl );
		}
		
		if( (newVariableCondition->vcTbl = (VariableCondition_Tuple**)PtrMap_Find( map, self->vcTbl )) == NULL && self->vcTbl ) {
			Index	idx_I;
			
			newVariableCondition->vcTbl = Memory_Alloc_2DComplex( VariableCondition_Tuple, newVariableCondition->indexCount, newVariableCondition->vcVarCountTbl, "VC->vcTbl" );
			for( idx_I = 0; idx_I < newVariableCondition->indexCount; idx_I++ ) {
				memcpy( newVariableCondition->vcTbl[idx_I], self->vcTbl[idx_I], sizeof(VariableCondition_Tuple) * newVariableCondition->vcVarCountTbl[idx_I] );
			}
			PtrMap_Append( map, newVariableCondition->vcTbl, self->vcTbl );
		}
		
		if( (newVariableCondition->valueTbl = (VariableCondition_Value*)PtrMap_Find( map, self->valueTbl )) == NULL && self->valueTbl ) {
			newVariableCondition->valueTbl = Memory_Alloc_Array( VariableCondition_Value, newVariableCondition->valueCount, "VC->valueTbl" );
			memcpy( newVariableCondition->valueTbl, self->valueTbl, sizeof(VariableCondition_Value) * newVariableCondition->indexCount );
			PtrMap_Append( map, newVariableCondition->valueTbl, self->valueTbl );
		}
	}
	else {
		newVariableCondition->_set = self->_set;
		newVariableCondition->indexTbl = self->indexTbl;
		newVariableCondition->vcVarCountTbl = self->vcVarCountTbl;
		newVariableCondition->vcTbl = self->vcTbl;
		newVariableCondition->valueTbl = self->valueTbl;
	}
	
	if( ownMap ) {
		Stg_Class_Delete( map );
	}
	
	return (void*)newVariableCondition;
}


/*--------------------------------------------------------------------------------------------------------------------------
** Macros
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Virtual functions
*/


/*--------------------------------------------------------------------------------------------------------------------------
** Build functions
*/

void _VariableCondition_AssignFromXML( void* variableCondition, Stg_ComponentFactory* cf, void* data ) {
}

void _VariableCondition_Build( void* variableCondition, void* data ) {
	VariableCondition*				self = (VariableCondition*)variableCondition;
	VariableCondition_ValueIndex	val_I;
	Index									i;
	
	/* Read the dictionary */
	self->_readDictionary( self, self->dictionary );
	
	/* Obtain the set */
	self->_set = self->_getSet(self);
	if (self->_set)
		IndexSet_GetMembers(self->_set, &self->indexCount, &self->indexTbl);
	else {
		self->indexCount = 0;
		self->indexTbl = NULL;
	}
	
	/* Only build the index related tables if there are active BCs */
	if ( self->indexCount ) {
		/* Build the variable to condition table */
		self->vcVarCountTbl = Memory_Alloc_Array( VariableCondition_StgVariableIndex, self->indexCount, "VC->vcVarCountTbl" );
		
		for (i = 0; i < self->indexCount; i++) {
			/* For the index, get the number of "variables" that have been assigned conditions */
			self->vcVarCountTbl[i] = self->_getVariableCount(self, self->indexTbl[i]);
		}

		self->vcTbl = Memory_Alloc_2DComplex( VariableCondition_Tuple, self->indexCount, self->vcVarCountTbl, "VC->vcTbl" );
		for ( i = 0; i < self->indexCount; i++ ) {
			VariableCondition_StgVariableIndex vcVar_I;

			for ( vcVar_I = 0; vcVar_I < self->vcVarCountTbl[i]; vcVar_I++ ) {
				StgVariable* var;

				/* For the index's variable, get the variable i.d. and value i.d. */
				self->vcTbl[i][vcVar_I].varIndex = self->_getVariableIndex(self, self->indexTbl[i], vcVar_I);
				self->vcTbl[i][vcVar_I].valIndex = self->_getValueIndex(self, self->indexTbl[i], vcVar_I);

				/* Force the building of the variable (to be safe) */
				var = self->variable_Register->_variable[self->vcTbl[i][vcVar_I].varIndex];
				Stg_Component_Build( var, data, False );
			}
		}
	}	

	self->valueCount = self->_getValueCount(self);
	self->valueTbl = Memory_Alloc_Array( VariableCondition_Value, self->valueCount, "VC->valueTbl" );
	for (val_I = 0; val_I < self->valueCount; val_I++)
		self->valueTbl[val_I] = self->_getValue(self, val_I);

	/* Build mapping. */
	self->mapping = UIntMap_New();
	for( i = 0; i < self->indexCount; i++ )
		UIntMap_Insert( self->mapping, self->indexTbl[i], i );
}

void _VariableCondition_Initialise( void* variableCondition, void* data ) {
	VariableCondition*	self = (VariableCondition*)variableCondition;
	Index						i;
	Index                       cf_I;

	/* lets init any condition functions that require it */
	if(self->conFunc_Register){
	    for( cf_I = 0; cf_I < self->conFunc_Register->count; cf_I++ ) {
	       if( self->conFunc_Register->_cf[cf_I]->init )
	          ConditionFunction_InitFunc(self->conFunc_Register->_cf[cf_I], self->context);
	    }
	}
	
	for( i = 0; i < self->indexCount; i++ ) {
		VariableCondition_StgVariableIndex	vcVar_I;
		
		for( vcVar_I = 0; vcVar_I < self->vcVarCountTbl[i]; vcVar_I++ ) {
			StgVariable* var;
			
			/* Force the building of the variable (to be safe) */
			var = self->variable_Register->_variable[self->vcTbl[i][vcVar_I].varIndex];
			Stg_Component_Initialise( var, data, False );
		}
	}
}

void _VariableCondition_Execute( void* variableCondition, void* data ) {
	VariableCondition* self = (VariableCondition*)variableCondition;
	
	VariableCondition_Apply( self, data );
}

void _VariableCondition_Destroy( void* variableCondition, void* data ) {
	VariableCondition* self = (VariableCondition*)variableCondition;

	if (self->mapping) Stg_Class_Delete(self->mapping);
	if (self->_set) Stg_Class_Delete(self->_set);
	if (self->indexTbl) Memory_Free(self->indexTbl);
	if (self->vcVarCountTbl) Memory_Free(self->vcVarCountTbl);
	if (self->vcTbl) Memory_Free(self->vcTbl);
	if (self->valueTbl) Memory_Free(self->valueTbl);
}

void _VariableCondition_Apply( void* variableCondition, void* context ) {
	VariableCondition*	self = (VariableCondition*)variableCondition;
	Index						i;
	
	for (i = 0; i < self->indexCount; i++)
		VariableCondition_ApplyToIndex(variableCondition, self->indexTbl[i], context);
}


/*--------------------------------------------------------------------------------------------------------------------------
** Functions
*/

void VariableCondition_Apply( void* variableCondition, void* context ) {
	assert( variableCondition );
	((VariableCondition*)variableCondition)->_apply( variableCondition, context );
}

Bool VariableCondition_IsCondition( void* variableCondition, Index localIndex, Index variableIndex ) {
	assert( variableCondition );
	return ((VariableCondition*)variableCondition)->_isCondition( variableCondition, localIndex, variableIndex );
}


void VariableCondition_ApplyToVariable( void* variableCondition, VariableCondition_StgVariableIndex varIndex, void* context ) {
	VariableCondition*	self = (VariableCondition*)variableCondition;
	Index			i;
	
	for (i = 0; i < self->indexCount; i++)
		VariableCondition_ApplyToIndexVariable(variableCondition, self->indexTbl[i], varIndex, context);
}


void VariableCondition_ApplyToIndex( void* variableCondition, Index localIndex, void* context ) {
	VariableCondition*		self = (VariableCondition*)variableCondition;
	StgVariable*			var;
	StgVariable_Index			varIndex;
	VariableCondition_ValueIndex	val_I;
	ConditionFunction*		cf;
	Index				index, i;
	Stream*				errorStr = Journal_Register( Error_Type, self->type );

	/* Ensure that the index provided (localIndex) has a condition attached to it */
	insist( UIntMap_Map( self->mapping, localIndex, &index ), == True );
	
	/* For each variable that has a condition at this index */
	for (i = 0; i < self->vcVarCountTbl[index]; i++)
	{
		varIndex = self->vcTbl[index][i].varIndex;
		assert( varIndex != (unsigned)-1 );
		
		var = self->variable_Register->_variable[varIndex];
		
		val_I = self->vcTbl[index][i].valIndex;
		assert( val_I != (unsigned)-1 );
		
		switch (self->valueTbl[val_I].type)
		{
			case VC_ValueType_Double:
				Journal_Firewall( var->dataTypeCounts[0] == 1, errorStr,
					"Error - in %s: while applying values for variable condition "
					"\"%s\", to index %d - asked to apply a scalar %s to Variable \"%s\" "
					"which has %d components. Specify a scalar Variable instead.\n",
					__func__, self->name, self->indexTbl[index], "double",
					var->name, var->dataTypeCounts[0] );
				StgVariable_SetValueDouble(
					var, 
					self->indexTbl[index], 
					self->valueTbl[val_I].as.typeDouble );
				break;
			
			case VC_ValueType_DoubleArray:
				StgVariable_SetValue(
					var, 
					self->indexTbl[index], 
					self->valueTbl[val_I].as.typeArray.array );
				break;
			
			case VC_ValueType_CFIndex:
				Journal_Firewall( self->valueTbl[val_I].as.typeCFIndex != (unsigned)-1, errorStr,
					"Error - in %s: trying to apply to index %d of variable \"%s\", which "
					"is supposed to be a condition function, but the cond. func. wasn't "
					"found in the c.f. register.\n", __func__, localIndex, var->name );
				cf = self->conFunc_Register->_cf[self->valueTbl[val_I].as.typeCFIndex];
				ConditionFunction_Apply(
					cf, 
					localIndex, 
					varIndex, 
					context, 
					StgVariable_GetStructPtr( var, self->indexTbl[index] ) );
				break;
			
			case VC_ValueType_Int:
				Journal_Firewall( var->dataTypeCounts[0] == 1, errorStr,
					"Error - in %s: while applying values for variable condition "
					"\"%s\", to index %d - asked to apply a scalar %s to Variable \"%s\" "
					"which has %d components. Specify a scalar Variable instead.\n",
					__func__, self->name, self->indexTbl[index], "int",
					var->name, var->dataTypeCounts[0] );
				StgVariable_SetValueInt(
					var, 
					self->indexTbl[index], 
					self->valueTbl[val_I].as.typeInt );
				break;
			
			case VC_ValueType_Short:
				Journal_Firewall( var->dataTypeCounts[0] == 1, errorStr,
					"Error - in %s: while applying values for variable condition "
					"\"%s\", to index %d - asked to apply a scalar %s to Variable \"%s\" "
					"which has %d components. Specify a scalar Variable instead.\n",
					__func__, self->name, self->indexTbl[index], "short",
					var->name, var->dataTypeCounts[0] );
				StgVariable_SetValueShort(
					var, 
					self->indexTbl[index], 
					self->valueTbl[val_I].as.typeShort );
				break;
			
			case VC_ValueType_Char:
				Journal_Firewall( var->dataTypeCounts[0] == 1, errorStr,
					"Error - in %s: while applying values for variable condition "
					"\"%s\", to index %d - asked to apply a scalar %s to Variable \"%s\" "
					"which has %d components. Specify a scalar Variable instead.\n",
					__func__, self->name, self->indexTbl[index], "char",
					var->name, var->dataTypeCounts[0] );
				StgVariable_SetValueChar(
					var, 
					self->indexTbl[index], 
					self->valueTbl[val_I].as.typeChar );
				break;
			
			case VC_ValueType_Ptr:
				StgVariable_SetValuePointer(
					var, 
					self->indexTbl[index], 
					self->valueTbl[val_I].as.typePtr );
				break;
			
			default:
				assert(0);
				break;
		}
	}
}


void VariableCondition_ApplyToIndexVariable(
		void*				variableCondition, 
		Index				localIndex, 
		VariableCondition_StgVariableIndex	varIndex,
		void*				context )
{
	VariableCondition*	self = (VariableCondition*)variableCondition;
	StgVariable_Index		globalVarIndex;
	StgVariable*		var;
	ConditionFunction*	cf;
	Index			index;
	
/*
 * NOTE: This algorithm here is RIDICULOUSLY slow. I've added a mapping
 *       to the class, that should help.

	for (index = 0; index < self->indexCount; index++)
		if (self->indexTbl[index] == localIndex)
			break;
*/

	if(!UIntMap_Map( self->mapping, localIndex, &index ))
	    return;
	
	globalVarIndex = self->vcTbl[index][varIndex].varIndex;
	var = self->variable_Register->_variable[globalVarIndex];
		
	switch (self->valueTbl[self->vcTbl[index][varIndex].valIndex].type)
	{
		case VC_ValueType_Double:
			StgVariable_SetValueDouble(
				var, 
				self->indexTbl[index], 
				self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typeDouble );
			break;
		
		case VC_ValueType_CFIndex:
			cf = self->conFunc_Register->_cf[self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typeCFIndex];
			ConditionFunction_Apply( 
				cf, 
				localIndex, 
				globalVarIndex, 
				context, 
				StgVariable_GetStructPtr( var, self->indexTbl[index]) );
			break;
		
		case VC_ValueType_DoubleArray:
			StgVariable_SetValue(
				var, 
				self->indexTbl[index], 
				self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typeArray.array );
			break;
		
		case VC_ValueType_Int:
			StgVariable_SetValueInt(
				var, 
				self->indexTbl[index], 
				self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typeInt );
			break;
		
		case VC_ValueType_Short:
			StgVariable_SetValueShort(
				var, 
				self->indexTbl[index], 
				self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typeShort );
			break;
		
		case VC_ValueType_Char:
			StgVariable_SetValueChar(
				var, 
				self->indexTbl[index], 
				self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typeChar );
			break;
		
		case VC_ValueType_Ptr:
			StgVariable_SetValuePointer(
				var, 
				self->indexTbl[index], 
				self->valueTbl[self->vcTbl[index][varIndex].valIndex].as.typePtr );
			break;
		
		default:
			assert(0);
			break;
	}
}


Bool _VariableCondition_IsCondition( void* variableCondition, Index localIndex, Index inputVarIndex ) {
	VariableCondition*		self = (VariableCondition*)variableCondition;
	VariableCondition_StgVariableIndex	vcVar_I;
	Index				i;
	StgVariable_Index                  varIndexToTryMatch = 0;
	StgVariable_Index                  subVarIndexToTryMatch = 0;
	StgVariable*                       variableToTryMatch = NULL;
	StgVariable*                       subVariableToTryMatch = NULL;
	StgVariable_Index                  subVariable_I = 0;

	/* if the set isn't initialised, this is a NULL BC : False */
	if ( !self->_set ) {
		return False;
	}

	/* first check if the index they've given us is actually in the list this VC applies to */
	/* quick check, since we have the set available */
	if ( localIndex >= self->_set->size ) {
		Stream* warning = Journal_Register( ErrorStream_Type, self->type );
		Journal_Printf( warning, "Error- In %s: Tried to check an index %d larger than the size of "
			"the set (%d).\n", __func__, localIndex, self->_set->size );
		assert(0);	
		return False;
	}

	if(!UIntMap_Map( self->mapping, localIndex, &i ))
	    return False;
/*
	if ( !IndexSet_IsMember( self->_set, localIndex ) ) {
		return False;
	}
	for (i = 0; i < self->indexCount; i++)
		if (self->indexTbl[i] == localIndex)
			break;
			
	if (i == self->indexCount)
		return False;
*/
	
	/* now check if the Variable they've given us is actually in the list to apply at the given index */
	for (vcVar_I = 0; vcVar_I < self->vcVarCountTbl[i]; vcVar_I++) {
		varIndexToTryMatch = self->vcTbl[i][vcVar_I].varIndex;
		variableToTryMatch = self->variable_Register->_variable[varIndexToTryMatch];

		if ( varIndexToTryMatch == inputVarIndex) {
			return True;
		}
		else if ( variableToTryMatch->subVariablesCount >= 1 ) {
			/* 2nd chance draw is that if this Var has sub-components, we should test if the input argument is
			 * actually one of those - in which case we should consider it has a condition applied to it. */
			
			for ( subVariable_I = 0; subVariable_I < variableToTryMatch->subVariablesCount; subVariable_I++ ) {
			  /* TODO: next few lines bit slow! Maybe need to cache subvar indices on variable */
				subVariableToTryMatch = variableToTryMatch->components[subVariable_I];
				if ( subVariableToTryMatch == NULL ) continue;
				
				subVarIndexToTryMatch = Variable_Register_GetIndex( self->variable_Register,
					subVariableToTryMatch->name );

				if ( subVarIndexToTryMatch == inputVarIndex) {
					return True;
				}
			}
		}
	}
	
	return False;
}


VariableCondition_ValueIndex VariableCondition_GetValueIndex (void* variableCondition, Index localIndex, StgVariable_Index varIndex )
{
	VariableCondition*		self = (VariableCondition*)variableCondition;
	VariableCondition_StgVariableIndex	vcVar_I;
	Index				i;
	
	/* if the set isn't initialised, this is a NULL BC : False */
	if ( !self->_set ) {
		return False;
	}

	/* first check if the index they've given us is actually in the list this VC applies to */
	/* quick check, since we have the set available */
	if ( localIndex >= self->_set->size ) {
		Stream* warning = Journal_Register( ErrorStream_Type, self->type );
		Journal_Printf( warning, "Error- In %s: Tried to check an index %d larger than the size of "
			"the set (%d).\n", __func__, localIndex, self->_set->size );
		assert(0);	
		return False;
	}
	if ( !IndexSet_IsMember( self->_set, localIndex ) ) {
		return (VariableCondition_ValueIndex)-1;
	}
	for (i = 0; i < self->indexCount; i++)
		if (self->indexTbl[i] == localIndex)
			break;
			
	if (i == self->indexCount)
		return (VariableCondition_ValueIndex)-1;
	
	/* now check if the Variable they've given us is actually in the list to apply at the given index */
	for (vcVar_I = 0; vcVar_I < self->vcVarCountTbl[i]; vcVar_I++)
		if (self->vcTbl[i][vcVar_I].varIndex == varIndex)
			return self->vcTbl[i][vcVar_I].valIndex;
	
	return (VariableCondition_ValueIndex)-1;
}


void VariableCondition_PrintConcise( void* variableCondition, Stream* stream ) {
	VariableCondition*		self = (VariableCondition*)variableCondition;
	
	self->_printConcise( self, stream );
}


