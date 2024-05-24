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
#include "StGermain/Base/Extensibility/src/Extensibility.h"

#include "types.h"
#include "Variable.h"
#include "Variable_Register.h"
#include "VariableCondition.h"
#include "VariableCondition_Register.h"
#include "ConditionFunction.h"
#include "ConditionFunction_Register.h"
#include "ContextEntryPoint.h"
#include "AbstractContext.h"
#include "PythonVC.h"

#include <string.h>
#include <assert.h>


const Type PythonVC_Type = "PythonVC";

/*--------------------------------------------------------------------------------------------------------------------------
** Constructor
*/

PythonVC* _PythonVC_DefaultNew( Name name ) {
	/* Variables set in this function */
	SizeT                                               _sizeOfSelf = sizeof(PythonVC);
	Type                                                       type = PythonVC_Type;
	Stg_Class_DeleteFunction*                               _delete = _PythonVC_Delete;
	Stg_Class_PrintFunction*                                 _print = NULL;
	Stg_Class_CopyFunction*                                   _copy = NULL;
	Stg_Component_DefaultConstructorFunction*   _defaultConstructor = (Stg_Component_DefaultConstructorFunction*)_PythonVC_DefaultNew;
	Stg_Component_ConstructFunction*                     _construct = _PythonVC_AssignFromXML;
	Stg_Component_BuildFunction*                             _build = _PythonVC_Build;
	Stg_Component_InitialiseFunction*                   _initialise = _PythonVC_Initialise;
	Stg_Component_ExecuteFunction*                         _execute = NULL;
	Stg_Component_DestroyFunction*                         _destroy = _PythonVC_Destroy;
	AllocationType                               nameAllocationType = NON_GLOBAL;
	VariableCondition_BuildSelfFunc*                     _buildSelf = NULL;
	VariableCondition_PrintConciseFunc*               _printConcise = NULL;
	VariableCondition_ReadDictionaryFunc*           _readDictionary = NULL;
	VariableCondition_GetSetFunc*                           _getSet = NULL;
	VariableCondition_GetVariableCountFunc*       _getVariableCount = NULL;
	VariableCondition_GetVariableIndexFunc*       _getVariableIndex = NULL;
	VariableCondition_GetValueIndexFunc*             _getValueIndex = NULL;
	VariableCondition_GetValueCountFunc*             _getValueCount = NULL;
	VariableCondition_GetValueFunc*                       _getValue = NULL;
	VariableCondition_ApplyFunc*                             _apply = _PythonVC_Apply;
	VariableCondition_IsConditionFunc*                 _isCondition = _PythonVC_IsCondition;

	return _PythonVC_New(  PYTHONVC_PASSARGS  );
}

PythonVC* _PythonVC_New(  PYTHONVC_DEFARGS  ) {
	PythonVC* self;
	
	/* Allocate memory/General info */
	assert( _sizeOfSelf >= sizeof(PythonVC) );
	self = (PythonVC*)_VariableCondition_New(  VARIABLECONDITION_PASSARGS  );
	
	/* Virtual info */
	
	/* Stg_Class info */
    self->_indexSets = NULL;
	self->_indexSetCount = 0;

	return self;
}


/*--------------------------------------------------------------------------------------------------------------------------
** General virtual functions
*/

void _PythonVC_SetupIndexSetArray( void* pythonVC, unsigned count ) {
    PythonVC* self = (PythonVC*)pythonVC;
    int ii;

    if (self->_indexSets) Memory_Free(self->_indexSets);

    self->_indexSetCount = count;
    self->_indexSets = (IndexSet**)Memory_Alloc_Array( IndexSet*, count, "IndexSets" );
    for (ii=0; ii<count; ii++)
        self->_indexSets[ii] = NULL;
}

void _PythonVC_SetIndexSetAtArrayPosition( void* pythonVC, IndexSet* indexSet, unsigned position ) {
	PythonVC*	self = (PythonVC*)pythonVC;

    Journal_Firewall( position < self->_indexSetCount, NULL,
        "Error - IndexSet position is outside IndexSet array." );
    self->_indexSets[position] = indexSet;

}

void _PythonVC_Delete( void* pythonVC ) {
	PythonVC*	self = (PythonVC*)pythonVC;
    
    if (self->_indexSets) Memory_Free(self->_indexSets);

    _VariableCondition_Delete(self);
}

void _PythonVC_Destroy( void* pythonVC, void* data ) {_VariableCondition_Destroy( pythonVC, data );}

void _PythonVC_Build( void* pythonVC, void* data ) {}

void _PythonVC_Initialise( void* pythonVC, void* data ) {}

void _PythonVC_AssignFromXML( void* pythonVC, Stg_ComponentFactory* cf, void* data ) {}

void _PythonVC_Apply( void* variableCondition, void* context ) {}

Bool _PythonVC_IsCondition( void* pythonVC, Index localIndex, Index variableIndex ) {
	PythonVC*	self = (PythonVC*)pythonVC;
    
    if (self->_indexSets[variableIndex] != NULL)
        return IndexSet_IsMember( self->_indexSets[variableIndex], localIndex );

    return False;

}
