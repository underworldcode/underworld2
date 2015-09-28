/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_types_h__
#define __StGermain_Base_Context_types_h__
   
   /* types/classes */
   typedef struct Codelet Codelet;
   typedef struct _Variable Variable;
   typedef struct _Variable_Register Variable_Register;
   typedef struct _VariableCondition VariableCondition;
   typedef struct PythonVC PythonVC;
   typedef struct _VariableCondition_Register VariableCondition_Register;
   typedef struct _ConditionFunction ConditionFunction;
   typedef struct _ConditionFunction_Register ConditionFunction_Register;
   typedef Index ConditionFunction_Index;

   /* Variable_Register types */
   typedef Index Variable_Set_Index;
   typedef Index Variable_Index;
   typedef Index Dof_Index;
   
   /* VariableCondition_Register types */
   typedef struct _VariableCondition_Register_Entry VariableCondition_Register_Entry;
   
   /* VariableCondition types */
   typedef enum {
      VC_ValueType_Double = 1,
      VC_ValueType_Int,
      VC_ValueType_Short,
      VC_ValueType_Char,
      VC_ValueType_Ptr,
      VC_ValueType_DoubleArray,
      VC_ValueType_CFIndex,
      VC_ValueType_None
   } VariableCondition_ValueType;
   
   typedef Index VariableCondition_Index;
   typedef struct _VariableCondition_Value VariableCondition_Value;
   typedef struct _VariableCondition_Tuple VariableCondition_Tuple;
   typedef Index VariableCondition_ValueIndex;
   typedef Index VariableCondition_VariableIndex;

   /* Context types/classes */
   typedef struct AbstractContext AbstractContext;
   typedef struct ContextEntryPoint ContextEntryPoint;
   
   typedef AbstractContext Context;
   
   /* AbstractContext types */
   typedef struct Context_CallInfo Context_CallInfo;

   typedef Stg_ObjectList Pointer_Register;
   typedef Stg_ObjectList Register_Register;
   
#endif /* __StGermain_Base_Context_types_h__ */
