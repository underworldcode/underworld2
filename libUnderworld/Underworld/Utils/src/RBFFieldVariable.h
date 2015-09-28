/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Utils_RBFFieldVariable_h__
#define __Underworld_Utils_RBFFieldVariable_h__

	/** Textual name of this class */
	extern const Type RBFFieldVariable_Type;
	
	/** RBFFieldVariable contents */
	#define __RBFFieldVariable                                 \
		/* General info */                                      \
		__FieldVariable                                         \
		                                                        \
		/* Member info */                                       \
      RBFManager*                 rbfManager;                 \
		Bool                        useShepardCorrection;       \
		double                      defaultValue;               \
		double                      offset;                     \
		double                      shepardCorrectionThreshold; \
      SwarmVariable*              swarmVariable;

	struct RBFFieldVariable { __RBFFieldVariable };	



	/** Creation implementation */
	void* _RBFFieldVariable_DefaultNew( Name name );

	
	#ifndef ZERO
	#define ZERO 0
	#endif

   #define RBFFIELDVARIABLE_DEFARGS \
                FIELDVARIABLE_DEFARGS
                
   #define RBFFIELDVARIABLE_PASSARGS \
                FIELDVARIABLE_PASSARGS  

   RBFFieldVariable* _RBFFieldVariable_New(  RBFFIELDVARIABLE_DEFARGS  );

	/** Member initialisation implementation */
	void _RBFFieldVariable_Init( 
   RBFFieldVariable*          self,
   RBFManager*                rbfManager,
   Bool                       useShepardCorrection,
   double                     defaultValue,
   double                     offset,
   double                     shepardCorrectionThreshold,
   SwarmVariable*             swarmVariable ) ;
	
	void _RBFFieldVariable_AssignFromXML( void* RBFFieldVariable, Stg_ComponentFactory* cf, void* data ) ;

	void _RBFFieldVariable_Build( void* RBFFieldVariable, void* data ) ;

	void _RBFFieldVariable_Execute( void* RBFFieldVariable, void* data ) ;

	void _RBFFieldVariable_Destroy( void* RBFFieldVariable, void* data ) ;

	void _RBFFieldVariable_Initialise( void* RBFFieldVariable, void* data ) ;
	
   /** Interpolate the value of the RBF variable at a particular coord **/
	InterpolationResult _RBFFieldVariable_InterpolateValueAt( void* RBFFieldVariable, double* coord, double* value );

	/** Implementations of the min and max val functions */
	void _RBFFieldVariable_CacheMinMaxGlobalFieldMagnitude( void* feVariable );

	InterpolationResult  RBFFieldVariable_InterpolateGradientValueAt( void* _RBFFieldVariable, double* coord, double* value, int axis );



#endif /* __Underworld_Utils_RBFFieldVariable_h__ */

