/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_Stg_ObjectList_h__
#define __StGermain_Base_Foundation_Stg_ObjectList_h__

   /* function prototype interface */
   typedef Index (Stg_ObjectList_AppendFunction) ( void* objects, void* objectPtr );
   typedef Index (Stg_ObjectList_PrependFunction) ( void* objects, void* objectPtr );
   typedef Index (Stg_ObjectList_ReplaceAllFunction)
      ( void* objects, ReplacementOption option, void* objectPtr );
   typedef Index (Stg_ObjectList_ReplaceFunction)
      ( void* objs, Name toReplace, ReplacementOption option, void* objectPtr );
   typedef Index (Stg_ObjectList_InsertBeforeFunction)
      ( void* objects, Name reference, void* objectPtr );
   typedef Index (Stg_ObjectList_InsertAfterFunction)
      ( void* objects, Name reference, void* objectPtr );
   typedef Index (Stg_ObjectList_RemoveFunction)
      ( void* objects, Name reference, ReplacementOption option );
   typedef Index (Stg_ObjectList_GetIndexFunction) ( void* objects, const Name toGet );
   typedef void* (Stg_ObjectList_GetFunction) ( void* objects, const Name toGet );
   typedef void (Stg_ObjectList_AllocMoreMemoryFunction) ( void* objects );
   typedef void (Stg_ObjectList_InsertAtIndexFunction)
      ( void* objects, Index index, void* objectPtr );
   typedef void (Stg_ObjectList_RemoveByIndexFunction)
      ( void* objects, Index index, ReplacementOption option );
   typedef void (Stg_ObjectList_DeleteAllObjectsFunction) ( void* objects );
   
   /** Textual name of this class */
   extern const Type Stg_ObjectList_Type;

   extern const int DEFAULT_LIST_INITIAL_SIZE;
   extern const int DEFAULT_LIST_DELTA;

   /** \def __ObjectList See ObjectList */
   #define __Stg_ObjectList \
      /* General info */ \
      __Stg_Class \
      \
      /* Virtual info */ \
      Stg_ObjectList_AppendFunction*           _append; \
      Stg_ObjectList_PrependFunction*          _prepend; \
      Stg_ObjectList_ReplaceAllFunction*       _replaceAll; \
      Stg_ObjectList_ReplaceFunction*          _replace; \
      Stg_ObjectList_InsertBeforeFunction*     _insertBefore; \
      Stg_ObjectList_InsertAfterFunction*      _insertAfter; \
      Stg_ObjectList_RemoveFunction*           _remove; \
      Stg_ObjectList_GetIndexFunction*         _getIndex; \
      Stg_ObjectList_GetFunction*              _get; \
      Stg_ObjectList_AllocMoreMemoryFunction*  _allocMoreMemory; \
      Stg_ObjectList_InsertAtIndexFunction*    _insertAtIndex; \
      Stg_ObjectList_RemoveByIndexFunction*    _removeByIndex; \
      Stg_ObjectList_DeleteAllObjectsFunction* _deleteAllObjects; \
      \
      /* Stg_ObjectList info */ \
      Stg_ObjectPtr* data; \
      Index          count; \
      Index          _size; \
      Index          _delta; \
      Bool           _noJournalingInCopy;

   struct _Stg_ObjectList { __Stg_ObjectList };
   
   /* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   /* Object new and construct interface */
   #define Stg_ObjectList_New() \
      Stg_ObjectList_New2( DEFAULT_LIST_INITIAL_SIZE, DEFAULT_LIST_DELTA )
      
   Stg_ObjectList* Stg_ObjectList_New2( Index initialSize, Index delta );
   
   #define Stg_ObjectList_Init( self ) \
      Stg_ObjectList_Init2( self, DEFAULT_LIST_INITIAL_SIZE, DEFAULT_LIST_DELTA )
   
   void Stg_ObjectList_Init2( Stg_ObjectList* self, Index initialSize, Index delta );
   
   /** Creation implementation */
   
   #ifndef ZERO
   #define ZERO 0
   #endif

   #define STG_OBJECTLIST_DEFARGS \
      STG_CLASS_DEFARGS, \
      Stg_ObjectList_AppendFunction*                     _append, \
      Stg_ObjectList_PrependFunction*                   _prepend, \
      Stg_ObjectList_ReplaceAllFunction*             _replaceAll, \
      Stg_ObjectList_ReplaceFunction*                   _replace, \
      Stg_ObjectList_InsertBeforeFunction*         _insertBefore, \
      Stg_ObjectList_InsertAfterFunction*           _insertAfter, \
      Stg_ObjectList_RemoveFunction*                     _remove, \
      Stg_ObjectList_GetIndexFunction*                 _getIndex, \
      Stg_ObjectList_GetFunction*                           _get, \
      Stg_ObjectList_AllocMoreMemoryFunction*   _allocMoreMemory, \
      Stg_ObjectList_InsertAtIndexFunction*       _insertAtIndex, \
      Stg_ObjectList_RemoveByIndexFunction*       _removeByIndex, \
      Stg_ObjectList_DeleteAllObjectsFunction* _deleteAllObjects, \
      Index                                          initialSize, \
      Index                                                delta

   #define STG_OBJECTLIST_PASSARGS \
      STG_CLASS_PASSARGS, \
      _append, \
      _prepend, \
      _replaceAll, \
      _replace, \
      _insertBefore, \
      _insertAfter, \
      _remove, \
      _getIndex, \
      _get, \
      _allocMoreMemory, \
      _insertAtIndex, \
      _removeByIndex, \
      _deleteAllObjects, \
      initialSize, \
      delta            

   Stg_ObjectList* _Stg_ObjectList_New( STG_OBJECTLIST_DEFARGS );
   
   /** Initialisation implementation */
   void _Stg_ObjectList_Init( Stg_ObjectList* self, Index initialSize, Index delta );
   
   /** Stg_Class_Delete() implementation. Note that we assume ownership of all objects in this list! */
   void _Stg_ObjectList_Delete( void* namedObjectList );
   
   /** Stg_Class_Print() implementation */
   void _Stg_ObjectList_Print( void* objectList, struct Stream* stream );
   
   /** Copy implementation */
   void* _Stg_ObjectList_Copy( void* namedObjectList, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
   
   /* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   /** Append Object instance to list. Returns the index where the new object was inserted (the last element) */
   Index Stg_ObjectList_Append( void* objectList, void* objectPtr );
   
   /** Append Stg_Class instance to list. Returns the index where the new object was inserted (the last element) */
   Index Stg_ObjectList_ClassAppend( void* objectList, void* objectPtr, Name name );
   
   /** Append C pointer to list. Returns the index where the new object was inserted 
   (the last element). See notes in ObjectAdaptor.h about the responsibilities of the
   3 function pointers required - especially the DeletePointer function. */
   Index Stg_ObjectList_PointerAppend( 
      void*                                    objectList, 
      void*                                    objectPtr, 
      Name                                     name, 
      Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
      Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
      Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy );
   
   /** Append global C pointer to list. Returns the index where the new object was inserted (the last element) */
   Index Stg_ObjectList_GlobalPointerAppend( void* objectList, void* objectPtr, Name name );
   
   /** Prepend object to list */
   Index Stg_ObjectList_Prepend( void* objectList, void* objectPtr );
   
   /** Prepend class to list */
   Index Stg_ObjectList_ClassPrepend( void* objectList, void* objectPtr, Name name );
   
   /** Prepend pointer to list */
   Index Stg_ObjectList_PointerPrepend(
      void*                                    objectList, 
      void*                                    objectPtr, 
      Name                                     name, 
      Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
      Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
      Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy );
   
   /** Prepend global pointer to list */
   Index Stg_ObjectList_GlobalPointerPrepend( void* objectList, void* objectPtr, Name name );
   
   /** Replace whole list with a object */
   Index Stg_ObjectList_ReplaceAll( void* objectList, ReplacementOption option, void* objectPtr );
   
   /** Replace whole list with a class */
   Index Stg_ObjectList_ClassReplaceAll( void* objectList, ReplacementOption option, void* objectPtr, Name name );
   
   /** Replace whole list with a pointer */
   Index Stg_ObjectList_PointerReplaceAll( 
      void*                                    objectList, 
      ReplacementOption                        option, 
      void*                                    objectPtr, 
      Name                                     name, 
      Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
      Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
      Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy );
   
   /** Replace whole list with a global pointer */
   Index Stg_ObjectList_GlobalPointerReplaceAll( void* objectList, ReplacementOption option, void* objectPtr, Name name );
   
   /** Replace a specific entry with a object */
   Index Stg_ObjectList_Replace( 
      void*             objectList, 
      Name              toReplace,
      ReplacementOption option, 
      void*             objectPtr );
   
   /** Replace a specific entry with a class */
   Index Stg_ObjectList_ClassReplace( 
      void*             objectList, 
      Name              toReplace,
      ReplacementOption option, 
      void*             objectPtr, 
      Name              name );
   
   /** Replace a specific entry with a pointer*/
   Index Stg_ObjectList_PointerReplace( 
      void*                                    objectList, 
      Name                                     toReplace,
      ReplacementOption                        option, 
      void*                                    objectPtr, 
      Name                                     name,
      Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
      Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
      Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy );
   
   /** Replace a specific entry with a global pointer */
   Index Stg_ObjectList_GlobalPointerReplace( 
      void*             objectList, 
      Name              toReplace,
      ReplacementOption option, 
      void*             objectPtr, 
      Name              name );
   
   /** Insert object before a specific entry */
   Index Stg_ObjectList_InsertBefore( void* objectList, Name reference, void* objectPtr );
   
   /** Insert class before a specific entry */
   Index Stg_ObjectList_ClassInsertBefore( void* objectList, Name reference, void* objectPtr, Name name );
   
   /** Insert pointer before a specific entry */
   Index Stg_ObjectList_PointerInsertBefore( 
      void*                                    objectList, 
      Name                                     reference, 
      void*                                    objectPtr, 
      Name                                     name, 
      Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
      Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
      Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy );
   
   /** Insert global pointer before a specific entry */
   Index Stg_ObjectList_GlobalPointerInsertBefore( void* objectList, Name reference, void* objectPtr, Name name );
   
   /** Insert object at a specific Index */   
   void Stg_ObjectList_InsertAtIndex( void* objects, Index index, void* objectPtr );

   /** Insert object after a specific entry */ 
   Index Stg_ObjectList_InsertAfter( void* objectList, Name reference, void* objectPtr );
   
   /** Insert class after a specific entry */ 
   Index Stg_ObjectList_ClassInsertAfter( void* objectList, Name reference, void* objectPtr, Name name );
   
   /** Insert pointer after a specific entry */ 
   Index Stg_ObjectList_PointerInsertAfter( 
      void*                                    objectList, 
      Name                                     reference, 
      void*                                    objectPtr, 
      Name                                     name, 
      Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
      Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
      Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy );
   
   /** Insert global pointer after a specific entry */ 
   Index Stg_ObjectList_GlobalPointerInsertAfter( void* objectList, Name reference, void* objectPtr, Name name );
   
   /** Removes object from list */
   Index Stg_ObjectList_Remove( void* objectList, Name reference, ReplacementOption option );
   
   /** Find an object's index in the list, by name. Returns (unsigned)-1 if not found. */
   Index Stg_ObjectList_GetIndex( void* objectList, const Name toGet );
   
   /** Get an object's ptr from the list, by name. Returns NULL if not found. */
   Stg_Object* Stg_ObjectList_Get( void* objectList, const Name toGet );
   
   /** Deletes all the objects in the list. */
   void Stg_ObjectList_DeleteAllObjects( void* objectList );
   
   /** Prints the names of all the object in the list. */
   void Stg_ObjectList_PrintAllEntryNames( void* objectList, void* _stream );
   
   /** Calls Print on all the objects in the list. */
   void Stg_ObjectList_PrintAllObjects( void* objectList, void* _stream );
   
   /* Obtain the object at a given index in the list. */
   #define Stg_ObjectList_AtMacro( self, index ) \
      ( (self)->data[index] ) 
   Stg_Object* Stg_ObjectList_AtFunc( void* objectList, Index index );
   #ifdef MACRO_AS_FUNC
      /** Obtain the object at a given index in the list. */
      #define Stg_ObjectList_At Stg_ObjectList_AtFunc
   #else
      /** Obtain the object at a given index in the list. */
      #define Stg_ObjectList_At Stg_ObjectList_AtMacro
   #endif

   /* Caution: using this function implies you _know_ that the stored object at
    * given index is stored inside an ObjectAdaptor. If this isn't the case, you
    * should use Stg_ObjectList_At() instead. */
   #define Stg_ObjectList_ObjectAt( self, index ) \
      ( Stg_ObjectAdaptor_Object( (Stg_ObjectAdaptor*)( (self)->data[index] ) ) )
   
   /* Obtain the number of objects in the list. */
   #define Stg_ObjectList_CountMacro( self ) \
      ( (self)->count )
   Index Stg_ObjectList_CountFunc( void* objectList );
   #ifdef MACRO_AS_FUNC
      /** Obtain the number of objects in the list. */
      #define Stg_ObjectList_Count Stg_ObjectList_CountFunc
   #else
      /** Obtain the number of objects in the list. */
      #define Stg_ObjectList_Count Stg_ObjectList_CountMacro
   #endif
   
   /* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
   
   /* Default implementation to: Append to list. Returns the index where the new object was inserted (the last element) */
   Index _Stg_ObjectList_Append( void* objects, void* objectPtr );

   /* Default implementation to: Prepend to list */
   Index _Stg_ObjectList_Prepend( void* objects, void* objectPtr );

   /* Default implementation to: Replace whole list */
   Index _Stg_ObjectList_ReplaceAll( void* objects, ReplacementOption option, void* objectPtr );

   /* Default implementation to: Replace a specific entry */
   Index _Stg_ObjectList_Replace( void* objects, Name toReplace, ReplacementOption option, void* objectPtr );

   /* Default implementation to: Insert before a specific entry */
   Index _Stg_ObjectList_InsertBefore( void* objects, Name reference, void* objectPtr );

   /* Default implementation to: Insert after a specific entry */ 
   Index _Stg_ObjectList_InsertAfter( void* objects, Name reference, void* objectPtr );

   /* Default implementation to: Remove an entry by name */
   Index _Stg_ObjectList_Remove( void* namedStg_ObjectList, Name reference, ReplacementOption option );
   
   /* Default implementation to: Find an object's index in the list, by name. Returns (unsigned)-1 if not found. */
   Index _Stg_ObjectList_GetIndex( void* objects, Name toGet );

   /* Default implementation to: Get an object's ptr from the list, by name. Returns NULL if not found. */
   Stg_Object* _Stg_ObjectList_Get( void* objects, Name toGet );

   /* Default implementation to: Deletes all the objects in the list. */
   void _Stg_ObjectList_DeleteAllObjects( void* namedObjectList );

   /** Allocates more memory, when capacity reached */
   void _Stg_ObjectList_AllocMoreMemory( void* namedObjectList );
   
   /** Inserts a ptr at a given index */
   void _Stg_ObjectList_InsertAtIndex( void* objects, Index index, void* objectPtr );
   
   /** Removes a ptr at a given index */
   void _Stg_ObjectList_RemoveByIndex( void* objects, Index index, ReplacementOption option );

   /** Prints the first 'number' of names of objects in list which are most similar to 'name' */
   void Stg_ObjectList_PrintSimilar( void* objectList, Name name, void* _stream, unsigned int number );

#endif /* __StGermain_Base_Foundation_Stg_ObjectList_h__ */

