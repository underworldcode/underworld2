/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include "types.h"
#include "Memory.h"
#include "forwardDecl.h"

#include "Class.h"
#include "Object.h"
#include "ObjectAdaptor.h"
#include "ObjectList.h"
#include "CommonRoutines.h"
#include "shortcuts.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/** Textual name of this class */
const Type Stg_ObjectList_Type = "Stg_ObjectList";

Stg_ObjectList* Stg_ObjectList_New2( Index initialSize, Index delta ) {
   /* Variables set in this function */
   SizeT                                          _sizeOfSelf = sizeof(Stg_ObjectList);
   Type                                                  type = Stg_ObjectList_Type;
   Stg_Class_DeleteFunction*                          _delete = _Stg_ObjectList_Delete;
   Stg_Class_PrintFunction*                            _print = _Stg_ObjectList_Print;
   Stg_Class_CopyFunction*                              _copy = _Stg_ObjectList_Copy;
   Stg_ObjectList_AppendFunction*                     _append = _Stg_ObjectList_Append;
   Stg_ObjectList_PrependFunction*                   _prepend = _Stg_ObjectList_Prepend;
   Stg_ObjectList_ReplaceAllFunction*             _replaceAll = _Stg_ObjectList_ReplaceAll;
   Stg_ObjectList_ReplaceFunction*                   _replace = _Stg_ObjectList_Replace;
   Stg_ObjectList_InsertBeforeFunction*         _insertBefore = _Stg_ObjectList_InsertBefore;
   Stg_ObjectList_InsertAfterFunction*           _insertAfter = _Stg_ObjectList_InsertAfter;
   Stg_ObjectList_RemoveFunction*                     _remove = _Stg_ObjectList_Remove;
   Stg_ObjectList_GetIndexFunction*                 _getIndex = _Stg_ObjectList_GetIndex;
   Stg_ObjectList_GetFunction*                           _get = _Stg_ObjectList_Get;
   Stg_ObjectList_AllocMoreMemoryFunction*   _allocMoreMemory = _Stg_ObjectList_AllocMoreMemory;
   Stg_ObjectList_InsertAtIndexFunction*       _insertAtIndex = _Stg_ObjectList_InsertAtIndex;
   Stg_ObjectList_RemoveByIndexFunction*       _removeByIndex = _Stg_ObjectList_RemoveByIndex;
   Stg_ObjectList_DeleteAllObjectsFunction* _deleteAllObjects = _Stg_ObjectList_DeleteAllObjects;

   return _Stg_ObjectList_New( STG_OBJECTLIST_PASSARGS );
}
   
void Stg_ObjectList_Init2( Stg_ObjectList* self, Index initialSize, Index delta ) {
   /* General info */
   self->type = Stg_ObjectList_Type;
   self->_sizeOfSelf = sizeof(Stg_ObjectList);
   self->_deleteSelf = False;
   
   /* Virtual info */
   self->_delete = _Stg_ObjectList_Delete;
   self->_print = _Stg_ObjectList_Print;
   self->_copy = _Stg_ObjectList_Copy;
   self->_append = _Stg_ObjectList_Append;
   self->_prepend = _Stg_ObjectList_Prepend;
   self->_replaceAll = _Stg_ObjectList_ReplaceAll;
   self->_replace = _Stg_ObjectList_Replace; 
   self->_insertBefore = _Stg_ObjectList_InsertBefore;  
   self->_insertAfter = _Stg_ObjectList_InsertAfter;  
   self->_remove = _Stg_ObjectList_Remove;  
   self->_getIndex = _Stg_ObjectList_GetIndex;  
   self->_get = _Stg_ObjectList_Get;  
   self->_allocMoreMemory = _Stg_ObjectList_AllocMoreMemory;  
   self->_insertAtIndex = _Stg_ObjectList_InsertAtIndex;  
   self->_removeByIndex = _Stg_ObjectList_RemoveByIndex;  
   self->_deleteAllObjects = _Stg_ObjectList_DeleteAllObjects;
   _Stg_Class_Init( (Stg_Class*)self );
   _Stg_ObjectList_Init( (Stg_ObjectList*)self, initialSize, delta );

}
   
