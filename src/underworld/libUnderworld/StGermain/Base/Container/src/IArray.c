/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <StGermain/Base/Foundation/src/Foundation.h>
#include "types.h"
#include "ISet.h"
#include "IMap.h"
#include "IArray.h"


const Type IArray_Type = "IArray";


int IArray_Cmp( const void* l, const void* r );
void IArray_ZeroAll( IArray* self );


IArray* IArray_New() {
    IArray* self;
    SizeT _sizeOfSelf = sizeof(IArray);
    Type type = IArray_Type;
    Stg_Class_DeleteFunction* _delete = _IArray_Delete;
    Stg_Class_PrintFunction* _print = NULL;
    Stg_Class_CopyFunction* _copy = NULL;

    self = _IArray_New( IARRAY_PASSARGS );
    return self;
}

IArray* _IArray_New( IARRAY_DEFARGS ) {
    IArray* self;

    self = (IArray*)_Stg_Class_New( STG_CLASS_PASSARGS );
    _IArray_Init( self );
    return self;
}

void IArray_Init( IArray* self ) {
    _Stg_Class_Init( (Stg_Class*)self );
    _IArray_Init( self );
}

void _IArray_Init( void* _self ) {
    IArray* self = (IArray*)_self;

    self->delta = 100;
    self->maxSize = 0;
    self->size = 0;
    self->ptr = NULL;
    self->own = True;
}

void IArray_Destruct( IArray* self ) {
    IArray_Clear( self );
}

void _IArray_Delete( void* self ) {
    IArray_Destruct( (IArray*)self );
    _Stg_Class_Delete( self );
}

void IArray_Copy( void* _self, const void* _op ) {
    IArray* self = (IArray*)_self;
    const IArray* op = (const IArray*)_op;

    assert( self );
    self->delta = op->delta;
    self->maxSize = op->maxSize;
    self->size = op->size;
    self->ptr = AllocArray( int, self->size );
    memcpy( self->ptr, op->ptr, sizeof(int) * self->size );
}

void IArray_SetDelta( void* _self, int delta ) {
    IArray* self = (IArray*)_self;

    assert( self );
    self->delta = delta;
}

void IArray_Resize( void* _self, int size ) {
    IArray* self = (IArray*)_self;

    assert( self && self->delta );
    if( !self->own )
        IArray_ZeroAll( self );
    self->maxSize = size / self->delta + ((size % self->delta) ? 1 : 0);
    self->maxSize *= self->delta;
    self->size = size;
    self->ptr = (int*)ReallocArray( self->ptr, int, self->maxSize );
}

void IArray_SoftResize( void* _self, int size ) {
    IArray* self = (IArray*)_self;

    assert( self && self->delta );
    if( !self->own || size > self->maxSize )
        IArray_Resize( self, size );
    else
        self->size = size;
}

void IArray_SetProxy( void* _self, int size, int* ptr ) {
    IArray* self = (IArray*)_self;

    if( self->own ) {
        if( self->ptr )
            free( self->ptr );
        self->own = False;
    }
    self->maxSize = size;
    self->size = size;
    self->ptr = ptr;
}

void IArray_Set( void* _self, int nItms, const int* itms ) {
    IArray* self = (IArray*)_self;

    assert( self && (!nItms || itms) );
    IArray_Resize( self, nItms );
    memcpy( self->ptr, itms, nItms * sizeof(int) );
}

void IArray_Add( void* _self, int nItms, const int* itms ) {
    IArray* self = (IArray*)_self;
    int oldSize;

    assert( self && (!nItms || itms) );
    oldSize = self->size;
    IArray_Resize( self, oldSize + nItms );
    memcpy( self->ptr + oldSize, itms, nItms * sizeof(int) );
}

void IArray_Remove( void* _self, int nItms, const int* locals, IMap* map ) {
    IArray* self = (IArray*)_self;
    ISet toRemObj, *toRem = &toRemObj;
    int* ord, pos;
    int i_i;

    assert( self && (!nItms || locals) && map );
    ISet_Init( toRem );
    ISet_UseArray( toRem, nItms, locals );
    ord = AllocArray( int, ISet_GetSize( toRem ) );
    memcpy( ord, locals, nItms * sizeof(int) );
    qsort( ord, nItms, sizeof(int), IArray_Cmp );
    IMap_Clear( map );
    IMap_SetMaxSize( map, nItms );
    for( i_i = 0, pos = self->size - 1; 
         i_i < nItms && pos > ord[i_i];
         i_i++, pos-- )
    {
        while( ISet_Has( toRem, pos ) && pos > ord[i_i] )
            pos--;
        if( pos <= ord[i_i] )
            break;
        self->ptr[ord[i_i]] = self->ptr[pos];
        IMap_Insert( map, pos, ord[i_i] );
    }
    if( IMap_GetSize( map ) < nItms )
        IMap_SetMaxSize( map, IMap_GetSize( map ) );
    ISet_Destruct( toRem );
    FreeArray( ord );

    IArray_Resize( self, self->size - nItms );
}

void IArray_Append( void* _self, int itm ) {
    IArray* self = (IArray*)_self;

    assert( self );
    if( self->size == self->maxSize )
        IArray_Resize( self, self->size + 1 );
    else
        self->size++;
    self->ptr[self->size - 1] = itm;
}

void IArray_Push( void* _self, int itm ) {
    IArray* self = (IArray*)_self;

    if( self->size == self->maxSize )
        IArray_Resize( self, self->size + 1 );
    else
        self->size++;
    self->ptr[self->size - 1] = itm;
}

void IArray_Clear( void* self ) {
    IArray_Resize( self, 0 );
}

int IArray_GetSize( const void* self ) {
    assert( self );
    return ((IArray*)self)->size;
}

int* IArray_GetPtr( const void* self ) {
    assert( self );
    return ((IArray*)self)->ptr;
}

void IArray_GetArray( const void* self, int* size, const int** ptr ) {
    assert( self );
    if( size )
        *size = ((IArray*)self)->size;
    *ptr = ((IArray*)self)->ptr;
}

int IArray_Cmp( const void* l, const void* r ) {
    assert( *(int*)l != *(int*)r );
    return (*(int*)l > *(int*)r) ? 1 : -1;
}

void IArray_ZeroAll( IArray* self ) {
    self->maxSize = 0;
    self->size = 0;
    self->ptr = NULL;
    self->own = True;
}


