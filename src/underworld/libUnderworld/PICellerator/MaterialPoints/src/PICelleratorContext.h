/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __PICellerator_MaterialPoints_Context_h__
#define __PICellerator_MaterialPoints_Context_h__
	
	/* Textual name of this class */
	extern const Type PICelleratorContext_Type;
	
	#define __PICelleratorContext \
		/* General info */ \
		__FiniteElementContext \
		\
		/* Virtual info */ \

	struct PICelleratorContext { __PICelleratorContext };


	
	/* Constructors ----------------------------------------------------------------------------------------------------*/
	
	/** Constructor */
	void* _PICelleratorContext_DefaultNew( Name name );
	
	PICelleratorContext* PICelleratorContext_New( 
		Name			name,
		double		start,
		double		stop,
		MPI_Comm		communicator,
		Dictionary*	dictionary );
	
	/** Creation implementation / Virtual constructor */
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define PICELLERATORCONTEXT_DEFARGS \
                FINITEELEMENTCONTEXT_DEFARGS

	#define PICELLERATORCONTEXT_PASSARGS \
                FINITEELEMENTCONTEXT_PASSARGS

	PICelleratorContext* _PICelleratorContext_New(  PICELLERATORCONTEXT_DEFARGS  );
	
	/** Initialisation implementation */
	void _PICelleratorContext_Init( void* context );

	/* Virtual Functions -----------------------------------------------------------------------------------------------*/

	void _PICelleratorContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data );
	
	/* Stg_Class_Delete implementation */
	void _PICelleratorContext_Delete( void* context );

	/* Destroy implementation */	
   void _PICelleratorContext_Destroy( void* component, void* data );

	/* Print implementation */
	void _PICelleratorContext_Print( void* context, Stream* stream );
	
	/* Set the dt */
	void _PICelleratorContext_SetDt( void* context, double dt );

	void _PICelleratorContext_AssignFromXML( void* context, Stg_ComponentFactory* cf, void* data );
	
	/* Public functions -----------------------------------------------------------------------------------------------*/
	/* Private functions -----------------------------------------------------------------------------------------------*/
#endif /* __PICelleratorContext_h__ */