/* Creation implementation */
Stg_ObjectList* _Stg_ObjectList_New( STG_OBJECTLIST_DEFARGS ) {
   Stg_ObjectList* self;
   
   /* Allocate memory */
   assert( _sizeOfSelf >= sizeof(Stg_ObjectList) );
   self = (Stg_ObjectList*)_Stg_Class_New( STG_CLASS_PASSARGS );

   /* Virtual functions */
   self->_append = _append;
   self->_prepend = _prepend;
   self->_replaceAll = _replaceAll;
   self->_replace = _replace;
   self->_insertBefore = _insertBefore;
   self->_insertAfter = _insertAfter;
   self->_remove = _remove;
   self->_getIndex = _getIndex;  
   self->_get = _get;
   self->_allocMoreMemory = _allocMoreMemory;
   self->_insertAtIndex = _insertAtIndex;
   self->_removeByIndex = _removeByIndex;
   self->_deleteAllObjects = _deleteAllObjects;
   
   /* ObjectList info */
   _Stg_ObjectList_Init( self, initialSize, delta );
   
   return self;
}


/* Initialisation implementation */
void _Stg_ObjectList_Init( Stg_ObjectList* self, Index initialSize, Index delta ) {
   self->count = 0;
   self->_size = initialSize;
   self->_delta = delta;
   self->data = (Stg_ObjectPtr*)Memory_Alloc_Array( Stg_ObjectPtr, initialSize, "ObjectList->data" );
   self->_noJournalingInCopy = False;
}

/* Stg_Class_Delete implementation */
void _Stg_ObjectList_Delete( void* namedObjectList ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;

/*TODO - reference counter needs to be used to make sure
 we are not deleting objects that are being referenced to
 from other objects.*/
   
#if 0
   Stg_ObjectList_DeleteAllObjects( self );
#endif
   
   Memory_Free( self->data );

   /* Stg_Class_Delete parent class */
   _Stg_Class_Delete( self );

}
   
   
/* Print implementation */
void _Stg_ObjectList_Print( void* objectList, struct Stream* stream ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;

   /* General info */
   Journal_Printf( stream, "Stg_ObjectList (ptr):%p\n", (void*)self );
   Stream_Indent( stream );
   
   /* Print parent class */
   _Stg_Class_Print( self, stream );

   /* Virtual info */
   Journal_Printf( stream, "_append(func ptr): %p\n", (void*)self->_append );
   Journal_Printf( stream, "_prepend(func ptr): %p\n", (void*)self->_prepend );
   Journal_Printf( stream, "_replaceAll(func ptr): %p\n", (void*)self->_replaceAll );
   Journal_Printf( stream, "_replace(func ptr): %p\n", (void*)self->_replace );
   Journal_Printf( stream, "_insertBefore(func ptr): %p\n", (void*)self->_insertBefore );
   Journal_Printf( stream, "_insertAfter(func ptr): %p\n", (void*)self->_insertAfter );
   Journal_Printf( stream, "_remove(func ptr): %p\n", (void*)self->_remove );
   Journal_Printf( stream, "_getIndex(func ptr): %p\n", (void*)self->_getIndex );
   Journal_Printf( stream, "_get(func ptr): %p\n", (void*)self->_get );
   Journal_Printf( stream, "_allocMoreMemory(func ptr): %p\n", (void*)self->_allocMoreMemory );
   Journal_Printf( stream, "_insertAtIndex(func ptr): %p\n", (void*)self->_insertAtIndex );
   Journal_Printf( stream, "_removeByIndex(func ptr): %p\n", (void*)self->_removeByIndex );
   Journal_Printf( stream, "_deleteAllObjects(func ptr): %p\n", (void*)self->_deleteAllObjects );

   /* Stg_ObjectList */
   Journal_Printf( stream, "_size: %u\n", self->_size );
   Journal_Printf( stream, "_delta: %u\n", self->_delta );
   Journal_Printf( stream, "_noJournalingInCopy: %u\n", self->_noJournalingInCopy );
   Journal_Printf( stream, "count: %u\n", self->count );
   Journal_Printf( stream, "data[0-%d]:\n", self->count );
   Stg_ObjectList_PrintAllObjects( self, stream );
   
   Stream_UnIndent( stream );
}

