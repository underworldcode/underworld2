/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "pcu/pcu.h"

#include "types.h"
#include "forwardDecl.h"

#include "Memory.h"


#ifndef MAX
#define MAX( x, y ) ((x > y) ? x : y)
#endif

/** Setups the pointer locations in a 2D array. */
void Memory_SetupPointer_2DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength );

/** Setups the pointer locations in a 3D array. */
void Memory_SetupPointer_3DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength );

/** Moves data inside an enlarged 2D array to the appropriate position.
 * Note that it just moves the data of the array - in the case of a traditional
 * 2D array, will also need to separately update the pointer indices. */
void Memory_Relocate_2DArrayData(
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index newX, 
	Index newY );

/** Moves data inside an enlarged 3D array to the appropriate position.
 * Note that it just moves the data of the array - in the case of a traditional
 * 2D array, will also need to separately update the pointer indices. */
void Memory_Relocate_3DArrayData( 
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index oldZ,
	Index newX, 
	Index newY,
	Index newZ );

void* _Memory_Alloc_Func(
	SizeT size,
	Type type,
	const char* const name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	
	result = _Memory_InternalMalloc( size );
	
	return result;
}

void* _Memory_Alloc_Array_Func(
	SizeT itemSize,
	Index arrayLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	size = Memory_Length_1DArray( itemSize, arrayLength );
	result = _Memory_InternalMalloc( size );
	
	return result;
}

void* _Memory_Alloc_2DArray_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
		
	size = Memory_Length_2DArray( itemSize, xLength, yLength );
	result = _Memory_InternalMalloc( size );
	
	
	Memory_SetupPointer_2DArray( result, itemSize, xLength, yLength );
		
	return result;
}

void* _Memory_Alloc_3DArray_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
	
	size = Memory_Length_3DArray( itemSize, xLength, yLength, zLength );
	result = _Memory_InternalMalloc( size );
	
	Memory_SetupPointer_3DArray( result, itemSize, xLength, yLength, zLength );

	return result;

}
		
void* _Memory_Alloc_4DArray_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Index wLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer		result;
	Pointer*	array1;
	Pointer**	array2;
	Pointer***	array3;
	SizeT size;
	Index i, j, k;
	int ptrHeader1, ptrHeader2, ptrHeader3;
	int ySize, yzProduct, yzwProduct, zwProduct;
		
	size = Memory_Length_4DArray( itemSize, xLength, yLength, zLength, wLength );
	result = _Memory_InternalMalloc( size );
	
	ptrHeader1 = sizeof(Pointer) * xLength;
	ptrHeader2 = sizeof(Pointer) * xLength * yLength;
	ptrHeader3 = sizeof(Pointer) * xLength * yLength * zLength;
	
	ySize = sizeof(Pointer) * yLength;
	yzProduct = yLength * zLength;
	yzwProduct = yLength * zLength * wLength;
	zwProduct = zLength * wLength;
	
	array1 = (Pointer*) result;
	for ( i = 0; i < xLength; ++i )
	{
		array1[i] = (Pointer)( (ArithPointer)result + ptrHeader1 + ( i * ySize ) );
	}
	
	array2 = (Pointer**) result;
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLength; ++j )
		{
			array2[i][j] = (Pointer)( (ArithPointer)result
				+ ptrHeader1
				+ ptrHeader2
				+ ( ( (i * yzProduct) + (j * zLength) ) * sizeof(Pointer) ) );
		}
	}
	
	array3 = (Pointer***) result;
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLength; ++j )
		{
			for ( k = 0; k < zLength; ++k )
			{
				array3[i][j][k] = (Pointer)( (ArithPointer)result
					+ ptrHeader1
					+ ptrHeader2
					+ ptrHeader3
					+ ( ( (i * yzwProduct) + (j * zwProduct) + (k * wLength) ) * itemSize ) );
			}
		}
	}
		
	return result;
}


void* _Memory_Alloc_2DArrayAs1D_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
		
	size = Memory_Length_2DAs1D( itemSize, xLength, yLength );
	result = _Memory_InternalMalloc( size );
	
	return result;
}

void* _Memory_Alloc_3DArrayAs1D_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
		
	size = Memory_Length_3DAs1D( itemSize, xLength, yLength, zLength );
	result = _Memory_InternalMalloc( size );
		
	return result;
}

