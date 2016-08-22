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
#include <PICellerator/PICellerator.h>

#include <Underworld/Rheology/Rheology.h>

#include "Utils.h"

#include <stdio.h>

Bool Underworld_Utils_Init( int* argc, char** argv[] ) {
    Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();

    Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ );

    Stg_ComponentRegister_Add( componentRegister, UnderworldContext_Type, (Name)"0", _UnderworldContext_DefaultNew  );
    RegisterParent( UnderworldContext_Type, PICelleratorContext_Type );

    Stg_ComponentRegister_Add( componentRegister, VectorAssemblyTerm_NA__Fn_Type, (Name)"0", _VectorAssemblyTerm_NA__Fn_DefaultNew  );
    RegisterParent( VectorAssemblyTerm_NA__Fn_Type, ForceTerm_Type );

    Stg_ComponentRegister_Add( componentRegister, VectorSurfaceAssemblyTerm_NA__Fn__ni_Type, (Name)"0", _VectorSurfaceAssemblyTerm_NA__Fn__ni_DefaultNew  );
    RegisterParent( VectorSurfaceAssemblyTerm_NA__Fn__ni_Type, ForceTerm_Type );

    Stg_ComponentRegister_Add( componentRegister, MatrixAssemblyTerm_NA_i__NB_i__Fn_Type, (Name)"0", _MatrixAssemblyTerm_NA_i__NB_i__Fn_DefaultNew  );
    RegisterParent( MatrixAssemblyTerm_NA_i__NB_i__Fn_Type, StiffnessMatrixTerm_Type );

    Stg_ComponentRegister_Add( componentRegister, MatrixAssemblyTerm_NA__NB__Fn_Type, (Name)"0", _MatrixAssemblyTerm_NA__NB__Fn_DefaultNew  );
     RegisterParent( MatrixAssemblyTerm_NA__NB__Fn_Type, StiffnessMatrixTerm_Type );

    Stg_ComponentRegister_Add( Stg_ComponentRegister_Get_ComponentRegister(), Fn_Integrate_Type, (Name)"0", _Fn_Integrate_DefaultNew  );
    RegisterParent(Fn_Integrate_Type, Stg_Component_Type);

    Stg_ComponentRegister_Add( componentRegister, VectorAssemblyTerm_VEP_Type, (Name)"0", _VectorAssemblyTerm_VEP_DefaultNew  );
    RegisterParent( VectorAssemblyTerm_VEP_Type, ForceTerm_Type );

    return True;
}
