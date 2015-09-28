/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_Variable_h__
#define __StGermain_Base_Context_Variable_h__

#include <Python.h>

	/** Textual name for Variable class. */
	extern const Type Variable_Type;

	typedef enum {
		Variable_DataType_Char,
		Variable_DataType_Short,
		Variable_DataType_Int,
		Variable_DataType_Float,
		Variable_DataType_Double,
		Variable_DataType_Pointer,
		Variable_DataType_Size /* Marker for derivatives to enum */
	} Variable_DataType;

	typedef Index (Variable_ArraySizeFunc) ( void* self );

	/* See Variable */

	#define __Variable \
		/* General info */ \
		__Stg_Component \
		\
		AbstractContext*			context; \
		/* Virtual info */ \
		\
		/* Variable info */ \
		Index							offsetCount; /**< Number of fields in this variable. */ \
		SizeT*						offsets; /**< The offset in bytes these fields are in the struct.*/ \
		Variable_DataType*		dataTypes; /**< A list of original data types. */ \
		Index*						dataTypeCounts; /**< A list of the number of data. */ \
		SizeT*						structSizePtr; /**< A pointer to the size of the structure. */ \
		void**						arrayPtrPtr; /**< A pointer to a pointer to the 1D array data. */ \
		Index*						arraySizePtr; /**< A ptr to the size/count of the 1D array data. Note that if this is NULL, the arraySizeFunc will be used */ \
		Variable_ArraySizeFunc*	arraySizeFunc; /**< A func ptr to the size/count of the 1D array data. */ \
		\
		SizeT*						dataSizes; /**< The size in bytes of each field in this variable. */ \
		SizeT							structSize; /**< The size of the structure. */ \
		void*							arrayPtr; /**< A pointer to the 1D array of structures. */ \
		Index							arraySize; /**< The size/count of the 1D array of structures. */ \
		Index							subVariablesCount; /**< The number of subvariables. Necessary since determined by whether user passes in names or not at init time. */ \
		Variable**					components; /**< For each component of this variable that we made a variable for, the pointer to the variable. */ \
		Bool							allocateSelf; \
		Variable_Register*		vr; \
		\
		Variable*					parent;

	struct _Variable { __Variable };

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
	Variable* Variable_New( 
		Name								name,
		AbstractContext*				context,
		Index								dataCount,
		SizeT*							dataOffsets,
		Variable_DataType*			dataTypes,
		Index*							dataTypeCounts,
		Name*								dataNames,
		SizeT*							structSizePtr,
		Index*							arraySizePtr,
		Variable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr );
	
	Variable* _Variable_DefaultNew( Name name );
	
	/** Creates a new Variable. A Variable holds the run-time information of a complex data type created by the programmer.
	  * Essentially it associates a textual name to a variable in the program that the user can use to access or modify.
	  *
	  * See Variable_New for more info.
	  * 
	  * This constructor is a shortcut to create a Variable of a scalar in an array. The stride/struct size is the size
	  * of the dataType. There is no casting. */
	Variable* Variable_NewScalar( 
		Name								name,
		AbstractContext*				context,
		Variable_DataType				dataType,
		Index*							arraySizePtr,
		Variable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr );
	
	/** Creates a new Variable. A Variable holds the run-time information of a complex data type created by the programmer.
	  * Essentially it associates a textual name to a variable in the program that the user can use to access or modify.
	  *
	  * See Variable_New for more info.
	  * 
	  * This constructor is a shortcut to create a Variable of a vector in an array. The names of the vector components are
	  * optional and are specified via the variable arguement list at the end. A 0 value signifies no name for the associated
	  * vector component. The stride/struct size is the size of the dataType. There is no casting.*/
	Variable* Variable_NewVector( 
		Name								name,
		AbstractContext*				context,
		Variable_DataType				dataType,
		Index								dataTypeCount,
		Index*							arraySizePtr,
		Variable_ArraySizeFunc*		arraySizeFunc,
		void**							arrayPtrPtr,
		Variable_Register*			vr,
		... 						/* vector component names */ );

	Variable* Variable_NewVector2( 
		Name								name,
		AbstractContext*				context,
		Variable_DataType				dataType,
		Index								dataTypeCount,
		Index*							arraySizePtr,
		Variable_ArraySizeFunc*		arraySizeFunc,
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
			Variable_DataType*            dataTypes, \
			Index*                   dataTypeCounts, \
			Name*                         dataNames, \
			SizeT*                    structSizePtr, \
			Index*                     arraySizePtr, \
			Variable_ArraySizeFunc*   arraySizeFunc, \
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

	Variable* _Variable_New(  VARIABLE_DEFARGS  );
	
	/** Init interface. */
	void _Variable_Init(
		Variable*					self, 
		AbstractContext*			context,
		Index							dataCount,
		SizeT*						dataOffsets,
		Variable_DataType*		dataTypes,
		Index*						dataTypeCounts,
		Name*							dataNames,
		SizeT*						structSizePtr,
		Index*						arraySizePtr,
		Variable_ArraySizeFunc*	arraySizeFunc,
		void**						arrayPtrPtr,
		Bool							allocateSelf,
		Variable_Register*		vr );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	/** Stg_Class_Delete Variable implementation */
	void _Variable_Delete( void* variable );
	
	/** Print Variable implementation */
	void _Variable_Print( void* variable, Stream* stream );
	
	/** Copy implementation */
	#define Variable_Copy( self ) \
		(Variable*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Variable_DeepCopy( self ) \
		(Variable*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	void* _Variable_Copy( void* variable, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/** Build implementation */
	void _Variable_Build( void* variable, void* data );
	
	/** Initialisation implementation */
	void _Variable_Initialise( void* variable, void* data );
	
	/** Execution implementation */
	void _Variable_Execute( void* variable, void* data );
	
	void _Variable_AssignFromXML( void* variable, Stg_ComponentFactory* cf, void* data );
	
	void _Variable_Destroy( void* variable, void* data );
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Private Member functions: Fundamental address calculators
	*/
	
	/** Implementation of "get" the structure in an array, that the requested data member is in. Private (Do not directly
	 *  use!)
	 *  NB:- the array_I needs to be cast to an ArithPointer below - see __Variable_GetPtr() for reason. */
	#define __Variable_GetStructPtr( self, array_I ) \
		( ( (self)->offsetCount < 2 ) ? \
			( (void*)( (ArithPointer)(self)->arrayPtr + (ArithPointer)(array_I) * (ArithPointer)( (self)->structSize) + (ArithPointer)( (self)->offsets[0] ) ))  : \
			( (void*)((ArithPointer)(self)->arrayPtr + (ArithPointer)(array_I) * (ArithPointer)((self)->structSize)) ) )
	#ifdef CAUTContextUS
		/** Implementation of "get" the structure in an array that the requested data member is in if CAUTContextUS is defined.
		 *  It ensures that array_I is within its bounds. Private (Do not directly use!) */
		#define _Variable_GetStructPtr( self, array_I ) \
			( ((array_I) < (self)->arraySize) ? \
				__Variable_GetStructPtr( (self), (array_I) ) : \
				/*TODO : call J_Firewall, then return NULL. */ \
				(void*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable %s, in macro _Variable_GetStructPtr: requested array index %d " \
					"out of range (array size is %d). Exiting.\n", (self)->name, array_I, \
					(self)->arraySize ), NULL) )
	#else
		/** Implementation of "get" the structure in an array, that the requested data member is in if CAUTContextUS is NOT
		 *  defined. It DOES NOT ensure that array_I is within its bounds. Private (Do not directly use!) */
		#define _Variable_GetStructPtr __Variable_GetStructPtr
	#endif
	
	
/* BUG!:	( (void*)((ArithPointer)_Variable_GetStructPtr( (self), (array_I) ) + (self)->offsets[component_I]) ) */

	/** Implementation of "get" the requested data member in a structure in an array. Private (Do not directly use!)
		NB - we cast the array_I to an ArithPointer since its usually passed in as an Index (unsigned int), which on
		some architectures such as the SGI Altix is of a different size to the ArithPointer (unsigned long) */
	#define __Variable_GetPtr( self, array_I, component_I, vector_I ) \
		( (void*)((ArithPointer)(self)->arrayPtr + (ArithPointer)(array_I) * (self)->structSize + (self)->offsets[component_I]) )
	#ifdef CAUTContextUS
		/** Implementation of "get" the requested data member in a structure in an array if CAUTContextUS is defined.
		 *  It ensures that component_I  and array_I are within its bounds. Private (Do not directly use!) */
		#define _Variable_GetPtr( self, array_I, component_I, vector_I ) \
			( ((component_I) < (self)->offsetCount) ? \
				__Variable_GetPtr( (self), (array_I), (component_I), (vector_I) ) :\
				(void*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Offset array_I out of range\n" ), NULL) )
	#else
		/** Implementation of "get" the requested data member in a structure in an array if CAUTContextUS is NOT
		 *  defined. It DOES NOT ensure that component_I and array_I are within its bounds. Private (Do not directly
		 *  use!) */
		#define _Variable_GetPtr __Variable_GetPtr
	#endif
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "char" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "char" inbuilt type, return a pointer to that char (type casted to char*). Private
	 * (Do not directly use!). */
	#define _Variable_GetPtrChar( self, array_I ) \
		( (char*)_Variable_GetStructPtr( (self), (array_I) ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a "char" inbuilt type, return a pointer to that char (type casted to char*).
		 *  It ensures the Variable is of a "char" and is not complex. */
		#define Variable_GetPtrChar( self, array_I ) \
			( ((self)->offsetCount == 1 && (self)->dataTypes[0] == Variable_DataType_Char) ? \
				_Variable_GetPtrChar( (self), (array_I) ) : \
				(char*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is not of a char or is complex, but this macro assumes so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of a "char" inbuilt type, return a pointer to that char (type casted to char*).
		 *  It DOES NOT ensure the Variable is of a "char" and is not complex. */
		 #define Variable_GetPtrChar _Variable_GetPtrChar
	#endif
	
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char. Private (Do not directly use!). */
	#define _Variable_GetValueAtChar( self, array_I, vector_I ) \
		( Variable_GetPtrChar( (self), (array_I) )[(vector_I)] )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a vector "char" inbuilt type, return that char. It ensures that vector_I is
		 *  within range. */
		#define Variable_GetValueAtChar( self, array_I, vector_I ) \
			( ((vector_I) < (self)->dataTypeCounts[0]) ? \
				_Variable_GetValueAtChar( (self), (array_I), (vector_I) ) : \
				(char)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Index into vector of inbuild type is out of range.\n" ) )
	#else
		/** Assuming this is a Variable of a vector "char" inbuilt type, return that char. It DOES NOT ensure that
		 *  vector_I is within range. */
		 #define Variable_GetValueAtChar _Variable_GetValueAtChar
	#endif
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return a pointer to that char.  */
	#define Variable_GetPtrAtChar( self, array_I, vector_I ) \
		( &_Variable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a short */
	#define Variable_GetValueAtCharAsShort( self, array_I, vector_I ) \
		( (short)Variable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as an int */
	#define Variable_GetValueAtCharAsInt( self, array_I, vector_I ) \
		( (int)Variable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a float */
	#define Variable_GetValueAtCharAsFloat( self, array_I, vector_I ) \
		( (float)Variable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a double */
	#define Variable_GetValueAtCharAsDouble( self, array_I, vector_I ) \
		( (double)Variable_GetValueAtChar( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, return that char as a pointer (void*) */
	#define Variable_GetValueAtCharAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)Variable_GetValueAtChar( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "char" inbuilt type, set that char to a new value (internally type casted to char) */
	#define Variable_SetValueAtChar( self, array_I, vector_I, value ) \
		( *Variable_GetPtrAtChar( (self), (array_I), (vector_I) ) = (char)(value) )
	
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char */
	#define Variable_GetValueChar( self, array_I ) \
		( Variable_GetValueAtChar( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a short */
	#define Variable_GetValueCharAsShort( self, array_I ) \
		( (short)Variable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as an int */
	#define Variable_GetValueCharAsInt( self, array_I ) \
		( (int)Variable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a float */
	#define Variable_GetValueCharAsFloat( self, array_I ) \
		( (float)Variable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a double */
	#define Variable_GetValueCharAsDouble( self, array_I ) \
		( (double)Variable_GetValueChar( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, return that char as a pointer (void*) */
	#define Variable_GetValueCharAsPointer( self, array_I ) \
		( (void*)( (int)Variable_GetValueChar( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a scalar "char" inbuilt type, set that char to a new value (internally type casted to
	 *  char) */
	#define Variable_SetValueChar( self, array_I, value ) \
		( *Variable_GetPtrChar( (self), (array_I) ) = (char)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "short" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "short" inbuilt type, return a pointer to that short (type casted to short*). Private
	 * (Do not directly use!). */
	#define _Variable_GetPtrShort( self, array_I ) \
		( (short*)_Variable_GetStructPtr( (self), (array_I) ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a "short" inbuilt type, return a pointer to that short (type casted to short*).
		 *  It ensures the Variable is of a "short" and is not complex. */
		#define Variable_GetPtrShort( self, array_I ) \
			( ((self)->offsetCount == 1 && (self)->dataTypes[0] == Variable_DataType_Short) ? \
				_Variable_GetPtrShort( (self), (array_I) ) : \
				(short*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is not of a short or is complex, but this macro assumes so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of a "short" inbuilt type, return a pointer to that short (type casted to short*).
		 *  It DOES NOT ensure the Variable is of a "short" and is not complex. */
		 #define Variable_GetPtrShort _Variable_GetPtrShort
	#endif
	
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short. Private (Do not directly use!). */
	#define _Variable_GetValueAtShort( self, array_I, vector_I ) \
		( Variable_GetPtrShort( (self), (array_I) )[(vector_I)] )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a vector "short" inbuilt type, return that short. It ensures that vector_I is
		 *  within range. */
		#define Variable_GetValueAtShort( self, array_I, vector_I ) \
			( ((vector_I) < (self)->dataTypeCounts[0]) ? \
				_Variable_GetValueAtShort( (self), (array_I), (vector_I) ) : \
				(short)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Index into vector of inbuild type is out of range.\n" ) )
	#else
		/** Assuming this is a Variable of a vector "short" inbuilt type, return that short. It DOES NOT ensure that
		 *  vector_I is within range. */
		 #define Variable_GetValueAtShort _Variable_GetValueAtShort
	#endif
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return a pointer to that short.  */
	#define Variable_GetPtrAtShort( self, array_I, vector_I ) \
		( &_Variable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a char */
	#define Variable_GetValueAtShortAsChar( self, array_I, vector_I ) \
		( (char)Variable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as an int */
	#define Variable_GetValueAtShortAsInt( self, array_I, vector_I ) \
		( (int)Variable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a float */
	#define Variable_GetValueAtShortAsFloat( self, array_I, vector_I ) \
		( (float)Variable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a double */
	#define Variable_GetValueAtShortAsDouble( self, array_I, vector_I ) \
		( (double)Variable_GetValueAtShort( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, return that short as a pointer (void*) */
	#define Variable_GetValueAtShortAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)Variable_GetValueAtShort( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "short" inbuilt type, set that short to a new value (internally type casted to
	 *  short)*/
	#define Variable_SetValueAtShort( self, array_I, vector_I, value ) \
		( *Variable_GetPtrAtShort( self, array_I, vector_I ) = (short)(value) )
	
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short */
	#define Variable_GetValueShort( self, array_I ) \
		( Variable_GetValueAtShort( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a char */
	#define Variable_GetValueShortAsChar( self, array_I ) \
		( (char)Variable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as an int */
	#define Variable_GetValueShortAsInt( self, array_I ) \
		( (int)Variable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a float */
	#define Variable_GetValueShortAsFloat( self, array_I ) \
		( (float)Variable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a double */
	#define Variable_GetValueShortAsDouble( self, array_I ) \
		( (double)Variable_GetValueShort( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, return that short as a pointer (void*) */
	#define Variable_GetValueShortAsPointer( self, array_I ) \
		( (void*)( (int)Variable_GetValueShort( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a "short" inbuilt type, set that short to a new value (internally type casted to
	 *  short)*/
	#define Variable_SetValueShort( self, array_I, value ) \
		( *Variable_GetPtrShort( self, array_I ) = (short)(value) )
	

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "int" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "int" inbuilt type, return a pointer to that int (type casted to int*). Private
	 * (Do not directly use!). */
	#define _Variable_GetPtrInt( self, array_I ) \
		( (int*)_Variable_GetStructPtr( (self), (array_I) ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a "int" inbuilt type, return a pointer to that int (type casted to int*).
		 *  It ensures the Variable is of a "int" and is not complex. */
		#define Variable_GetPtrInt( self, array_I ) \
			( ((self)->offsetCount == 1 && (self)->dataTypes[0] == Variable_DataType_Int) ? \
				_Variable_GetPtrInt( (self), (array_I) ) : \
				(int*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is not of a int or is complex, but this macro assumes so.\n" ),NULL) )
	#else
		/** Assuming this is a Variable of a "int" inbuilt type, return a pointer to that int (type casted to int*).
		 *  It DOES NOT ensure the Variable is of a "int" and is not complex. */
		 #define Variable_GetPtrInt _Variable_GetPtrInt
	#endif
	
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int. Private (Do not directly use!). */
	#define _Variable_GetValueAtInt( self, array_I, vector_I ) \
		( Variable_GetPtrInt( (self), (array_I) )[(vector_I)] )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a vector "int" inbuilt type, return that int. It ensures that vector_I is
		 *  within range. */
		#define Variable_GetValueAtInt( self, array_I, vector_I ) \
			( ((vector_I) < (self)->dataTypeCounts[0]) ? \
				_Variable_GetValueAtInt( (self), (array_I), (vector_I) ) : \
				(int)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Index into vector of inbuild type is out of range.\n" ) )
	#else
		/** Assuming this is a Variable of a vector "int" inbuilt type, return that int. It DOES NOT ensure that
		 *  vector_I is within range. */
		 #define Variable_GetValueAtInt _Variable_GetValueAtInt
	#endif
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return a pointer to that int.  */
	#define Variable_GetPtrAtInt( self, array_I, vector_I ) \
		( &_Variable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a char */
	#define Variable_GetValueAtIntAsChar( self, array_I, vector_I ) \
		( (char)Variable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a short */
	#define Variable_GetValueAtIntAsShort( self, array_I, vector_I ) \
		( (short)Variable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a float */
	#define Variable_GetValueAtIntAsFloat( self, array_I, vector_I ) \
		( (float)Variable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a double */
	#define Variable_GetValueAtIntAsDouble( self, array_I, vector_I ) \
		( (double)Variable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, return that int as a pointer (void*) */
	#define Variable_GetValueAtIntAsPointer( self, array_I, vector_I ) \
		( (void*)Variable_GetValueAtInt( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "int" inbuilt type, set that int to a new value (internally type casted to int) */
	#define Variable_SetValueAtInt( self, array_I, vector_I, value ) \
		( *Variable_GetPtrAtInt( (self), (array_I), (vector_I) ) = (int)(value) )
	
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int */
	#define Variable_GetValueInt( self, array_I ) \
		( Variable_GetValueAtInt( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a char */
	#define Variable_GetValueIntAsChar( self, array_I ) \
		( (char)Variable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a short */
	#define Variable_GetValueIntAsShort( self, array_I ) \
		( (short)Variable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a float */
	#define Variable_GetValueIntAsFloat( self, array_I ) \
		( (float)Variable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a double */
	#define Variable_GetValueIntAsDouble( self, array_I ) \
		( (double)Variable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, return that int as a pointer (void*) */
	#define Variable_GetValueIntAsPointer( self, array_I ) \
		( (void*)Variable_GetValueInt( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "int" inbuilt type, set that int to a new value (internally type casted to int) */
	#define Variable_SetValueInt( self, array_I, value ) \
		( *Variable_GetPtrInt( (self), (array_I) ) = (int)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "float" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "float" inbuilt type, return a pointer to that float (type casted to float*). Private
	 * (Do not directly use!). */
	#define _Variable_GetPtrFloat( self, array_I ) \
		( (float*)_Variable_GetStructPtr( (self), (array_I) ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a "float" inbuilt type, return a pointer to that float (type casted to float*).
		 *  It ensures the Variable is of a "float" and is not complex. */
		#define Variable_GetPtrFloat( self, array_I ) \
			( ((self)->offsetCount == 1 && (self)->dataTypes[0] == Variable_DataType_Float) ? \
				_Variable_GetPtrFloat( (self), (array_I) ) : \
				(float*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is not of a float or is complex, but this macro assumes so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of a "float" inbuilt type, return a pointer to that float (type casted to float*).
		 *  It DOES NOT ensure the Variable is of a "float" and is not complex. */
		 #define Variable_GetPtrFloat _Variable_GetPtrFloat
	#endif
	
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float. Private (Do not directly use!). */
	#define _Variable_GetValueAtFloat( self, array_I, vector_I ) \
		( Variable_GetPtrFloat( (self), (array_I) )[(vector_I)] )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a vector "float" inbuilt type, return that float. It ensures that vector_I is
		 *  within range. */
		#define Variable_GetValueAtFloat( self, array_I, vector_I ) \
			( ((vector_I) < (self)->dataTypeCounts[0]) ? \
				_Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) : \
				(float)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Index into vector of inbuild type is out of range.\n" ) )
	#else
		/** Assuming this is a Variable of a vector "float" inbuilt type, return that float. It DOES NOT ensure that
		 *  vector_I is within range. */
		 #define Variable_GetValueAtFloat _Variable_GetValueAtFloat
	#endif
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return a pointer to that float.  */
	#define Variable_GetPtrAtFloat( self, array_I, vector_I ) \
		( &_Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a char */
	#define Variable_GetValueAtFloatAsChar( self, array_I, vector_I ) \
		( (char)Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a short */
	#define Variable_GetValueAtFloatAsShort( self, array_I, vector_I ) \
		( (short)Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as an int */
	#define Variable_GetValueAtFloatAsInt( self, array_I, vector_I ) \
		( (int)Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a double */
	#define Variable_GetValueAtFloatAsDouble( self, array_I, vector_I ) \
		( (double)Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, return that float as a pointer (void*) */
	#define Variable_GetValueAtFloatAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)Variable_GetValueAtFloat( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "float" inbuilt type, set that float to a new value (internally type casted to
	 *  float) */
	#define Variable_SetValueAtFloat( self, array_I, vector_I, value ) \
		( *Variable_GetPtrAtFloat( (self), (array_I), (vector_I) ) = (float)(value) )
	
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float */
	#define Variable_GetValueFloat( self, array_I ) \
		( Variable_GetValueAtFloat( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a char */
	#define Variable_GetValueFloatAsChar( self, array_I ) \
		( (char)Variable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a short */
	#define Variable_GetValueFloatAsShort( self, array_I ) \
		( (short)Variable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as an int */
	#define Variable_GetValueFloatAsInt( self, array_I ) \
		( (int)Variable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a double */
	#define Variable_GetValueFloatAsDouble( self, array_I ) \
		( (double)Variable_GetValueFloat( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, return that float as a pointer (void*) */
	#define Variable_GetValueFloatAsPointer( self, array_I ) \
		( (void*)( (int)Variable_GetValueFloat( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a scalar "float" inbuilt type, set that float to a new value (internally type casted to
	 *  float) */
	#define Variable_SetValueFloat( self, array_I, value ) \
		( *Variable_GetPtrFloat( (self), (array_I) ) = (float)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "double" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "double" inbuilt type, return a pointer to that double (type casted to double*).
	 *  Private (Do not directly use!). */
	#define _Variable_GetPtrDouble( self, array_I ) \
		( (double*)_Variable_GetStructPtr( (self), (array_I) ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a "double" inbuilt type, return a pointer to that double (type casted to
		 *  double*). It ensures the Variable is of a "double" and is not complex. */
		#define Variable_GetPtrDouble( self, array_I ) \
			( ((self)->offsetCount == 1 && (self)->dataTypes[0] == Variable_DataType_Double) ? \
				_Variable_GetPtrDouble( (self), (array_I) ) : \
				(double*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is not of a double or is complex, but this macro assumes so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of a "double" inbuilt type, return a pointer to that double (type casted to
		 *   double*). It DOES NOT ensure the Variable is of a "double" and is not complex. */
		 #define Variable_GetPtrDouble _Variable_GetPtrDouble
	#endif
	
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double. Private (Do not directly use!). */
	#define _Variable_GetValueAtDouble( self, array_I, vector_I ) \
		( (Variable_GetPtrDouble( (self), (array_I) ))[(vector_I)] )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a vector "double" inbuilt type, return that double. It ensures that vector_I is
		 *  within range. */
		#define Variable_GetValueAtDouble( self, array_I, vector_I ) \
			( ((vector_I) < (self)->dataTypeCounts[0]) ? \
				_Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) : \
				(double)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Index into vector of inbuild type is out of range.\n" ) )
	#else
		/** Assuming this is a Variable of a vector "double" inbuilt type, return that double. It DOES NOT ensure that
		 *  vector_I is within range. */
		 #define Variable_GetValueAtDouble _Variable_GetValueAtDouble
	#endif
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return a pointer to that double.  */
	#define Variable_GetPtrAtDouble( self, array_I, vector_I ) \
		( &_Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a char */
	#define Variable_GetValueAtDoubleAsChar( self, array_I, vector_I ) \
		( (char)Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a short */
	#define Variable_GetValueAtDoubleAsShort( self, array_I, vector_I ) \
		( (short)Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a int */
	#define Variable_GetValueAtDoubleAsInt( self, array_I, vector_I ) \
		( (int)Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a float */
	#define Variable_GetValueAtDoubleAsFloat( self, array_I, vector_I ) \
		( (float)Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a double */
	#define Variable_GetValueAtDoubleAsDouble( self, array_I, vector_I ) \
		( (double)Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, return that double as a pointer (void*) */
	#define Variable_GetValueAtDoubleAsPointer( self, array_I, vector_I ) \
		( (void*)( (int)Variable_GetValueAtDouble( (self), (array_I), (vector_I) ) ) )
	
	/** Assuming this is a Variable of a vector "double" inbuilt type, set that double to a new value (internally type casted to
	 *  double) */
	#define Variable_SetValueAtDouble( self, array_I, vector_I, value ) \
		( *Variable_GetPtrAtDouble( (self), (array_I), (vector_I) ) = (double)(value) )
	
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double */
	#define Variable_GetValueDouble( self, array_I ) \
		( Variable_GetValueAtDouble( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a char */
	#define Variable_GetValueDoubleAsChar( self, array_I ) \
		( (char)Variable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a short */
	#define Variable_GetValueDoubleAsShort( self, array_I ) \
		( (short)Variable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a int */
	#define Variable_GetValueDoubleAsInt( self, array_I ) \
		( (int)Variable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a float */
	#define Variable_GetValueDoubleAsFloat( self, array_I ) \
		( (float)Variable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a double */
	#define Variable_GetValueDoubleAsDouble( self, array_I ) \
		( (double)Variable_GetValueDouble( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, return that double as a pointer (void*) */
	#define Variable_GetValueDoubleAsPointer( self, array_I ) \
		( (void*)( (int)Variable_GetValueDouble( (self), (array_I) ) ) )
	
	/** Assuming this is a Variable of a scalar "double" inbuilt type, set that double to a new value (internally type casted to
	 *  double) */
	#define Variable_SetValueDouble( self, array_I, value ) \
		( *Variable_GetPtrDouble( (self), (array_I) ) = (double)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of a "pointer" inbuilt type
	*/
	
	/** Assuming this is a Variable of a "pointer" inbuilt type, return a pointer to that pointer (type casted to void**).
	 *  Private. (Do not directly use!). */
	#define _Variable_GetPtrPointer( self, array_I ) \
		( (void**)_Variable_GetStructPtr( (self), (array_I) ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a "pointer" inbuilt type, return a pointer to that pointer (type casted to
		 *  void**). It ensures the Variable is of a "pointer" and is not complex. */
		#define Variable_GetPtrPointer( self, array_I ) \
			( ((self)->offsetCount == 1 && (self)->dataTypes[0] == Variable_DataType_Pointer) ? \
				_Variable_GetPtrPointer( (self), (array_I) ) : \
				(void**)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is not of a pointer or is complex, but this macro assumes so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of a "pointer" inbuilt type, return a pointer to that pointer (type casted to
		 *  void**). It DOES NOT ensure the Variable is of a "pointer" and is not complex. */
		 #define Variable_GetPtrPointer _Variable_GetPtrPointer
	#endif
	
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer. Private (Do not directly use!). */
	#define _Variable_GetValueAtPointer( self, array_I, vector_I ) \
		( Variable_GetPtrPointer( (self), (array_I) )[(vector_I)] )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer. It ensures that vector_I is
		 *  within range. */
		#define Variable_GetValueAtPointer( self, array_I, vector_I ) \
			( ((vector_I) < (self)->dataTypeCounts[0]) ? \
				_Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) : \
				(void*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Index into vector of inbuild type is out of range.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer. It DOES NOT ensure that
		 *  vector_I is within range. */
		 #define Variable_GetValueAtPointer _Variable_GetValueAtPointer
	#endif
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return a pointer to that pointer.  */
	#define Variable_GetPtrAtPointer( self, array_I, vector_I ) \
		( &_Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a char pointer (char*) */
	#define Variable_GetValueAtPointerAsCharPointer( self, array_I, vector_I ) \
		( (char*)Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a short pointer (short*) */
	#define Variable_GetValueAtPointerAsShortPointer( self, array_I, vector_I ) \
		( (short*)Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as an int pointer (int*) */
	#define Variable_GetValueAtPointerAsIntPointer( self, array_I, vector_I ) \
		( (int*)Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a float pointer (float*) */
	#define Variable_GetValueAtPointerAsFloatPointer( self, array_I, vector_I ) \
		( (float*)Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, return that pointer as a double pointer (double*) */
	#define Variable_GetValueAtPointerAsDoublePointer( self, array_I, vector_I ) \
		( (double*)Variable_GetValueAtPointer( (self), (array_I), (vector_I) ) )
	
	/** Assuming this is a Variable of a vector "pointer" inbuilt type, set that pointer to a new value (internally type casted to
	 *  void*) */
	#define Variable_SetValueAtPointer( self, array_I, vector_I, value ) \
		( *Variable_GetPtrAtPointer( (self), (array_I), (vector_I) ) = (void*)(value) )
	
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer (void*) */
	#define Variable_GetValuePointer( self, array_I ) \
		( Variable_GetValueAtPointer( (self), (array_I), 0 ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a char pointer (char*) */
	#define Variable_GetValuePointerAsCharPointer( self, array_I ) \
		( (char*)Variable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a short pointer (short*) */
	#define Variable_GetValuePointerAsShortPointer( self, array_I ) \
		( (short*)Variable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as an int pointer (int*) */
	#define Variable_GetValuePointerAsIntPointer( self, array_I ) \
		( (int*)Variable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a float pointer (float*) */
	#define Variable_GetValuePointerAsFloatPointer( self, array_I ) \
		( (float*)Variable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, return that pointer as a double pointer (double*) */
	#define Variable_GetValuePointerAsDoublePointer( self, array_I ) \
		( (double*)Variable_GetValuePointer( (self), (array_I) ) )
	
	/** Assuming this is a Variable of a scalar "pointer" inbuilt type, set that pointer to a new value (internally type casted to
	 *  void*) */
	#define Variable_SetValuePointer( self, array_I, value ) \
		( *Variable_GetPtrPointer( (self), (array_I) ) = (void*)(value) )
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: Assuming this is a Variable of any inbuilt type
	*/
	
	/** Assuming this is a Variable of any inbuilt type, return a char (type casted to char). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _Variable_GetValueAsChar( self, array_I ) \
		( ((self)->dataTypes[0] == Variable_DataType_Char) ?	Variable_GetValueChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Short) ?	Variable_GetValueShortAsChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Int) ?	Variable_GetValueIntAsChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Float) ?	Variable_GetValueFloatAsChar( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Double) ?	Variable_GetValueDoubleAsChar( (self), (array_I) ) : \
		  (char)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, return a char (type casted to char). It ensures the Variable
		 * is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsChar( self, array_I ) \
			( ((self)->offsetCount == 1) ? \
				_Variable_GetValueAsChar( (self), (array_I) ) : \
				(char)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ) )
	#else
		/** Assuming this is a Variable of any inbuilt type, return a char (type casted to char). It DOES NOT ensure the
		 *  Variable is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsChar _Variable_GetValueAsChar
	#endif
	
	/** Assuming this is a Variable of any inbuilt type, return a short (type casted to short). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _Variable_GetValueAsShort( self, array_I ) \
		( ((self)->dataTypes[0] == Variable_DataType_Char) ?	Variable_GetValueCharAsShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Short) ?	Variable_GetValueShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Int) ?	Variable_GetValueIntAsShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Float) ?	Variable_GetValueFloatAsShort( (self), (array_I) ) : \
		  ((self)->dataTypes[0] == Variable_DataType_Double) ?	Variable_GetValueDoubleAsShort( (self), (array_I) ) : \
		  (short)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, return a short (type casted to short). It ensures the Variable
		 * is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsShort( self, array_I ) \
			( ((self)->offsetCount == 1) ? \
				_Variable_GetValueAsShort( (self), (array_I) ) : \
				(short)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ) )
	#else
		/** Assuming this is a Variable of any inbuilt type, return a short (type casted to short). It DOES NOT ensure the
		 *  Variable is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsShort _Variable_GetValueAsShort
	#endif
	
	/** Assuming this is a Variable of any inbuilt type, return a int (type casted to int). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _Variable_GetValueAsInt( self, array_I ) \
		( (self)->dataTypes[0] == Variable_DataType_Char ?	Variable_GetValueCharAsInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Short ?	Variable_GetValueShortAsInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Int ?	Variable_GetValueInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Float ?	Variable_GetValueFloatAsInt( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Double ?	Variable_GetValueDoubleAsInt( (self), (array_I) ) : \
		  (int)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, return a int (type casted to int). It ensures the Variable
		 * is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsInt( self, array_I ) \
			( (self)->offsetCount == 1 ? \
				_Variable_GetValueAsInt( (self), (array_I) ) : \
				(int)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ) )
	#else
		/** Assuming this is a Variable of any inbuilt type, return a int (type casted to int). It DOES NOT ensure the
		 *  Variable is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsInt _Variable_GetValueAsInt
	#endif
	
	/** Assuming this is a Variable of any inbuilt type, return a float (type casted to float). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer.*/
	#define _Variable_GetValueAsFloat( self, array_I ) \
		( (self)->dataTypes[0] == Variable_DataType_Char ?	Variable_GetValueCharAsFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Short ?	Variable_GetValueShortAsFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Int ?	Variable_GetValueIntAsFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Float ?	Variable_GetValueFloat( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Double ?	Variable_GetValueDoubleAsFloat( (self), (array_I) ) : \
		  (float)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, return a float (type casted to float). It ensures the Variable
		 * is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsFloat( self, array_I ) \
			( (self)->offsetCount == 1 ? \
				_Variable_GetValueAsFloat( (self), (array_I) ) : \
				(float)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ) )
	#else
		/** Assuming this is a Variable of any inbuilt type, return a float (type casted to float). It DOES NOT ensure the
		 *  Variable is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsFloat _Variable_GetValueAsFloat
	#endif
	
	/** Assuming this is a Variable of any inbuilt type, return a double (type casted to double). Private. (Do not directly
	 *  use!). Resolution order: char, short, int, float, double, pointer. */
	#define _Variable_GetValueAsDouble( self, array_I ) \
		( (self)->dataTypes[0] == Variable_DataType_Char ?	Variable_GetValueCharAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Short ?	Variable_GetValueShortAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Int ?	Variable_GetValueIntAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Float ?	Variable_GetValueFloatAsDouble( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Double ?	Variable_GetValueDouble( (self), (array_I) ) : \
		  (double)Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, return a double (type casted to double). It ensures the
		 *  Variable is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsDouble( self, array_I ) \
			( (self)->offsetCount == 1 ? \
				_Variable_GetValueAsDouble( (self), (array_I) ) : \
				(double)Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ) )
	#else
		/** Assuming this is a Variable of any inbuilt type, return a double (type casted to double). It DOES NOT ensure
		 *  the Variable is not complex. Resolution order: char, short, int, float, double, pointer. */
		#define Variable_GetValueAsDouble _Variable_GetValueAsDouble
	#endif
	
	/** Assuming this is a Variable of any inbuilt type, return a pointer (type casted to void*). Private. (Do not directly
	 *  use!). Resolution order: pointer, short, int, float, double, pointer. */
	#define _Variable_GetValueAsPointer( self, array_I ) \
		( (self)->dataTypes[0] == Variable_DataType_Char ?	Variable_GetValueCharAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Short ?	Variable_GetValueShortAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Int ?	Variable_GetValueIntAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Float ?	Variable_GetValueFloatAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Double ?	Variable_GetValueDoubleAsPointer( (self), (array_I) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Pointer ?	Variable_GetValuePointer( (self), (array_I) ) : \
		  (Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ), NULL) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, return a pointer (type casted to void*). It ensures the Variable
		 * is not complex. Resolution order: pointer, short, int, float, double, pointer. */
		#define Variable_GetValueAsPointer( self, array_I ) \
			( (self)->offsetCount == 1 ? \
				_Variable_GetValueAsPointer( (self), (array_I) ) : \
				(void*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of any inbuilt type, return a pointer (type casted to void*). It DOES NOT ensure the
		 *  Variable is not complex. Resolution order: pointer, short, int, float, double, pointer. */
		#define Variable_GetValueAsPointer _Variable_GetValueAsPointer
	#endif
	

	/** Assuming this is a Variable of any inbuilt type, set the value. Private. (Do not directly use!). Resolution order:
	 *  pointer, short, int, float, double, pointer. */
	#define _Variable_SetValueFromInbuilt( self, array_I, value ) \
		( (self)->dataTypes[0] == Variable_DataType_Char ?	Variable_SetValueChar( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Short ?	Variable_SetValueShort( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Int ?	Variable_SetValueInt( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Float ?	Variable_SetValueFloat( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Double ?	Variable_SetValueDouble( (self), (array_I), (value) ) : \
		  (self)->dataTypes[0] == Variable_DataType_Pointer ?	Variable_SetValuePointer( (self), (array_I), (value) ) : \
		  (Journal_Firewall( \
			0, \
			Journal_Register( Error_Type, Variable_Type ), \
			"Variable is not of any inbuilt type, but this macro assumes so.\n" ), NULL) )
	#ifdef CAUTContextUS
		/** Assuming this is a Variable of any inbuilt type, set the value. It ensures the Variable is not complex.
		  * Resolution order: pointer, short, int, float, double, pointer. */
		#define Variable_SetValueFromInbuilt( self, array_I, value ) \
			( (self)->offsetCount == 1 ? \
				_Variable_SetValueFromInbuilt( (self), (array_I), (value) ) : \
				(void*)(Journal_Firewall( \
					0, \
					Journal_Register( Error_Type, Variable_Type ), \
					"Variable is complex, but this macro assumes not so.\n" ), NULL) )
	#else
		/** Assuming this is a Variable of any inbuilt type, set the value. It DOES NOT ensure the Variable is not complex.
		 *  Resolution order: pointer, short, int, float, double, pointer. */
		#define Variable_SetValueFromInbuilt _Variable_SetValuefromInBuilt
	#endif
	
	/** Finds the new size of the array - replaces the limited ptr way of doing this. */
	Index _Variable_GetNewArraySize( Variable* self );

	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Member functions: for any Variable
	*/
	
	/** "Get" the requested data from the Variable that may be in an array. The returned value is a void*. If the Variable is
	 *  of a basic type, then the pointer to the scalar or the vector itself with the structure (if relevant) is returned. If
	 *  the Variable is of a complex type (i.e. it has more than one data component), then the pointer to the structure the
	 *  variable repesents is returned. */
	#define Variable_GetStructPtr _Variable_GetStructPtr
	
	/** Generic Variable "set". Works for any Variable. For complex Variables, the value must be the array of bytes of all the
	 * data members concatenated (i.e. in marshalled form, not the structured form). Inherently slow operation. */
	void Variable_SetValue( void* variable, Index array_I, void* value );
	
	/** Generic Variable "get". Works for any Variable. For complex Variables, the value must be the array of already allocated
	 * bytes of all the data members concatenated (i.e. in marshalled form, not the structured form). Inherently slow
	 * operation. */
	void Variable_GetValue( void* variable, Index array_I, void* value );

	/** Updates the local members in variable from pointers
	 *  Used then the data is resized or extened.
	 *  If the data is an array which the array length is changed, this function expects the length */
	void Variable_Update( void* variable );

	/** Sets value to all entries in array of variable */
	void Variable_SetValueDoubleAll( void* variable, double value ) ;

	void Variable_SaveToFileAsDoubles( void* variable, char* filename );

	void Variable_ReadFromFileAsDoubles( void* variable, char* filename );
	
	/** Returns || variable - comparison || / || variable ||, 
	  * where ||x|| indicates the Euclidean Norm which is the square root of the sum of the squares for each component in x
	  * i.e.  \f$ ||x|| = \sqrt{\sum_i x_i^2} \f$*/
	double Variable_ValueCompare( void* variable, void* _comparison );
	/** Checks whether || variable - comparison || / || variable || < tolerance */
	Bool Variable_ValueCompareWithinTolerance( void* variable, void* comparison, double tolerance );

    PyObject* Variable_getAsNumpyArray( void* variable );

    size_t Variable_SizeOfDataType( Variable_DataType dataType );

#endif /* __StGermain_Base_Context_Variable__ */

