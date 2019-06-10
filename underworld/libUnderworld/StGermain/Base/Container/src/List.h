/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Container_List_h__
#define __StGermain_Base_Container_List_h__

   /** Textual name of this class */
   extern const Type List_Type;

   /** Virtual function types */

   /** Class contents */
   #define __List \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      \
      /* List info */ \
      unsigned  nItems; \
      Stg_Byte* items; \
      unsigned  itemSize; \
      unsigned  maxItems; \
      unsigned  delta;

   struct List { __List };

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Constructors
   */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define LIST_DEFARGS \
      STG_CLASS_DEFARGS

   #define LIST_PASSARGS \
      STG_CLASS_PASSARGS

   List* List_New();
   List* _List_New( LIST_DEFARGS );
   void _List_Init( List* self );

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Virtual functions
   */

   void _List_Delete( void* list );
   void _List_Print( void* list, Stream* stream );

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Public functions
   */

   void List_SetDelta( void* list, unsigned delta );
   void List_SetItemSize( void* list, unsigned itemSize );
   void List_Clear( void* list );

   void List_Insert( void* list, unsigned index, void* data );
   void List_Append( void* list, void* data );
   void List_Prepend( void* list, void* data );
   void List_Remove( void* list, void* data );

   void* List_GetItem( void* list, unsigned index );
   unsigned List_GetSize( void* list );
   Bool List_Exists( void* list, void* data );

   #define List_Get( list, index, type )      \
      ((type*)List_GetItem( list, index ))

   /*--------------------------------------------------------------------------------------------------------------------------
   ** Private Member functions
   */

   void List_Expand( List* self );
   void List_Contract( List* self );
   void List_Destruct( List* self );

#endif /* __StGermain_Base_Container_List_h__ */

