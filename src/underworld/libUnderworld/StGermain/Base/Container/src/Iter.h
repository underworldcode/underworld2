/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_Iter_h__
#define __StGermain_Base_Container_Iter_h__

   extern const Type Iter_Type;

   typedef void (Iter_NextFunc)( void* self );
        
   #define __Iter \
      __Stg_Class \
      Iter_NextFunc* nextFunc; \
      Bool           valid;

   struct Iter { __Iter };

   #ifndef ZERO
   #define ZERO 0
   #endif

   #define ITER_DEFARGS \
      STG_CLASS_DEFARGS, \
         Iter_NextFunc* nextFunc

   #define ITER_PASSARGS \
      STG_CLASS_PASSARGS, \
         nextFunc

   Iter* _Iter_New( ITER_DEFARGS );

   void _Iter_Init( void* self );

   void _Iter_Delete( void* self );

   void Iter_Copy( void* self, const void* op );

   void Iter_Next( void* self );

   Bool Iter_IsValid( void* self );

#endif /* __StGermain_Base_Container_Iter_h__ */
