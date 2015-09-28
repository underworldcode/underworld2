/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Foundation_shortcuts_h__
#define __StGermain_Base_Foundation_shortcuts_h__

/* If the system does not have these macros, define it here */
#ifndef MAX
#define MAX( a, b ) ( ( (a) > (b) ) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN( a, b ) ( ( (a) < (b) ) ? (a) : (b) )
#endif


#ifndef NDEBUG
#define VirtualCall( self, method, ... )						\
	(assert( self ), assert( (self)->method ), (self)->method( __VA_ARGS__ ))
#else
#define VirtualCall( self, method, ... )	\
	(self)->method( __VA_ARGS__ )
#endif


#define AllocArray( type, size )					\
	((size) ? Memory_Alloc_Array_Unnamed( type, size ) : NULL)

#define AllocNamedArray( type, size, name )				\
	((size) ? Memory_Alloc_Array( type, size, name ) : NULL)

#define AllocArray2D( type, size0, size1 )						\
	((size0 && size1) ? Memory_Alloc_2DArray_Unnamed( type, size0, size1 ) : NULL)

#define AllocNamedArray2D( type, size0, size1, name )					\
	((size0 && size1) ? Memory_Alloc_2DArray( type, size0, size1, name ) : NULL)

#define AllocComplex2D( type, base, sizes )						\
	((base && sizes) ? Memory_Alloc_2DComplex_Unnamed( type, base, sizes ) : NULL)

#define AllocNamedComplex2D( type, base, sizes, name )					\
	((base && sizes) ? Memory_Alloc_2DComplex( type, base, sizes, name ) : NULL)

#define ReallocArray( ptr, type, size )					\
	((ptr) ? ((size) ? Memory_Realloc_Array( ptr, type, size ) : 	\
		  (Memory_Free( ptr ), NULL)) : 			\
	 (size) ? Memory_Alloc_Array_Unnamed( type, size ) : NULL)

#define ReallocNamedArray( ptr, type, size, name )			\
	((ptr) ? ((size) ? Memory_Realloc_Array( ptr, type, size ) : 	\
		  (Memory_Free( ptr ), NULL)) : 			\
	 ((size) ? Memory_Alloc_Array( type, size, name ) : NULL))

#define ReallocArray2D( ptr, type, size0, size1 )						\
	((ptr) ? ((size0 && size1) ? Memory_Realloc_2DArray( ptr, type, size0, size1 ) :	\
		  (Memory_Free( ptr ), NULL)) : 						\
	 (size0 && size1) ? Memory_Alloc_2DArray_Unnamed( type, size0, size1 ) : NULL)


#define FreeArray( ptr )			\
	if( ptr )				\
		Memory_Free( ptr )
	
	
#define FreeArray2D( nBlocks, ptr )					\
	do {								\
		if( ptr ) {						\
			unsigned	block_i;			\
			for( block_i = 0; block_i < nBlocks; block_i++ ) { \
				if( (ptr)[block_i] ) {			\
					Memory_Free( (ptr)[block_i] );	\
				}					\
			}						\
			Memory_Free( ptr );				\
		}							\
	} while( 0 )
	
	
#define KillArray( ptr )			\
	do {					\
		if( ptr ) {			\
			Memory_Free( ptr );	\
			(ptr) = NULL;		\
		}				\
	} while( 0 )
	
	
#define KillArray2D( nBlocks, ptr )					\
	do {								\
		if( ptr ) {						\
			unsigned	block_i;			\
			for( block_i = 0; block_i < nBlocks; block_i++ ) { \
				if( (ptr)[block_i] ) {			\
					Memory_Free( (ptr)[block_i] );	\
				}					\
			}						\
			Memory_Free( ptr );				\
			(ptr) = NULL;					\
		}							\
	} while( 0 )

#define DeleteArray( ptr, size )					\
	do {								\
		if( ptr ) {						\
			unsigned	__class_i;			\
			for( __class_i = 0; __class_i < size; __class_i++ ) \
				if( ptr[__class_i] ) Stg_Delete_Class( ptr[__class_i] ); \
			ptr = NULL;					\
		}							\
	} while( 0 )

#define DeleteOwnArray( ptr, own, size )				\
	do {								\
		if( ptr ) {						\
			unsigned	__class_i;			\
			for( __class_i = 0; __class_i < size; __class_i++ ) \
				if( ptr[__class_i] && own[__class_i] )	\
					Stg_Delete_Class( ptr[__class_i] ); \
			Memory_Free( ptr );				\
			Memory_Free( own );				\
			ptr = NULL;					\
			own = NULL;					\
		}							\
	} while( 0 )

#define FreeObject( obj ) \
	if( obj ) (Stg_Class_Delete( obj ), (obj) = NULL)

#define KillObject( obj ) \
	if( obj ) (Stg_Class_Delete( obj ), (obj) = NULL)


#ifndef NDEBUG
#define insist( stmnt, expr ) assert( (stmnt) expr )
#else
#define insist( stmnt, expr ) stmnt
#endif
	
#endif /* __StGermain_Base_Foundation_shortcuts_h__ */
