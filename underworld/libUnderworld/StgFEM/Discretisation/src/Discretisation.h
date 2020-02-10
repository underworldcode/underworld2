/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __StgFEM_Discretisation_Discretisation_h__
#define __StgFEM_Discretisation_Discretisation_h__
	
	
	#include "types.h"

	#include <petsc.h>
	#include <petscvec.h>
	#include <petscmat.h>
	#include <petscksp.h>
#if( (PETSC_VERSION_MAJOR==3 && PETSC_VERSION_MINOR==0) || (PETSC_VERSION_MAJOR<3) )
	#include <petscmg.h>
#endif	
	#include <petscsnes.h>
	#include "PETScErrorChecking.h"

	#include "FeMesh_Algorithms.h"
	#include "FeMesh_ElementType.h"
	#include "ElementType.h"
	#include "ElementType_Register.h"
	#include "ConstantElementType.h"
	#include "LinearElementType.h"
	#include "BilinearElementType.h"
	#include "TrilinearElementType.h"
	#include "Biquadratic.h"
	#include "Triquadratic.h"
	#include "LinearTriangleElementType.h"
	#include "BilinearInnerElType.h"
	#include "TrilinearInnerElType.h"
    #include "dQ13DElementType.h"
    #include "dQ12DElementType.h"
	#include "dQ1Generator.h"

    #include "Element.h"
	#include "FeMesh.h"
	#include "C0Generator.h"
	#include "C2Generator.h"
	#include "Inner2DGenerator.h"
	#include "LinkedDofInfo.h"
	#include "FeEquationNumber.h"
	#include "FeVariable.h"

    #include "IrregularMeshGaussLayout.h"

	#include "Init.h"
	#include "Finalise.h"

#endif /* __StgFEM_Discretisation_Discretisation_h__ */