void* _Stg_ObjectList_Copy( void* namedObjectList, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Stg_ObjectList* newObjectList;
   
   newObjectList = (Stg_ObjectList*)_Stg_Class_Copy( self, dest, deep, nameExt, ptrMap );
   
   newObjectList->_append = self->_append;
   newObjectList->_prepend = self->_prepend;
   newObjectList->_append = self->_append;
   newObjectList->_replaceAll = self->_replaceAll;
   newObjectList->_replace = self->_replace;
   newObjectList->_insertBefore = self->_insertBefore;
   newObjectList->_insertAfter = self->_insertAfter;
   newObjectList->_remove = self->_remove;
   newObjectList->_getIndex = self->_getIndex;
   newObjectList->_get = self->_get;
   newObjectList->_allocMoreMemory = self->_allocMoreMemory;
   newObjectList->_insertAtIndex = self->_insertAtIndex;
   newObjectList->_removeByIndex = self->_removeByIndex;
   newObjectList->_deleteAllObjects = self->_deleteAllObjects;
   newObjectList->count = self->count;
   newObjectList->_size = self->_size;
   newObjectList->_delta = self->_delta;
   newObjectList->_noJournalingInCopy = self->_noJournalingInCopy;
   
   /*
    * As this class is used by the Journal backend, making Journal calls when used in the Journal backend,
    * is problematic...  in this case work around it. It seems to only be an issue for copying. 
    */
   if( self->_noJournalingInCopy ) {
      assert( deep );
   }
   else {
      Journal_Firewall( deep, Journal_Register( Error_Type, Stg_ObjectList_Type ), "Shallow copy not yet implemented\n");
   }
   if( deep ) {
      unsigned obj_I;
      
      newObjectList->data = (Stg_ObjectPtr*)Memory_Alloc_Array( 
         Stg_ObjectPtr, 
         newObjectList->_size, 
         "ObjectList->data" );
      
      for( obj_I = 0; obj_I < newObjectList->count; obj_I++ ) {
         newObjectList->data[obj_I] = (Stg_ObjectPtr)Stg_Class_Copy( self->data[obj_I], NULL, deep, nameExt, ptrMap );
      }
   }
   
   return newObjectList;
}


/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

static void GlobalPrint( void* ptr, struct Stream* stream ) {
   Journal_Printf( stream, "(ptr): %p\n", ptr );
}
   
Index Stg_ObjectList_Append( void* objectList, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return self->_append( self, objectPtr );
}   

Index Stg_ObjectList_ClassAppend( void* objectList, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Append( self, Stg_ObjectAdaptor_NewOfClass( objectPtr, name, True, False ) );
}

Index Stg_ObjectList_PointerAppend( 
   void*                                    objectList, 
   void*                                    objectPtr, 
   Name                                     name, 
   Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
   Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
   Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Append( self,
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, True, False, ptrDelete, ptrPrint, ptrCopy ) );
}

Index Stg_ObjectList_GlobalPointerAppend( void* objectList, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Append( self, Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, False, True, 0, GlobalPrint, 0 ) );
}

Index Stg_ObjectList_Prepend( void* objectList, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return self->_prepend( self, objectPtr );
}

Index Stg_ObjectList_ClassPrepend( void* objectList, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Prepend( self, Stg_ObjectAdaptor_NewOfClass( objectPtr, name, True, False ) );
}

