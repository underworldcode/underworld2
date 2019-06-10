/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgDomain_Utils_FieldVariable_Register_h__
#define __StgDomain_Utils_FieldVariable_Register_h__
   
   extern const Type FieldVariable_Register_Type;
   
   #define __FieldVariable_Register \
      /* General info */ \
      __NamedObject_Register \
      \
      /* Virtual info */ \
      \
      /* Stg_Class info */ \

   struct FieldVariable_Register { __FieldVariable_Register };
   
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Constructor
   */
   
   FieldVariable_Register* FieldVariable_Register_New( void );
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** General virtual functions
   */
   
   /*--------------------------------------------------------------------------------------------------------------------------
   ** Public functions
   */
   #define FieldVariable_Register_Add NamedObject_Register_Add

   #define FieldVariable_Register_GetIndex NamedObject_Register_GetIndex 

   #define FieldVariable_Register_GetByName( self, fieldVariableName ) \
      ( (FieldVariable*) NamedObject_Register_GetByName( self, fieldVariableName ) ) 

   #define FieldVariable_Register_GetByIndex( self, fieldVariableIndex ) \
      ( (FieldVariable*) NamedObject_Register_GetByIndex( self, fieldVariableIndex ) )

   #define FieldVariable_Register_PrintAllEntryNames NamedObject_Register_PrintAllEntryNames

#endif /* __StgDomain_Utils_FieldVariable_Register_h__ */
