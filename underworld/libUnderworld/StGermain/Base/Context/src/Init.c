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
#include "Variable.h"
#include "Variable_Register.h"
#include "VariableCondition.h"
#include "VariableCondition_Register.h"
#include "ConditionFunction.h"
#include "ConditionFunction_Register.h"
#include "PythonVC.h"
#include "AbstractContext.h"
#include "ContextEntryPoint.h"
#include "Init.h"

#include <stdio.h>

#define PY_ARRAY_UNIQUE_SYMBOL stg_ARRAY_API
#include <numpy/arrayobject.h>
#define NUMPY_IMPORT_ARRAY_RETVAL False

/* Note that the following is required as `import_array()` is */
/* actually a macro which calls `return` under certain        */
/* circumstances. In itself that is fine, but because         */
/* `BaseContext_Init()` returns `Bool` some compilers         */
/* complain. This was encoutered first for `numpy` 1.19.0.    */
/* Perhaps this call should go somewhere else altogether,     */
/* like `main()`.                                             */
void numpy_import(){
   import_array();
}


Bool BaseContext_Init( int* argc, char** argv[] ) {
   Stream* typedStream;
   
   import_numpy();
      
   Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ );

   /* Create global ConditionFunction register. */
   condFunc_Register = ConditionFunction_Register_New();

   /* Create global VariableCondition register. */
   variableCondition_Register = VariableCondition_Register_New();
   
   /* Adding default constructors of various components to the Stg_ComponentRegister */
   Stg_ComponentRegister_Add(
      Stg_ComponentRegister_Get_ComponentRegister(), StgVariable_Type,
      (Name)"0", (Stg_Component_DefaultConstructorFunction*)_StgVariable_DefaultNew );

   Stg_ComponentRegister_Add(
      Stg_ComponentRegister_Get_ComponentRegister(), PythonVC_Type,
      "0", (Stg_Component_DefaultConstructorFunction*)_PythonVC_DefaultNew );


   /* Register Parents for All Classes */
   RegisterParent( StgVariable_Type, Stg_Component_Type );
   RegisterParent( VariableCondition_Register_Type, Stg_Class_Type );
   RegisterParent( Variable_Register_Type, Stg_Class_Type );
   RegisterParent( VariableCondition_Type, Stg_Component_Type );
   RegisterParent( ConditionFunction_Type, Stg_Class_Type );
   RegisterParent( ConditionFunction_Register_Type, Stg_Class_Type );
   RegisterParent( PythonVC_Type, VariableCondition_Type );
   RegisterParent( AbstractContext_Type, Stg_Component_Type );
   RegisterParent( ContextEntryPoint_Type, EntryPoint_Type );
   
   return True;
}


