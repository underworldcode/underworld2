/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_SLE_SystemSetup_SystemSetup_h__
#define __StgFEM_SLE_SystemSetup_SystemSetup_h__

	#include "types.h"

	#include "EntryPoint.h"

	#include "StiffnessMatrix.h"
	#include "StiffnessMatrixTerm.h"
	#include "SolutionVector.h"
	#include "ForceVector.h"
	#include "ForceTerm.h"

	#include "SystemLinearEquations.h"
	#include "SLE_Solver.h"

	#include "PETScMGSolver.h"
	#include "MGOpGenerator.h"
	#include "SROpGenerator.h"

	#include "FiniteElementContext.h"

	#include "Init.h"
	#include "Finalise.h"

#endif /* __StgFEM_SLE_SystemSetup_h__ */
