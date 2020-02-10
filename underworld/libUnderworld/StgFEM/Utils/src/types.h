/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Utils_types_h__
#define __StgFEM_Utils_types_h__

   /* FE types/classes */
	typedef struct SemiLagrangianIntegrator		SemiLagrangianIntegrator;
	typedef struct IrregularMeshParticleLayout	IrregularMeshParticleLayout;
	typedef struct MeshParticleLayout		MeshParticleLayout;

   /* output streams: initialised in PDE_Rework_Init() */
   extern Stream* StgFEM_Utils_Debug;

#endif /* __StgFEM_Utils_types_h__ */