Index Stg_ObjectList_PointerPrepend( 
   void*                                    objectList, 
   void*                                    objectPtr, 
   Name                                     name, 
   Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
   Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
   Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Prepend( self,
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, True, False, ptrDelete, ptrPrint, ptrCopy ) );
}

Index Stg_ObjectList_GlobalPointerPrepend( void* objectList, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Prepend( self, Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, False, True, 0, GlobalPrint, 0 ) );
}

Index Stg_ObjectList_ReplaceAll( void* objectList, ReplacementOption option, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return self->_replaceAll( self, option, objectPtr );
}

Index Stg_ObjectList_ClassReplaceAll( void* objectList, ReplacementOption option, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_ReplaceAll( self, option, Stg_ObjectAdaptor_NewOfClass( objectPtr, name, True, False ) );
}

Index Stg_ObjectList_PointerReplaceAll( 
   void*                                    objectList, 
   ReplacementOption                        option, 
   void*                                    objectPtr, 
   Name                                     name, 
   Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
   Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
   Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_ReplaceAll( 
      self, 
      option, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, True, False, ptrDelete, ptrPrint, ptrCopy ) );
}

Index Stg_ObjectList_GlobalPointerReplaceAll( void* objectList, ReplacementOption option, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_ReplaceAll(
      self,
      option,
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, False, True, 0, GlobalPrint, 0 ) );
}

Index Stg_ObjectList_Replace( 
   void*             objectList, 
   Name              toReplace,
   ReplacementOption option, 
   void*             objectPtr )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return self->_replace( self, toReplace, option, objectPtr );
}

Index Stg_ObjectList_ClassReplace( 
   void*             objectList, 
   Name              toReplace,
   ReplacementOption option, 
   void*             objectPtr, 
   Name              name )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Replace( self, toReplace, option, Stg_ObjectAdaptor_NewOfClass( objectPtr, name, True, False ) );
}

Index Stg_ObjectList_PointerReplace( 
   void*                                    objectList, 
   Name                                     toReplace,
   ReplacementOption                        option, 
   void*                                    objectPtr, 
   Name                                     name,
   Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
   Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
   Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Replace( 
      self, 
      toReplace, 
      option, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, True, False, ptrDelete, ptrPrint, ptrCopy ) );
}

Index Stg_ObjectList_GlobalPointerReplace( 
   void*             objectList, 
   Name              toReplace,
   ReplacementOption option, 
   void*             objectPtr, 
   Name              name )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_Replace( 
      self, 
      toReplace, 
      option, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, False, True, 0, GlobalPrint, 0 ) );
}

Index Stg_ObjectList_InsertBefore( void* objectList, Name reference, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return self->_insertBefore( self, reference, objectPtr );   
}

Index Stg_ObjectList_ClassInsertBefore( void* objectList, Name reference, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_InsertBefore( self, reference, Stg_ObjectAdaptor_NewOfClass( objectPtr, name, True, False ) );
}

Index Stg_ObjectList_PointerInsertBefore( 
   void*                                    objectList, 
   Name                                     reference, 
   void*                                    objectPtr, 
   Name                                     name, 
   Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
   Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
   Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_InsertBefore( 
      self, 
      reference, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, True, False, ptrDelete, ptrPrint, ptrCopy ) );
}

Index Stg_ObjectList_GlobalPointerInsertBefore( void* objectList, Name reference, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_InsertBefore( 
      self, 
      reference, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, False, True, 0, GlobalPrint, 0 ) );
}


Index Stg_ObjectList_InsertAfter( void* objectList, Name reference, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return self->_insertAfter( self, reference, objectPtr );
}


void Stg_ObjectList_InsertAtIndex( void* objects, Index index, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objects;

   /* Runs error checking and then inserts */
   assert( objectPtr );
   assert( self->count <= self->_size );
   if ( self->count == self->_size ) {
      self->_allocMoreMemory( self );
   }

   self->_insertAtIndex( self, index, objectPtr );
}

Index Stg_ObjectList_ClassInsertAfter( void* objectList, Name reference, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_InsertAfter( self, reference, Stg_ObjectAdaptor_NewOfClass( objectPtr, name, True, False ) );
}

