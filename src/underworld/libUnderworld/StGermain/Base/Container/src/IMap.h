/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_IMap_h__
#define __StGermain_Base_Container_IMap_h__

extern const double IMap_TableFactor;

extern const Type IMap_Type;
        
#define __IMap                                  \
    __Stg_Class                                 \
    int maxSize;                                \
    int curSize;                                \
    int tblSize;                                \
    IMapItem* tbl;                              \
    Bool* used;

struct IMap { __IMap };

#ifndef ZERO
#define ZERO 0
#endif

#define IMAP_DEFARGS                            \
    STG_CLASS_DEFARGS

#define IMAP_PASSARGS                           \
    STG_CLASS_PASSARGS

IMap* IMap_New();

IMap* _IMap_New( IMAP_DEFARGS );

void IMap_Init( IMap* self );

void _IMap_Init( void* self );

void IMap_Destruct( IMap* self );

void _IMap_Delete( void* self );

void IMap_Copy( void* self, const void* op );

void IMap_SetMaxSize( void* _self, int maxSize );

void IMap_Insert( void* _self, int key, int val );

void IMap_SetValue( void* _self, int key, int val );

void IMap_Remove( void* _self, int key );

void IMap_Clear( void* _self );

int IMap_GetSize( const void* self );

int IMap_GetMaxSize( const void* self );

int IMap_Map( const void* _self, int key );

Bool IMap_TryMap( const void* _self, int key, int* val );

Bool IMap_Has( const void* _self, int key );

int IMap_Hash( const void* self, int key );

void IMap_First( const void* _self, IMapIter* iter );

#endif /* __StGermain_Base_Container_IMap_h__ */
