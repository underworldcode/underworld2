/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_Variable_Register_h__
#define __StGermain_Base_Context_Variable_Register_h__
	
	
	extern const Type Variable_Register_Type;

	#define __Variable_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		/** Total number of variables counted */ \
		StgVariable_Index                          count; \
		SizeT                                   _size; \
		SizeT                                   _delta; \
		StgVariable**                              _variable; \
		Stream*					errorStream;
		
	struct _Variable_Register { __Variable_Register };
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/
	Variable_Register*	Variable_Register_New(void);
	
	void			Variable_Register_Init(Variable_Register* self);
	
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define VARIABLE_REGISTER_DEFARGS \
                STG_CLASS_DEFARGS

	#define VARIABLE_REGISTER_PASSARGS \
                STG_CLASS_PASSARGS

	Variable_Register*	_Variable_Register_New(  VARIABLE_REGISTER_DEFARGS  );
		
	void			_Variable_Register_Init(void* variable_Register);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void	_Variable_Register_Delete(void* variable_Register);
	
	void	_Variable_Register_Print(void* variable_Register, Stream* stream);

	void*	_Variable_Register_Copy( void* vr, void* dest, Bool deep, Name nameExt, PtrMap* ptrMap );
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Macros
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Virtual functions
	*/
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Build functions
	*/
	
	/* Add a variable... do before building */
	StgVariable_Index	Variable_Register_Add(void* variable_Register, StgVariable* variable);
	
	void		Variable_Register_BuildAll(void* variable_Register);
	
	
	/*--------------------------------------------------------------------------------------------------------------------------
	** Public Functions & Macros
	*/
	
	StgVariable_Index	Variable_Register_GetIndex(void* variable_Register, Name name);

	StgVariable*	Variable_Register_GetByName(void* variable_Register, Name name);

	StgVariable*	Variable_Register_GetByIndex( void* variable_Register, StgVariable_Index varIndex);	
	
#endif /* __StGermain_Base_Context_Variable_Register_h__ */