Index Stg_ObjectList_PointerInsertAfter( 
   void*                                    objectList, 
   Name                                     reference, 
   void*                                    objectPtr, 
   Name                                     name, 
   Stg_ObjectAdaptor_DeletePointerFunction* ptrDelete,
   Stg_ObjectAdaptor_PrintPointerFunction*  ptrPrint,
   Stg_ObjectAdaptor_CopyPointerFunction*   ptrCopy )
{
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_InsertAfter( 
      self, 
      reference, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, True, False, ptrDelete, ptrPrint, ptrCopy ) );
}

Index Stg_ObjectList_GlobalPointerInsertAfter( void* objectList, Name reference, void* objectPtr, Name name ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_InsertAfter( 
      self, 
      reference, 
      Stg_ObjectAdaptor_NewOfPointer( objectPtr, name, False, True, 0, GlobalPrint, 0 ) );
}

Index Stg_ObjectList_Remove( void* objectList, Name reference, ReplacementOption option ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;

   return self->_remove( self, reference, option );
}

Index Stg_ObjectList_GetIndex( void* objectList, const Name toGet ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;

   return self->_getIndex( self, toGet );
}


Stg_Object* Stg_ObjectList_Get( void* objectList, const Name objectName ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;

   return self->_get( self, objectName );
}


void Stg_ObjectList_DeleteAllObjects( void* objectList ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;

   self->_deleteAllObjects( self );
}

void Stg_ObjectList_PrintAllEntryNames( void* objectList, void* _stream ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   Stream*         stream  = (Stream*)_stream;
   Index           object_I;
   Index           count = self->count;

   Journal_Printf( stream, "{ " );

   for ( object_I = 0 ; object_I < count - 1 ; object_I++ ) {
      Journal_Printf( stream, "'%s' (%s), ", self->data[ object_I ]->name, self->data[ object_I ]->type );
   }
   Journal_Printf( stream, "'%s' (%s) }\n", self->data[ object_I ]->name, self->data[ object_I ]->type );
}

void Stg_ObjectList_PrintAllObjects( void* objectList, void* _stream ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   Stream*         stream = (Stream*)_stream;
   Index           objectIndex = 0;
   
   for ( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      Stg_Class_Print( self->data[ objectIndex ], stream );
   }
}

Stg_Object* Stg_ObjectList_AtFunc( void* objectList, Index index ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_AtMacro( self, index );
}

Index Stg_ObjectList_CountFunc( void* objectList ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   
   return Stg_ObjectList_CountMacro( self );
}

