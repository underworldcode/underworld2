/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
**	A memory management module that allows analysis of memory usage to a fidelity chosen by the developer,
**	much like the Journal allows for streaming.
**
** <b>Assumptions</b>
**	The memory module is an itegral part of the Foundation module and will only begin operation after Foundation_Init() is
**	called. Likewise, the memory module will stop operation after Foundation_Finalise() is called.
**
**	Using Memory module functions outside while it is not in operation or disabled will most likely result in a segmentation
**	fault.
**
** <b>Comments</b>
**	Currently, the memory module is NOT thread safe. Future revisions of the module may possibly include this 
**	functionality.
**
** <b>Description</b>
**
**	The memory module is designed to extend traditional C malloc() and free() functions to include statistical recording 
**	and array allocation facilities. It contains a number of different functions which are used to replace malloc() and 
**	free(). Typically these functions accept additional parameters regarding statistical information.
**
**	Allocations are organised into <b>types</b> - a textual value, indicating the type of the data object being 
**	allocated. Each type, can be further sub-grouped into <b>names</b>, which is a textual tag to indicate  
**	particular instances or member variables. These are added as part of the allocation routine.
**
**	It also includes information such as the file, function and line number the allocation was made from, to allow easy
**	debugging.
**
**	The memory module provides facilities to perform general types of allocations - objects and arrays from 1D to 4D.
**	For performance reasons, a set of allocation functions (2D to 4D) can be allocated as a 1D array, and used via Access
**	macros. To conserve space, Complex arrays (2D to 3D) can also be created. Complex arrays basically allow the second
**	(and third) dimensions to have varying lengths. For example, a 2D complex array may have 3 rows, where the length of each
**	row is 4, 2 and 3 respectively.
**
**~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