void* _Memory_Alloc_4DArrayAs1D_Func(
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength,
	Index wLength,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result;
	SizeT size;
		
	size = Memory_Length_4DAs1D( itemSize, xLength, yLength, zLength, wLength );
	result = _Memory_InternalMalloc( size );
	
	return result;
}

void* _Memory_Alloc_2DComplex_Func(
	SizeT itemSize,
	Index xLength,
	Index* yLengths,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer		result;
	Pointer*	array;
	SizeT size;
	Index i;
	
	size = Memory_Length_2DComplex( itemSize, xLength, yLengths );
	
	if ( 0 == size )
		return NULL;
	
	result = _Memory_InternalMalloc( size );
	
	
	array = (Pointer*) result;
	
	/* Start of data area for the first array */
	array[0] = (Pointer) ( (ArithPointer)array + sizeof(Pointer) * xLength);
	
	for ( i = 1; i < xLength; ++i )
	{
		/* The previous array plus the previous length offset */
		array[i] = (Pointer)( (ArithPointer)array[i - 1] + ( itemSize * yLengths[i - 1] ) );
	}
    
	return result;
}


Index** _Memory_Alloc_3DSetup_Func(
	Index xLength,
	Index* yLengths,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Index** result;
	
	result = (Index**) _Memory_Alloc_2DComplex_Func( sizeof(Index), xLength, yLengths,
		"Index", "MEMORY_SETUP", fileName, funcName, lineNumber );
	
	return result;
}


void* _Memory_Alloc_3DComplex_Func(
	SizeT itemSize,
	Index xLength,
	Index* yLengths,
	Index** zLengths,
	Type type,
	Name name,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer		result;
	Pointer*	array1;
	Pointer**	array2;
	SizeT size;
	Index i, j;
	ArithPointer startPos, diffSize;

	size = Memory_Length_3DComplex( itemSize, xLength, yLengths, zLengths );
	result = _Memory_InternalMalloc( size );
		
	array1 = (Pointer*) result;
	
	/* Start of data area for the first array */
	startPos = (ArithPointer)result;
	diffSize = (ArithPointer)( sizeof(Pointer) * xLength );
	for (i = 0; i < xLength; ++i)
	{
		array1[i] = (Pointer)( startPos + diffSize );
		
		/* The previous array plus the previous length offset */
		startPos = (ArithPointer)array1[i];
		diffSize = (ArithPointer)( sizeof(Pointer) * yLengths[i] );
	}
	
	array2 = (Pointer**)result;
	
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLengths[i]; ++j )
		{
			array2[i][j] = (Pointer)( startPos + diffSize );
			startPos = (ArithPointer)array2[i][j];
			diffSize = (ArithPointer)( zLengths[i][j] * itemSize );
		}
	}
	
	
	return result;
}


void* _Memory_Realloc_Func( 
	void* ptr, 
	SizeT newSize,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	Pointer result = NULL;

	result = _Memory_InternalRealloc( ptr, newSize );
		
	return result;
}


void* _Memory_Realloc_Array_Func( 
	void* ptr, 
	SizeT itemSize, 
	Index newLength,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	SizeT newSize;
	
	Pointer result = NULL;
		
	newSize = itemSize * newLength;
	result = _Memory_InternalRealloc( ptr, newSize );
	
	return result;
}


void* _Memory_Realloc_2DArray_Func(
	void* ptr,
	SizeT itemSize, 
	Index oldX,
	Index oldY,
	Index newX, 
	Index newY,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	SizeT newSize = 0;
	Pointer result = NULL;
	Pointer* array;
	
	if ( ptr != NULL ) {
		array = (Pointer*)ptr;
		oldX = ( (ArithPointer)array[0] - (ArithPointer)ptr ) / sizeof(Pointer);
		oldY = ( (ArithPointer)array[1] - (ArithPointer)array[0] ) / itemSize;
	}
	
	newSize = Memory_Length_2DArray( itemSize, newX, newY );
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL ) {
		Memory_Relocate_2DArrayData( (Pointer)( (ArithPointer)result + (newX * sizeof(Pointer)) ),
					 (Pointer)( (ArithPointer)result + (oldX * sizeof(Pointer)) ),
					 itemSize, oldX, oldY, newX, newY );
	}
	
	Memory_SetupPointer_2DArray( result, itemSize, newX, newY );
	return result;
}

