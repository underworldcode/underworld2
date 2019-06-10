/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Automation_Stg_CallGraph_h__
#define __StGermain_Base_Automation_Stg_CallGraph_h__
	
	/* Templates of virtual functions */
	typedef struct {
		void*			functionPtr;
		Type			name;			/* assume its a const char* (i.e. we just need to copy ptr */
		void*			parentPtr;
		Type			parentName;		/* assume its a const char* (i.e. we just need to copy ptr */
		double			time;
		SizeT			memory;
		Index			called;
		Index			returned;
	} _Stg_CallGraph_Entry;
	
	typedef struct __Stg_CallGraph_Stack _Stg_CallGraph_Stack;
	struct __Stg_CallGraph_Stack {
		void*			functionPtr;
		Type			name;			/* assume its a const char* (i.e. we just need to copy ptr */
		double			time;
		SizeT			memory;
		Index			entry_I;
		_Stg_CallGraph_Stack*	pop;
	};
	
	/* Textual name of this class */
	extern const Type Stg_CallGraph_Type;
	
	/* Stg_Component information */
	#define __Stg_CallGraph \
		/* General info */ \
		__Stg_Class \
		\
		/* Virtual info */ \
		\
		/* Stg_CallGraph info */ \
		_Stg_CallGraph_Stack*	_stack; \
		_Stg_CallGraph_Entry*	table; \
		Index			_tableSize; \
		Index			_tableCount;
	struct Stg_CallGraph { __Stg_CallGraph };
	
	
	/** StGermain call graph singleton */
	extern Stg_CallGraph* stgCallGraph;
	
	/** Create a new CallGraph */
	Stg_CallGraph* Stg_CallGraph_New();
	
	/** Initialise an existing CallGraph */
	void Stg_CallGraph_Init( void* callGraph );
	
	/* Creation implementation */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define STG_CALLGRAPH_DEFARGS \
                STG_CLASS_DEFARGS

	#define STG_CALLGRAPH_PASSARGS \
                STG_CLASS_PASSARGS

	Stg_CallGraph* _Stg_CallGraph_New(  STG_CALLGRAPH_DEFARGS  );
	
	
	/* Class Administration members ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	/* Initialisation implementation */
	void _Stg_CallGraph_Init( Stg_CallGraph* self );
	
	/* Delete implementation */
	void _Stg_CallGraph_Delete( void* callGraph );
	
	/* Print implementation */
	void _Stg_CallGraph_Print( void* callGraph, Stream* stream );
	
	/* Copy implementation */
	void* _Stg_CallGraph_Copy( void* callGraph, void* dest, Bool deep, Name nameExt, struct PtrMap* ptrMap );
	
	
	/* Public member functions ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	
	/* Copy the component */
	#define Stg_CallGraph_Copy( self ) \
		(Stg_CallGraph*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define Stg_CallGraph_DeepCopy(self) \
		(Stg_CallGraph*)Stg_Class_Copy( self, NULL, True, NULL, NULL )
	
	/** Push a new function call onto the stack, and lodge trace */
	void Stg_CallGraph_Push( void* callGraph, void* functionPtr, Type name );
	
	/** Pop the function on the stack, and finalise trace */
	void Stg_CallGraph_Pop( void* callGraph );
	
#endif /* __StGermain_Base_Automation_Stg_CallGraph_h__ */