#ifndef __StGermain_Base_Foundation_Memory_h__
#define __StGermain_Base_Foundation_Memory_h__

	/* See implementations for _Memory_InternalMalloc and _Memory_InternalFree at the bottom of Memory.c */
	void* _Memory_InternalMalloc( SizeT size );

	/* Replacement for realloc() */
	void* _Memory_InternalRealloc( void* ptr, SizeT size );

	/* Replacement for free() */
	void _Memory_InternalFree( void* ptr );


    /** Allocates a single instance of a given primitive or class. */
    #define Memory_Alloc( type, name ) \
        (type*) _Memory_Alloc_Func( sizeof(type), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a single instance of a given primitive or class, without assigning a name to the allocation. */
	#define Memory_Alloc_Unnamed( type ) \
		Memory_Alloc( type, Name_Invalid )


    /** Allocates a given number of bytes, associating the allocation with a textual type and name. */
    #define Memory_Alloc_Bytes( size, typeName, name ) \
        _Memory_Alloc_Func( (size), (typeName), (name), __FILE__, __func__, __LINE__ )	

	/** Allocates a given number of bytes, associating the allocation with a textual type only. */
	#define Memory_Alloc_Bytes_Unnamed( size, typeName ) \
		Memory_Alloc_Bytes( (size), (typeName), Name_Invalid )


    /** Allocates a 1D array of a primitive or class. */
    #define Memory_Alloc_Array( type, arrayLength, name ) \
        (type*) _Memory_Alloc_Array_Func( sizeof(type), (arrayLength), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 1D array of a primitive or class without a name. */
	#define Memory_Alloc_Array_Unnamed( type, arrayLength ) \
		Memory_Alloc_Array( type, (arrayLength), Name_Invalid )

    /** Allocates a 1D array of items each of itemSize number of bytes. Allocation is given a textual type and name. */
    #define Memory_Alloc_Array_Bytes( itemSize, arrayLength, typeName, name ) \
        _Memory_Alloc_Array_Func( (itemSize), (arrayLength), (typeName), (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 1D array of items each of itemSize number of bytes. Allocation is given a textual type only. */
	#define Memory_Alloc_Array_Bytes_Unnamed( itemSize, arrayLength, typeName ) \
		Memory_Alloc_Array_Bytes( (itemSize), (arrayLength), (typeName), Name_Invalid )

	/** Allocates a 2D array of any primative or class. */
	#define Memory_Alloc_2DArray( type, xLength, yLength, name ) \
		(type**) _Memory_Alloc_2DArray_Func( sizeof(type), (xLength), (yLength), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 2D array of any primative or class without a name. */
	#define Memory_Alloc_2DArray_Unnamed( type, xLength, yLength ) \
		Memory_Alloc_2DArray( type, (xLength), (yLength), Name_Invalid )
		

	/** Allocates a 3D array of a primative or class. */
	#define Memory_Alloc_3DArray( type, xLength, yLength, zLength, name ) \
		(type***) _Memory_Alloc_3DArray_Func \
			( sizeof(type), (xLength), (yLength), (zLength), #type, (name), __FILE__, __func__, __LINE__ )
	
	/** Allocates a 3D array of a primative or class without a name. */
	#define Memory_Alloc_3DArray_Unnamed( type, xLength, yLength, zLength ) \
		Memory_Alloc_3DArray( type, (xLength), (yLength), (zLength), Name_Invalid )
		

	/** Allocates a 4D array of a primative or class. */
	#define Memory_Alloc_4DArray( type, xLength, yLength, zLength, wLength, name ) \
		(type****) _Memory_Alloc_4DArray_Func \
			( sizeof(type), (xLength), (yLength), (zLength), (wLength), #type, (name), __FILE__, __func__, __LINE__ )
	
	/** Allocates a 4D array of a primative or class without a name. */
	#define Memory_Alloc_4DArray_Unnamed( type, xLength, yLength, zLength, wLength ) \
		Memory_Alloc_4DArray( type, (xLength), (yLength), (zLength), (wLength), Name_Invalid )
	

    /** Allocates a 2D array from a 1D array. Use in conjunction with the Memory_Access2D marco. */
    #define Memory_Alloc_2DArrayAs1D( type, xLength, yLength, name ) \
        (type*) _Memory_Alloc_2DArrayAs1D_Func \
            ( sizeof(type), (xLength), (yLength), #type, (name), __FILE__, __func__, __LINE__ );

	/** Allocates a 2D array from a 1D array without a name. Use in conjunction with the Memory_Access2D marco. */
	#define Memory_Alloc_2DArrayAs1D_Unnamed( type, xLength, yLength ) \
		Memory_Alloc_2DArrayAs1D( type, (xLength), (yLength), Name_Invalid );


    /** Allocates a 3D array from a 1D array. Use in conjunction with the Memory_Access3D marco. */
    #define Memory_Alloc_3DArrayAs1D( type, xLength, yLength, zLength, name ) \
        (type*) _Memory_Alloc_3DArrayAs1D_Func \
            ( sizeof(type), (xLength), (yLength), (zLength), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 3D array from a 1D array without a name. Use in conjunction with the Memory_Access3D marco. */
	#define Memory_Alloc_3DArrayAs1D_Unnamed( type, xLength, yLength, zLength ) \
		Memory_Alloc_3DArrayAs1D( type, (xLength), (yLength), (zLength), Name_Invalid );

	
    /** Allocates a 4D array from a 1D array. Use in conjunction with the Memory_Access4D marco. */
    #define Memory_Alloc_4DArrayAs1D( type, xLength, yLength, zLength, wLength, name ) \
        (type*) _Memory_Alloc_4DArrayAs1D_Func \
            ( sizeof(type), (xLength), (yLength), (zLength), (wLength), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 4D array from a 1D array without a name. Use in conjunction with the Memory_Access4D marco. */
	#define Memory_Alloc_4DArrayAs1D_Unnamed( type, xLength, yLength, zLength, wLength ) \
		Memory_Alloc_4DArrayAs1D( type, (xLength), (yLength), (zLength), (wLength), Name_Invalid );


	/** Allocates a 2D complex array. Each array in the 2nd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	#define Memory_Alloc_2DComplex( type, xLength, yLengths, name ) \
		(type**) _Memory_Alloc_2DComplex_Func( sizeof(type), (xLength), (yLengths), #type, (name), __FILE__, __func__, __LINE__ )
	
	/** Allocates a 2D complex array without a name. Each array in the 2nd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	#define Memory_Alloc_2DComplex_Unnamed( type, xLength, yLengths ) \
		Memory_Alloc_2DComplex( type, (xLength), (yLengths), Name_Invalid )


	/** Allocates a 2D complex array used to represent 3rd dimension lengths in a 3D complex array.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	#define Memory_Alloc_3DSetup( xLength, yLengths ) \
		_Memory_Alloc_3DSetup_Func( (xLength), (yLengths), __FILE__, __func__, __LINE__ )
	
	
	/** Allocates a 3D complex array. Each array in the 2nd and 3rd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 ** @param zLengths A matrix of lengths for the 3rd dimension. Allocated by Memory_Alloc_3DSetup.
	 **/
	#define Memory_Alloc_3DComplex( type, xLength, yLengths, zLengths, name ) \
		(type***) _Memory_Alloc_3DComplex_Func \
			( sizeof(type), (xLength), (yLengths), (zLengths), #type, (name), __FILE__, __func__, __LINE__ )

	/** Allocates a 3D complex array without a name. Each array in the 2nd and 3rd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 ** @param zLengths A matrix of lengths for the 3rd dimension. Allocated by Memory_Alloc_3DSetup.
	 **/
	#define Memory_Alloc_3DComplex_Unnamed( type, xLength, yLengths, zLengths ) \
		Memory_Alloc_3DComplex( type, (xLength), (yLengths), (zLengths), Name_Invalid )


    /** Resizes the bytes of a single object. */
    #define Memory_Realloc( ptr, newSize ) \
        _Memory_Realloc_Func( (ptr), (newSize), Type_Invalid, __FILE__, __func__, __LINE__ )

    /** Resizes a 1D array. */
    #define Memory_Realloc_Array( ptr, type, newLength ) \
        (type*) _Memory_Realloc_Array_Func( (ptr), sizeof(type), (newLength), #type, __FILE__, __func__, __LINE__ )

    /** Resizes a 1D array of items, where each item is of itemSize bytes. */
    #define Memory_Realloc_Array_Bytes( ptr, itemSize, newLength ) \
        _Memory_Realloc_Array_Func( (ptr), (itemSize), (newLength), Type_Invalid, __FILE__, __func__, __LINE__ )

	/** Resizes a 2D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Assumes the original dimension of X is greater than 1.
	 **/
	#define Memory_Realloc_2DArray( ptr, type, newX, newY ) \
		(type**) _Memory_Realloc_2DArray_Func \
			( (ptr), sizeof(type), 0, 0, (newX), (newY), #type, __FILE__, __func__, __LINE__ )

	/** Resizes a 3D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Assumes the original dimension of X and Y is greater than 1.
	 **/
	#define Memory_Realloc_3DArray( ptr, type, newX, newY, newZ ) \
		(type***) _Memory_Realloc_3DArray_Func \
			( (ptr), sizeof(type), 0, 0, 0, (newX), (newY), (newZ), #type, __FILE__, __func__, __LINE__ )

	/** Resizes a 2D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Safe to used for when X is equal to 1.
	 **/
	#define Memory_Realloc_2DArraySafe( ptr, type, oldX, oldY, newX, newY ) \
		(type**) _Memory_Realloc_2DArray_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (newX), (newY), #type, __FILE__, __func__, __LINE__ )

	/** Resizes a 3D array.
	 ** The integrity of the data is only maintained if enlarging the size of the array.
	 ** Safe to used for when X and/or Y is equal to 1.
	 **/
	#define Memory_Realloc_3DArraySafe( ptr, type, oldX, oldY, oldZ, newX, newY, newZ ) \
		(type***) _Memory_Realloc_3DArray_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (oldZ), (newX), (newY), (newZ), #type, __FILE__, __func__, __LINE__ )
	



	/** Resizes a 2D array in 1D form. */
	#define Memory_Realloc_2DArrayAs1D( ptr, type, oldX, oldY, newX, newY ) \
		(type*) _Memory_Realloc_2DArrayAs1D_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (newX), (newY), #type, __FILE__, __func__, __LINE__ )
		
	/** Resizes a 3D array in 1D form. */
	#define Memory_Realloc_3DArrayAs1D( ptr, type, oldX, oldY, oldZ, newX, newY, newZ ) \
		(type*) _Memory_Realloc_3DArrayAs1D_Func \
			( (ptr), sizeof(type), (oldX), (oldY), (oldZ), (newX), (newY), (newZ), #type, __FILE__, __func__, __LINE__ )

		
    /** Deallocates the memory of any allocation. */
    #define Memory_Free( ptr ) \
        _Memory_Free_Func( (ptr) )

	
	/** Displays a summary of the given type and name. */
	#define Memory_Print_Type_Name( type, name ) \
		Memory_Print_Type_Name_Func( #type, (name) );
		
	/** Displays a summary of the given type. */
	#define Memory_Print_Type( type ) \
		Memory_Print_Type_Func( #type );


	/** Retrives a value from a 2D array allocated as a 1D array. Use in conjunction with Memory_Alloc_2DArrayAs1D. */
	#define Memory_Access2D( array2D, x, y, yLength ) \
		array2D[ ( (x) * (yLength) ) + (y) ]

	/** Retrives a value from a 3D array allocated as a 1D array. Use in conjunction with Memory_Alloc_3DArrayAs1D. */
	#define Memory_Access3D( array3D, x, y, z, yLength, zLength ) \
		array3D[ ( (x) * (yLength) * (zLength) ) + ( (y) * (zLength) ) + (z) ]

	/** Retrives a value from a 4D array allocated as a 1D array. Use in conjunction with Memory_Alloc_4DArrayAs1D. */
	#define Memory_Access4D( array4D, x, y, z, w, yLength, zLength, wLength ) \
		array4D[ ( (x) * (yLength) * (zLength) * (wLength) ) + ( (y) * (zLength) * (wLength) ) + ( (z) * (wLength) ) + (w) ]


	/** Returns the number of bytes used in a 1D array. */
	SizeT Memory_Length_1DArray( SizeT itemSize, Index length );

	/** Returns the number of bytes used in a 2D array. */
	SizeT Memory_Length_2DArray( SizeT itemSize, Index xLength, Index yLength );
	
	/** Returns the number of bytes used in a 3D array. */
	SizeT Memory_Length_3DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength );
	
	/** Returns the number of bytes used in a 4D array. */
	SizeT Memory_Length_4DArray( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength );

	/** Returns the number of bytes used in a 2D array allocated as 1D. */
	SizeT Memory_Length_2DAs1D( SizeT itemSize, Index xLength, Index yLength );
	
	/** Returns the number of bytes used in a 3D array allocated as 1D. */
	SizeT Memory_Length_3DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength );
	
	/** Returns the number of bytes used in a 4D array allocated as 1D. */
	SizeT Memory_Length_4DAs1D( SizeT itemSize, Index xLength, Index yLength, Index zLength, Index wLength );

	/** Returns the number of bytes used in a 2D complex array. */
	SizeT Memory_Length_2DComplex( SizeT itemSize, Index xLength, Index* yLengths );
	
	/** Returns the number of bytes used in a 3D complex array. */
	SizeT Memory_Length_3DComplex( SizeT itemSize, Index xLength, Index* yLengths, Index** zLengths );

	/** \internal Allocates a single object. */
	void* _Memory_Alloc_Func(
		SizeT size,
		Type type,
		const char* const name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 1D array. */
	void* _Memory_Alloc_Array_Func(
		SizeT itemSize,
		Index arrayLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );
	
	/** \internal Allocates a 2D array. */
	void* _Memory_Alloc_2DArray_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );
	
	/** \internal Allocates a 3D Array. */
	void* _Memory_Alloc_3DArray_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Index zLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );
	
	/** \internal Allocates a 4D Array. */
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
		int lineNumber );

	/** \internal Allocates a 2D array from a 1D array. Use in conjunction with the Memory_Access2D marco. */
	void* _Memory_Alloc_2DArrayAs1D_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 3D array from a 1D array. Use in conjunction with the Memory_Access3D marco. */
	void* _Memory_Alloc_3DArrayAs1D_Func(
		SizeT itemSize,
		Index xLength,
		Index yLength,
		Index zLength,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 4D array from a 1D array. Use in conjunction with the Memory_Access4D marco. */
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
		int lineNumber );

	/** \internal Allocates a 2D complex array. Each array in the 2nd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	void* _Memory_Alloc_2DComplex_Func(
		SizeT itemSize,
		Index xLength,
		Index* yLengths,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 2D complex array used to represent 3rd dimension lengths in a 3D complex array.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 **/
	Index** _Memory_Alloc_3DSetup_Func(
		Index xLength,
		Index* yLengths,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Allocates a 3D complex array. Each array in the 2nd and 3rd dimension may have varying lengths.
	 **
	 ** @param yLengths A set of lengths for the 2nd dimension.
	 ** @param zLengths A matrix of lengths for the 3rd dimension. Allocated by Memory_Alloc_3DSetup.
	 **/
	void* _Memory_Alloc_3DComplex_Func(
		SizeT itemSize,
		Index xLength,
		Index* yLengths,
		Index** zLengths,
		Type type,
		Name name,
		const char* fileName,
		const char* funcName,
		int lineNumber );


	/** \internal Resizes the bytes of a single object. */
	void* _Memory_Realloc_Func(
		void* ptr,
		SizeT newSize,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 1D array. */
	void* _Memory_Realloc_Array_Func( 
		void* ptr, 
		SizeT itemSize, 
		Index newLength,
		Type type,
		const char* fileName,
		const char* funcName,
		int lineNumber );

	/** \internal Resizes a 2D array. */
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
		int lineNumber );

	/** \internal Resizes a 3D array. */
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
		int lineNumber );

	/** \internal Resizes a 2D array in 1D form. */
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
		int lineNumber );

	/** \internal Resizes a 3D array in 1D form. */
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
		int lineNumber );


	/** \internal Frees any allocation. */
	void _Memory_Free_Func( void* ptr );	

	/*
	 * Macro equivalents
	 *
	 * The following private functions are macro equivalents of some of the Memory module functions.
	 * Thesy are implemented to ensure that when the Memory module is disabled, minimal performance impact
	 * is ensured.
	 *
	 * Functions such as the normal 2D to 4D array and as well as the 2D and 3D complex allocations do not
	 * have macro equivalents due to the difficulty of creation. These operations only have a function implementation.
	 *
	 */

	#define _Memory_Alloc_Macro( size ) \
		_Memory_InternalMalloc( (size) )

	#define _Memory_Alloc_Array_Macro( itemSize, arrayLength ) \
		_Memory_InternalMalloc( (itemSize) * (arrayLength) )
	
	#define _Memory_Alloc_2DArrayAs1D_Macro( itemSize, xLength, yLength ) \
		_Memory_InternalMalloc( (itemSize) * (xLength) * (yLength) )
	
	#define _Memory_Alloc_3DArrayAs1D_Macro( itemSize, xLength, yLength, zLength ) \
		_Memory_InternalMalloc( (itemSize) * (xLength) * (yLength) * (zLength) )

	#define _Memory_Alloc_4DArrayAs1D_Macro( itemSize, xLength, yLength, zLength, wLength ) \
		_Memory_InternalMalloc( (itemSize) * (xLength) * (yLength) * (zLength) * (wLength) )
	
	#define _Memory_Realloc_Macro( ptr, newSize ) \
		_Memory_InternalRealloc( (ptr), (newSize) )

	#define _Memory_Realloc_Array_Macro( ptr, itemSize, newLength ) \
		_Memory_InternalRealloc( (ptr), (itemSize) * (newLength) )

	#define _Memory_Free_Macro( ptr ) \
		_Memory_InternalFree( (ptr) )


/*
 * Shortcuts.
 */

#define MemAlloc			Memory_Alloc
#define MemRealloc( a, b, c )		Memory_Realloc( a, b )
#define MemArray			Memory_Alloc_Array
#define MemRearray( a, b, c, d )	Memory_Realloc_Array( a, b, c )
#define MemArray2D			Memory_Alloc_2DArray
#define MemArray3D			Memory_Alloc_3DArray
#define MemFree				Memory_Free

#endif /* __StGermain_Base_Foundation_Memory_h__ */
