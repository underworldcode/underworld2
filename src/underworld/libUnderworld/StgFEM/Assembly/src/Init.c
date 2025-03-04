/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <mpi.h>
#include <StGermain/libStGermain/src/StGermain.h>
#include <StgDomain/libStgDomain/src/StgDomain.h>
#include <StgFEM/Discretisation/src/Discretisation.h>
#include <StgFEM/SLE/src/SLE.h>

#include "Assembly.h"

#include <stdio.h>

Stream* StgFEM_Assembly_Debug = NULL;

/*
 * Initialises the Linear Algebra package, then any init for this package
 * such as streams etc.
 */
Bool StgFEM_Assembly_Init( int* argc, char** argv[] ) {
   Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();
   int                    tmp;

    
   Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context" ), "In: %s\n", __func__ );
   tmp = Stream_GetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ) );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ), 0 );
   Stream_Flush( Journal_Register( InfoStream_Type, (Name)"Context" ) );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" ), tmp );
   
   /* initialise this level's streams */
   StgFEM_Assembly_Debug = Stream_RegisterChild( StgFEM_Debug, "Assembly" );
   
   Stg_ComponentRegister_Add( componentRegister, GradientStiffnessMatrixTerm_Type, (Name)"0", _GradientStiffnessMatrixTerm_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, DivergenceMatrixTerm_Type, (Name)"0", _DivergenceMatrixTerm_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, LaplacianStiffnessMatrixTerm_Type, (Name)"0", _LaplacianStiffnessMatrixTerm_DefaultNew );
   Stg_ComponentRegister_Add( componentRegister, IsoviscousStressTensorTerm_Type, (Name)"0", _IsoviscousStressTensorTerm_DefaultNew );

   RegisterParent( GradientStiffnessMatrixTerm_Type, StiffnessMatrixTerm_Type );
   RegisterParent( DivergenceMatrixTerm_Type, StiffnessMatrixTerm_Type );
   RegisterParent( LaplacianStiffnessMatrixTerm_Type, StiffnessMatrixTerm_Type );
   RegisterParent( IsoviscousStressTensorTerm_Type, StiffnessMatrixTerm_Type );

   return True;
}


