/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_STreeMap_h__
#define __StGermain_Base_Container_STreeMap_h__

extern const Type STreeMap_Type;

   #define __STreeMap \
      __STree \
      int keySize; \
      int valSize;

   struct STreeMap { __STreeMap };

   #ifndef ZERO
   #define ZERO 0
   #endif

   #define STREEMAP_DEFARGS \
      STREE_DEFARGS

   #define STREEMAP_PASSARGS \
      STREE_PASSARGS

   STreeMap* STreeMap_New();

   STreeMap* _STreeMap_New( STREE_DEFARGS );

   void STreeMap_Init( STreeMap* self );

   void _STreeMap_Init( void* self );

   void STreeMap_Destruct( STreeMap* self );

   void _STreeMap_Delete( void* self );

   void STreeMap_SetItemSize( void* _self, int keySize, int valSize );

   void STreeMap_Insert( void* _self, const void* key, const void* val );

   void STreeMap_Copy( void* self, const void* op );

   void* STreeMap_Map( const void* _self, const void* key );

   Bool STreeMap_HasKey( const void* _self, const void* key );

#endif /* __StGermain_Base_Container_STreeMap_h__ */
