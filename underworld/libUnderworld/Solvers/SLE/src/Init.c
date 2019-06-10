/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/
#include <mpi.h>
#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>

#include "SLE.h"

#include <stdio.h>

Bool Solvers_SLE_Init( int* argc, char** argv[] ) {
    Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

    Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 

    Stg_ComponentRegister_Add( componentRegister, AugLagStokes_SLE_Type, (Name)"0", _AugLagStokes_SLE_DefaultNew );				
    RegisterParent( AugLagStokes_SLE_Type, Stokes_SLE_Type );

	return True;
}


