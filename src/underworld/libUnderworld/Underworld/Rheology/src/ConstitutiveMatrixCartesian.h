/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Underworld_Rheology_ConstitutiveMatrixCartesian_h__
#define __Underworld_Rheology_ConstitutiveMatrixCartesian_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/src/Function.hpp>
#include <Underworld/Function/src/FEMCoordinate.hpp>

struct ConstitutiveMatrixCartesian_cppdata
{
    Fn::Function::func func_visc1;
    Fn::Function::func func_visc2;
    Fn::Function::func func_director;
    std::shared_ptr<FEMCoordinate> input;
};

void _ConstitutiveMatrixCartesian_Set_Fn_Visc1(    void* _self, Fn::Function* fn_visc1    );
void _ConstitutiveMatrixCartesian_Set_Fn_Visc2(    void* _self, Fn::Function* fn_visc2    );
void _ConstitutiveMatrixCartesian_Set_Fn_Director( void* _self, Fn::Function* fn_director );

}
extern "C" {
#endif

	/** Textual name of this class - This is a global pointer which is used for times when you need to refer to class and not a particular instance of a class */
	extern const Type ConstitutiveMatrixCartesian_Type;

	/** ConstitutiveMatrixCartesian class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
	#define __ConstitutiveMatrixCartesian \
		/* Macro defining parent goes here - This means you can cast this class as its parent */ \
		__ConstitutiveMatrix \
		\
		/* Virtual functions go here */ \
		\
		/* ConstitutiveMatrixCartesian info */ \
		double** Dtilda_B; \
		double*  Ni;       \
		int      beenHere; \
        void*    cppdata;

	struct ConstitutiveMatrixCartesian { __ConstitutiveMatrixCartesian };
	
	#ifndef ZERO
	#define ZERO 0
	#endif

	#define CONSTITUTIVEMATRIXCARTESIAN_DEFARGS \
                CONSTITUTIVEMATRIX_DEFARGS

	#define CONSTITUTIVEMATRIXCARTESIAN_PASSARGS \
                CONSTITUTIVEMATRIX_PASSARGS

	ConstitutiveMatrixCartesian* _ConstitutiveMatrixCartesian_New(  CONSTITUTIVEMATRIXCARTESIAN_DEFARGS  );
	
	void _ConstitutiveMatrixCartesian_Delete( void* constitutiveMatrix );
	void _ConstitutiveMatrixCartesian_Print( void* constitutiveMatrix, Stream* stream );
   void _ConstitutiveMatrixCartesian_Init( ConstitutiveMatrixCartesian* self );

	void* _ConstitutiveMatrixCartesian_DefaultNew( Name name ) ;
void _ConstitutiveMatrixCartesian_AssignFromXML( void* constitutiveMatrix, Stg_ComponentFactory* cf, void* data ) ;
	void _ConstitutiveMatrixCartesian_Build( void* constitutiveMatrix, void* data ) ;
	void _ConstitutiveMatrixCartesian_Initialise( void* constitutiveMatrix, void* data ) ;
	void _ConstitutiveMatrixCartesian_Execute( void* constitutiveMatrix, void* data ) ;
	void _ConstitutiveMatrixCartesian_Destroy( void* constitutiveMatrix, void* data ) ;

	void _ConstitutiveMatrixCartesian_AssembleElement( 
		void*                                                constitutiveMatrix,
		StiffnessMatrix*                                     stiffnessMatrix, 
		Element_LocalIndex                                   lElement_I, 
		SystemLinearEquations*                               sle,
		FiniteElementContext*                                context,
		double**                                             elStiffMat ) ;

	void _ConstitutiveMatrixCartesian2D_SetValueInAllEntries( void* constitutiveMatrix, double value ) ;
	void _ConstitutiveMatrixCartesian3D_SetValueInAllEntries( void* constitutiveMatrix, double value ) ;

	double _ConstitutiveMatrixCartesian2D_GetIsotropicViscosity( void* constitutiveMatrix ) ;
	double _ConstitutiveMatrixCartesian3D_GetIsotropicViscosity( void* constitutiveMatrix ) ;

	void _ConstitutiveMatrixCartesian2D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) ;
	void _ConstitutiveMatrixCartesian3D_IsotropicCorrection( void* constitutiveMatrix, double isotropicCorrection ) ;

	void _ConstitutiveMatrixCartesian2D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, const XYZ director );
	void _ConstitutiveMatrixCartesian3D_SetSecondViscosity( void* constitutiveMatrix, double deltaViscosity, const XYZ director );

	void _ConstitutiveMatrixCartesian2D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );
	void _ConstitutiveMatrixCartesian3D_Assemble_D_B( void* constitutiveMatrix, double** GNx, Node_Index node_I, double** D_B );

	void _ConstitutiveMatrixCartesian2D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) ;
	void _ConstitutiveMatrixCartesian3D_CalculateStress( void* constitutiveMatrix, SymmetricTensor strainRate, SymmetricTensor stress ) ;

#ifdef __cplusplus
}
#endif

#endif