void* _Memory_Realloc_3DArray_Func( 
	void* ptr, 
	SizeT itemSize, 
	Index oldX,
	Index oldY,
	Index oldZ,
	Index newX, 
	Index newY, 
	Index newZ,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	SizeT newSize = 0;
	Pointer result = NULL;
	Pointer* array;
	Pointer** array2;
	
	if ( ptr != NULL ) {
		array = (Pointer*)ptr;
		array2 = (Pointer**)ptr;
		
		oldX = ( (ArithPointer)array[0] - (ArithPointer)ptr ) / sizeof(Pointer);
		oldY = ( (ArithPointer)array[1] - (ArithPointer)array[0] ) / sizeof(Pointer);
		oldZ = ( (ArithPointer)array2[0][1] - (ArithPointer)array2[0][0] ) / itemSize;
	}
	
	newSize = Memory_Length_3DArray( itemSize, newX, newY, newZ );
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL ) {
		Memory_Relocate_3DArrayData( (Pointer)( (ArithPointer)result + ( (newX + (newX * newY)) * sizeof(Pointer) ) ),
					 (Pointer)( (ArithPointer)result + ( (oldX + (oldX * oldY)) * sizeof(Pointer) ) ),
					 itemSize, oldX, oldY, oldZ, newX, newY, newZ );
	}
	
	Memory_SetupPointer_3DArray( result, itemSize, newX, newY, newZ );
	
	return result;
}

void* _Memory_Realloc_2DArrayAs1D_Func( 
	void* ptr, 
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index newX, 
	Index newY,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	SizeT newSize;
	Pointer result = NULL;
	
	newSize = itemSize * newX * newY;
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL )
	{
		Memory_Relocate_2DArrayData( result, result, itemSize, oldX, oldY, newX, newY );
	}
	return result;
}

void* _Memory_Realloc_3DArrayAs1D_Func( 
	void* ptr, 
	SizeT itemSize,
	Index oldX, 
	Index oldY, 
	Index oldZ, 
	Index newX, 
	Index newY, 
	Index newZ,
	Type type,
	const char* fileName,
	const char* funcName,
	int lineNumber )
{
	SizeT newSize;
	Pointer result = NULL;
	
	newSize = itemSize * newX * newY * newZ;
	result = _Memory_InternalRealloc( ptr, newSize );
	
	if ( ptr != NULL ) {
		Memory_Relocate_3DArrayData( result, result, itemSize, oldX, oldY, oldZ, newX, newY, newZ );
	}
	
	return result;
}


/* This function should only be called on ptrs allocated by StGermain's Memory routines */
void _Memory_Free_Func( void* ptr ) {
	if( !ptr )
        return;
    _Memory_InternalFree( ptr );
}
	
SizeT Memory_Length_1DArray( SizeT itemSize, Index length )
{
	return itemSize * length;
}

SizeT Memory_Length_2DArray( SizeT itemSize, Index xLength, Index yLength )
{
	return ( sizeof(Pointer) * xLength ) + ( itemSize * xLength * yLength );
}

SizeT Memory_Length_3DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength )
{
	return ( sizeof(Pointer) * ( xLength + xLength * yLength ) ) + ( itemSize * xLength * yLength * zLength );
}


SizeT Memory_Length_4DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength )
{
	return ( sizeof(Pointer) * ( xLength + xLength * yLength + xLength * yLength * zLength ) )
		 + ( itemSize * xLength * yLength * zLength * wLength );
}

SizeT Memory_Length_2DComplex( SizeT itemSize, Index xLength, Index* yLengths )
{
	Index i;
	SizeT result = 0;
	for ( i = 0; i < xLength; ++i )
	{
		result += yLengths[i];
	}
	result *= itemSize;
	result += sizeof(Pointer) * xLength;
	
	return result;
	
}

SizeT Memory_Length_3DComplex( SizeT itemSize, Index xLength, Index* yLengths, Index** zLengths )
{
	Index i, j;
	SizeT result = 0;
	SizeT ptrSize = xLength;
	
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLengths[i]; ++j )
		{
			result += zLengths[i][j];
		}
		ptrSize += yLengths[i];
	}
	result *= itemSize;
	result += sizeof(Pointer) * ptrSize; 
	
	return result;
}

SizeT Memory_Length_2DAs1D( SizeT itemSize, Index xLength, Index yLength )
{
	return itemSize * xLength * yLength;
}

