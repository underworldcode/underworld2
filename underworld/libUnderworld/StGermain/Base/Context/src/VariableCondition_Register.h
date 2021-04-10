/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_VariableCondition_Register_h__
#define __StGermain_Base_Context_VariableCondition_Register_h__
	

	/* Child classes must define these factories */
	typedef VariableCondition*	(VariableCondition_Register_FactoryFunc)
		( AbstractContext* context, Variable_Register* variable_Register, ConditionFunction_Register* conFunc_Register, Dictionary* dictionary, void* data );
	
	
	/** Textual name of this class */
	extern const Type VariableCondition_Register_Type;
	
	
	/** Global, default instantiation of the register... will be loaded with in built types (built in Init.c) */
	extern VariableCondition_Register* variableCondition_Register;
	
	
	/* Register entry info */
	#define __VariableCondition_Register_Entry \
		Type					type; \
		VariableCondition_Register_FactoryFunc*	factory;
	struct _VariableCondition_Register_Entry { __VariableCondition_Register_Entry };
	
	
	/* VariableCondition_Register info */
	#define __VariableCondition_Register \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* VariableCondition_Register info */ \
		VariableCondition_Index			count; \
		SizeT					_size; \
		SizeT					_delta; \
		VariableCondition_Register_Entry*	entry;
	struct _VariableCondition_Register { __VariableCondition_Register };
	
	
	/* Create a new VariableCondition_Register */
	VariableCondition_Register* VariableCondition_Register_New(void);
	
	/* Initialise a VariableCondition_Register */
	void VariableCondition_Register_Init(
			void*				variableCondition_Register);
	
	/* Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define VARIABLECONDITION_REGISTER_DEFARGS \
                STG_CLASS_DEFARGS

	#define VARIABLECONDITION_REGISTER_PASSARGS \
                STG_CLASS_PASSARGS

	VariableCondition_Register* _VariableCondition_Register_New(  VARIABLECONDITION_REGISTER_DEFARGS  );
	
	/* Initialisation implementation */
	void _VariableCondition_Register_Init( void* variableCondition_Register);
	
	
	/* Stg_Class_Delete implementation */
	void _VariableCondition_Register_Delete( void* variableCondition_Register );
	
	/* Print implementation */
	void _VariableCondition_Register_Print( void* variableCondition_Register, Stream* stream );
	
	
	/* Add a new entry */
	VariableCondition_Index VariableCondition_Register_Add( 
		void*					variableCondition_Register, 
		Type					type, 
		VariableCondition_Register_FactoryFunc*	factory );
	
	/* Get the handle to an entry */
	VariableCondition_Index VariableCondition_Register_GetIndex( void* variableCondition_Register, Type type );
	
	/* Get an entry from the register */
	#define VariableCondition_Register_At( variableCondition_Register, handle ) \
		( &(variableCondition_Register)->entry[(handle)] )
	VariableCondition_Register_Entry* _VariableCondition_Register_At( 
		void*					variableCondition_Register, 
		VariableCondition_Index			handle );
	
	/* Create a new instance of a type */
	VariableCondition* VariableCondition_Register_CreateNew(
		void* context,
		void* variableCondition_Register, 
		void* variable_Register, 
		void* conFunc_Register, 
		Type type, 
		void* dictionary, 
		void* data );
	
#endif /* __StGermain_Base_Context_VariableCondition_Register_h__ */

