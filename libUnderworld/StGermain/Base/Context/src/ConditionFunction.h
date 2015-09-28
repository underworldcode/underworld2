/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_ConditionFunction_h__
#define __StGermain_Base_Context_ConditionFunction_h__


   typedef void (ConditionFunction_ApplyFunc) ( Index index, Variable_Index var_I, void* context, void* data, void* result );
   typedef void (ConditionFunction_InitFunc)  ( void* context, void* data );
   
   extern const Type ConditionFunction_Type;
   
   #define __ConditionFunction \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      \
      /* Stg_Class info */ \
      Dictionary*                  dictionary; \
      ConditionFunction_ApplyFunc* apply; \
      ConditionFunction_InitFunc*  init;  \
      Name                         name;  \
      void*                        data;
      
   struct _ConditionFunction { __ConditionFunction };
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Constructor
   */
   
   ConditionFunction* ConditionFunction_New( ConditionFunction_ApplyFunc* apply, Name name, void* data );
   ConditionFunction* ConditionFunction_New2( ConditionFunction_ApplyFunc* apply, ConditionFunction_InitFunc* init, Name name, void* data );
      
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define CONDITIONFUNCTION_DEFARGS \
      STG_CLASS_DEFARGS, \
      ConditionFunction_ApplyFunc* apply, \
      ConditionFunction_InitFunc*  init, \
      Name                         name,  \
      void*                        data

   #define CONDITIONFUNCTION_PASSARGS \
      STG_CLASS_PASSARGS, \
      apply, \
      init, \
      name,  \
      data

   ConditionFunction* _ConditionFunction_New( CONDITIONFUNCTION_DEFARGS );
   
   void _ConditionFunction_Init( void* conditionFunction, ConditionFunction_ApplyFunc* apply, ConditionFunction_InitFunc* init, Name name, void* data );
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** General virtual functions
   */

   void _ConditionFunction_Delete( void* conditionFunction );
   
   void _ConditionFunction_Print( void* conditionFunction, Stream* stream );
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Macros
   */
   
   #define ConditionFunction_Apply(self, index, var_I, context, result) \
      (self)->apply((index), (var_I), (context), (self->data), (result))

   #define ConditionFunction_InitFunc(self, context) \
      (self)->init( (self->data), (context) )
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Virtual functions
   */
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Build functions
   */
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Functions
   */
   
   /* Apply the condition function */
   void _ConditionFunction_Apply( 
      void*          conditionFunction,
      Index          index,
      Variable_Index var_I,
      void*          context,
      void*          result);


#endif /* __StGermain_Base_Context_ConditionFunction_h__ */

