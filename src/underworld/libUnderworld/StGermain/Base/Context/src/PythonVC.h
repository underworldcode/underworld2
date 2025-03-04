/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StGermain_Base_Context_PythonVC_h__
#define __StGermain_Base_Context_PythonVC_h__
	

	extern const Type PythonVC_Type;
	
	#define __PythonVC \
		/* General info */ \
		__VariableCondition \
		\
		/* Virtual info */ \
		\
		/* Stg_Class info */ \
		IndexSet** 	 _indexSets; \
		unsigned 	 _indexSetCount;

	struct PythonVC { __PythonVC };

	/*--------------------------------------------------------------------------------------------------------------------------
	** Constructor
	*/

	PythonVC* _PythonVC_DefaultNew( Name name );
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PYTHONVC_DEFARGS \
                VARIABLECONDITION_DEFARGS

	#define PYTHONVC_PASSARGS \
                VARIABLECONDITION_PASSARGS

	PythonVC* _PythonVC_New(  PYTHONVC_DEFARGS  );

	/*--------------------------------------------------------------------------------------------------------------------------
	** General virtual functions
	*/
	
	void _PythonVC_Delete( void* pythonVC );

	void _PythonVC_Destroy( void* pythonVC, void* data );

	/* Copy */
	#define PythonVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )
	#define PythonVC_Copy( self ) \
		(VariableCondition*)Stg_Class_Copy( self, NULL, False, NULL, NULL )

	void _PythonVC_Build(  void* pythonVC, void* data );

    void _PythonVC_Initialise( void* pythonVC, void* data );

    void _PythonVC_AssignFromXML( void* pythonVC, Stg_ComponentFactory* cf, void* data );

    void _PythonVC_SetupIndexSetArray( void* pythonVC, unsigned count );

    void _PythonVC_SetIndexSetAtArrayPosition( void* pythonVC, IndexSet* indexSet, unsigned position );

    void _PythonVC_Apply( void* variableCondition, void* context );

    Bool _PythonVC_IsCondition( void* variableCondition, Index localIndex, Index variableIndex );
	
#endif /* __StGermain_Base_Context_PythonVC_h__ */

