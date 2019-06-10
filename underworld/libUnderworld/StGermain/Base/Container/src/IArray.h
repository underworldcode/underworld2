/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

 
#ifndef __StGermain_Base_Container_IArray_h__
#define __StGermain_Base_Container_IArray_h__

extern const Type IArray_Type;
        
#define __IArray                                \
    __Stg_Class                                 \
    int delta;                                  \
    int maxSize;                                \
    int size;                                   \
    int* ptr;                                   \
    Bool own;

struct IArray { __IArray };

#ifndef ZERO
#define ZERO 0
#endif

#define IARRAY_DEFARGS                          \
    STG_CLASS_DEFARGS

#define IARRAY_PASSARGS                         \
    STG_CLASS_PASSARGS

IArray* IArray_New();

IArray* _IArray_New( IARRAY_DEFARGS );

void IArray_Init( IArray* self );

void _IArray_Init( void* self );

void IArray_Destruct( IArray* self );

void _IArray_Delete( void* self );

void IArray_Copy( void* self, const void* op );

void IArray_SetDelta(void* self, int delta);

void IArray_Resize(void* self, int size);

void IArray_SoftResize(void* self, int size);

void IArray_SetProxy(void* self, int size, int* ptr);

void IArray_Set(void* self, int nItms, const int* itms);

void IArray_Add(void* self, int nItms, const int* itms);

void IArray_Remove(void* self, int nItms, const int* locals, IMap* map);

void IArray_Append(void* self, int itm);

void IArray_Push(void* self, int itm);

void IArray_Clear(void* self);

int IArray_GetSize(const void* self);

int* IArray_GetPtr(const void* self);

void IArray_GetArray(const void* self, int* size, const int** ptr);

#endif /* __StGermain_Base_Container_IArray_h__ */
