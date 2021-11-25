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
#include <StgFEM/libStgFEM/src/StgFEM.h>

#include "Utils.h"

#include <stdio.h>

Stream* StgFEM_Utils_Debug = NULL;

/** Initialises the Linear Algebra package, then any init for this package
such as streams etc */
Bool StgFEM_Utils_Init( int* argc, char** argv[] ) {
   Stg_ComponentRegister* componentRegister = Stg_ComponentRegister_Get_ComponentRegister();
   int tmp;

   Journal_Printf( Journal_Register( DebugStream_Type, (Name)"Context"  ), "In: %s\n", __func__ ); 
   tmp = Stream_GetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context" )  );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context"  ), 0 );
   Stream_Flush( Journal_Register( InfoStream_Type, (Name)"Context" )  );
   Stream_SetPrintingRank( Journal_Register( InfoStream_Type, (Name)"Context"  ), tmp );

   /* initialise this level's streams */
   StgFEM_Utils_Debug = Stream_RegisterChild( StgFEM_Debug, "StgFEM" );

   Stg_ComponentRegister_Add( componentRegister, SemiLagrangianIntegrator_Type, (Name)"0", _SemiLagrangianIntegrator_DefaultNew  );
   RegisterParent( SemiLagrangianIntegrator_Type,	Stg_Component_Type );

    Stg_ComponentRegister_Add( componentRegister, IrregularMeshParticleLayout_Type, "0", (Stg_Component_DefaultConstructorFunction*) _IrregularMeshParticleLayout_DefaultNew);
    RegisterParent( IrregularMeshParticleLayout_Type,	GlobalParticleLayout_Type	);

	Stg_ComponentRegister_Add( componentRegister, MeshParticleLayout_Type, (Name)"0", _MeshParticleLayout_DefaultNew );
	RegisterParent( MeshParticleLayout_Type,   PerCellParticleLayout_Type );

   return True;
}


