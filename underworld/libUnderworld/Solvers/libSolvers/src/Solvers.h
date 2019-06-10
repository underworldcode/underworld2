/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __Solvers_h__
#define __Solvers_h__

#include "SLE/SLE.h"
#include "Assembly/Assembly.h"
#include "KSPSolvers/KSPSolvers.h"

#ifdef HAVE_PETScEXT
	#include "petscext.h"
	#include "petscext_vec.h"
	#include "petscext_mat.h"
	#include "petscext_pc.h"
	#include "petscext_ksp.h"
	#include "petscext_snes.h"
	#include "petscext_utils.h"
#endif

#include "Init.h"
#include "Finalise.h"

#endif