/* Private member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

Index _Stg_ObjectList_Append( void* namedObjectList, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;

   assert( objectPtr );
   assert( self->count <= self->_size );
   if ( self->count == self->_size ) {
      self->_allocMoreMemory( self );
   }
   
   self->data[self->count] = (Stg_Object*) objectPtr;
   
   return (self->count)++;
}

Index _Stg_ObjectList_Prepend( void* namedObjectList, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;

   assert( objectPtr );
   assert( self->count <= self->_size );
   if ( self->count == self->_size ) {
      self->_allocMoreMemory( self );
   }

   self->_insertAtIndex( self, 0, objectPtr );
   return 0;
}

Index _Stg_ObjectList_ReplaceAll( void* namedObjectList, ReplacementOption option, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;

   assert( objectPtr );
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if ( DELETE == option ) {
         Stg_Class_Delete( self->data[objectIndex] );
      }
      self->data[objectIndex] = 0;
   }
   self->data[0] = (Stg_Object*) objectPtr; 
   self->count = 1;
   return 0;
}

Index _Stg_ObjectList_Replace( void* namedObjectList, Name toReplace, ReplacementOption option, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;

   assert( objectPtr );
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if ( toReplace == self->data[objectIndex]->name ) {
         if ( DELETE == option ) {
            Stg_Class_Delete( self->data[objectIndex] );
         }
         self->data[objectIndex] = (Stg_Object*) objectPtr; 
         return objectIndex;
      }
   }
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if (0 == strcmp( toReplace, self->data[objectIndex]->name ) ) {
         if ( DELETE == option ) {
            Stg_Class_Delete( self->data[objectIndex] );
         }
         self->data[objectIndex] = (Stg_Object*) objectPtr; 
         return objectIndex;
      }
   }

   fprintf( stderr, "Error: %s(): specified object \"%s\" to replace not found.\n",
      __func__, toReplace ); 
   assert( 0 );
   return (Index) -1;
}

Index _Stg_ObjectList_InsertBefore( void* namedObjectList, Name reference, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;

   assert( objectPtr );
   assert( self->count <= self->_size );
   if ( self->count == self->_size ) {
      self->_allocMoreMemory( self );
   }

   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if ( reference == self->data[objectIndex]->name ) {
         self->_insertAtIndex( self, objectIndex, objectPtr );
         return objectIndex;
      }
   }
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if (0 == strcmp( reference, self->data[objectIndex]->name ) ) {
         self->_insertAtIndex( self, objectIndex, objectPtr );
         return objectIndex;
      }
   }

   fprintf( stderr, "Error: %s(): specified object \"%s\" to insert before not found.\n",
      __func__, reference ); 
   assert( 0 );
   return (Index) -1;
}

Index _Stg_ObjectList_InsertAfter( void* namedObjectList, Name reference, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;

   assert( objectPtr );
   assert( self->count <= self->_size );
   if ( self->count == self->_size ) {
      self->_allocMoreMemory( self );
   }

   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if ( reference == self->data[objectIndex]->name ) {
         self->_insertAtIndex( self, objectIndex+1, objectPtr );
         return objectIndex;
      }
   }
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if (0 == strcmp( reference, self->data[objectIndex]->name ) ) {
         self->_insertAtIndex( self, objectIndex+1, objectPtr );
         return objectIndex;
      }
   }

   fprintf( stderr, "Error: %s(): specified object \"%s\" to insert after not found.\n",
      __func__, reference ); 
   assert( 0 );
   return (Index) -1;
}

Index _Stg_ObjectList_Remove( void* namedObjectList, Name reference, ReplacementOption option ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;
   Stream*         errorStream = Journal_Register( Error_Type, self->type );

   objectIndex = self->_getIndex( self, reference );

   Journal_Firewall( objectIndex != (Index) -1, errorStream, 
         "Error: %s(): specified object \"%s\" to remove not found.\n", __func__, reference ); 
   
   self->_removeByIndex( self, objectIndex, option );

   return objectIndex;
}

Index _Stg_ObjectList_GetIndex( void* namedObjectList, const Name toGet ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;
   
   /* Find the object and return it */
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if ( toGet == self->data[objectIndex]->name ) return objectIndex;
   }
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if (0 == strcmp( toGet, self->data[objectIndex]->name ) ) return objectIndex;
   }
         
   return (unsigned int) -1;
}

Stg_Object* _Stg_ObjectList_Get( void* objectList, const Name toGet ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objectList;
   Index objectIndex;
                                                                                                                                    
   /* Find the object and return it */
   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) { 
      if ( toGet == self->data[objectIndex]->name ) { 
         if ( self->data[objectIndex]->type == Stg_ObjectAdaptor_Type ) { 
            return Stg_ObjectAdaptor_Object( (Stg_ObjectAdaptor*)self->data[objectIndex] ); 
         } 
         else { 
            return self->data[objectIndex]; 
         } 
      }
   }

   for( objectIndex = 0; objectIndex < self->count; objectIndex++ ) { 
      if (0 == strcmp( toGet, self->data[objectIndex]->name ) ) { 
         if ( self->data[objectIndex]->type == Stg_ObjectAdaptor_Type ) { 
            return Stg_ObjectAdaptor_Object( (Stg_ObjectAdaptor*)self->data[objectIndex] ); 
         } 
         else { 
            return self->data[objectIndex]; 
         } 
      }
   }
   return NULL;
}

