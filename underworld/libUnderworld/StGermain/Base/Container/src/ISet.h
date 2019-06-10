/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_ISet_h__
#define __StGermain_Base_Container_ISet_h__

extern const double ISet_TableFactor;

extern const Type ISet_Type;
        
#define __ISet                                  \
   __Stg_Class                                  \
   int maxSize;                                 \
   int curSize;                                 \
   int tblSize;                                 \
   ISetItem* tbl;                               \
   Bool* used;

struct ISet { __ISet };

#ifndef ZERO
#define ZERO 0
#endif

#define ISET_DEFARGS                            \
   STG_CLASS_DEFARGS

#define ISET_PASSARGS                           \
   STG_CLASS_PASSARGS

ISet* ISet_New();

ISet* _ISet_New( ISET_DEFARGS );

void ISet_Init( ISet* self );

void _ISet_Init( void* self );

void ISet_Destruct( ISet* self );

void _ISet_Delete( void* self );

void ISet_Copy( void* self, const void* op );

void ISet_UseArray( void* _self, int size, const int* array );

void ISet_SetMaxSize( void* _self, int maxSize );

void ISet_Insert( void* _self, int key );

Bool ISet_TryInsert( void* _self, int key );

void ISet_Remove( void* _self, int key );

Bool ISet_TryRemove( void* _self, int key );

void ISet_Clear( void* _self );

void ISet_Union( void* _self, const void* _op );

void ISet_Isect( void* _self, const void* _op );

void ISet_Subtr( void* _self, const void* _op );

int ISet_GetMaxSize( const void* self );

int ISet_GetSize( const void* self );

void ISet_GetArray( const void* _self, int* keys );

Bool ISet_Has( const void* _self, int key );

int ISet_Hash( const void* self, int key );

void ISet_First( const void* _self, ISetIter* iter );

#endif /* __StGermain_Base_Container_ISet_h__ */
