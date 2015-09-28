/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Assembly_types_h__
#define __StgFEM_Assembly_types_h__
	
	/* FE types/classes */
	typedef struct GradientStiffnessMatrixTerm		GradientStiffnessMatrixTerm;
	typedef struct DivergenceMatrixTerm             	DivergenceMatrixTerm;
	typedef struct LaplacianStiffnessMatrixTerm		LaplacianStiffnessMatrixTerm;
	typedef struct IsoviscousStressTensorTerm		IsoviscousStressTensorTerm;
	typedef struct PressureGradForceTerm			PressureGradForceTerm;
	typedef struct PressureGradMatrixTerm			PressureGradMatrixTerm;
	typedef struct MassMatrixTerm MassMatrixTerm;

	/* output streams: initialised in StgFEM_Assembly_Init() */

#endif /* __StgFEM_Assembly_types_h__ */