void _Stg_ObjectList_DeleteAllObjects( void* namedObjectList ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;
   Index           objectIndex;
   
   for ( objectIndex = 0; objectIndex < self->count; objectIndex++ ) {
      if( self->data[objectIndex] ) {
         Stg_Class_Delete( self->data[objectIndex] );
         self->data[objectIndex] = 0;
      }
   }
   self->count = 0;
}

void _Stg_ObjectList_AllocMoreMemory( void* namedObjectList ) {
   Stg_ObjectList* self = (Stg_ObjectList*) namedObjectList;

   self->_size += self->_delta;
   self->data = Memory_Realloc_Array( self->data, Stg_ObjectPtr, self->_size );
   assert( self->data );
}

void _Stg_ObjectList_InsertAtIndex( void* objects, Index index, void* objectPtr ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objects;
   
   /* Move current content down one position, and add  */
   if (index != self->count) {
      memmove( &self->data[index+1], &self->data[index], sizeof(Stg_ObjectPtr) * (self->count - index) );
   }

   self->data[index] = (Stg_Object*) objectPtr;
   self->count++;
}

void _Stg_ObjectList_RemoveByIndex( void* objects, Index index, ReplacementOption option ) {
   Stg_ObjectList* self = (Stg_ObjectList*) objects;
   
   assert( index < self->count );

   if ( option == DELETE )
      Stg_Class_Delete( self->data[index] );
   
   /* Move current content up one position */
   if (index != self->count - 1) {
      memmove( &self->data[index], &self->data[index+1], sizeof(Stg_ObjectPtr) * (self->count - index - 1) );
   }

   self->count--;
}

typedef struct {
   Stg_Object* objectPtr;
   float       percentageSimilar;
} Stg_ObjectList_SimilarityObject;

int _Stg_ObjectList_SimilarityCompare( const void *ptr1, const void *ptr2 ) {
   Stg_ObjectList_SimilarityObject* similarityPtr1 = (Stg_ObjectList_SimilarityObject*) ptr1;
   Stg_ObjectList_SimilarityObject* similarityPtr2 = (Stg_ObjectList_SimilarityObject*) ptr2;

   if ( similarityPtr1->percentageSimilar > similarityPtr2->percentageSimilar )
      return -1;
   else 
      return 1;
}

void Stg_ObjectList_PrintSimilar( void* objectList, Name name, void* _stream, unsigned int number ) {
   Stg_ObjectList*                  self = (Stg_ObjectList*)objectList;
   Stream*                          stream  = (Stream*)_stream;
   Stg_ObjectList_SimilarityObject* similarityArray;
   float                            stringLength = (float) strlen( name );
   float                            objectStringLength;
   Index                            object_I;
   unsigned int                     substringLength;

   similarityArray = Memory_Alloc_Array( Stg_ObjectList_SimilarityObject, self->count, "similarityArray");

   for ( object_I = 0 ; object_I < self->count ; object_I++ ) {
      substringLength = Stg_LongestMatchingSubsequenceLength( self->data[object_I]->name, name, False );
      objectStringLength = (float) strlen( self->data[object_I]->name );

      similarityArray[ object_I ].objectPtr = self->data[object_I];
      similarityArray[ object_I ].percentageSimilar = 
         (float) substringLength * 100.0 / MAX( objectStringLength, stringLength );
   }

   qsort( similarityArray, (size_t)self->count, sizeof( Stg_ObjectList_SimilarityObject ), _Stg_ObjectList_SimilarityCompare );
   
   if ( number > self->count )
      number = self->count;
   for ( object_I = 0 ; object_I < number ; object_I++ ) {
      Journal_Printf(
         stream,
         "%s (%.2f%% similar)\n",
         similarityArray[ object_I ].objectPtr->name,
         similarityArray[ object_I ].percentageSimilar );
   }
   Memory_Free( similarityArray );
}