SizeT Memory_Length_3DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength )
{
	return itemSize * xLength * yLength * zLength;
}

SizeT Memory_Length_4DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength )
{
	return itemSize * xLength * yLength * zLength * wLength;
}


void Memory_SetupPointer_2DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength )
{
	Pointer* array;
	int ptrHeaderSize;
	int ySize;
	Index i;
	
	ptrHeaderSize = sizeof(Pointer) * xLength;
	ySize = itemSize * yLength;
	
	array = (Pointer*) ptr;
	for (i = 0; i < xLength; ++i )
	{
		array[i] = (Pointer)( (ArithPointer)ptr + ptrHeaderSize + ( i * ySize ) ) ;
	}

}

void Memory_SetupPointer_3DArray(
	void* ptr,
	SizeT itemSize,
	Index xLength,
	Index yLength,
	Index zLength )
{
	Pointer*	array1;
	Pointer**	array2;
	Index i, j;
	int ptrHeader1, ptrHeader2;
	int ySize, yzProduct;
	
	ptrHeader1 = sizeof(Pointer) * xLength;
	ptrHeader2 = sizeof(Pointer) * xLength * yLength;
	
	ySize = sizeof(Pointer) * yLength;
	yzProduct = yLength * zLength;
	
	array1 = (Pointer*) ptr;
	for ( i = 0; i < xLength; ++i )
	{
		array1[i] = (Pointer)( (ArithPointer)ptr + ptrHeader1 + ( i * ySize ) );
	}
	
	array2 = (Pointer**) ptr;
	for ( i = 0; i < xLength; ++i )
	{
		for ( j = 0; j < yLength; ++j )
		{
			array2[i][j] = (Pointer)( (ArithPointer)ptr
				+ ptrHeader1
				+ ptrHeader2
				+ ( ( (i * yzProduct) + (j * zLength) ) * itemSize ) );
		}
	}
}


void Memory_Relocate_2DArrayData(
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index newX, 
	Index newY )
{
	int i, j;
	
	if ( ( oldX > newX ) || ( oldY > newY ) )
	{
		/* Cannot handle these combinations yet. */
		assert( 0 );
		return;
	}
	
	/* copy in reverse order to avoid overwritting data */
	/* Note: using memmove because this function is often called to relocate data within an existing
	 * array that's been resized - thus data may be overlapping */
	for ( i = oldX - 1; i >= 0; --i ) {
		for ( j = oldY - 1; j >= 0; --j ) {
			memmove( (Pointer)((ArithPointer)destPtr + ( ((i * newY) + j) * itemSize ) ),
				(Pointer)((ArithPointer)srcPtr + (  ((i * oldY) + j) * itemSize ) ),
				itemSize );
		}
	}
}

void Memory_Relocate_3DArrayData( 
	void* destPtr, 
	void* srcPtr,
	SizeT itemSize, 
	Index oldX, 
	Index oldY, 
	Index oldZ,
	Index newX, 
	Index newY,
	Index newZ )
{
	int i, j, k;
	
	if ( ( oldX > newX ) || ( oldY > newY ) || ( oldZ > newZ ) )
	{
		/* Cannot handle these combinations yet. */
		return;
	}
	
	/* copy in reverse order to avoid overwritting data */
	/* Note: using memmove because this function is often called to relocate data within an existing
	 * array that's been resized - thus data may be overlapping */
	for ( i = oldX - 1; i >= 0; --i ) {
		for ( j = oldY - 1; j >= 0; --j ) {
			for ( k = oldZ - 1; k >= 0; --k ) {
				memmove( (Pointer)((ArithPointer)destPtr + ( ((i * newY * newZ) + (j * newZ) + k) * itemSize ) ),
					(Pointer)((ArithPointer)srcPtr  + ( ((i * oldY * oldZ) + (j * oldZ) + k) * itemSize ) ),
					itemSize );
			}
		}
	}
}

void* _Memory_InternalMalloc( SizeT size ) {
	void* result;

	if( !size )
	  return NULL;

	result = malloc( size );

	return result;

}

void* _Memory_InternalRealloc( void* ptr, SizeT size ) {
	void* result;

	result =  realloc( ptr, size );

	return result;
}

void _Memory_InternalFree( void* ptr ) {
	if( !ptr ) return;
	free( ptr );
}

