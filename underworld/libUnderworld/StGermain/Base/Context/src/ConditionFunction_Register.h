/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_ConditionFunction_Register_h__
#define __StGermain_Base_Context_ConditionFunction_Register_h__
   
   extern const Type ConditionFunction_Register_Type;

   /* Global default instantiation of the register, created in Init.c */
   extern ConditionFunction_Register* condFunc_Register;   
   
   #define __ConditionFunction_Register \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      \
      /* Stg_Class info */ \
      ConditionFunction_Index count; \
      SizeT                   _size; \
      SizeT                   _delta; \
      ConditionFunction**     _cf; \
      
   struct _ConditionFunction_Register { __ConditionFunction_Register };
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Constructor
   */
   
   ConditionFunction_Register* ConditionFunction_Register_New(void);
   
   void ConditionFunction_Register_Init(ConditionFunction_Register* self);
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define CONDITIONFUNCTION_REGISTER_DEFARGS \
      STG_CLASS_DEFARGS

   #define CONDITIONFUNCTION_REGISTER_PASSARGS \
      STG_CLASS_PASSARGS

   ConditionFunction_Register* _ConditionFunction_Register_New( CONDITIONFUNCTION_REGISTER_DEFARGS );
      
   void _ConditionFunction_Register_Init( void* conditionFunction_Register );
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** General virtual functions
   */
   
   void _ConditionFunction_Register_Delete( void* conditionFunction_Register );
   
   void _ConditionFunction_Register_Print( void* conditionFunction_Register, Stream* stream );
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Macros
   */


   /*--------------------------------------------------------------------------------------------------------------------------
   ** Virtual functions
   */


   /*--------------------------------------------------------------------------------------------------------------------------
   ** Build functions
   */
   
   ConditionFunction_Index ConditionFunction_Register_Add( void* conditionFunction_Register, ConditionFunction* cf );
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Functions
   */
   
   ConditionFunction_Index ConditionFunction_Register_GetIndex( void* conditionFunction_Register, Name name );

   void ConditionFunction_Register_PrintNameOfEachFunc( void* conditionFunction_Register, Stream* stream );

   
#endif /* __StGermain_Base_Context_ConditionFunction_Register_h__ */

