/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_StgVariable_h__
#define __StGermain_Base_Context_StgVariable_h__

#include <Python.h>

	/** Textual name for Variable class. */
	extern const Type StgVariable_Type;

	typedef enum {
		StgVariable_DataType_Char,
		StgVariable_DataType_Short,
		StgVariable_DataType_Int,
		StgVariable_DataType_Long,
		StgVariable_DataType_Float,
		StgVariable_DataType_Double,
		StgVariable_DataType_Pointer,
		StgVariable_DataType_Size /* Marker for derivatives to enum */
	} StgVariable_DataType;

	typedef Index (StgVariable_ArraySizeFunc) ( void* self );

	/* See Variable */

	#define __StgVariable \
		/* General info */ \
		__Stg_Component \
		\
		AbstractContext*			context; \
		/* Virtual info */ \
		\
		/* Variable info */ \
		Index							offsetCount; /**< Number of fields in this variable. */ \
		SizeT*						offsets; /**< The offset in bytes these fields are in the struct.*/ \
		StgVariable_DataType*		dataTypes; /**< A list of original data types. */ \
		Index*						dataTypeCounts; /**< A list of the number of data. */ \
		SizeT*						structSizePtr; /**< A pointer to the size of the structure. */ \
		void**						arrayPtrPtr; /**< A pointer to a pointer to the 1D array data. */ \
		Index*						arraySizePtr; /**< A ptr to the size/count of the 1D array data. Note that if this is NULL, the arraySizeFunc will be used */ \
		StgVariable_ArraySizeFunc*	arraySizeFunc; /**< A func ptr to the size/count of the 1D array data. */ \
		\
		SizeT*						dataSizes; /**< The size in bytes of each field in this variable. */ \
		SizeT							structSize; /**< The size of the structure. */ \
		void*							arrayPtr; /**< A pointer to the 1D array of structures. */ \
		Index							arraySize; /**< The size/count of the 1D array of structures. */ \
		Index							subVariablesCount; /**< The number of subvariables. Necessary since determined by whether user passes in names or not at init time. */ \
		StgVariable**					components; /**< For each component of this variable that we made a variable for, the pointer to the variable. */ \
		Bool							allocateSelf; \
		Variable_Register*		vr; \
		\
		StgVariable*					parent;

	struct _StgVariable { __StgVariable };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructors
	*/

	/** Creates a new Variable. A Variable holds the run-time information of a complex data type created by the programmer.
	  * Essentially it associates a textual name to a variable in the program that the user can use to access or modify.
	  *
	  * For example, if the program has an array of doubles that stores temperature, a Variable can be created with the name
	  * "temperature", which will know how to access any item of this array. This name can then be added to a list of variables
	  * to dump, where the code doing so has at compile-time has no idea about the existance of this variable.
	  * 
	  * This constructor is the most complex, allowing the captured data to be a composition of items in a struct, of which
	  * itself may be in an array. Each item can be named, causing the construction of a new variable for each, but only if the
	  * register is made available and the name is not 0. If the register is made available, then this variable is also added
	  * to the register. In the case where there is only one item, but it is a vector, the name list must reperesent the name
	  * each each vector item (as opposed to items of the complex type). 
	  * If structSizePtr is 0 and dataCount is 1, it will attempt to work out the size from the dataType
	  * (otherwise it will assert). The casting allows the Variable to be seen in a different type to its actual representation
	  * (e.g. as an arry of floats instead of doubles). If dataCastTypes == 0, then no casting is done for any component. The
	  * size of the struct, the size of the array and the pointer to the array are provided as pointers to this information, of
	  * which will get resolved/dereferenced at the build phase. */
	StgVariable* StgVariable_New( 
		Name								name,
		AbstractContext*				context,
		Index								dataCount,
		SizeT*							dataOffsets,
		StgVariable_DataType*			dataTypes,
		Index*							dataTypeCounts,
		Name*								dataNames,
		SizeT*							structSizePtr,
		Index*							arraySizePtr,
		StgVariable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr );
	
	StgVariable* _StgVariable_DefaultNew( Name name );
	
	/** Creates a new Variable. A Variable holds the run-time information of a complex data type created by the programmer.
	  * Essentially it associates a textual name to a variable in the program that the user can use to access or modify.
	  *
	  * See StgVariable_New for more info.
	  * 
	  * This constructor is a shortcut to create a Variable of a scalar in an array. The stride/struct size is the size
	  * of the dataType. There is no casting. */
	StgVariable* StgVariable_NewScalar( 
		Name								name,
		AbstractContext*				context,
		StgVariable_DataType				dataType,
		Index*							arraySizePtr,
		StgVariable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr );
	
	/** Creates a new Variable. A Variable holds the run-time information of a complex data type created by the programmer.
	  * Essentially it associates a textual name to a variable in the program that the user can use to access or modify.
	  *
	  * See StgVariable_New for more info.
	  * 
	  * This constructor is a shortcut to create a Variable of a vector in an array. The names of the vector components are
	  * optional and are specified via the variable arguement list at the end. A 0 value signifies no name for the associated
	  * vector component. The stride/struct size is the size of the dataType. There is no casting.*/
	StgVariable* StgVariable_NewVector( 
		Name								name,
		AbstractContext*				context,
		StgVariable_DataType				dataType,
		Index								dataTypeCount,
		Index*							arraySizePtr,
		StgVariable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr,
		... 						/* vector component names */ );

	StgVariable* StgVariable_NewVector2( 
		Name								name,
		AbstractContext*				context,
		StgVariable_DataType				dataType,
		Index								dataTypeCount,
		Index*							arraySizePtr,
		StgVariable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr,
		char**							dataNames );
	
	/** Constructor interface. */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define VARIABLE_DEFARGS \
		STG_COMPONENT_DEFARGS, \
			Index                         dataCount, \
			SizeT*                      dataOffsets, \
			StgVariable_DataType*            dataTypes, \
			Index*                   dataTypeCounts, \
			Name*                         dataNames, \
			SizeT*                    structSizePtr, \
			Index*                     arraySizePtr, \
			StgVariable_ArraySizeFunc*   arraySizeFunc, \
			void**                      arrayPtrPtr, \
			Variable_Register*                   vr

	#define VARIABLE_PASSARGS \
		STG_COMPONENT_PASSARGS, \
			dataCount, \
			dataOffsets, \
			dataTypes, \
			dataTypeCounts, \
			dataNames, \
			structSizePtr, \
			arraySizePtr, \
			arraySizeFunc, \
			arrayPtrPtr, \
			vr            

	StgVariable* _StgVariable_New(  VARIABLE_DEFARGS  );
	
	/** Init interface. */
	void _StgVariable_Init(
		StgVariable*					self, 
		AbstractContext*			context,
		Index							dataCount,
		SizeT*						dataOffsets,
		StgVariable_DataType*		dataTypes,
		Index*						dataTypeCounts,
		Name*							dataNames,
		SizeT*						structSizePtr,
		Index*						arraySizePtr,
		StgVariable_ArraySizeFunc*	arraySizeFunc,
		void**						arrayPtrPtr,
		Bool							allocateSelf,
		Variable_Register*		vr );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete Variable implementation */
	void _StgVariable_Delete( void* variable );
	
	/** Print Variable implementation */
	void _StgVariable_Print( void* variable, Stream* stream );
	
	/** Copy implementation */
	#define StgVariable_Copy( self ) \
		(StgVariable*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define StgVariable_DeepCopy( self ) \
		(StgVariable*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _StgVariable_Copy( void* variable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Build implementation */
	void _StgVariable_Build( void* variable, void* data );
	
	/** Initialisation implementation */
	void _StgVariable_Initialise( void* variable, void* data );
	
	/** Execution implementation */
	void _StgVariable_Execute( void* variable, void* data );
	
	void _StgVariable_AssignFromXML( void* variable, Stg_ComponentFactory* cf, void* data );
	
	void _StgVariable_Destroy( void* variable, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions: Fundamental address calculators
	*/
	
	/** Implementation of "get" the structure in an array, that the requested data member is in. Private (Do not directly
	 *  use!)
	 *  NB:- the array_I needs to be cast to an ArithPointer below - see __StgVariable_GetPtr() for reason. */
	#define __StgVariable_GetStructPtr( self, array_I ) \
		( ( (self)->offsetCount < 2 ) ? \
			( (void*)( (ArithPointer)(self)->arrayPtr + (ArithPointer)(array_I) * (ArithPointer)( (self)->structSize) + (ArithPointer)( (self)->offsets[0] ) ))  : \
			( (void*)((ArithPointer)(self)->arrayPtr + (ArithPointer)(array_I) * (ArithPointer)((self)->structSize)) ) )
	#define _StgVariable_GetStructPtr __StgVariable_GetStructPtr

	
/* BUG!:	( (void*)((ArithPointer)_StgVariable_GetStructPtr( (self), (array_I) ) + (self)->offsets[component_I]) ) */

	/** Implementation of "get" the requested data member in a structure in an array. Private (Do not directly use!)
		NB - we cast the array_I to an ArithPointer since its usually passed in as an Index (unsigned int), which on
		some architectures such as the SGI Altix is of a different size to the ArithPointer (unsigned long) */
	#define __StgVariable_GetPtr( self, array_I, component_I, vector_I ) \
		( (void*)((ArithPointer)(self)->arrayPtr + (ArithPointer)(array_I) * (self)->structSize + (self)->offsets[component_I]) )
	#define _StgVariable_GetPtr __StgVariable_GetPtr

	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "char" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "char" inbuilt type, return a pointer to that char (type casted to char*). Private
	 * (Do not directly use!). */
	#define _StgVariable_GetPtrChar( self, array_I ) \
		( (char*)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrChar _StgVariable_GetPtrChar

	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtChar( self, array_I, vector_I ) \
		( StgVariable_GetPtrChar( (self), (array_I) )[(vector_I)] )
	#define StgVariable_GetValueAtChar _StgVariable_GetValueAtChar

	/** Assuming this is a Variable of a vector "char" inbuilt type, return a pointer to that char.  */
	#define StgVariable_GetPtrAtChar( self, array_I, vector_I ) \
		( &_StgVariable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a short */
	#define StgVariable_GetValueAtCharAsShort( self, array_I, vector_I ) \
		( (short)StgVariable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as an int */
	#define StgVariable_GetValueAtCharAsInt( self, array_I, vector_I ) \
		( (int)StgVariable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a float */
	#define StgVariable_GetValueAtCharAsFloat( self, array_I, vector_I ) \
		( (float)StgVariable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a double */
	#define StgVariable_GetValueAtCharAsDouble( self, array_I, vector_I ) \
		( (double)StgVariable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a pointer (void*) */
	#define StgVariable_GetValueAtCharAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)StgVariable_GetValueAtChar( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, set that char to a new value (internally type casted to char) */
	#define StgVariable_SetValueAtChar( self, array_I, vector_I, value ) \
		( *StgVariable_GetPtrAtChar( (self), (array_I), (vector_I) ) = (char)(value) )
	
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char */
	#define StgVariable_GetValueChar( self, array_I ) \
		( StgVariable_GetValueAtChar( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a short */
	#define StgVariable_GetValueCharAsShort( self, array_I ) \
		( (short)StgVariable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as an int */
	#define StgVariable_GetValueCharAsInt( self, array_I ) \
		( (int)StgVariable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a float */
	#define StgVariable_GetValueCharAsFloat( self, array_I ) \
		( (float)StgVariable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a double */
	#define StgVariable_GetValueCharAsDouble( self, array_I ) \
		( (double)StgVariable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a pointer (void*) */
	#define StgVariable_GetValueCharAsPointer( self, array_I ) \
		( (void*)( (int)StgVariable_GetValueChar( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, set that char to a new value (internally type casted to
	 *  char) */
	#define StgVariable_SetValueChar( self, array_I, value ) \
		( *StgVariable_GetPtrChar( (self), (array_I) ) = (char)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "short" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "short" inbuilt type, return a pointer to that short (type casted to short*). Private
	 * (Do not directly use!). */
	#define _StgVariable_GetPtrShort( self, array_I ) \
		( (short*)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrShort _StgVariable_GetPtrShort

	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtShort( self, array_I, vector_I ) \
		( StgVariable_GetPtrShort( (self), (array_I) )[(vector_I)] )
	#define StgVariable_GetValueAtShort _StgVariable_GetValueAtShort

	/** Assuming this is a Variable of a vector "short" inbuilt type, return a pointer to that short.  */
	#define StgVariable_GetPtrAtShort( self, array_I, vector_I ) \
		( &_StgVariable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a char */
	#define StgVariable_GetValueAtShortAsChar( self, array_I, vector_I ) \
		( (char)StgVariable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as an int */
	#define StgVariable_GetValueAtShortAsInt( self, array_I, vector_I ) \
		( (int)StgVariable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a float */
	#define StgVariable_GetValueAtShortAsFloat( self, array_I, vector_I ) \
		( (float)StgVariable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a double */
	#define StgVariable_GetValueAtShortAsDouble( self, array_I, vector_I ) \
		( (double)StgVariable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a pointer (void*) */
	#define StgVariable_GetValueAtShortAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)StgVariable_GetValueAtShort( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, set that short to a new value (internally type casted to
	 *  short)*/
	#define StgVariable_SetValueAtShort( self, array_I, vector_I, value ) \
		( *StgVariable_GetPtrAtShort( self, array_I, vector_I ) = (short)(value) )
	
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short */
	#define StgVariable_GetValueShort( self, array_I ) \
		( StgVariable_GetValueAtShort( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a char */
	#define StgVariable_GetValueShortAsChar( self, array_I ) \
		( (char)StgVariable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as an int */
	#define StgVariable_GetValueShortAsInt( self, array_I ) \
		( (int)StgVariable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a float */
	#define StgVariable_GetValueShortAsFloat( self, array_I ) \
		( (float)StgVariable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a double */
	#define StgVariable_GetValueShortAsDouble( self, array_I ) \
		( (double)StgVariable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a pointer (void*) */
	#define StgVariable_GetValueShortAsPointer( self, array_I ) \
		( (void*)( (int)StgVariable_GetValueShort( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, set that short to a new value (internally type casted to
	 *  short)*/
	#define StgVariable_SetValueShort( self, array_I, value ) \
		( *StgVariable_GetPtrShort( self, array_I ) = (short)(value) )
	

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "int" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "int" inbuilt type, return a pointer to that int (type casted to int*). Private
	 * (Do not directly use!). */
	#define _StgVariable_GetPtrInt( self, array_I ) \
		( (int*)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrInt _StgVariable_GetPtrInt

	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtInt( self, array_I, vector_I ) \
		( StgVariable_GetPtrInt( (self), (array_I) )[(vector_I)] )
	#define StgVariable_GetValueAtInt _StgVariable_GetValueAtInt

	/** Assuming this is a Variable of a vector "int" inbuilt type, return a pointer to that int.  */
	#define StgVariable_GetPtrAtInt( self, array_I, vector_I ) \
		( &_StgVariable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a char */
	#define StgVariable_GetValueAtIntAsChar( self, array_I, vector_I ) \
		( (char)StgVariable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a short */
	#define StgVariable_GetValueAtIntAsShort( self, array_I, vector_I ) \
		( (short)StgVariable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a float */
	#define StgVariable_GetValueAtIntAsFloat( self, array_I, vector_I ) \
		( (float)StgVariable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a double */
	#define StgVariable_GetValueAtIntAsDouble( self, array_I, vector_I ) \
		( (double)StgVariable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a pointer (void*) */
	#define StgVariable_GetValueAtIntAsPointer( self, array_I, vector_I ) \
		( (void*)StgVariable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, set that int to a new value (internally type casted to int) */
	#define StgVariable_SetValueAtInt( self, array_I, vector_I, value ) \
		( *StgVariable_GetPtrAtInt( (self), (array_I), (vector_I) ) = (int)(value) )
	
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int */
	#define StgVariable_GetValueInt( self, array_I ) \
		( StgVariable_GetValueAtInt( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a char */
	#define StgVariable_GetValueIntAsChar( self, array_I ) \
		( (char)StgVariable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a short */
	#define StgVariable_GetValueIntAsShort( self, array_I ) \
		( (short)StgVariable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a float */
	#define StgVariable_GetValueIntAsFloat( self, array_I ) \
		( (float)StgVariable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a double */
	#define StgVariable_GetValueIntAsDouble( self, array_I ) \
		( (double)StgVariable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a pointer (void*) */
	#define StgVariable_GetValueIntAsPointer( self, array_I ) \
		( (void*)StgVariable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, set that int to a new value (internally type casted to int) */
	#define StgVariable_SetValueInt( self, array_I, value ) \
		( *StgVariable_GetPtrInt( (self), (array_I) ) = (int)(value) )
	


	#define _StgVariable_GetPtrLong( self, array_I ) \
		( (long*)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrLong _StgVariable_GetPtrLong


	/** Assuming this is a Variable of a vector "long" inbuilt type, return that long. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtLong( self, array_I, vector_I ) \
		( StgVariable_GetPtrLong( (self), (array_I) )[(vector_I)] )
	#define StgVariable_GetValueAtLong _StgVariable_GetValueAtLong

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "float" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "float" inbuilt type, return a pointer to that float (type casted to float*). Private
	 * (Do not directly use!). */
	#define _StgVariable_GetPtrFloat( self, array_I ) \
		( (float*)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrFloat _StgVariable_GetPtrFloat

	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtFloat( self, array_I, vector_I ) \
		( StgVariable_GetPtrFloat( (self), (array_I) )[(vector_I)] )
	#define StgVariable_GetValueAtFloat _StgVariable_GetValueAtFloat

	/** Assuming this is a Variable of a vector "float" inbuilt type, return a pointer to that float.  */
	#define StgVariable_GetPtrAtFloat( self, array_I, vector_I ) \
		( &_StgVariable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a char */
	#define StgVariable_GetValueAtFloatAsChar( self, array_I, vector_I ) \
		( (char)StgVariable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a short */
	#define StgVariable_GetValueAtFloatAsShort( self, array_I, vector_I ) \
		( (short)StgVariable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as an int */
	#define StgVariable_GetValueAtFloatAsInt( self, array_I, vector_I ) \
		( (int)StgVariable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a double */
	#define StgVariable_GetValueAtFloatAsDouble( self, array_I, vector_I ) \
		( (double)StgVariable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a pointer (void*) */
	#define StgVariable_GetValueAtFloatAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)StgVariable_GetValueAtFloat( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, set that float to a new value (internally type casted to
	 *  float) */
	#define StgVariable_SetValueAtFloat( self, array_I, vector_I, value ) \
		( *StgVariable_GetPtrAtFloat( (self), (array_I), (vector_I) ) = (float)(value) )
	
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float */
	#define StgVariable_GetValueFloat( self, array_I ) \
		( StgVariable_GetValueAtFloat( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a char */
	#define StgVariable_GetValueFloatAsChar( self, array_I ) \
		( (char)StgVariable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a short */
	#define StgVariable_GetValueFloatAsShort( self, array_I ) \
		( (short)StgVariable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as an int */
	#define StgVariable_GetValueFloatAsInt( self, array_I ) \
		( (int)StgVariable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a double */
	#define StgVariable_GetValueFloatAsDouble( self, array_I ) \
		( (double)StgVariable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a pointer (void*) */
	#define StgVariable_GetValueFloatAsPointer( self, array_I ) \
		( (void*)( (int)StgVariable_GetValueFloat( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, set that float to a new value (internally type casted to
	 *  float) */
	#define StgVariable_SetValueFloat( self, array_I, value ) \
		( *StgVariable_GetPtrFloat( (self), (array_I) ) = (float)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "double" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "double" inbuilt type, return a pointer to that double (type casted to double*).
	 *  Private (Do not directly use!). */
	#define _StgVariable_GetPtrDouble( self, array_I ) \
		( (double*)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrDouble _StgVariable_GetPtrDouble

	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtDouble( self, array_I, vector_I ) \
		( (StgVariable_GetPtrDouble( (self), (array_I) ))[(vector_I)] )
	#define StgVariable_GetValueAtDouble _StgVariable_GetValueAtDouble

	/** Assuming this is a Variable of a vector "double" inbuilt type, return a pointer to that double.  */
	#define StgVariable_GetPtrAtDouble( self, array_I, vector_I ) \
		( &_StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a char */
	#define StgVariable_GetValueAtDoubleAsChar( self, array_I, vector_I ) \
		( (char)StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a short */
	#define StgVariable_GetValueAtDoubleAsShort( self, array_I, vector_I ) \
		( (short)StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a int */
	#define StgVariable_GetValueAtDoubleAsInt( self, array_I, vector_I ) \
		( (int)StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a float */
	#define StgVariable_GetValueAtDoubleAsFloat( self, array_I, vector_I ) \
		( (float)StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a double */
	#define StgVariable_GetValueAtDoubleAsDouble( self, array_I, vector_I ) \
		( (double)StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a pointer (void*) */
	#define StgVariable_GetValueAtDoubleAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)StgVariable_GetValueAtDouble( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, set that double to a new value (internally type casted to
	 *  double) */
	#define StgVariable_SetValueAtDouble( self, array_I, vector_I, value ) \
		( *StgVariable_GetPtrAtDouble( (self), (array_I), (vector_I) ) = (double)(value) )
	
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double */
	#define StgVariable_GetValueDouble( self, array_I ) \
		( StgVariable_GetValueAtDouble( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a char */
	#define StgVariable_GetValueDoubleAsChar( self, array_I ) \
		( (char)StgVariable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a short */
	#define StgVariable_GetValueDoubleAsShort( self, array_I ) \
		( (short)StgVariable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a int */
	#define StgVariable_GetValueDoubleAsInt( self, array_I ) \
		( (int)StgVariable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a float */
	#define StgVariable_GetValueDoubleAsFloat( self, array_I ) \
		( (float)StgVariable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a double */
	#define StgVariable_GetValueDoubleAsDouble( self, array_I ) \
		( (double)StgVariable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a pointer (void*) */
	#define StgVariable_GetValueDoubleAsPointer( self, array_I ) \
		( (void*)( (int)StgVariable_GetValueDouble( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, set that double to a new value (internally type casted to
	 *  double) */
	#define StgVariable_SetValueDouble( self, array_I, value ) \
		( *StgVariable_GetPtrDouble( (self), (array_I) ) = (double)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "pointer" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "pointer" inbuilt type, return a pointer to that pointer (type casted to void**).
	 *  Private. (Do not directly use!). */
	#define _StgVariable_GetPtrPointer( self, array_I ) \
		( (void**)_StgVariable_GetStructPtr( (self), (array_I) ) )
	#define StgVariable_GetPtrPointer _StgVariable_GetPtrPointer
	
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer. Private (Do not directly use!). */
	#define _StgVariable_GetValueAtPointer( self, array_I, vector_I ) \
		( StgVariable_GetPtrPointer( (self), (array_I) )[(vector_I)] )
	#define StgVariable_GetValueAtPointer _StgVariable_GetValueAtPointer

	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return a pointer to that pointer.  */
	#define StgVariable_GetPtrAtPointer( self, array_I, vector_I ) \
		( &_StgVariable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a char pointer (char*) */
	#define StgVariable_GetValueAtPointerAsCharPointer( self, array_I, vector_I ) \
		( (char*)StgVariable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a short pointer (short*) */
	#define StgVariable_GetValueAtPointerAsShortPointer( self, array_I, vector_I ) \
		( (short*)StgVariable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as an int pointer (int*) */
	#define StgVariable_GetValueAtPointerAsIntPointer( self, array_I, vector_I ) \
		( (int*)StgVariable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a float pointer (float*) */
	#define StgVariable_GetValueAtPointerAsFloatPointer( self, array_I, vector_I ) \
		( (float*)StgVariable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a double pointer (double*) */
	#define StgVariable_GetValueAtPointerAsDoublePointer( self, array_I, vector_I ) \
		( (double*)StgVariable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, set that pointer to a new value (internally type casted to
	 *  void*) */
	#define StgVariable_SetValueAtPointer( self, array_I, vector_I, value ) \
		( *StgVariable_GetPtrAtPointer( (self), (array_I), (vector_I) ) = (void*)(value) )
	
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer (void*) */
	#define StgVariable_GetValuePointer( self, array_I ) \
		( StgVariable_GetValueAtPointer( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a char pointer (char*) */
	#define StgVariable_GetValuePointerAsCharPointer( self, array_I ) \
		( (char*)StgVariable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a short pointer (short*) */
	#define StgVariable_GetValuePointerAsShortPointer( self, array_I ) \
		( (short*)StgVariable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as an int pointer (int*) */
	#define StgVariable_GetValuePointerAsIntPointer( self, array_I ) \
		( (int*)StgVariable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a float pointer (float*) */
	#define StgVariable_GetValuePointerAsFloatPointer( self, array_I ) \
		( (float*)StgVariable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a double pointer (double*) */
	#define StgVariable_GetValuePointerAsDoublePointer( self, array_I ) \
		( (double*)StgVariable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, set that pointer to a new value (internally type casted to
	 *  void*) */
	#define StgVariable_SetValuePointer( self, array_I, value ) \
		( *StgVariable_GetPtrPointer( (self), (array_I) ) = (void*)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of any inbuilt type
	*/
	
	/** Assuming this is a Variable of any inbuilt type, return a char (type casted to char). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _StgVariable_GetValueAsChar( self, array_I ) \
		( ((self)->dataTypes[0] == StgVariable_DataType_Char) ?	StgVariable_GetValueChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Short) ?	StgVariable_GetValueShortAsChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Int) ?	StgVariable_GetValueIntAsChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Float) ?	StgVariable_GetValueFloatAsChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Double) ?	StgVariable_GetValueDoubleAsChar( (self), (array_I) ) : \
		  (char)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#define StgVariable_GetValueAsChar _StgVariable_GetValueAsChar

	/** Assuming this is a Variable of any inbuilt type, return a short (type casted to short). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _StgVariable_GetValueAsShort( self, array_I ) \
		( ((self)->dataTypes[0] == StgVariable_DataType_Char) ?	StgVariable_GetValueCharAsShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Short) ?	StgVariable_GetValueShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Int) ?	StgVariable_GetValueIntAsShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Float) ?	StgVariable_GetValueFloatAsShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == StgVariable_DataType_Double) ?	StgVariable_GetValueDoubleAsShort( (self), (array_I) ) : \
		  (short)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#define StgVariable_GetValueAsShort _StgVariable_GetValueAsShort

	/** Assuming this is a Variable of any inbuilt type, return a int (type casted to int). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _StgVariable_GetValueAsInt( self, array_I ) \
		( (self)->dataTypes[0] == StgVariable_DataType_Char ?	StgVariable_GetValueCharAsInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Short ?	StgVariable_GetValueShortAsInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Int ?	StgVariable_GetValueInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Float ?	StgVariable_GetValueFloatAsInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Double ?	StgVariable_GetValueDoubleAsInt( (self), (array_I) ) : \
		  (int)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#define StgVariable_GetValueAsInt _StgVariable_GetValueAsInt

	/** Assuming this is a Variable of any inbuilt type, return a float (type casted to float). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer.*/
	#define _StgVariable_GetValueAsFloat( self, array_I ) \
		( (self)->dataTypes[0] == StgVariable_DataType_Char ?	StgVariable_GetValueCharAsFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Short ?	StgVariable_GetValueShortAsFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Int ?	StgVariable_GetValueIntAsFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Float ?	StgVariable_GetValueFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Double ?	StgVariable_GetValueDoubleAsFloat( (self), (array_I) ) : \
		  (float)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#define StgVariable_GetValueAsFloat _StgVariable_GetValueAsFloat

	/** Assuming this is a Variable of any inbuilt type, return a double (type casted to double). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _StgVariable_GetValueAsDouble( self, array_I ) \
		( (self)->dataTypes[0] == StgVariable_DataType_Char ?	StgVariable_GetValueCharAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Short ?	StgVariable_GetValueShortAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Int ?	StgVariable_GetValueIntAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Float ?	StgVariable_GetValueFloatAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Double ?	StgVariable_GetValueDouble( (self), (array_I) ) : \
		  (double)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#define StgVariable_GetValueAsDouble _StgVariable_GetValueAsDouble

	/** Assuming this is a Variable of any inbuilt type, return a pointer (type casted to void*). Private. (Do not directly
	 *  use!). Resolution order: pointer, short, int, float, double, pointer. */
	#define _StgVariable_GetValueAsPointer( self, array_I ) \
		( (self)->dataTypes[0] == StgVariable_DataType_Char ?	StgVariable_GetValueCharAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Short ?	StgVariable_GetValueShortAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Int ?	StgVariable_GetValueIntAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Float ?	StgVariable_GetValueFloatAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Double ?	StgVariable_GetValueDoubleAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Pointer ?	StgVariable_GetValuePointer( (self), (array_I) ) : \
		  (Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ), NULL) )
	#define StgVariable_GetValueAsPointer _StgVariable_GetValueAsPointer


	/** Assuming this is a Variable of any inbuilt type, set the value. Private. (Do not directly use!). Resolution order:
	 *  pointer, short, int, float, double, pointer. */
	#define _StgVariable_SetValueFromInbuilt( self, array_I, value ) \
		( (self)->dataTypes[0] == StgVariable_DataType_Char ?	StgVariable_SetValueChar( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Short ?	StgVariable_SetValueShort( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Int ?	StgVariable_SetValueInt( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Float ?	StgVariable_SetValueFloat( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Double ?	StgVariable_SetValueDouble( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == StgVariable_DataType_Pointer ?	StgVariable_SetValuePointer( (self), (array_I), (value) ) : \
		  (Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, StgVariable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ), NULL) )
	#define StgVariable_SetValueFromInbuilt _StgVariable_SetValuefromInBuilt

	/** Finds the new size of the array - replaces the limited ptr way of doing this. */
	Index _StgVariable_GetNewArraySize( StgVariable* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: for any Variable
	*/
	
	/** "Get" the requested data from the Variable that may be in an array. The returned value is a void*. If the Variable is
	 *  of a basic type, then the pointer to the scalar or the vector itself with the structure (if relevant) is returned. If
	 *  the Variable is of a complex type (i.e. it has more than one data component), then the pointer to the structure the
	 *  variable repesents is returned. */
	#define StgVariable_GetStructPtr _StgVariable_GetStructPtr
	
	/** Generic Variable "set". Works for any Variable. For complex Variables, the value must be the array of bytes of all the
	 * data members concatenated (i.e. in marshalled form, not the structured form). Inherently slow operation. */
	void StgVariable_SetValue( void* variable, Index array_I, void* value );
	
	/** Generic Variable "get". Works for any Variable. For complex Variables, the value must be the array of already allocated
	 * bytes of all the data members concatenated (i.e. in marshalled form, not the structured form). Inherently slow
	 * operation. */
	void StgVariable_GetValue( void* variable, Index array_I, void* value );

	/** Updates the local members in variable from pointers
	 *  Used then the data is resized or extened.
	 *  If the data is an array which the array length is changed, this function expects the length */
	void StgVariable_Update( void* variable );

	/** Sets value to all entries in array of variable */
	void StgVariable_SetValueDoubleAll( void* variable, double value ) ;

	void StgVariable_SaveToFileAsDoubles( void* variable, char* filename );

	void StgVariable_ReadFromFileAsDoubles( void* variable, char* filename );
	
	/** Returns || variable - comparison || / || variable ||, 
	  * where ||x|| indicates the Euclidean Norm which is the square root of the sum of the squares for each component in x
	  * i.e.  \f$ ||x|| = \sqrt{\sum_i x_i^2} \f$*/
	double StgVariable_ValueCompare( void* variable, void* _comparison );
	/** Checks whether || variable - comparison || / || variable || < tolerance */
	Bool StgVariable_ValueCompareWithinTolerance( void* variable, void* comparison, double tolerance );

    PyObject* StgVariable_getAsNumpyArray( void* variable );

    size_t StgVariable_SizeOfDataType( StgVariable_DataType dataType );

#endif /* __StGermain_Base_Context_StgVariable__ */

